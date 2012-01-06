/*
 * libmedia-service
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyunjun Ko <zzoon.ko@samsung.com>, Haejeong Kim <backto.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
* This file includes API sets for syncronization with other module.
*
* @file          media-svc-thumb.c
* @author      chaolong.lin(chaolong.lin@samsung.com)
* @version     1.0
* @brief       This file is the implementation of all APIs for thumbnail manager
*/

#define _GNU_SOURCE
#include <time.h>

#ifdef _PERFORMANCE_CHECK_
#include "media-info-debug.h"
#endif

#include "media-svc-thumb.h"
#include "media-svc-debug.h"
#include "media-svc-error.h"
#include "media-svc-util.h"
#include "media-img-codec.h"
#include "media-img-codec-parser.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <mm_file.h>
#include <mm_util_imgp.h>
#include <mm_util_jpeg.h>
#include <drm-service.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <string.h>
#include <libexif/exif-data.h>


#if defined(_PERFORMANCE_CHECK_) && defined(_USE_LOG_FILE_)
static double g_video_save = 0;
static double g_img_save = 0;
static double g_agif_save = 0;

static double g_video_thumb = 0;
static double g_img_thumb = 0;
static double g_agif_thumb = 0;
#endif

#define UNKNOWN_TAG             	               	"Unknown"
#define MB_SVC_THUMB_WIDTH					320
#define MB_SVC_THUMB_HEIGHT					240
#define MB_SVC_THUMB_LENGTH					256
#define MB_SVC_THUMB_LENGTH_LIMIT			10000
#define MB_SVC_EXIF_BUFFER_LEN_MAX			1023
#define MB_SVC_THUMB_BUFFER_LEN_MAX			4095

/* The maximum of resolution to be able to get thumbnail is 3000 x 3000, except for only jpeg */
#define MB_SVC_MAX_ALLOWED_MEM_FOR_THUMB 	9000000
/* The maximum of resolution of jpeg image which is NOT resized to make thumb */
#define MB_SVC_MAX_ALLOWED_MEM_NOT_RESIZED_FOR_THUMB 	105000

typedef struct{
	int size;
	bool is_drm;
	int width;
	int height;
	int orientation;
	ImgCodecType image_type;
	minfo_file_type content_type;
	const char* file_full_path;
	char* thumb_hash_path;
}mb_svc_thumb_info_s;

enum Exif_Orientation {
    NOT_AVAILABLE=0,
    NORMAL  =1,
    HFLIP   =2,
    ROT_180 =3,
    VFLIP   =4,
    TRANSPOSE   =5,
    ROT_90  =6,
    TRANSVERSE  =7,
    ROT_270 =8
};

static int __mb_svc_thumb_cp(char *src_file_full_path, char *dest_file_full_path);
static int __mb_svc_thumb_save(const void *image, mb_svc_thumb_info_s thumb_info, size_t max_thumb_length, mb_svc_image_meta_record_s *img_meta_record);
static int __get_thumbfile_size(char *filepath);
static int __mb_svc_get_exif_info(ExifData *ed, char *buf, int *value, int ifdtype, long tagtype);
static int __mb_svc_get_time_val_from_str(char *buf);
static void __mb_svc_split_to_double(char *input, double *arr, int *num);

static void __mb_svc_split_to_double(char *input, double *arr, int *num)
{
	char tmp_arr[255] = { 0, };
	int len = strlen(input);
	int i = 0, idx = 0, tmp_idx = 0;
	int is_prev_space = 0;

	for (;;) {
		if (input[len - 1] == ' ') {
			len--;
		} else {
			break;
		}
	}

	for (i = 0; i < len; i++) {
		if (idx > 2) {
			break;
		}

		if (input[i] == ' ') {
			if (is_prev_space == 1) {
				continue;
			}
			if (idx <= 2) {
				arr[idx++] = atof(tmp_arr);
			}
			tmp_idx = 0;
			is_prev_space = 1;
			continue;
		}

		tmp_arr[tmp_idx] = input[i];
		tmp_arr[++tmp_idx] = '\0';
		is_prev_space = 0;
	}

	if (i == len) {
		if (idx <= 2) {
			arr[idx++] = atof(tmp_arr);
		}
		*num = idx;
		return;
	} else {
		*num = idx--;
		return;
	}
}

int _mb_svc_get_size_using_evas(const char *file_path, int *width, int *height)
{
	ecore_evas_init();

	Ecore_Evas *ee = ecore_evas_buffer_new(0, 0);

	if (!ee) {
		mb_svc_debug("ecore_evas_new fails");
		return -1;
	}
	Evas *evas = ecore_evas_get(ee);
	if (!evas) {
		mb_svc_debug("ecore_evas_get fails");
		ecore_evas_free(ee);
		return -1;
	}

	Evas_Object *image_object = evas_object_image_add(evas);
	if (!image_object) {
		mb_svc_debug("evas_object_image_add fails");
		ecore_evas_free(ee);
		return -1;
	}

	evas_object_image_file_set(image_object, file_path, NULL);
	evas_object_image_size_get(image_object, width, height);

	mb_svc_debug("Width : %d, Height : %d", width, height);
	
	ecore_evas_free(ee);
	ecore_evas_shutdown();

	return 0;
}

static int __mb_svc_get_time_val_from_str(char *buf)
{
	int year, month, day, hour, min, sec;
	struct tm val;
	time_t ret;

	sscanf(buf, "%4d:%2d:%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min,
	       &sec);
	mb_svc_debug("%d / %d / %d  %d : %d : %d", year, month, day, hour, min,
		     sec);
	strptime(buf, "%Y:%m:%d %H:%M:%S", &val);

	ret = mktime(&val);
	mb_svc_debug("time_t : %ld\n", ret);
	/* mb_svc_debug("time to char : %s\n", ctime(&ret) ); */

	return ret;
}

static int __mb_svc_get_exif_info(ExifData *ed, char *buf, int *value, int ifdtype, long tagtype)
{
	ExifEntry *entry;
	ExifIfd ifd;
	ExifTag tag;

	if (ed == NULL) {
		return -1;
	}

	ifd = ifdtype;
	tag = tagtype;

	/* entry = exif_content_get_entry(ed->ifd[ifd], tag); */
	entry = exif_data_get_entry(ed, tag);
	if (entry) {
		if (tag == EXIF_TAG_ORIENTATION ||
				tag == EXIF_TAG_PIXEL_X_DIMENSION || 
				tag == EXIF_TAG_PIXEL_Y_DIMENSION) {

			if (value == NULL) {
				mb_svc_debug("value is NULL");
				return -1;
			}

			ExifByteOrder mByteOrder = exif_data_get_byte_order(ed);
			short exif_value = exif_get_short(entry->data, mByteOrder);
			mb_svc_debug("%s : %d", exif_tag_get_name_in_ifd(tag,ifd), exif_value);
			*value = (int)exif_value;
		} else {
			/* Get the contents of the tag in human-readable form */
			if (buf == NULL) {
				mb_svc_debug("buf is NULL");
				return -1;
			}
			exif_entry_get_value(entry, buf,
					     MB_SVC_EXIF_BUFFER_LEN_MAX);
			buf[strlen(buf)] = '\0';

			if (*buf) {
				mb_svc_debug("%s: %s\n",
					     exif_tag_get_name_in_ifd(tag, ifd),
					     buf);
			}
		}
	}

	return 0;
}

int
mb_svc_get_exif_gps_info(ExifData *ed, double *gps_value,
			 int ifdtype, long tagtype)
{
	ExifEntry *entry;
	ExifIfd ifd;
	ExifTag tag;
	char buf[MB_SVC_EXIF_BUFFER_LEN_MAX + 1] = { '\0' };

	if (ed == NULL) {
		return -1;
	}

	memset(buf, 0x00, MB_SVC_EXIF_BUFFER_LEN_MAX + 1);

	if (gps_value == NULL) {
		mb_svc_debug("gps_value==NULL ");
		return -1;
	}

	*gps_value = MINFO_DEFAULT_GPS;

	ifd = ifdtype;
	tag = tagtype;

	entry = exif_data_get_entry(ed, tag);

	if (entry) {
		if (tag == EXIF_TAG_GPS_LATITUDE
		    || tag == EXIF_TAG_GPS_LONGITUDE) {
			/* Get the contents of the tag in human-readable form */
			exif_entry_get_value(entry, buf,
					     MB_SVC_EXIF_BUFFER_LEN_MAX);
			buf[strlen(buf)] = '\0';

			if (*buf) {
				mb_svc_debug("%s: %s\n",
					     exif_tag_get_name_in_ifd(tag, ifd),
					     buf);
			}

			double tmp_arr[3] = { 0.0, 0.0, 0.0 };
			int count = 0;

			__mb_svc_split_to_double(buf, tmp_arr, &count);
			if (count != 3) {
				mb_svc_debug("Value is invalid");
				return -1;
			}

			*gps_value =
			    tmp_arr[0] + tmp_arr[1] / 60 + tmp_arr[2] / 3600;
			mb_svc_debug("GPS value is %f", *gps_value);

		} else {
			mb_svc_debug
			    ("tag is not EXIF_TAG_GPS_LATITUDE or EXIF_TAG_GPS_LONGITUDE");
			return -1;
		}
	}

	return 0;
}

int
mb_svc_drm_get_buffer(const char *file_full_path, unsigned char **buffer,
		      int *size)
{
	DRM_RESULT res;
	DRM_FILE_HANDLE hDrmFile = NULL;

	mb_svc_debug("mb_svc_drm_get_buffer\n");

	if (file_full_path == NULL || buffer == NULL || size == NULL) {
		mb_svc_debug
		    ("file_full_path==NULL || buffer==NULL || size==NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	res =
	    drm_svc_open_file(file_full_path, DRM_PERMISSION_DISPLAY,
			      &hDrmFile);
	if (res == DRM_RESULT_SUCCESS) {
		drm_file_attribute_t drm_attr;
		/* get attribute to get file size  */
		drm_svc_get_fileattribute(file_full_path, &drm_attr);
		/* read drm file  */
		if (drm_attr.size < 2 * 1024 * 1024) {
			*buffer = (unsigned char *)malloc(drm_attr.size);
			if (*buffer) {
				drm_svc_read_file(hDrmFile, *buffer,
						  drm_attr.size,
						  (unsigned int *)size);
			}
			mb_svc_debug
			    ("mb_svc_drm_get_buffer : buffer=%p, size=%d",
			     *buffer, drm_attr.size);
		}
		drm_svc_close_file(hDrmFile);
		return 1;
	}
	return 0;
}

static int __get_thumbfile_size(char *filepath)
{
	FILE *fp;
	int file_size = 0;
	fp = fopen(filepath, "r");
	if (!fp) {
		mb_svc_debug("file open failed!\n");
		return -1;
	}

	fseek(fp, 0l, SEEK_END);

	file_size = ftell(fp);
	fclose(fp);

	return file_size;
}

static void __mb_svc_image_rotate_180(Evas_Object *obj)
{
	unsigned int *data;
	unsigned int *p1, *p2, tmp;
	int x, hw, iw, ih;

	evas_object_image_size_get(obj, &iw, &ih);
	data = evas_object_image_data_get(obj, 1);

	hw = iw * ih;
	x = (hw / 2);
	p1 = data;
	p2 = data + hw - 1;
	for (; --x > 0;) {
		tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
	evas_object_image_data_set(obj, data);
	evas_object_image_data_update_add(obj, 0, 0, iw, ih);
}

static void __mb_svc_image_rotate_90(Evas_Object *obj)
{
	unsigned int *data, *data2, *to, *from;
	int x, y, w, hw, iw, ih;
	evas_object_image_size_get(obj, &iw, &ih);

	data = evas_object_image_data_get(obj, EINA_FALSE);	/* for reading */
	/* memcpy */
	data2 = malloc(iw * ih * sizeof(unsigned int));
	memcpy(data2, data, iw * ih * sizeof(unsigned int));

	/* set width, height */
	w = ih;
	ih = iw;
	iw = w;
	hw = w * ih;

	/* set width, height to image obj */
	evas_object_image_size_set(obj, iw, ih);
	data = evas_object_image_data_get(obj, EINA_TRUE);
	to = data + w - 1;
	hw = -hw - 1;
	from = data2;

	for (x = iw; --x >= 0;) {
		for (y = ih; --y >= 0;) {
			*to = *from;
			from++;
			to += w;
		}

		to += hw;
	}

	if (data2) {
		free(data2);
	}

	evas_object_image_data_set(obj, data);
	evas_object_image_data_update_add(obj, 0, 0, iw, ih);
}

static void __mb_svc_image_rotate_270(Evas_Object *obj)
{
	unsigned int *data, *data2, *to, *from;
	int x, y, w, hw, iw, ih;
	evas_object_image_size_get(obj, &iw, &ih);

	data = evas_object_image_data_get(obj, EINA_FALSE);	/* for reading */
	/* memcpy */
	data2 = malloc(iw * ih * sizeof(unsigned int));
	memcpy(data2, data, iw * ih * sizeof(unsigned int));

	/* set width, height */
	w = ih;
	ih = iw;
	iw = w;
	hw = w * ih;

	/* set width, height to image obj */
	evas_object_image_size_set(obj, iw, ih);
	data = evas_object_image_data_get(obj, EINA_TRUE);

	to = data + hw - w;
	w = -w;
	hw = hw + 1;
	from = data2;

	for (x = iw; --x >= 0;) {
		for (y = ih; --y >= 0;) {
			*to = *from;
			from++;
			to += w;
		}

		to += hw;
	}

	if (data2) {
		free(data2);
	}

	evas_object_image_data_set(obj, data);
	evas_object_image_data_update_add(obj, 0, 0, iw, ih);
}

static int
__mb_svc_thumb_save(const void *image, mb_svc_thumb_info_s thumb_info,
		    size_t max_thumb_length,
		    mb_svc_image_meta_record_s *img_meta_record)
{
	mb_svc_debug("");
	char thumb_pathname[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	int nwrite = -1;
	int err = 0;
	char *src = (char *)image;

	gsize size = 0;
	char *data = NULL;
	unsigned char *buffer = NULL;
	int b_size = 0;
	bool use_default = FALSE;
	int ret_len = -1;

	int thumb_width = 0;
	int thumb_height = 0;
	double ratio_wh;

#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	long entire_s = 0L, entire_e = 0L;
#endif

#ifdef _USE_LOG_FILE_
	mediainfo_init_file_debug();
	mediainfo_file_dbg("File : %s", thumb_info.file_full_path);
#endif

	mb_svc_debug("hash : %s(%d)", thumb_info.thumb_hash_path,
		     strlen(thumb_info.thumb_hash_path));
	strncpy(thumb_pathname, thumb_info.thumb_hash_path,
		sizeof(thumb_pathname));
	if (strlen(thumb_pathname) < strlen(thumb_info.thumb_hash_path)) {
		mb_svc_debug("Error for the length of thumb path");
		return MB_SVC_ERROR_INTERNAL;
	}

	ecore_evas_init();

	if (thumb_info.content_type == MINFO_ITEM_VIDEO) {
		{
			bool portrait = FALSE;
			if (thumb_info.width < thumb_info.height)
				portrait = TRUE;

			if (portrait) {
				ratio_wh =
				    (double)thumb_info.width /
				    (double)thumb_info.height;
				thumb_height = MB_SVC_THUMB_LENGTH;
				thumb_width = thumb_height * ratio_wh;
			} else {
				ratio_wh =
				    (double)thumb_info.height /
				    (double)thumb_info.width;
				thumb_width = MB_SVC_THUMB_LENGTH;
				thumb_height = thumb_width * ratio_wh;
			}

			mb_svc_debug("ratio : %f\n", ratio_wh);
			mb_svc_debug("Thumb width : %d, Thumb height : %d\n",
				     thumb_width, thumb_height);

			int i = thumb_info.width * thumb_info.height;
			if (!(size = i)) {
				mb_svc_debug("size != i\n");
				return MB_SVC_ERROR_CREATE_THUMBNAIL;
			}

			if (!src) {
				mb_svc_debug("src is null\n");
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_VIDEO_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {
				mb_svc_debug("evas Start : %s",
					     thumb_info.file_full_path);

#ifdef _PERFORMANCE_CHECK_
				entire_s = mediainfo_get_debug_time();
#endif

				unsigned int buf_size = 0;
				if (mm_util_get_image_size
				    (MM_UTIL_IMG_FMT_RGB888, thumb_width,
				     thumb_height, &buf_size) < 0) {
					mb_svc_debug
					    ("Failed to get buffer size");
					return MB_SVC_ERROR_INTERNAL;
				}
				mb_svc_debug("mm_util_get_image_size : %d",
					     buf_size);

				unsigned char *dst =
				    (unsigned char *)malloc(buf_size);

				if (mm_util_resize_image
				    ((unsigned char *)src, thumb_info.width,
				     thumb_info.height, MM_UTIL_IMG_FMT_RGB888,
				     dst, (unsigned int *)&thumb_width,
				     (unsigned int *)&thumb_height) < 0) {
					mb_svc_debug
					    ("Failed to resize the thumbnails");
					if (dst)
						free(dst);
					return MB_SVC_ERROR_INTERNAL;
				}

				Ecore_Evas *ee =
					ecore_evas_buffer_new(thumb_width, thumb_height);
				Evas *evas = ecore_evas_get(ee);

				Evas_Object *img = NULL;
				img = evas_object_image_add(evas);

				if (img == NULL) {
					mb_svc_debug("image object is NULL\n");
					if (dst)
						free(dst);
					ecore_evas_free(ee);
					ecore_evas_shutdown();
					return -1;
				}

				evas_object_image_colorspace_set(img,
								 EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(img, thumb_width,
							   thumb_height);
				evas_object_image_fill_set(img, 0, 0,
							   thumb_width,
							   thumb_height);

				unsigned char *m = NULL;
				m = evas_object_image_data_get(img, 1);
#if 1				/* Use self-logic to convert from RGB888 to RGBA */

#ifdef _PERFORMANCE_CHECK_
				start = mediainfo_get_debug_time();
#endif
				int i = 0, j;
				for (j = 0; j < thumb_width * 3 * thumb_height;
				     j += 3) {
					m[i++] = (dst[j + 2]);
					m[i++] = (dst[j + 1]);
					m[i++] = (dst[j]);
					m[i++] = 0x0;
				}
#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double convert =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Convert from RGB888 to RGBA : %f",
					     convert);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg
				    ("Convert from RGB888 to RGBA : %f, ( W:%d, H:%d)",
				     convert, thumb_width, thumb_height);
#endif
#endif

#else				/* Use mmf api to convert from RGB888 to RGBA */
				int mm_ret = 0;
				if ((mm_ret =
				     mm_util_convert_colorspace(dst,
								thumb_width,
								thumb_height,
								MM_UTIL_IMG_FMT_RGB888,
								m,
								MM_UTIL_IMG_FMT_ARGB8888))
				    < 0) {
					mb_svc_debug
					    ("Failed to change from rgb888 to argb8888 %d",
					     mm_ret);
					if (dst)
						free(dst);
					return MB_SVC_ERROR_INTERNAL;
				}
#endif				/* End of use mmf api to convert from RGB888 to RGBA */

				evas_object_image_data_set(img, m);
				evas_object_image_data_update_add(img, 0, 0,
								  thumb_width,
								  thumb_height);

#ifdef _PERFORMANCE_CHECK_
				start = mediainfo_get_debug_time();
#endif
				if (evas_object_image_save
				    (img, thumb_pathname, NULL,
				     "quality=100 compress=1")) {
					mb_svc_debug
					    ("evas_object_image_save success\n");
					data = NULL;
				} else {
					/* Temporary code to avoid race condition */
					if (g_file_test
						(thumb_pathname, G_FILE_TEST_EXISTS)) {
						mb_svc_debug("Thumbnail already exists");

						ecore_evas_free(ee);
						ecore_evas_shutdown();

						return 0;
					} else {
						mb_svc_debug("Thumbnail doesn't exist");
						mb_svc_debug("evas_object_image_save fail\n");
						use_default = TRUE;
						data = NULL;
					}
				}

#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double save =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Save( %d / %d ) : %f",
						   thumb_width, thumb_height,
						   save);
				g_video_save += save;
				mediainfo_file_dbg("video save sum: %f",
						   g_video_save);
#endif
#endif
				if (dst)
					free(dst);
				ecore_evas_free(ee);

#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_video_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making video : %f\n\n",
					     make_video_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making video : %f",
						   make_video_thumb);
				g_video_thumb += make_video_thumb;
				mediainfo_file_dbg("Making video sum: %f\n\n",
						   g_video_thumb);
#endif
#endif
			}
		}
		mb_svc_debug("thumb_pathname is %s\n", thumb_pathname);
	}

	else if (thumb_info.content_type == MINFO_ITEM_IMAGE) {
		mb_svc_debug
		    ("item type is image, imgtype is %d,_is_real_drm is %d\n",
		     thumb_info.image_type, thumb_info.is_drm);

		int total_mem = 0;
		bool portrait = FALSE;

		if (thumb_info.image_type <= 0) {
			mb_svc_debug("Unknown image type");
			ret_len =
			    snprintf(thumb_pathname, sizeof(thumb_pathname),
				     "%s", DEFAULT_IMAGE_THUMB);
			if (ret_len < 0) {
				mb_svc_debug("Error when snprintf");
				return MB_SVC_ERROR_INTERNAL;
			}
			use_default = TRUE;
			data = NULL;

		} else if (thumb_info.is_drm) {

			mb_svc_debug("this image file is not real drm\n");
			mb_svc_drm_get_buffer(thumb_info.file_full_path,
					      &buffer, &b_size);
			if (b_size <= 0) {
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_IMAGE_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {
				data = (char *)buffer;
				size = b_size;
			}

		} else if (src && (thumb_info.size > 0)) {
			data = src;
			size = thumb_info.size;
			mb_svc_debug("get thumbnail of exif %d: %d\n",
				     (int)data, size);

		} else {
#ifdef _PERFORMANCE_CHECK_
			entire_s = mediainfo_get_debug_time();
#endif
			if (thumb_info.width <= 0 && thumb_info.height <= 0) {
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg
				    ("Failed to get w and h before : %s",
				     thumb_info.file_full_path);
				start = mediainfo_get_debug_time();
#endif

				/* using evas to get w/h */
				Ecore_Evas *ee =
					ecore_evas_buffer_new(thumb_width, thumb_height);
				if (!ee) {
					mb_svc_debug("ecore_evas_new fails");
					return -1;
				}
				Evas *evas = ecore_evas_get(ee);
				if (!evas) {
					mb_svc_debug("ecore_evas_get fails");
					ecore_evas_free(ee);
					return -1;
				}

				Evas_Object *image_object =
				    evas_object_image_add(evas);
				if (!image_object) {
					mb_svc_debug
					    ("evas_object_image_add fails");
					ecore_evas_free(ee);
					return -1;
				}

				evas_object_image_file_set(image_object,
							   thumb_info.
							   file_full_path,
							   NULL);
				evas_object_image_size_get(image_object,
							   &(thumb_info.width),
							   &(thumb_info.height));
				ecore_evas_free(ee);

#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double get_size =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("get_size : %f", get_size);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("get_size : %f", get_size);
#endif
#endif
			}

			mb_svc_debug("image width : %d, height : %d",
				     thumb_info.width, thumb_info.height);

			if (img_meta_record) {
				img_meta_record->width = thumb_info.width;
				img_meta_record->height = thumb_info.height;
			}

			if (thumb_info.width <= 0 || thumb_info.height <= 0) {
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_IMAGE_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {

				total_mem = thumb_info.width * thumb_info.height;

				if (thumb_info.width < thumb_info.height)
					portrait = TRUE;

				if (portrait) {
					ratio_wh =
					    (double)thumb_info.width /
					    (double)thumb_info.height;
					thumb_height = MB_SVC_THUMB_LENGTH;
					thumb_width = thumb_height * ratio_wh;
				} else {
					ratio_wh =
					    (double)thumb_info.height /
					    (double)thumb_info.width;
					thumb_width = MB_SVC_THUMB_LENGTH;
					thumb_height = thumb_width * ratio_wh;
				}
				mb_svc_debug("ratio : %f\n", ratio_wh);
				mb_svc_debug
				    ("Thumb width : %d, Thumb height : %d\n",
				     thumb_width, thumb_height);
			}

			if (thumb_info.image_type != IMG_CODEC_AGIF
			    && !use_default) {
				mb_svc_debug
				    ("this is non-agif image file,is_real_drm is %d\n",
				     thumb_info.is_drm);

 				if (thumb_info.image_type == IMG_CODEC_JPEG
				    && total_mem <= MB_SVC_MAX_ALLOWED_MEM_NOT_RESIZED_FOR_THUMB) {
	
					/* Using mm-util to make thumb of small original jepg */
					mb_svc_debug("This is small jpeg. It's not resized");

					int orientation = thumb_info.orientation;
					mm_util_img_rotate_type rot_type = MM_UTIL_ROTATE_0;

					mb_svc_debug("Orientation : %d", orientation);
					if (orientation == ROT_90) {
						rot_type = MM_UTIL_ROTATE_270;
					} else if (orientation == ROT_180) {
						rot_type = MM_UTIL_ROTATE_180;
					} else if (orientation == ROT_270) {
						rot_type = MM_UTIL_ROTATE_90;
					}

					if (orientation == ROT_90 || orientation == ROT_180 || orientation == ROT_270) {

						mm_util_jpeg_yuv_data decoded = {0,};
					
						err = mm_util_decode_from_jpeg_file(&decoded, 
											(char *)thumb_info.file_full_path, 
											MM_UTIL_JPEG_FMT_YUV420);
						if (err < 0) {
							mb_svc_debug("mm_util_decode_from_jpeg_turbo_memory failed : %d", err);
							return err;
						}
					
						mb_svc_debug("decoded size:%d, w:%d, h:%d\n", decoded.size, decoded.width, decoded.height);
	
						unsigned char *rotated = NULL;
						unsigned int rot_w = decoded.height;
						unsigned int rot_h = decoded.width;
						unsigned int rot_size = 0;

						if (orientation == ROT_180) {
							rot_w = decoded.width;
							rot_h = decoded.height;
						}
					
						err = mm_util_get_image_size(MM_UTIL_JPEG_FMT_YUV420, rot_w, rot_h, &rot_size);
						if (err < 0) {
							mb_svc_debug("mm_util_get_image_size failed : %d", err);
							if (decoded.data != NULL) free(decoded.data);
							return err;
						}
					
						mb_svc_debug("Size of Rotated : %d", rot_size);
					
						rotated = (unsigned char *)malloc(rot_size);
						err = mm_util_rotate_image(decoded.data, decoded.width, decoded.height, 
													MM_UTIL_JPEG_FMT_YUV420,
													rotated, &rot_w, &rot_h, 
													rot_type);

						if (err < 0) {
							mb_svc_debug("mm_util_rotate_image failed : %d", err);
							if (decoded.data != NULL) free(decoded.data);
							if (rotated != NULL) free(rotated);
							return err;
						} else {
							mb_svc_debug("mm_util_rotate_image succeed");
						}

						err = mm_util_jpeg_encode_to_file(thumb_pathname, rotated, rot_w, rot_h, MM_UTIL_JPEG_FMT_YUV420, 100);
				
						if (err < 0) {
							mb_svc_debug("mm_util_jpeg_encode_to_file failed: %d", err);
							if (decoded.data != NULL) free(decoded.data);
							if (rotated != NULL) free(rotated);
							return err;
						}

						if (decoded.data != NULL) free(decoded.data);
						if (rotated != NULL) free(rotated);
						data = NULL;

					} else {
						err = __mb_svc_thumb_cp((char *)thumb_info.file_full_path, thumb_pathname);
						if (err < 0) {
							mb_svc_debug("__mb_svc_thumb_cp failed: %d", err);
							return -1;
						}
					}
				} else if (thumb_info.image_type == IMG_CODEC_JPEG
				    || total_mem <= MB_SVC_MAX_ALLOWED_MEM_FOR_THUMB) {
					/* using evas to make thumbnail of general images */
					Ecore_Evas *ee =
						 ecore_evas_buffer_new(thumb_width,
								  thumb_height);
					if (!ee) {
						mb_svc_debug
						    ("Failed to create a ecore evas\n");
						ecore_evas_shutdown();
						return -1;
					}

					Evas *evas = ecore_evas_get(ee);
					if (!evas) {
						mb_svc_debug
						    ("Failed to ecore_evas_get\n");
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					Ecore_Evas *resize_img_ee;
					resize_img_ee =
					    ecore_evas_buffer_new(thumb_width,
								  thumb_height);

					if (!resize_img_ee) {
						mb_svc_debug
						    ("Failed to create a new ecore evas buffer\n");
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					Evas *resize_img_e =
					    ecore_evas_get(resize_img_ee);
					if (!resize_img_e) {
						mb_svc_debug
						    ("Failed to ecore_evas_get\n");
						ecore_evas_free(ee);
						ecore_evas_free(resize_img_ee);
						ecore_evas_shutdown();
						return -1;
					}

					Evas_Object *source_img =
					    evas_object_image_add(resize_img_e);
					if (!source_img) {
						mb_svc_debug
						    ("evas_object_image_add failed\n");
						ecore_evas_free(ee);
						ecore_evas_free(resize_img_ee);
						ecore_evas_shutdown();
						return -1;
					}

					evas_object_image_load_size_set
					    (source_img, thumb_width,
					     thumb_height);
					evas_object_image_file_set(source_img,
								   thumb_info.
								   file_full_path,
								   NULL);

					evas_object_image_fill_set(source_img,
								   0, 0,
								   thumb_width,
								   thumb_height);
					evas_object_image_filled_set(source_img,
								     1);
					evas_object_resize(source_img,
							   thumb_width,
							   thumb_height);
					evas_object_show(source_img);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					Evas_Object *ret_image =
					    evas_object_image_add(evas);
					/* evas_object_image_data_set(ret_image, NULL); */
					evas_object_image_size_set(ret_image,
								   thumb_width,
								   thumb_height);
					evas_object_image_fill_set(ret_image, 0,
								   0,
								   thumb_width,
								   thumb_height);

					evas_object_image_filled_set(ret_image,
								     EINA_TRUE);
					/* evas_object_image_data_copy_set(ret_image, (int *)ecore_evas_buffer_pixels_get(resize_img_ee)); */
					evas_object_image_data_set(ret_image,
								   (int *)
								   ecore_evas_buffer_pixels_get
								   (resize_img_ee));
					evas_object_image_data_update_add
					    (ret_image, 0, 0, thumb_width,
					     thumb_height);

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double copy_set =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("copy_set : %f", copy_set);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("copy_set : %f",
							   copy_set);
#endif
#endif

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					/* check it's orientation */
					if (thumb_info.orientation == ROT_90) {
						mb_svc_debug
						    ("This needs to be rotated -90");
						__mb_svc_image_rotate_90
						    (ret_image);
					} else if (thumb_info.orientation ==
						   ROT_180) {
						mb_svc_debug
						    ("This needs to be rotated -180");
						__mb_svc_image_rotate_180
						    (ret_image);
					} else if (thumb_info.orientation ==
						   ROT_270) {
						mb_svc_debug
						    ("This needs to be rotated -270");
						__mb_svc_image_rotate_270
						    (ret_image);
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double rotate =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("rotate : %f", rotate);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("rotate : %f",
							   rotate);
#endif
#endif

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					mb_svc_debug("evas_object_image_save start(%s)", thumb_pathname);

					if (evas_object_image_save
					    (ret_image, thumb_pathname, NULL,
					     "quality=100 compress=1")) {
						mb_svc_debug
						    ("evas_object_image_save success\n");
					} else {
						mb_svc_debug
						    ("evas_object_image_save fails\n");
						
						/* Temporary code to avoid race condition */
						if (g_file_test
							(thumb_pathname, G_FILE_TEST_EXISTS)) {
							mb_svc_debug("Thumbnail already exists");
							
							evas_object_del(source_img);
							ecore_evas_free(resize_img_ee);
							ecore_evas_free(ee);
	
							ecore_evas_shutdown();
	
							return 0;
						} else {
							mb_svc_debug("Thumbnail doesn't exist");
							use_default = TRUE;
						}
					}

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double save =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Save( %d / %d ) : %f",
					     thumb_width, thumb_height, save);
					g_img_save += save;
					mediainfo_file_dbg("image save sum: %f",
							   g_img_save);
#endif
#endif
					evas_object_del(source_img);
					ecore_evas_free(resize_img_ee);
					ecore_evas_free(ee);
					data = NULL;
				} else {
					mb_svc_debug
					    ("It's too big to make thumbnails for memory");
					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
					data = NULL;
				}
#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_img_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making img : %f\n\n",
					     make_img_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making img : %f",
						   make_img_thumb);
				g_img_thumb += make_img_thumb;
				mediainfo_file_dbg("Making image sum: %f\n\n",
						   g_img_thumb);
#endif
#endif
			} else if (thumb_info.image_type == IMG_CODEC_AGIF && !use_default) {	/* For agif files */
				mb_svc_debug("this is agif file\n");
#ifdef _PERFORMANCE_CHECK_
				entire_s = mediainfo_get_debug_time();
#endif
				if (src == NULL) {
					mb_svc_debug
					    ("There's no thumbnail from this agif file");

					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
				} else if (thumb_info.width <= 0
					   || thumb_info.height <= 0) {
					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
				} else {
					/* Using evas to get agif thumbnails */
					unsigned int buf_size = 0;
					if (mm_util_get_image_size
					    (MM_UTIL_IMG_FMT_RGB888,
					     thumb_width, thumb_height,
					     &buf_size) < 0) {
						mb_svc_debug
						    ("Failed to get buffer size");
						return MB_SVC_ERROR_INTERNAL;
					}
					mb_svc_debug
					    ("mm_util_get_image_size : %d",
					     buf_size);

					unsigned char *dst =
					    (unsigned char *)malloc(buf_size);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					if (mm_util_resize_image
					    ((unsigned char *)src,
					     thumb_info.width,
					     thumb_info.height,
					     MM_UTIL_IMG_FMT_RGB888, dst,
					     (unsigned int *)&thumb_width,
					     (unsigned int *)&thumb_height) <
					    0) {
						mb_svc_debug
						    ("Failed to resize the thumbnails");
						if (dst)
							free(dst);
						return MB_SVC_ERROR_INTERNAL;
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double resize =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Resize : %f", resize);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("Resize : %f",
							   resize);
#endif
#endif
					Ecore_Evas *ee =
					    ecore_evas_buffer_new(thumb_info.width,
								  thumb_info.height);
					Evas *evas = ecore_evas_get(ee);

					Evas_Object *img = NULL;
					img = evas_object_image_add(evas);

					if (img == NULL) {
						printf("img object is NULL\n");
						if (dst)
							free(dst);
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					evas_object_image_colorspace_set(img,
									 EVAS_COLORSPACE_ARGB8888);;
					evas_object_image_size_set(img,
								   thumb_width,
								   thumb_height);
					evas_object_image_fill_set(img, 0, 0,
								   thumb_width,
								   thumb_height);
					/* evas_object_image_size_set(img, thumb_info.width, thumb_info.height); */
					/* evas_object_image_fill_set(img, 0, 0, thumb_info.width, thumb_info.height); */

					unsigned char *m = NULL;
					m = evas_object_image_data_get(img, 1);
#if 1
#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					int i = 0, j;
/*
					for(j = 0; j < thumb_info.width *3 * thumb_info.height; j+=3 ) {
						m[i++] = (src[j+2]);
						m[i++] = (src[j+1]);
						m[i++] = (src[j]);
						m[i++] = 0x0;
					}
*/
					for (j = 0;
					     j < thumb_width * 3 * thumb_height;
					     j += 3) {
						m[i++] = (dst[j + 2]);
						m[i++] = (dst[j + 1]);
						m[i++] = (dst[j]);
						m[i++] = 0x0;
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double convert =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug
					    ("Convert from RGB888 to RGBA : %f",
					     convert);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Convert from RGB888 to RGBA : %f",
					     convert);
#endif
#endif
#else				/* Using evas to get agif thumbnails */
					int mm_ret = 0;
					if ((mm_ret =
					     mm_util_convert_colorspace(dst,
									thumb_width,
									thumb_height,
									MM_UTIL_IMG_FMT_RGB888,
									m,
									MM_UTIL_IMG_FMT_ARGB8888))
					    < 0) {
						mb_svc_debug
						    ("Failed to change from rgb888 to argb8888 : %d",
						     mm_ret);
						if (dst)
							free(dst);
						return MB_SVC_ERROR_INTERNAL;
					}
#endif

					evas_object_image_data_set(img, m);
					evas_object_image_data_update_add(img,
									  0, 0,
									  thumb_width,
									  thumb_height);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					if (evas_object_image_save
					    (img, thumb_pathname, NULL,
					     "quality=100 compress=1")) {
						mb_svc_debug
						    ("evas_object_image_save success\n");
					} else {
						/* Temporary code to avoid race condition */
						if (g_file_test
							(thumb_pathname, G_FILE_TEST_EXISTS)) {
							mb_svc_debug("Thumbnail already exists");
							
							ecore_evas_free(ee);
	
							ecore_evas_shutdown();
	
							return 0;
						} else {
							mb_svc_debug("Thumbnail doesn't exist");
							mb_svc_debug("evas_object_image_save fail\n");
							use_default = TRUE;
						}
					}

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double save =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Save( %d / %d ) : %f",
					     thumb_width, thumb_height, save);
					g_agif_save += save;
					mediainfo_file_dbg("agif save sum: %f",
							   g_agif_save);
#endif
#endif
					ecore_evas_free(ee);
					if (src)
						free(src);
					if (dst)
						free(dst);
					data = NULL;
				}
#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_agif_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making agif thumb : %f\n\n",
					     make_agif_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making agif : %f",
						   make_agif_thumb);
				g_agif_thumb += make_agif_thumb;
				mediainfo_file_dbg("Making agif sum: %f\n\n",
						   g_agif_thumb);
#endif
#endif
			}
		}
	}

	if (data != NULL) {
		int fd;
		/* fd = open(thumb_pathname, O_RDWR | O_CREAT | O_APPEND | O_SYNC, 600); */
		fd = open(thumb_pathname, O_RDWR | O_CREAT, 600);
		if (fd < 0) {
			mb_svc_debug("File open fails");
			return MB_SVC_ERROR_FILE_IO;
		}

		nwrite = write(fd, data, size);
		close(fd);

		g_free(data);
	}

	size = __get_thumbfile_size(thumb_pathname);
	if (size == 0) {
		mb_svc_debug("File size is ZERO : %s", thumb_pathname);
		ret_len =
		    snprintf(thumb_pathname, sizeof(thumb_pathname), "%s",
			     DEFAULT_IMAGE_THUMB);
		if (ret_len < 0) {
			mb_svc_debug("Error when snprintf");
			return MB_SVC_ERROR_INTERNAL;
		}
		use_default = TRUE;
	}

	mb_svc_debug("thumb_pathname is %s\n", thumb_pathname);

	if (use_default) {
		ret_len =
		    snprintf(thumb_info.thumb_hash_path, max_thumb_length, "%s",
			     DEFAULT_IMAGE_THUMB);
		if (ret_len < 0) {
			mb_svc_debug("Error when snprintf");
			return MB_SVC_ERROR_INTERNAL;
		}
	}

	mb_svc_debug("thumb file : %s\n", thumb_info.thumb_hash_path);

	err = g_file_test(thumb_info.thumb_hash_path, G_FILE_TEST_EXISTS);
	if (!err) {
		mb_svc_debug("failed to create thumbnail!\n");
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	chmod(thumb_info.thumb_hash_path, 0775);

	ecore_evas_shutdown();

#ifdef _USE_LOG_FILE_
	mediainfo_close_file_debug();
#endif
	return 0;
}

static int
__mb_svc_thumb_save_new(const void *image, mb_svc_thumb_info_s thumb_info,
			size_t max_thumb_length)
{
	mb_svc_debug("");
	char thumb_pathname[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	int nwrite = -1;
	int err = 0;
	char *src = (char *)image;
	gsize size = 0;
	char *data = NULL;
	unsigned char *buffer = NULL;
	int b_size = 0;
	bool use_default = FALSE;
	int ret_len = -1;

	int thumb_width = 0;
	int thumb_height = 0;
	double ratio_wh;

#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	long entire_s = 0L, entire_e = 0L;
#endif

#ifdef _USE_LOG_FILE_
	mediainfo_init_file_debug();
	mediainfo_file_dbg("File : %s", thumb_info.file_full_path);
#endif

	mb_svc_debug("hash : %s(%d)", thumb_info.thumb_hash_path,
		     strlen(thumb_info.thumb_hash_path));
	strncpy(thumb_pathname, thumb_info.thumb_hash_path,
		sizeof(thumb_pathname));
	if (strlen(thumb_pathname) < strlen(thumb_info.thumb_hash_path)) {
		mb_svc_debug("Error for the length of thumb path");
		return MB_SVC_ERROR_INTERNAL;
	}

	ecore_evas_init();

	if (thumb_info.content_type == MINFO_ITEM_VIDEO) {
		{
			bool portrait = FALSE;
			if (thumb_info.width < thumb_info.height)
				portrait = TRUE;

			if (portrait) {
				ratio_wh =
				    (double)thumb_info.width /
				    (double)thumb_info.height;
				thumb_height = MB_SVC_THUMB_LENGTH;
				thumb_width = thumb_height * ratio_wh;
			} else {
				ratio_wh =
				    (double)thumb_info.height /
				    (double)thumb_info.width;
				thumb_width = MB_SVC_THUMB_LENGTH;
				thumb_height = thumb_width * ratio_wh;
			}

			mb_svc_debug("ratio : %f\n", ratio_wh);
			mb_svc_debug("Thumb width : %d, Thumb height : %d\n",
				     thumb_width, thumb_height);

			int i = thumb_info.width * thumb_info.height;
			if (!(size = i)) {
				mb_svc_debug("size != i\n");
				return MB_SVC_ERROR_CREATE_THUMBNAIL;
			}

			if (!src) {
				mb_svc_debug("src is null\n");
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_VIDEO_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {
				mb_svc_debug("evas Start : %s",
					     thumb_info.file_full_path);

#ifdef _PERFORMANCE_CHECK_
				entire_s = mediainfo_get_debug_time();
#endif

				unsigned int buf_size = 0;
				if (mm_util_get_image_size
				    (MM_UTIL_IMG_FMT_RGB888, thumb_width,
				     thumb_height, &buf_size) < 0) {
					mb_svc_debug
					    ("Failed to get buffer size");
					return MB_SVC_ERROR_INTERNAL;
				}
				mb_svc_debug("mm_util_get_image_size : %d",
					     buf_size);

				unsigned char *dst =
				    (unsigned char *)malloc(buf_size);

				if (mm_util_resize_image
				    ((unsigned char *)src, thumb_info.width,
				     thumb_info.height, MM_UTIL_IMG_FMT_RGB888,
				     dst, (unsigned int *)&thumb_width,
				     (unsigned int *)&thumb_height) < 0) {
					mb_svc_debug
					    ("Failed to resize the thumbnails");
					if (dst)
						free(dst);
					return MB_SVC_ERROR_INTERNAL;
				}

				Ecore_Evas *ee =
/*
				    ecore_evas_new("software_x11", 0, 0,
						   thumb_width, thumb_height,
						   NULL);
*/
					ecore_evas_buffer_new(thumb_width,
								  thumb_height);
				Evas *evas = ecore_evas_get(ee);

				Evas_Object *img = NULL;
				img = evas_object_image_add(evas);

				if (img == NULL) {
					mb_svc_debug("image object is NULL\n");
					if (dst)
						free(dst);
					ecore_evas_free(ee);
					ecore_evas_shutdown();
					return -1;
				}

				evas_object_image_colorspace_set(img,
								 EVAS_COLORSPACE_ARGB8888);
				evas_object_image_size_set(img, thumb_width,
							   thumb_height);
				evas_object_image_fill_set(img, 0, 0,
							   thumb_width,
							   thumb_height);

				unsigned char *m = NULL;
				m = evas_object_image_data_get(img, 1);
#if 1				/* Use self-logic to convert from RGB888 to RGBA */

#ifdef _PERFORMANCE_CHECK_
				start = mediainfo_get_debug_time();
#endif
				int i = 0, j;
				for (j = 0; j < thumb_width * 3 * thumb_height;
				     j += 3) {
					m[i++] = (dst[j + 2]);
					m[i++] = (dst[j + 1]);
					m[i++] = (dst[j]);
					m[i++] = 0x0;
				}
#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double convert =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Convert from RGB888 to RGBA : %f",
					     convert);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg
				    ("Convert from RGB888 to RGBA : %f, ( W:%d, H:%d)",
				     convert, thumb_width, thumb_height);
#endif
#endif

#else				/* Use mmf api to convert from RGB888 to RGBA */
				int mm_ret = 0;
				if ((mm_ret =
				     mm_util_convert_colorspace(dst,
								thumb_width,
								thumb_height,
								MM_UTIL_IMG_FMT_RGB888,
								m,
								MM_UTIL_IMG_FMT_ARGB8888))
				    < 0) {
					mb_svc_debug
					    ("Failed to change from rgb888 to argb8888 %d",
					     mm_ret);
					if (dst)
						free(dst);
					return MB_SVC_ERROR_INTERNAL;
				}
#endif				/* End of use mmf api to convert from RGB888 to RGBA */

				evas_object_image_data_set(img, m);
				evas_object_image_data_update_add(img, 0, 0,
								  thumb_width,
								  thumb_height);

#ifdef _PERFORMANCE_CHECK_
				start = mediainfo_get_debug_time();
#endif
				if (evas_object_image_save
				    (img, thumb_pathname, NULL,
				     "quality=100 compress=1")) {
					mb_svc_debug
					    ("evas_object_image_save success\n");
					data = NULL;
				} else {
					mb_svc_debug
					    ("evas_object_image_save fail\n");
					use_default = TRUE;
					data = NULL;
				}

#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double save =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Save( %d / %d ) : %f",
						   thumb_width, thumb_height,
						   save);
				g_video_save += save;
				mediainfo_file_dbg("video save sum: %f",
						   g_video_save);
#endif
#endif
				if (dst)
					free(dst);
				ecore_evas_free(ee);

#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_video_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making video : %f\n\n",
					     make_video_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making video : %f",
						   make_video_thumb);
				g_video_thumb += make_video_thumb;
				mediainfo_file_dbg("Making video sum: %f\n\n",
						   g_video_thumb);
#endif
#endif

			}
		}
		mb_svc_debug("thumb_pathname is %s\n", thumb_pathname);
	}

	else if (thumb_info.content_type == MINFO_ITEM_IMAGE) {
		mb_svc_debug
		    ("item type is image, imgtype is %d,_is_real_drm is %d\n",
		     thumb_info.image_type, thumb_info.is_drm);

		int total_mem = 0;
		bool portrait = FALSE;

		if (thumb_info.image_type <= 0) {
			mb_svc_debug("Unknown image type");
			ret_len =
			    snprintf(thumb_pathname, sizeof(thumb_pathname),
				     "%s", DEFAULT_IMAGE_THUMB);
			if (ret_len < 0) {
				mb_svc_debug("Error when snprintf");
				return MB_SVC_ERROR_INTERNAL;
			}
			use_default = TRUE;
			data = NULL;

		} else if (thumb_info.is_drm) {

			mb_svc_debug("this image file is not real drm\n");
			mb_svc_drm_get_buffer(thumb_info.file_full_path,
					      &buffer, &b_size);
			if (b_size <= 0) {
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_IMAGE_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {
				data = (char *)buffer;
				size = b_size;
			}

		} else if (src && (thumb_info.size > 0)) {
			data = src;
			size = thumb_info.size;
			mb_svc_debug("get thumbnail of exif %d: %d\n",
				     (int)data, size);

		} else {
#ifdef _PERFORMANCE_CHECK_
			entire_s = mediainfo_get_debug_time();
#endif
			if (thumb_info.width <= 0 && thumb_info.height <= 0) {
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg
				    ("Failed to get w and h before : %s",
				     thumb_info.file_full_path);
				start = mediainfo_get_debug_time();
#endif

				Ecore_Evas *ee =
/*
				    ecore_evas_new("software_x11", 0, 0,
						   thumb_width, thumb_height,
						   NULL);
*/
					ecore_evas_buffer_new(thumb_width,
								  thumb_height);
				if (!ee) {
					mb_svc_debug("ecore_evas_new fails");
					return -1;
				}
				Evas *evas = ecore_evas_get(ee);
				if (!evas) {
					mb_svc_debug("ecore_evas_get fails");
					ecore_evas_free(ee);
					return -1;
				}

				Evas_Object *image_object =
				    evas_object_image_add(evas);
				if (!image_object) {
					mb_svc_debug
					    ("evas_object_image_add fails");
					ecore_evas_free(ee);
					return -1;
				}

				evas_object_image_file_set(image_object,
							   thumb_info.
							   file_full_path,
							   NULL);
				evas_object_image_size_get(image_object,
							   &(thumb_info.width),
							   &(thumb_info.
							     height));
				ecore_evas_free(ee);

#ifdef _PERFORMANCE_CHECK_
				end = mediainfo_get_debug_time();
				double get_size =
				    ((double)(end - start) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("get_size : %f", get_size);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("get_size : %f", get_size);
#endif
#endif
			}

			mb_svc_debug("image width : %d, height : %d",
				     thumb_info.width, thumb_info.height);

			if (thumb_info.width <= 0 || thumb_info.height <= 0) {
				ret_len =
				    snprintf(thumb_pathname,
					     sizeof(thumb_pathname), "%s",
					     DEFAULT_IMAGE_THUMB);
				if (ret_len < 0) {
					mb_svc_debug("Error when snprintf");
					return MB_SVC_ERROR_INTERNAL;
				}
				use_default = TRUE;
			} else {

				total_mem = thumb_info.width * thumb_info.height;

				if (thumb_info.width < thumb_info.height)
					portrait = TRUE;

				if (portrait) {
					ratio_wh =
					    (double)thumb_info.width /
					    (double)thumb_info.height;
					thumb_height = MB_SVC_THUMB_LENGTH;
					thumb_width = thumb_height * ratio_wh;
				} else {
					ratio_wh =
					    (double)thumb_info.height /
					    (double)thumb_info.width;
					thumb_width = MB_SVC_THUMB_LENGTH;
					thumb_height = thumb_width * ratio_wh;
				}
				mb_svc_debug("ratio : %f\n", ratio_wh);
				mb_svc_debug
				    ("Thumb width : %d, Thumb height : %d\n",
				     thumb_width, thumb_height);
			}

			if (thumb_info.image_type != IMG_CODEC_AGIF
			    && !use_default) {
				mb_svc_debug
				    ("this is non-agif image file,is_real_drm is %d\n",
				     thumb_info.is_drm);

				if (thumb_info.image_type == IMG_CODEC_JPEG
				    || total_mem <= MB_SVC_MAX_ALLOWED_MEM_FOR_THUMB) {
					Ecore_Evas *ee =
/*
					    ecore_evas_new("software_x11", 0, 0,
							   thumb_width,
							   thumb_height, NULL);
*/
					ecore_evas_buffer_new(thumb_width,
								  thumb_height);
					if (!ee) {
						mb_svc_debug
						    ("Failed to create a ecore evas\n");
						ecore_evas_shutdown();
						return -1;
					}

					Evas *evas = ecore_evas_get(ee);
					if (!evas) {
						mb_svc_debug
						    ("Failed to ecore_evas_get\n");
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					Ecore_Evas *resize_img_ee;
					resize_img_ee =
					    ecore_evas_buffer_new(thumb_width,
								  thumb_height);

					if (!resize_img_ee) {
						mb_svc_debug
						    ("Failed to create a new ecore evas buffer\n");
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					Evas *resize_img_e =
					    ecore_evas_get(resize_img_ee);
					if (!resize_img_e) {
						mb_svc_debug
						    ("Failed to ecore_evas_get\n");
						ecore_evas_free(ee);
						ecore_evas_free(resize_img_ee);
						ecore_evas_shutdown();
						return -1;
					}

					Evas_Object *source_img =
					    evas_object_image_add(resize_img_e);
					if (!source_img) {
						mb_svc_debug
						    ("evas_object_image_add failed\n");
						ecore_evas_free(ee);
						ecore_evas_free(resize_img_ee);
						ecore_evas_shutdown();
						return -1;
					}

					evas_object_image_load_size_set
					    (source_img, thumb_width,
					     thumb_height);
					evas_object_image_file_set(source_img,
								   thumb_info.
								   file_full_path,
								   NULL);

					evas_object_image_fill_set(source_img,
								   0, 0,
								   thumb_width,
								   thumb_height);
					evas_object_image_filled_set(source_img,
								     1);
					evas_object_resize(source_img,
							   thumb_width,
							   thumb_height);
					evas_object_show(source_img);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					Evas_Object *ret_image =
					    evas_object_image_add(evas);
					/* evas_object_image_data_set(ret_image, NULL); */
					evas_object_image_size_set(ret_image,
								   thumb_width,
								   thumb_height);
					evas_object_image_fill_set(ret_image, 0,
								   0,
								   thumb_width,
								   thumb_height);

					evas_object_image_filled_set(ret_image,
								     EINA_TRUE);
					/* evas_object_image_data_copy_set(ret_image, (int *)ecore_evas_buffer_pixels_get(resize_img_ee)); */
					evas_object_image_data_set(ret_image,
								   (int *)
								   ecore_evas_buffer_pixels_get
								   (resize_img_ee));
					evas_object_image_data_update_add
					    (ret_image, 0, 0, thumb_width,
					     thumb_height);

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double copy_set =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("copy_set : %f", copy_set);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("copy_set : %f",
							   copy_set);
#endif
#endif

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					/* check it's orientation */
					if (thumb_info.orientation == ROT_90) {
						mb_svc_debug
						    ("This needs to be rotated -90");
						__mb_svc_image_rotate_90
						    (ret_image);
					} else if (thumb_info.orientation ==
						   ROT_180) {
						mb_svc_debug
						    ("This needs to be rotated -180");
						__mb_svc_image_rotate_180
						    (ret_image);
					} else if (thumb_info.orientation ==
						   ROT_270) {
						mb_svc_debug
						    ("This needs to be rotated -270");
						__mb_svc_image_rotate_270
						    (ret_image);
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double rotate =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("rotate : %f", rotate);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("rotate : %f",
							   rotate);
#endif
#endif

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					if (evas_object_image_save
					    (ret_image, thumb_pathname, NULL,
					     "quality=100 compress=1")) {
						mb_svc_debug
						    ("evas_object_image_save success\n");
					} else {
						mb_svc_debug
						    ("evas_object_image_save fails\n");
						use_default = TRUE;
					}

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double save =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Save( %d / %d ) : %f",
					     thumb_width, thumb_height, save);
					g_img_save += save;
					mediainfo_file_dbg("image save sum: %f",
							   g_img_save);
#endif
#endif
					evas_object_del(source_img);
					ecore_evas_free(resize_img_ee);
					ecore_evas_free(ee);
					data = NULL;
				} else {
					mb_svc_debug
					    ("It's too big to make thumbnails for memory");
					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
					data = NULL;
				}
#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_img_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making img : %f\n\n",
					     make_img_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making img : %f",
						   make_img_thumb);
				g_img_thumb += make_img_thumb;
				mediainfo_file_dbg("Making image sum: %f\n\n",
						   g_img_thumb);
#endif
#endif
			} else if (thumb_info.image_type == IMG_CODEC_AGIF && !use_default) {	/* For agif files */
				mb_svc_debug("this is agif file\n");
#ifdef _PERFORMANCE_CHECK_
				entire_s = mediainfo_get_debug_time();
#endif
				if (src == NULL) {
					mb_svc_debug
					    ("There's no thumbnail from this agif file");

					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
				} else if (thumb_info.width <= 0
					   || thumb_info.height <= 0) {
					ret_len =
					    snprintf(thumb_pathname,
						     sizeof(thumb_pathname),
						     "%s", DEFAULT_IMAGE_THUMB);
					if (ret_len < 0) {
						mb_svc_debug
						    ("Error when snprintf");
						return MB_SVC_ERROR_INTERNAL;
					}
					use_default = TRUE;
				} else {
					unsigned int buf_size = 0;
					if (mm_util_get_image_size
					    (MM_UTIL_IMG_FMT_RGB888,
					     thumb_width, thumb_height,
					     &buf_size) < 0) {
						mb_svc_debug
						    ("Failed to get buffer size");
						return MB_SVC_ERROR_INTERNAL;
					}
					mb_svc_debug
					    ("mm_util_get_image_size : %d",
					     buf_size);

					unsigned char *dst =
					    (unsigned char *)malloc(buf_size);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					if (mm_util_resize_image
					    ((unsigned char *)src,
					     thumb_info.width,
					     thumb_info.height,
					     MM_UTIL_IMG_FMT_RGB888, dst,
					     (unsigned int *)&thumb_width,
					     (unsigned int *)&thumb_height) <
					    0) {
						mb_svc_debug
						    ("Failed to resize the thumbnails");
						if (dst)
							free(dst);
						return MB_SVC_ERROR_INTERNAL;
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double resize =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Resize : %f", resize);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg("Resize : %f",
							   resize);
#endif
#endif
					Ecore_Evas *ee =
/*
					    ecore_evas_new("software_x11", 0, 0,
							   thumb_info.width,
							   thumb_info.height,
							   NULL);
*/
						ecore_evas_buffer_new(thumb_info.width,
								  thumb_info.height);
					Evas *evas = ecore_evas_get(ee);

					Evas_Object *img = NULL;
					img = evas_object_image_add(evas);

					if (img == NULL) {
						printf("img object is NULL\n");
						if (dst)
							free(dst);
						ecore_evas_free(ee);
						ecore_evas_shutdown();
						return -1;
					}

					evas_object_image_colorspace_set(img,
									 EVAS_COLORSPACE_ARGB8888);;
					evas_object_image_size_set(img,
								   thumb_width,
								   thumb_height);
					evas_object_image_fill_set(img, 0, 0,
								   thumb_width,
								   thumb_height);
					/* evas_object_image_size_set(img, thumb_info.width, thumb_info.height); */
					/* evas_object_image_fill_set(img, 0, 0, thumb_info.width, thumb_info.height); */

					unsigned char *m = NULL;
					m = evas_object_image_data_get(img, 1);
#if 1
#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					int i = 0, j;

					for (j = 0;
					     j < thumb_width * 3 * thumb_height;
					     j += 3) {
						m[i++] = (dst[j + 2]);
						m[i++] = (dst[j + 1]);
						m[i++] = (dst[j]);
						m[i++] = 0x0;
					}
#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double convert =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug
					    ("Convert from RGB888 to RGBA : %f",
					     convert);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Convert from RGB888 to RGBA : %f",
					     convert);
#endif
#endif
#else				/* Using evas to get agif thumbnails */
					int mm_ret = 0;
					if ((mm_ret =
					     mm_util_convert_colorspace(dst,
									thumb_width,
									thumb_height,
									MM_UTIL_IMG_FMT_RGB888,
									m,
									MM_UTIL_IMG_FMT_ARGB8888))
					    < 0) {
						mb_svc_debug
						    ("Failed to change from rgb888 to argb8888 : %d",
						     mm_ret);
						if (dst)
							free(dst);
						return MB_SVC_ERROR_INTERNAL;
					}
#endif

					evas_object_image_data_set(img, m);
					evas_object_image_data_update_add(img,
									  0, 0,
									  thumb_width,
									  thumb_height);

#ifdef _PERFORMANCE_CHECK_
					start = mediainfo_get_debug_time();
#endif
					if (evas_object_image_save
					    (img, thumb_pathname, NULL,
					     "quality=100 compress=1")) {
						mb_svc_debug
						    ("evas_object_image_save success\n");
					} else {
						mb_svc_debug
						    ("evas_object_image_save fails\n");
						use_default = TRUE;
					}

#ifdef _PERFORMANCE_CHECK_
					end = mediainfo_get_debug_time();
					double save =
					    ((double)(end - start) /
					     (double)CLOCKS_PER_SEC);
					mb_svc_debug("Save : %f", save);
#ifdef _USE_LOG_FILE_
					mediainfo_file_dbg
					    ("Save( %d / %d ) : %f",
					     thumb_width, thumb_height, save);
					g_agif_save += save;
					mediainfo_file_dbg("agif save sum: %f",
							   g_agif_save);
#endif
#endif
					if (src)
						free(src);
					if (dst)
						free(dst);
					ecore_evas_free(ee);
					data = NULL;
				}
#ifdef _PERFORMANCE_CHECK_
				entire_e = mediainfo_get_debug_time();
				double make_agif_thumb =
				    ((double)(entire_e - entire_s) /
				     (double)CLOCKS_PER_SEC);
				mb_svc_debug("Making agif thumb : %f\n\n",
					     make_agif_thumb);
#ifdef _USE_LOG_FILE_
				mediainfo_file_dbg("Making agif : %f",
						   make_agif_thumb);
				g_agif_thumb += make_agif_thumb;
				mediainfo_file_dbg("Making agif sum: %f\n\n",
						   g_agif_thumb);
#endif
#endif
			}
		}
	}

	if (data != NULL) {
		int fd;
		/* fd = open(thumb_pathname, O_RDWR | O_CREAT | O_APPEND | O_SYNC, 600); */
		fd = open(thumb_pathname, O_RDWR | O_CREAT, 600);
		if (fd < 0) {
			mb_svc_debug("File open fails");
			return MB_SVC_ERROR_FILE_IO;
		}

		nwrite = write(fd, data, size);
		close(fd);

		g_free(data);

	}

	size = __get_thumbfile_size(thumb_pathname);
	if (size == 0) {
		mb_svc_debug("File size is ZERO : %s", thumb_pathname);
		ret_len =
		    snprintf(thumb_pathname, sizeof(thumb_pathname), "%s",
			     DEFAULT_IMAGE_THUMB);
		if (ret_len < 0) {
			mb_svc_debug("Error when snprintf");
			return MB_SVC_ERROR_INTERNAL;
		}
		use_default = TRUE;
	}

	mb_svc_debug("thumb_pathname is %s\n", thumb_pathname);

	if (use_default) {
		ret_len =
		    snprintf(thumb_info.thumb_hash_path, max_thumb_length, "%s",
			     DEFAULT_IMAGE_THUMB);
		if (ret_len < 0) {
			mb_svc_debug("Error when snprintf");
			return MB_SVC_ERROR_INTERNAL;
		}
	}

	mb_svc_debug("thumb file : %s\n", thumb_info.thumb_hash_path);

	err = g_file_test(thumb_info.thumb_hash_path, G_FILE_TEST_EXISTS);
	if (!err) {
		mb_svc_debug("failed to create thumbnail!\n");
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	chmod(thumb_info.thumb_hash_path, 0775);
	ecore_evas_shutdown();

#ifdef _USE_LOG_FILE_
	mediainfo_close_file_debug();
#endif
	return 0;
}

int
mb_svc_image_create_thumb(const char *file_full_path, char *thumb_hash_path,
			  size_t max_thumb_length, bool force,
			  mb_svc_image_meta_record_s *img_meta_record)
{
	unsigned int *thumb = NULL;
	int err = 0;
	ImgImageInfo image_info = { 0 };
	mb_svc_thumb_info_s thumb_info = { 0 };
	char file_ext[MB_SVC_FILE_EXT_LEN_MAX + 1] = { 0 };

	mb_svc_debug("_mb_svc_image_create_thumb------------start\n");

	if (file_full_path == NULL || thumb_hash_path == NULL) {
		mb_svc_debug("file_full_path==NULL || thumb_hash_path==NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (img_meta_record == NULL) {
		mb_svc_debug("img_meta_record == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_file_test
	    (thumb_hash_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		if (force) {
			if ((err = _mb_svc_thumb_rm(thumb_hash_path)) < 0) {
				mb_svc_debug
				    ("The existed thumbnail is removed");
			} else {
				mb_svc_debug("_mb_svc_thumb_rm failed : %d",
					     err);
			}
		} else {
			mb_svc_debug("Thumbnail already exists");
			return 0;
		}
	}

	thumb_info.image_type = ImgGetInfoFile(file_full_path, &image_info);
	mb_svc_debug("image type is %d\n", thumb_info.image_type);
	thumb_info.width = image_info.width;
	thumb_info.height = image_info.height;
	mb_svc_debug("image width:%d, height:%d \n", thumb_info.width,
		     thumb_info.height);

	if (thumb_info.width > MB_SVC_THUMB_LENGTH_LIMIT
	    || thumb_info.height > MB_SVC_THUMB_LENGTH_LIMIT) {
		mb_svc_debug
		    ("Widht or height is so big. Can't create thumbnail");
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	/* convert format and save */
	thumb_info.size = -1;
	if (!(_mb_svc_get_file_ext(file_full_path, file_ext))) {
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	mb_svc_debug("file ext  is %s\n", file_ext);
	if (strcasecmp(file_ext, "wbmp") == 0) {
		thumb_info.image_type = IMG_CODEC_WBMP;
	} else if ((strcasecmp(file_ext, "dcf") == 0)
		   || (strcasecmp(file_ext, "dm") == 0)) {
		thumb_info.image_type = IMG_CODEC_DRM;
	}

	if (thumb_info.image_type == IMG_CODEC_AGIF) {
		if (thumb_info.width * thumb_info.height > MB_SVC_MAX_ALLOWED_MEM_FOR_THUMB) {
			thumb = NULL;
		} else {
			thumb = ImgGetFirstFrameAGIFAtSize(file_full_path, &image_info);
		}
	}

	thumb_info.orientation = img_meta_record->orientation;
	thumb_info.file_full_path = file_full_path;
	thumb_info.thumb_hash_path = thumb_hash_path;
	thumb_info.content_type = MINFO_ITEM_IMAGE;
	thumb_info.is_drm = (drm_svc_is_drm_file(file_full_path) == DRM_TRUE);

	err =
	    __mb_svc_thumb_save(thumb, thumb_info, max_thumb_length,
				img_meta_record);
	if (err < 0) {
		mb_svc_debug("failed to save thumbnail!\n");
		
		int ret_len =
		    snprintf(thumb_info.thumb_hash_path, max_thumb_length, "%s",
			     DEFAULT_IMAGE_THUMB);
		if (ret_len < 0) {
			mb_svc_debug("Error when snprintf");
			return MB_SVC_ERROR_INTERNAL;
		}

		return err;
	}

	mb_svc_debug("_mb_svc_image_create_thumb------------leave\n");
	return 0;
}

int
mb_svc_image_create_thumb_new(const char *file_full_path, char *thumb_hash_path,
			      size_t max_thumb_length,
			      mb_svc_image_meta_record_s *img_meta_record)
{
	unsigned int *thumb = NULL;
	int err = 0;
	ImgImageInfo image_info = { 0 };
	mb_svc_thumb_info_s thumb_info = { 0 };
	char file_ext[MB_SVC_FILE_EXT_LEN_MAX + 1] = { 0 };

	mb_svc_debug("_mb_svc_image_create_thumb------------start\n");

	if (file_full_path == NULL || thumb_hash_path == NULL
	    || img_meta_record == NULL) {
		mb_svc_debug
		    ("file_full_path==NULL || thumb_hash_path==NULL || img_meta_record == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_file_test
	    (thumb_hash_path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
		mb_svc_debug("Thumbnail already exists");
		return 0;
	}

	thumb_info.image_type = ImgGetInfoFile(file_full_path, &image_info);
	thumb_info.width = image_info.width;
	thumb_info.height = image_info.height;

	if (thumb_info.width > MB_SVC_THUMB_LENGTH_LIMIT
	    || thumb_info.height > MB_SVC_THUMB_LENGTH_LIMIT) {
		mb_svc_debug
		    ("Widht or height is so big. Can't create thumbnail");
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	/* convert format and save */
	thumb_info.size = -1;
	if (!(_mb_svc_get_file_ext(file_full_path, file_ext))) {
		return MB_SVC_ERROR_CREATE_THUMBNAIL;
	}

	mb_svc_debug("file ext  is %s\n", file_ext);
	if (strcasecmp(file_ext, "wbmp") == 0) {
		thumb_info.image_type = IMG_CODEC_WBMP;
	} else if ((strcasecmp(file_ext, "dcf") == 0)
		   || (strcasecmp(file_ext, "dm") == 0)) {
		thumb_info.image_type = IMG_CODEC_DRM;
	}

	if (thumb_info.image_type == IMG_CODEC_AGIF) {
		thumb = ImgGetFirstFrameAGIFAtSize(file_full_path, &image_info);
	}

	thumb_info.file_full_path = file_full_path;
	thumb_info.thumb_hash_path = thumb_hash_path;
	thumb_info.content_type = MINFO_ITEM_IMAGE;
	thumb_info.is_drm = (drm_svc_is_drm_file(file_full_path) == DRM_TRUE);

	err = __mb_svc_thumb_save_new(thumb, thumb_info, max_thumb_length);
	if (err < 0) {
		mb_svc_debug("failed to save thumbnail!\n");
		return err;
	}

	img_meta_record->width = thumb_info.width;
	img_meta_record->height = thumb_info.height;

	mb_svc_debug("_mb_svc_image_create_thumb------------leave\n");
	return 0;
}

int
mb_svc_get_image_meta(const char *file_full_path,
						mb_svc_image_meta_record_s *image_record,
						bool *thumb_done)
{
	char buf[MB_SVC_EXIF_BUFFER_LEN_MAX + 1] = { '\0' };
	memset(buf, 0x00, MB_SVC_EXIF_BUFFER_LEN_MAX + 1);

	double value = 0.0;
	int orient_value = 0;
	int exif_width = 0;
	int exif_height = 0;
	ExifData *ed = NULL;

	if (file_full_path == NULL || image_record == NULL) {
		mb_svc_debug("file_full_path == NULL || image_record == NULL ");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	/* Load an ExifData object from an EXIF file */
	ed = exif_data_new_from_file(file_full_path);

	if (!ed) {
		mb_svc_debug("File not readable or no EXIF data in file %s\n",
			     file_full_path);
	}

	if (__mb_svc_get_exif_info
	    (ed, buf, NULL, EXIF_IFD_0,
	     EXIF_TAG_GPS_LATITUDE_REF) == 0) {
		if (strlen(buf) != 0) {
			if (mb_svc_get_exif_gps_info
			    (ed, &value, EXIF_IFD_GPS,
			     EXIF_TAG_GPS_LATITUDE) == 0) {
				if (strcmp(buf, "S") == 0) {
					value = -1 * value;
				}
				image_record->latitude = value;
			} else {
				image_record->latitude = MINFO_DEFAULT_GPS;
				mb_svc_debug
				    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LATITUDE  fails");
			}
		} else {
			image_record->latitude = MINFO_DEFAULT_GPS;
			mb_svc_debug
			    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LATITUDE is empty");
		}
	} else {
		image_record->latitude = MINFO_DEFAULT_GPS;
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LATITUDE  fails");
	}

	memset(buf, 0x00, sizeof(buf));

	if (__mb_svc_get_exif_info
	    (ed, buf, NULL, EXIF_IFD_0,
	     EXIF_TAG_GPS_LONGITUDE_REF) == 0) {
		if (strlen(buf) != 0) {
			if (mb_svc_get_exif_gps_info
			    (ed, &value, EXIF_IFD_GPS,
			     EXIF_TAG_GPS_LONGITUDE) == 0) {
				if (strcmp(buf, "W") == 0) {
					value = -1 * value;
				}
				image_record->longitude = value;
			} else {
				image_record->longitude = MINFO_DEFAULT_GPS;
				mb_svc_debug
				    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LONGITUDE  fails");
			}
		} else {
			image_record->longitude = MINFO_DEFAULT_GPS;
			mb_svc_debug
			    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LONGITUDE is empty");
		}
	} else {
		image_record->longitude = MINFO_DEFAULT_GPS;
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_GPS_LONGITUDE  fails");
	}

	memset(buf, 0x00, MB_SVC_EXIF_BUFFER_LEN_MAX + 1);

	if (__mb_svc_get_exif_info
	    (ed, buf, NULL, EXIF_IFD_0,
	     EXIF_TAG_IMAGE_DESCRIPTION) == 0) {
		if (strlen(buf) == 0) {
			strncpy(image_record->description, "No description",
				sizeof(image_record->description));
		} else {
			strncpy(image_record->description, buf,
				sizeof(image_record->description));
			image_record->
			    description[sizeof(image_record->description) - 1] =
			    '\0';
		}
	} else {
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_IMAGE_DESCRIPTION  fails");
	}

	memset(buf, 0x00, MB_SVC_EXIF_BUFFER_LEN_MAX + 1);

	if (__mb_svc_get_exif_info
	    (ed, buf, NULL, EXIF_IFD_0, EXIF_TAG_DATE_TIME) == 0) {
		if (strlen(buf) == 0) {
			mb_svc_debug("time  is NULL");
		} else {
			mb_svc_debug("time  is %s", buf);
			image_record->datetaken =
			    __mb_svc_get_time_val_from_str(buf);
		}
	} else {
		mb_svc_debug("__mb_svc_get_exif_info:EXIF_TAG_DATE_TIME  fails");
	}

	/* Get orientation value from exif. */
	if (__mb_svc_get_exif_info
	    (ed, NULL, &orient_value, EXIF_IFD_0,
	     EXIF_TAG_ORIENTATION) == 0) {
		if (orient_value >= NOT_AVAILABLE && orient_value <= ROT_270) {
			image_record->orientation = orient_value;
		} else {
			image_record->orientation = 0;
		}
	} else {
		image_record->orientation = 0;
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_ORIENTATION  fails");
	}

	/* Get width value from exif. */
	if (__mb_svc_get_exif_info
	    (ed, NULL, &exif_width, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION) == 0) {
		if (exif_width > 0) {
			image_record->width = exif_width;
		} else {
			image_record->width = 0;
		}
	} else {
		image_record->width = 0;
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_PIXEL_X_DIMENSION  fails");
	}

	/* Get height value from exif. */
	if (__mb_svc_get_exif_info
	    (ed, NULL, &exif_height, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION) == 0) {
		if (exif_width > 0) {
			image_record->height = exif_height;
		} else {
			image_record->height = 0;
		}
	} else {
		image_record->height = 0;
		mb_svc_debug
		    ("__mb_svc_get_exif_info:EXIF_TAG_PIXEL_Y_DIMENSION  fails");
	}

	//err = __mb_svc_get_thumb_and_save_from_exif(ed, file_full_path, image_record->orientation);
	//if (err == 0) *thumb_done = TRUE;

	if (ed != NULL) exif_data_unref(ed);

	return 0;
}

int
mb_svc_get_video_meta(const char *file_full_path,
		      mb_svc_video_meta_record_s *video_record)
{
	MMHandleType content = (MMHandleType) NULL;
	MMHandleType tag = (MMHandleType) NULL;
	char *p = NULL;
	int size = 0;
	int duration = 0;
	int width = 0;
	int height = 0;
	int err = -1;
	char *err_msg = NULL;
	double gps_value = 0.0;

	err = mm_file_create_content_attrs(&content, file_full_path);
	if (content != (MMHandleType) NULL) {
		err =
		    mm_file_get_attrs(content, &err_msg,
				      MM_FILE_CONTENT_DURATION, &duration,
				      NULL);
		if (err != 0) {
			mb_svc_debug("mm_file_get_attrs fails : %s", err_msg);
			free(err_msg);
			err_msg = NULL;
		}

		video_record->duration = duration;

		err =
		    mm_file_get_attrs(content, &err_msg,
				      MM_FILE_CONTENT_VIDEO_WIDTH, &width,
				      NULL);
		if (err != 0) {
			mb_svc_debug("mm_file_get_attrs fails : %s", err_msg);
			free(err_msg);
			err_msg = NULL;
		}

		video_record->width = width;

		err =
		    mm_file_get_attrs(content, &err_msg,
				      MM_FILE_CONTENT_VIDEO_HEIGHT, &height,
				      NULL);
		if (err != 0) {
			mb_svc_debug("mm_file_get_attrs fails : %s", err_msg);
			free(err_msg);
			err_msg = NULL;
		}

		video_record->height = height;
		mm_file_destroy_content_attrs(content);

		mb_svc_debug("duration of video is %d!!\n", duration);
		mb_svc_debug("width of video is %d!!\n", width);
		mb_svc_debug("height of video is %d!!\n", height);
	} else {
		mb_svc_debug
		    ("no tag information of video to invoke mm_file_create_content_attrs %s\n",
		     file_full_path);
		return MB_SVC_ERROR_INTERNAL;
	}

	err = mm_file_create_tag_attrs(&tag, file_full_path);
	if (!tag) {
		mb_svc_debug
		    ("no tag information of video to invoke mm_file_create_tag_attrs %s\n",
		     file_full_path);
		return MB_SVC_ERROR_INTERNAL;
	} else {
		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_TITLE, &p,
				      &size, NULL);
		if (err == 0 && size > 0) {
			strncpy(video_record->title, p,
				sizeof(video_record->title) - 1);
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			strncpy(video_record->title, UNKNOWN_TAG,
				sizeof(video_record->title) - 1);
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_ARTIST, &p,
				      &size, NULL);
		if (err == 0 && size > 0) {
			strncpy(video_record->artist, p,
				sizeof(video_record->artist) - 1);
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			strncpy(video_record->artist, UNKNOWN_TAG,
				sizeof(video_record->artist) - 1);
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_ALBUM, &p,
				      &size, NULL);
		if (err == 0 && size > 0) {
			strncpy(video_record->album, p,
				sizeof(video_record->album) - 1);
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			strncpy(video_record->album, UNKNOWN_TAG,
				sizeof(video_record->album) - 1);
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_DESCRIPTION,
				      &p, &size, NULL);
		if (err == 0 && size > 0) {
			strncpy(video_record->description, p,
				sizeof(video_record->description) - 1);
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			strncpy(video_record->description, UNKNOWN_TAG,
				sizeof(video_record->description) - 1);
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_LONGITUDE,
				      &gps_value, NULL);
		if (err == 0) {
			mb_svc_debug("longitude: %f", gps_value);

			if (gps_value == 0.0)
				gps_value = MINFO_DEFAULT_GPS;
			video_record->longitude = gps_value;
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			video_record->longitude = MINFO_DEFAULT_GPS;
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_LATIDUE,
				      &gps_value, NULL);
		if (err == 0) {
			mb_svc_debug("latitude: %f", gps_value);

			if (gps_value == 0.0)
				gps_value = MINFO_DEFAULT_GPS;
			video_record->latitude = gps_value;
		} else {
			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			video_record->latitude = MINFO_DEFAULT_GPS;
		}

		err =
		    mm_file_get_attrs(tag, &err_msg, MM_FILE_TAG_DATE, &p,
				      &size, NULL);
		if (err == 0 && size > 0) {
			mb_svc_debug("Date: %s[%d]", p, size);
			video_record->datetaken =
			    __mb_svc_get_time_val_from_str(p);
		} else {
			if (size <= 0) {
				mb_svc_debug("Size: [%d]", size);
			}

			if (err_msg) {
				mb_svc_debug("mm_file_get_attrs fails : %s",
					     err_msg);
				free(err_msg);
				err_msg = NULL;
			}
			video_record->datetaken = 0;
		}

		mb_svc_debug("title of video is %s!!\n", video_record->title);
		mb_svc_debug("artist of video is %s!!\n", video_record->artist);
		mb_svc_debug("album of video is %s!!\n", video_record->album);
		mb_svc_debug("description of video is %s!!\n",
			     video_record->description);
		mb_svc_debug("longitude of video is %f\n",
			     video_record->longitude);
		mb_svc_debug("latitude of video is %f\n",
			     video_record->latitude);
		mb_svc_debug("datetaken of video is %f\n",
			     video_record->datetaken);

		mm_file_destroy_tag_attrs(tag);
	}

	return 0;
}

int
mb_svc_video_create_thumb(const char *file_full_path, char *thumb_hash_path,
			  size_t max_thumb_length)
{
	MMHandleType content = (MMHandleType) NULL;
	void *frame = NULL;
	int err = -1;
	mb_svc_thumb_info_s thumb_info = { 0 };
	int video_track_num = 0;
	char *err_msg = NULL;

	mb_svc_debug("_mb_svc_video_generate_thumbnail------------start\n");
	if (file_full_path == NULL) {
		mb_svc_debug("file_full_path == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	thumb_info.is_drm = (drm_svc_is_drm_file(file_full_path) == DRM_TRUE);
	err = mm_file_create_content_attrs(&content, file_full_path);

	if (err < 0) {
		mb_svc_debug("mm_file_create_content_attrs fails : %d", err);
	}

	err =
	    mm_file_get_attrs(content, &err_msg,
			      MM_FILE_CONTENT_VIDEO_TRACK_COUNT,
			      &video_track_num, NULL);

	if (err != 0) {
		mb_svc_debug("mm_file_get_attrs fails : %s", err_msg);
		free(err_msg);
		err_msg = NULL;
	}

	/* MMF api handle both normal and DRM video */
	if (video_track_num > 0 || thumb_info.is_drm) {

		err = mm_file_get_attrs(content, &err_msg,
					MM_FILE_CONTENT_VIDEO_WIDTH,
					&thumb_info.width,
					MM_FILE_CONTENT_VIDEO_HEIGHT,
					&thumb_info.height,
					MM_FILE_CONTENT_VIDEO_THUMBNAIL, &frame, /* raw image is RGB888 format */
					&thumb_info.size, NULL);

		if (err != 0) {
			mb_svc_debug("mm_file_get_attrs fails : %s", err_msg);
			free(err_msg);
			err_msg = NULL;
		}

		thumb_info.size = thumb_info.width * 3 * thumb_info.height;
		mb_svc_debug("video width: %d\n", thumb_info.width);
		mb_svc_debug("video height: %d\n", thumb_info.height);
		mb_svc_debug("thumbnail size=%d\n", thumb_info.size);
		mb_svc_debug("video thumbnail: %p\n", frame);

		thumb_info.content_type = MINFO_ITEM_VIDEO;
		thumb_info.file_full_path = file_full_path;
		thumb_info.image_type = IMG_CODEC_NONE;
		thumb_info.thumb_hash_path = thumb_hash_path;
	} else {
		thumb_info.content_type = MINFO_ITEM_VIDEO;
		thumb_info.file_full_path = file_full_path;
		thumb_info.image_type = IMG_CODEC_NONE;
		thumb_info.thumb_hash_path = thumb_hash_path;

		mb_svc_debug("no contents information\n");
	}

	__mb_svc_thumb_save(frame, thumb_info, max_thumb_length, NULL);

	mm_file_destroy_content_attrs(content);
	mb_svc_debug("_mb_svc_video_generate_thumbnail------------leave\n");
	return 0;
}

int
_mb_svc_thumb_generate_hash_name(const char *file_full_path,
				 char *thumb_hash_path, size_t max_thumb_path)
{
	char *thumb_dir = NULL;
	char hash_name[255 + 1];
	char file_ext[MB_SVC_FILE_EXT_LEN_MAX + 1] = { 0 };
	int store_type = 0;

	if (file_full_path == NULL || thumb_hash_path == NULL
	    || max_thumb_path <= 0) {
		mb_svc_debug
		    ("file_full_path==NULL || thumb_hash_path == NULL || max_thumb_path <= 0");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	_mb_svc_get_file_ext(file_full_path, file_ext);
	store_type = _mb_svc_get_store_type_by_full(file_full_path);
	if (store_type == MINFO_PHONE) {
		thumb_dir = MB_SVC_THUMB_PHONE_PATH;
	} else if (store_type == MINFO_MMC) {
		thumb_dir = MB_SVC_THUMB_MMC_PATH;
	} else {
		thumb_dir = MB_SVC_THUMB_PHONE_PATH;
	}

	//hash_name = _mb_svc_generate_hash_name(file_full_path);
	int err = -1;
	err = mb_svc_generate_hash_code(file_full_path, hash_name, sizeof(hash_name));
	if (err < 0) {
		mb_svc_debug("mb_svc_generate_hash_code failed : %d", err);
		return MB_SVC_ERROR_INTERNAL;
	}

	int ret_len;
	ret_len =
	    snprintf(thumb_hash_path, max_thumb_path, "%s/.%s-%s.jpg",
		     thumb_dir, file_ext, hash_name);
	if (ret_len < 0) {
		mb_svc_debug("Error when snprintf");
		return MB_SVC_ERROR_INTERNAL;
	} else if (ret_len > max_thumb_path) {
		mb_svc_debug("Error for the length of thumb pathname");
		return MB_SVC_ERROR_INTERNAL;
	}

	mb_svc_debug("thumb hash : %s", thumb_hash_path);

	return 0;
}

/**
* @brief  Delete thumbnail file of original file.
* @param  file_full_path  original file.
* @return 0 ok, -1 failed.
*/
int _mb_svc_thumb_delete(const char *file_full_path)
{
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	int ret = -1;

	mb_svc_debug("_mb_svc_thumb_delete--enter\n");

	ret =
	    _mb_svc_thumb_generate_hash_name(file_full_path, thumb_path,
					     MB_SVC_FILE_PATH_LEN_MAX + 1);
	if (ret < 0) {
		return ret;
	}

	return _mb_svc_thumb_rm(thumb_path);
}

/**
* @brief  Copy thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_copy(const char *src_file_full_path,
		   const char *dest_file_full_path, char *dest_thumb_path)
{
	int result = 0;
	char src_thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };

	if (!dest_thumb_path) {
		mb_svc_debug("dest_thumb_path is NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("_mb_svc_thumb_copy--enter\n");

	result =
	    _mb_svc_thumb_generate_hash_name(src_file_full_path, src_thumb_path,
					     MB_SVC_FILE_PATH_LEN_MAX + 1);
	if (result < 0) {
		return result;
	}

	result =
	    _mb_svc_thumb_generate_hash_name(dest_file_full_path,
					     dest_thumb_path,
					     MB_SVC_FILE_PATH_LEN_MAX + 1);
	if (result < 0) {
		return result;
	}

	if (__mb_svc_thumb_cp(src_thumb_path, dest_thumb_path) != 0) {
		return MB_SVC_ERROR_COPY_THUMBNAIL;
	}

	return 0;
}

/**
* @brief  Move thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_move(const char *src_file_full_path,
		   const char *dest_file_full_path, char *dest_thumb_path)
{
	int result = 0;
	char src_thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1];

	if (!dest_thumb_path) {
		mb_svc_debug("dest_thumb_path is NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("_mb_svc_thumb_move--enter\n");

	result =
	    _mb_svc_thumb_generate_hash_name(src_file_full_path, src_thumb_path,
					     MB_SVC_FILE_PATH_LEN_MAX + 1);
	if (result < 0) {
		return result;
	}

	/* Get dest file thumbnail path    */
	result =
	    _mb_svc_thumb_generate_hash_name(dest_file_full_path,
					     dest_thumb_path,
					     MB_SVC_FILE_PATH_LEN_MAX + 1);
	if (result < 0) {
		return result;
	}

	if (rename(src_thumb_path, dest_thumb_path) < 0) {
		mb_svc_debug("thumb rename is failed");
		return MB_SVC_ERROR_MOVE_THUMBNAIL;
	}

	return 0;
/*
	if ( ( result = __mb_svc_thumb_cp(src_thumb_path, dest_thumb_path) ) != 0) {
		mb_svc_debug("__mb_svc_thumb_cp fails : %d", result);
		return MB_SVC_ERROR_MOVE_THUMBNAIL;
	}
	else 
	{
		return _mb_svc_thumb_rm(src_thumb_path);
	}
*/
}

/**
* @brief  Rename thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_rename(const char *src_file_full_path,
		     const char *dest_file_full_path, char *dest_thumb_path)
{
	return _mb_svc_thumb_move(src_file_full_path, dest_file_full_path,
				  dest_thumb_path);
}

static int
__mb_svc_thumb_cp(char *src_file_full_path, char *dest_file_full_path)
{
	FILE *src_fd, *dest_fd;
	char buf[16384];
	size_t num = 0;
	int ret = 0;

	if (src_file_full_path == NULL || dest_file_full_path == NULL ||
	    (strncmp
	     (src_file_full_path, dest_file_full_path,
	      MB_SVC_FILE_NAME_LEN_MAX) == 0)) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	src_fd = fopen(src_file_full_path, "rb");
	if (!src_fd) {
		mb_svc_debug("Src File open failed : %s", src_file_full_path);
		return MB_SVC_ERROR_FILE_NOT_EXSITED;
	}
	dest_fd = fopen(dest_file_full_path, "wb");
	if (!dest_fd) {
		fclose(src_fd);
		mb_svc_debug("Dst File open failed : %s", dest_file_full_path);
		return MB_SVC_ERROR_FILE_NOT_EXSITED;
	}
	while ((num = fread(buf, 1, sizeof(buf), src_fd)) > 0) {
		if (((int)num > sizeof(buf)) || ((int)num < 0)) {
			ret = MB_SVC_ERROR_FILE_IO;
		} else if (fwrite(buf, 1, num, dest_fd) != num) {
			ret = MB_SVC_ERROR_FILE_IO;
		}
	}

	fclose(src_fd);
	fclose(dest_fd);

	return ret;
}

int _mb_svc_thumb_rm(char *file_full_path)
{
	mb_svc_debug("_mb_svc_thumb_rm : %s", file_full_path);

	if (strncmp
	    (DEFAULT_IMAGE_THUMB, file_full_path,
	     strlen(DEFAULT_IMAGE_THUMB)) == 0
	    || strncmp(DEFAULT_VIDEO_THUMB, file_full_path,
		       strlen(DEFAULT_IMAGE_THUMB)) == 0) {
		mb_svc_debug("this is default thumb. Cannot remove");
		return 0;
	}

	if (unlink(file_full_path) == -1) {
		if (errno == ENOENT) {
			return MB_SVC_ERROR_FILE_NOT_EXSITED;
		} else {
			return MB_SVC_ERROR_INTERNAL;
		}
	}

	return 0;
}
