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

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/vfs.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <drm_client.h>
#include <mm_file.h>
#include <mm_error.h>
#include <libexif/exif-data.h>
#include <media-thumbnail.h>
#include "uuid.h"
#include "media-svc-util.h"
#include "media-svc-error.h"
#include "media-svc-debug.h"
#include "media-svc-env.h"
#include "media-svc-hash.h"
#include "media-svc-album.h"


#define MEDIA_SVC_FILE_EXT_LEN_MAX				6			/**<  Maximum file ext lenth*/

typedef enum {
	MEDIA_SVC_EXTRACTED_FIELD_NONE 			= 0x00000001,
	MEDIA_SVC_EXTRACTED_FIELD_TITLE 			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 1,
	MEDIA_SVC_EXTRACTED_FIELD_DESC 			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 2,
	MEDIA_SVC_EXTRACTED_FIELD_COPYRIGHT		= MEDIA_SVC_EXTRACTED_FIELD_NONE << 3,
	MEDIA_SVC_EXTRACTED_FIELD_AUTHOR		= MEDIA_SVC_EXTRACTED_FIELD_NONE << 4,
	MEDIA_SVC_EXTRACTED_FIELD_ARTIST			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 5,
	MEDIA_SVC_EXTRACTED_FIELD_GENRE			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 6,
	MEDIA_SVC_EXTRACTED_FIELD_ALBUM			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 7,
	MEDIA_SVC_EXTRACTED_FIELD_TRACKNUM		= MEDIA_SVC_EXTRACTED_FIELD_NONE << 8,
	MEDIA_SVC_EXTRACTED_FIELD_YEAR			= MEDIA_SVC_EXTRACTED_FIELD_NONE << 9,
	MEDIA_SVC_EXTRACTED_FIELD_CATEGORY		= MEDIA_SVC_EXTRACTED_FIELD_NONE << 10,
} media_svc_extracted_field_e;

#if 0
static char *__year_2_str(int year);

static char *__year_2_str(int year)
{
	static char ret[MEDIA_SVC_METADATA_LEN_MAX];

	if (year == -1 || year == 0) {
		_strncpy_safe(ret, MEDIA_SVC_TAG_UNKNOWN, MEDIA_SVC_METADATA_LEN_MAX);
	} else {
		snprintf(ret, MEDIA_SVC_METADATA_LEN_MAX - 1, "%d", year);
	}
	return ret;
}
#endif

char *_media_info_generate_uuid(void)
{
	uuid_t uuid_value;
	static char uuid_unparsed[50];	

	uuid_generate(uuid_value);
	uuid_unparse(uuid_value, uuid_unparsed);

	//media_svc_debug("UUID : %s", uuid_unparsed);
	return uuid_unparsed;
}

void _strncpy_safe(char *x_dst, const char *x_src, int max_len)
{
	if (!x_src || strlen(x_src) == 0) {
		media_svc_error("x_src is NULL");
		return;
	}

	if (max_len < 1) {
		media_svc_error("length is Wrong");
		return;
	}

    strncpy(x_dst, x_src, max_len-1);
	x_dst[max_len-1] = '\0';
}

int __media_svc_malloc_and_strncpy(char **dst, const char *src)
{
	int len = 0;

	if (!STRING_VALID(src)) {
		media_svc_error("invalid src");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	if (*dst) {
		SAFE_FREE(*dst);
	}

	len = strlen(src) + 1;
	*dst = malloc(len);

	if (*dst == NULL) {
		media_svc_error("malloc failed");
		return MEDIA_INFO_ERROR_INTERNAL;
	}

	strncpy(*dst, src, len);
	char *p = *dst;
	p[len - 1] = '\0';

	return MEDIA_INFO_ERROR_NONE;
}

static void __media_svc_split_to_double(char *input, double *arr, int *num)
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

static int __media_svc_get_exif_info(ExifData *ed,
										char *buf,
										int *i_value,
										double *d_value,
										int ifdtype,
										long tagtype)
{
	ExifEntry *entry;
	ExifIfd ifd;
	ExifTag tag;

	if (ed == NULL) {
		return -1;
	}

	ifd = ifdtype;
	tag = tagtype;

	entry = exif_data_get_entry(ed, tag);
	if (entry) {
		/* Get the contents of the tag in human-readable form */
		if (tag == EXIF_TAG_ORIENTATION ||
				tag == EXIF_TAG_PIXEL_X_DIMENSION ||
				tag == EXIF_TAG_PIXEL_Y_DIMENSION) {

			if (i_value == NULL) {
				media_svc_error("i_value is NULL");
				return -1;
			}

			ExifByteOrder mByteOrder = exif_data_get_byte_order(ed);
			short exif_value = exif_get_short(entry->data, mByteOrder);
			media_svc_debug("%s : %d", exif_tag_get_name_in_ifd(tag,ifd), exif_value);
			*i_value = (int)exif_value;

		} else if (tag == EXIF_TAG_GPS_LATITUDE || tag == EXIF_TAG_GPS_LONGITUDE) {

			if (d_value == NULL) {
				media_svc_error("d_value is NULL");
				return -1;
			}

			/* Get the contents of the tag in human-readable form */
			char gps_buf[MEDIA_SVC_METADATA_LEN_MAX + 1] = { '\0' };
			exif_entry_get_value(entry, gps_buf, sizeof(gps_buf));
			gps_buf[strlen(gps_buf)] = '\0';

			media_svc_debug("%s: %s\n", exif_tag_get_name_in_ifd(tag, ifd), gps_buf);

			double tmp_arr[3] = { 0.0, 0.0, 0.0 };
			int count = 0;

			__media_svc_split_to_double(gps_buf, tmp_arr, &count);
			if (count != 3) {
				media_svc_error("Value is invalid");
				return -1;
			}

			*d_value = tmp_arr[0] + tmp_arr[1] / 60 + tmp_arr[2] / 3600;
			media_svc_debug("GPS value is %f", *d_value);
		} else {

			if (buf == NULL) {
				media_svc_error("buf is NULL");
				return -1;
			}

			exif_entry_get_value(entry, buf, MEDIA_SVC_METADATA_LEN_MAX);
			buf[strlen(buf)] = '\0';

			if (*buf) {
				media_svc_debug("%s: %s\n", exif_tag_get_name_in_ifd(tag, ifd), buf);
			}
		}
	}

	return MEDIA_INFO_ERROR_NONE;
}

unsigned int _media_svc_get_current_time(void)
{
	struct timeval	t;
	unsigned int tval = 0;
	gettimeofday(&t, NULL);

	tval = t.tv_sec*1000000L + t.tv_usec;

	return tval/1000;
}

int _media_svc_check_escape_char(char ch)
{
	int i;
	char escape_char[3] = {'%', '_' ,'#'};

	for (i = 0; i < 3; i++) {
		if (ch == escape_char[i]) {
			return 1;
		}
	}

	return 0;
}

char *_media_svc_escape_str(char *input, int len)
{
	int i = 0;
	int j = 0;
	char *result = NULL;

	result = (char*)malloc(len * 2 * sizeof(char) + 1);
	if (result == NULL) {
		return NULL;
	}

	for (i = 0; i < len; i++, j++) {
		if (input[i] == '\0') break;

		if (_media_svc_check_escape_char(input[i])) {
			result[j] = '#';
			result[++j] = input[i];
		} else {
			result[j] = input[i];
		}
	}

	result[j] = '\0';

	return result;
}

int _media_svc_rename_file( const char *old_name, const char *new_name)
{
	if((old_name == NULL) || (new_name == NULL))
	{
		media_svc_error("invalid file name");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	if (rename(old_name, new_name) < 0) {
		media_svc_error("file rename is failed. errno : %s", strerror(errno));
		return MEDIA_INFO_ERROR_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;
}

bool _media_svc_remove_file(const char *path)
{
	int result = -1;

	result = remove(path);
	if (result == 0) {
		media_svc_debug("success to remove file");
		return TRUE;
	} else {
		media_svc_error("fail to remove file result errno = %s", strerror(errno));
		return FALSE;
	}
}

int _media_svc_remove_all_files_in_dir(const char *dir_path)
{
	struct dirent *entry = NULL;
	struct stat st;
	char filename[MEDIA_SVC_PATHNAME_SIZE] = {0};
	DIR *dir = NULL;

	dir = opendir(dir_path);
	if (dir == NULL) {
		media_svc_error("%s is not exist", dir_path);
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}
		snprintf(filename, sizeof(filename), "%s/%s", dir_path, entry->d_name);

		if (stat(filename, &st) != 0) {
			continue;
		}
		if (S_ISDIR(st.st_mode)) {
			continue;
		}
		if (unlink(filename) != 0) {
			media_svc_error("failed to remove : %s", filename);
			closedir(dir);
			return MEDIA_INFO_ERROR_INTERNAL;
		}
	}

	closedir(dir);
	return MEDIA_INFO_ERROR_NONE;
}

char *_media_svc_get_title_from_filepath (const char *path)
{
	char *filename = NULL;
	char *title = NULL;
	char	*ext = NULL;
	int filename_len = -1;
	int new_title_len = -1;

	media_svc_debug("title tag doesn't exist, so get from file path");

	if (!path) {
		media_svc_error("path is NULL");
		return NULL;
	}

	filename = g_path_get_basename(path);
	if ((filename == NULL) || (strlen(filename) < 1)) {
		media_svc_error("wrong file name");
		SAFE_FREE(filename);
		return NULL;
	}

	filename_len = strlen(filename);

	ext = g_strrstr(filename, ".");
	if (!ext) {
		media_svc_error("there is no file extention");
		return filename;
	}

	new_title_len = filename_len - strlen(ext);
	if (new_title_len < 1) {
		media_svc_error("title length is zero");
		SAFE_FREE(filename);
		return NULL;
	}

	title = g_strndup(filename, new_title_len < MEDIA_SVC_PATHNAME_SIZE ? new_title_len : MEDIA_SVC_PATHNAME_SIZE-1);

	SAFE_FREE(filename);

	media_svc_debug("extract title is [%s]", title);

	return title;
}

int _media_svc_save_image(void *image, int size, char *image_path)
{
	media_svc_debug("start save image, path: %s", image_path);
	if (!image) {
		media_svc_error("invalid image..");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	struct statfs fs;
	if (-1 == statfs(MEDIA_SVC_THUMB_PATH_PREFIX, &fs)) {
		media_svc_error("error in statfs");
		return MEDIA_INFO_ERROR_INTERNAL;
	}

	long bsize_kbytes = fs.f_bsize >> 10;

	if ((bsize_kbytes * fs.f_bavail) < 1024) {
		media_svc_error("not enought space...");
		return MEDIA_INFO_ERROR_INTERNAL;
	}

	FILE *fp = NULL;
	int nwrite = -1;
	if (image != NULL && size > 0) {
		fp = fopen(image_path, "w");

		if (fp == NULL) {
			media_svc_error("failed to open file");
			return MEDIA_INFO_ERROR_INTERNAL;
		}
		media_svc_debug("image size = [%d]",  size);

		nwrite = fwrite(image, 1, size, fp);
		if (nwrite != size) {
			media_svc_error("failed to write thumbnail");
			fclose(fp);
			return MEDIA_INFO_ERROR_INTERNAL;
		}
		fclose(fp);
	}

	media_svc_debug("save thumbnail success!!");

	return MEDIA_INFO_ERROR_NONE;
}

bool _media_svc_get_thumbnail_path(media_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format)
{
	char savename[MEDIA_SVC_PATHNAME_SIZE] = {0};
	char file_ext[MEDIA_SVC_FILE_EXT_LEN_MAX + 1] = {0};
	char *thumb_dir = NULL;
	char hash[255 + 1];
	char *thumbfile_ext = NULL;

	thumb_dir = (storage_type == MEDIA_SVC_STORAGE_INTERNAL) ? MEDIA_SVC_THUMB_INTERNAL_PATH : MEDIA_SVC_THUMB_EXTERNAL_PATH;

	memset(file_ext, 0, sizeof(file_ext));
	if (!_media_svc_get_file_ext(pathname, file_ext)) {
		media_svc_error("get file ext fail");
		return FALSE;
	}

	int err = -1;
	err = mb_svc_generate_hash_code(pathname, hash, sizeof(hash));
	if (err < 0) {
		media_svc_error("mb_svc_generate_hash_code failed : %d", err);
		return FALSE;
	}

	media_svc_debug("img format is [%s]", img_format);

	if((strstr(img_format, "jpeg") != NULL) || (strstr(img_format, "jpg") != NULL)) {
		thumbfile_ext = "jpg";
	} else if(strstr(img_format, "png") != NULL) {
		thumbfile_ext = "png";
	} else if(strstr(img_format, "gif") != NULL) {
		thumbfile_ext = "gif";
	} else if(strstr(img_format, "bmp") != NULL) {
		thumbfile_ext = "bmp";
	} else {
		media_svc_error("Not proper img format");
		return FALSE;
	}

	snprintf(savename, sizeof(savename), "%s/.%s-%s.%s", thumb_dir, file_ext, hash, thumbfile_ext);
	_strncpy_safe(thumb_path, savename, MEDIA_SVC_PATHNAME_SIZE);
	media_svc_debug("thumb_path is [%s]", thumb_path);

	return TRUE;
}

bool _media_svc_get_file_ext(const char *file_path, char *file_ext)
{
	int i = 0;

	for (i = strlen(file_path); i >= 0; i--) {
		if (file_path[i] == '.') {
			_strncpy_safe(file_ext, &file_path[i+1], MEDIA_SVC_FILE_EXT_LEN_MAX);
			return true;
		}

		if (file_path[i] == '/') {
			return false;
		}
	}
	return false;
}

int _media_svc_get_file_time(const char *full_path)
{
	struct stat statbuf;
	int fd = 0;

	memset(&statbuf, 0, sizeof(struct stat));
	fd = stat(full_path, &statbuf);
	if (fd == -1) {
		 media_svc_debug("stat(%s) fails.", full_path);
		 return MEDIA_INFO_ERROR_INTERNAL;
	 }

	 return statbuf.st_mtime;
}

int _media_svc_set_media_info(media_svc_content_info_s *content_info, media_svc_storage_type_e storage_type,
			  const char *path, const char *mime_type, media_svc_media_type_e media_type, bool refresh)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	char * media_uuid = NULL;
	char * file_name = NULL;
	struct stat st;
	drm_bool_type_e drm_type;

	ret = __media_svc_malloc_and_strncpy(&content_info->path, path);
	media_svc_retv_del_if(ret < 0, ret, content_info);

	memset(&st, 0, sizeof(struct stat));
	if (stat(path, &st) == 0) {
		content_info->modified_time = st.st_mtime;
		content_info->size = st.st_size;
		media_svc_debug("Modified time : %d", content_info->modified_time);
		media_svc_debug("Size : %lld", content_info->size);
	} else {
		media_svc_error("stat failed : %s", strerror(errno));
	}

	/* refresh is TRUE when file modified. so only modified_time and size are changed*/
	if(refresh) {
		media_svc_debug("refresh");
		return MEDIA_INFO_ERROR_NONE;
	}

	content_info->media_type = media_type;
	content_info->storage_type = storage_type;
	time(&content_info->added_time);

	media_uuid = _media_info_generate_uuid();
	media_svc_retvm_if(media_uuid == NULL, MEDIA_INFO_ERROR_INTERNAL, "Invalid UUID");

	ret = __media_svc_malloc_and_strncpy(&content_info->media_uuid, media_uuid);
	media_svc_retv_del_if(ret < 0, ret, content_info);

	ret = __media_svc_malloc_and_strncpy(&content_info->mime_type, mime_type);
	media_svc_retv_del_if(ret < 0, ret, content_info);

	file_name = g_path_get_basename(path);
	ret = __media_svc_malloc_and_strncpy(&content_info->file_name, file_name);
	SAFE_FREE(file_name);
	media_svc_retv_del_if(ret < 0, ret, content_info);
	//_strncpy_safe(content_info->file_name, file_name, sizeof(content_info->file_name));

	ret = drm_is_drm_file(content_info->path, &drm_type);
	if (ret < 0) {
		media_svc_error("drm_is_drm_file falied : %d", ret);
		drm_type = DRM_FALSE;
	}

	content_info->is_drm = drm_type;

	content_info->played_count = 0;
	content_info->last_played_time= 0;
	content_info->last_played_position= 0;
	content_info->favourate= 0;

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_extract_image_metadata(media_svc_content_info_s *content_info, media_svc_media_type_e media_type)
{
	media_svc_debug_func();

	if (content_info == NULL || media_type != MEDIA_SVC_MEDIA_TYPE_IMAGE) {
		media_svc_error("content_info == NULL || media_type != MEDIA_SVC_MEDIA_TYPE_IMAGE");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	char buf[MEDIA_SVC_METADATA_LEN_MAX + 1] = { '\0' };
	char description_buf[MEDIA_SVC_METADATA_DESCRIPTION_MAX + 1] = { '\0' };
	memset(buf, 0x00, sizeof(buf));
	memset(description_buf, 0x00, sizeof(description_buf));

	int ret = MEDIA_INFO_ERROR_NONE;
	double value = 0.0;
	int orient_value = 0;
	int exif_width = 0;
	int exif_height = 0;
	ExifData *ed = NULL;

	char *path = content_info->path;
	if (path == NULL) {
		media_svc_error("path is NULL");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	/* Load an ExifData object from an EXIF file */
	ed = exif_data_new_from_file(path);

	if (!ed) {
		media_svc_debug("There is no exif data in [ %s ]", path);
	}

	if (__media_svc_get_exif_info(ed, buf, NULL, NULL, EXIF_IFD_0, EXIF_TAG_GPS_LATITUDE_REF) == MEDIA_INFO_ERROR_NONE) {
		if (strlen(buf) != 0) {
			if (__media_svc_get_exif_info(ed, NULL, NULL, &value, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE) == MEDIA_INFO_ERROR_NONE) {

				if (strcmp(buf, "S") == 0) {
					value = -1 * value;
				}

				content_info->media_meta.latitude = value;
			} else {
				content_info->media_meta.latitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
				media_svc_debug("Use default gps value");
			}
		} else {
			content_info->media_meta.latitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
			media_svc_debug("Use default gps value");
		}
	} else {
		content_info->media_meta.latitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
		media_svc_debug("Use default gps value");
	}

	memset(buf, 0x00, sizeof(buf));

	if (__media_svc_get_exif_info(ed, buf, NULL, NULL, EXIF_IFD_0, EXIF_TAG_GPS_LONGITUDE_REF) == MEDIA_INFO_ERROR_NONE) {
		if (strlen(buf) != 0) {
			if (__media_svc_get_exif_info(ed, NULL, NULL, &value, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE) == MEDIA_INFO_ERROR_NONE) {
				if (strcmp(buf, "W") == 0) {
					value = -1 * value;
				}
				content_info->media_meta.longitude = value;
			} else {
				content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
				media_svc_debug("Use default gps value");
			}
		} else {
			content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
			media_svc_debug("Use default gps value");
		}
	} else {
		content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
		media_svc_debug("Use default gps value");
	}

	memset(buf, 0x00, sizeof(buf));

	if (__media_svc_get_exif_info(ed, description_buf, NULL, NULL, EXIF_IFD_0, EXIF_TAG_IMAGE_DESCRIPTION) == MEDIA_INFO_ERROR_NONE) {
		if (strlen(description_buf) == 0) {
			media_svc_debug("Use 'No description'");
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, "No description");
			media_svc_retv_del_if(ret < 0, ret, content_info);
		} else {
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, description_buf);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}
	} else {
		media_svc_debug("Use 'No description'");
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, "No description");
		media_svc_retv_del_if(ret < 0, ret, content_info);
	}

	memset(buf, 0x00, sizeof(buf));

	if (__media_svc_get_exif_info(ed, buf, NULL, NULL, EXIF_IFD_0, EXIF_TAG_DATE_TIME) == MEDIA_INFO_ERROR_NONE) {
		if (strlen(buf) == 0) {
			media_svc_debug("time  is NULL");
		} else {
			media_svc_debug("time  is %s", buf);
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.datetaken, buf);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}
	} else {
		media_svc_debug("time  is NULL");
	}

	/* Get orientation value from exif. */
	if (__media_svc_get_exif_info(ed, NULL, &orient_value, NULL, EXIF_IFD_0, EXIF_TAG_ORIENTATION) == MEDIA_INFO_ERROR_NONE) {
		if (orient_value >= NOT_AVAILABLE && orient_value <= ROT_270) {
			content_info->media_meta.orientation = orient_value;
		} else {
			content_info->media_meta.orientation = 0;
		}
	} else {
		content_info->media_meta.orientation = 0;
	}

	/* Get width value from exif. */
	if (__media_svc_get_exif_info(ed, NULL, &exif_width, NULL, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION) == MEDIA_INFO_ERROR_NONE) {
		if (exif_width > 0) {
			content_info->media_meta.width = exif_width;
		} else {
			content_info->media_meta.width = 0;
		}
	} else {
		content_info->media_meta.width = 0;
	}

	/* Get height value from exif. */
	if (__media_svc_get_exif_info(ed, NULL, &exif_height, NULL, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION) == MEDIA_INFO_ERROR_NONE) {
		if (exif_height > 0) {
			content_info->media_meta.height = exif_height;
		} else {
			content_info->media_meta.height = 0;
		}
	} else {
		content_info->media_meta.height = 0;
	}

	if (ed != NULL) exif_data_unref(ed);

	/* Extracting thumbnail */
	char thumb_path[MEDIA_SVC_PATHNAME_SIZE + 1] = {0, };
	int width = 0;
	int height = 0;

	ret = thumbnail_request_from_db_with_size(content_info->path, thumb_path, sizeof(thumb_path), &width, &height);
	if (ret < 0) {
		media_svc_error("thumbnail_request_from_db failed: %d", ret);
	} else {
		media_svc_debug("thumbnail_request_from_db success: %s", thumb_path);
	}

	content_info->media_meta.width = width;
	content_info->media_meta.height = height;
	ret = __media_svc_malloc_and_strncpy(&content_info->thumbnail_path, thumb_path);
	media_svc_retv_del_if(ret < 0, ret, content_info);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_extract_media_metadata(sqlite3 *handle, media_svc_content_info_s *content_info, media_svc_media_type_e media_type)
{
	MMHandleType content = 0;
	MMHandleType tag = 0;
	char *p = NULL;
	void *image = NULL;
	int size = -1;
	int extracted_field = MEDIA_SVC_EXTRACTED_FIELD_NONE;
	int mmf_error = -1;
	bool thumb_extracted_from_drm = FALSE;
	char *err_attr_name = NULL;
	char *title = NULL;
	bool extract_thumbnail = FALSE;
	bool append_album = FALSE;
	int album_id = 0;
	double gps_value = 0.0;
	int ret = MEDIA_INFO_ERROR_NONE;
	drm_bool_type_e drm_type;
	char *path = content_info->path;

	ret = drm_is_drm_file(path, &drm_type);
	if (ret < 0) {
		media_svc_error("drm_is_drm_file falied : %d", ret);
		drm_type = DRM_FALSE;
	}

	/*To do - code for DRM content*/
	if (drm_type) {
		bool invalid_file = FALSE;
		drm_file_type_e drm_file_type;
		drm_permission_type_e drm_perm_type = DRM_PERMISSION_TYPE_PLAY;
		drm_content_info_s contentInfo;
		drm_license_status_e license_status;
		memset(&contentInfo, 0x00, sizeof(drm_content_info_s));

		ret = drm_get_file_type(path, &drm_file_type);
		if (ret < 0) {
			media_svc_error("drm_get_file_type falied : %d", ret);
			drm_file_type = DRM_TYPE_UNDEFINED;
			invalid_file = TRUE;
		}

		ret = drm_get_content_info(path, &contentInfo);
		if (ret != DRM_RETURN_SUCCESS) {
			media_svc_error("drm_get_content_info() fails. : %d", ret);
			invalid_file = TRUE;
		}

		ret = drm_get_license_status(path, drm_perm_type, &license_status);
		if (ret != DRM_RETURN_SUCCESS) {
			media_svc_error("drm_get_license_status() fails. : %d", ret);
			invalid_file = TRUE;
		}

		if ((!invalid_file) && (license_status != DRM_LICENSE_STATUS_VALID)) {
			invalid_file = TRUE;
			if (drm_file_type == DRM_TYPE_OMA_V1) {

				if (strlen(contentInfo.title) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.title, contentInfo.title);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
				}

				if (strlen(contentInfo.description) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.description, contentInfo.description);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_DESC;
				}
			} else if (drm_file_type == DRM_TYPE_OMA_V2) {
				if (strlen(contentInfo.title) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.title, contentInfo.title);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
				}

				if (strlen(contentInfo.description) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.description, contentInfo.description);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_DESC;
				}

				if (strlen(contentInfo.copyright) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.copyright, contentInfo.copyright);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_COPYRIGHT;
				}

				if (strlen(contentInfo.author) > 0) {
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, contentInfo.author);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					 __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, contentInfo.author);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_AUTHOR;
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_ARTIST;
				}
			}
		}

		if (invalid_file) {
			if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_TITLE)) {
				title = _media_svc_get_title_from_filepath(path);
				if (title) {
					__media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
					SAFE_FREE(title);
					media_svc_retv_del_if(ret < 0, ret, content_info);
				} else {
					media_svc_error("Can't extract title from filepath");
					__media_svc_malloc_and_strncpy(&content_info->media_meta.title, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
				}
			}

			if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_DESC)) {
				__media_svc_malloc_and_strncpy(&content_info->media_meta.description, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}
			if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_AUTHOR)) {
				__media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}
			if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_ARTIST)) {
				__media_svc_malloc_and_strncpy(&content_info->media_meta.description, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}
			if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_COPYRIGHT)) {
				__media_svc_malloc_and_strncpy(&content_info->media_meta.copyright, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}

			__media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			__media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			__media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);

			return MEDIA_INFO_ERROR_NONE;
		}
	}

#if 0
	if (drm_svc_is_drm_file(content_info->path)) {
		bool invalid_file = FALSE;

		DRM_FILE_TYPE type = drm_svc_get_drm_type(content_info->path);

		if (type == DRM_FILE_TYPE_OMA) {
			drm_dcf_header_t header_info;
			memset(&header_info, 0, sizeof(drm_dcf_header_t));
			media_svc_debug("drm type is OMA");

			if (drm_svc_get_dcf_header_info(content_info->path, &header_info) != DRM_RESULT_SUCCESS) {
				media_svc_debug("cannot get dcf header info. just get the title");
				title = _media_svc_get_title_from_filepath(content_info->path);
				if (title) {
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
					SAFE_FREE(title);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
				} else {
					media_svc_error("Can't extract title from filepath");
					return MEDIA_INFO_ERROR_INTERNAL;
				}

/*
				_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
				_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
				_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
				_strncpy_safe(content_info->media_meta.author, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.author));
				_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/

				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);

				return MEDIA_INFO_ERROR_NONE;
			}

			if (drm_svc_has_valid_ro(content_info->path, DRM_PERMISSION_PLAY) != DRM_RESULT_SUCCESS) {
				media_svc_debug("no valid ro. can't extract meta data");
				invalid_file = TRUE;
			}

			if (header_info.version == DRM_OMA_DRMV1_RIGHTS) {
				media_svc_debug("DRM V1");
				if (invalid_file) {

					if (strlen(header_info.headerUnion.headerV1.contentName) > 0) {

						//_strncpy_safe(content_info->media_meta.title, header_info.headerUnion.headerV1.contentName, sizeof(content_info->media_meta.title));
						ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, header_info.headerUnion.headerV1.contentName);
						media_svc_retv_del_if(ret < 0, ret, content_info);

						extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
						media_svc_debug("extract title from DCF");
					}

					if (strlen(header_info.headerUnion.headerV1.contentDescription) > 0) {
						//_strncpy_safe(content_info->media_meta.description, header_info.headerUnion.headerV1.contentDescription, sizeof(content_info->media_meta.description));
						ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, header_info.headerUnion.headerV1.contentDescription);
						media_svc_retv_del_if(ret < 0, ret, content_info);

						extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_DESC;
						media_svc_debug("extract description from DCF");
					}
				}
			} else if (header_info.version == DRM_OMA_DRMV2_RIGHTS) {
				drm_user_data_common_t metadata;
				int type_index = -1;

				media_svc_debug("DRM V2");

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_TITLE, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.title, metadata.subBox.title.str, sizeof(content_info->media_meta.title));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, metadata.subBox.title.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
					media_svc_debug("extract title from odf");
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_DESCRIPTION, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.description, metadata.subBox.desc.str, sizeof(content_info->media_meta.description));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, metadata.subBox.desc.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_DESC;
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_COPYRIGHT, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.copyright, metadata.subBox.copyright.str, sizeof(content_info->media_meta.copyright));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.copyright, metadata.subBox.copyright.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_COPYRIGHT;
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_AUTHOR, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.composer, metadata.subBox.author.str, sizeof(content_info->media_meta.composer));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, metadata.subBox.author.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_AUTHOR;
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_PERFORMER, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.artist, metadata.subBox.performer.str, sizeof(content_info->media_meta.artist));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, metadata.subBox.performer.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_ARTIST;
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_GENRE, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.genre, metadata.subBox.genre.str, sizeof(content_info->media_meta.genre));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, metadata.subBox.genre.str);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					media_svc_debug("genre : %s", content_info->media_meta.genre);
					if ((strcasecmp("Ringtone", metadata.subBox.genre.str) == 0) | (strcasecmp("Alert tone", metadata.subBox.genre.str) == 0)) {
						content_info->media_type = MEDIA_SVC_MEDIA_TYPE_SOUND;
					}
					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_GENRE;
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_ALBUM, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.album, metadata.subBox.album.albumTitle, sizeof(content_info->media_meta.album));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, metadata.subBox.album.albumTitle);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_ALBUM;

					char track_num[MEDIA_SVC_METADATA_LEN_MAX] = {0,};
					snprintf(track_num, sizeof(track_num), "%d", metadata.subBox.album.trackNum);

					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.track_num, track_num);
					media_svc_retv_del_if(ret < 0, ret, content_info);

					//snprintf(content_info->media_meta.track_num, MEDIA_SVC_METADATA_LEN_MAX, "%d", metadata.subBox.album.trackNum);
				}

				if (drm_svc_get_user_data_box_info(content_info->path, DRM_UDTA_RECODINGYEAR, &metadata) == DRM_RESULT_SUCCESS) {
					//_strncpy_safe(content_info->media_meta.year, __year_2_str(metadata.subBox.recodingYear.recodingYear), sizeof(content_info->media_meta.year));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, __year_2_str(metadata.subBox.recodingYear.recodingYear));
					media_svc_retv_del_if(ret < 0, ret, content_info);

					extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_YEAR;
				}

				if (drm_svc_get_index_of_relative_contents(content_info->path, DRM_CONTENTS_INDEX_ALBUMJACKET, &type_index) == DRM_RESULT_SUCCESS) {
					char thumb_path[MEDIA_SVC_PATHNAME_SIZE+1] = {0};

					if (drm_svc_make_multipart_drm_full_path(content_info->path, type_index, MEDIA_SVC_PATHNAME_SIZE, thumb_path) == DRM_TRUE) {

						DRM_FILE_HANDLE hFile = DRM_HANDLE_NULL;

						media_svc_debug("drm image path : %s", thumb_path);

						if (drm_svc_open_file(thumb_path, DRM_PERMISSION_ANY, &hFile) == DRM_RESULT_SUCCESS) {
							int thumb_size = 0;

							if (drm_svc_seek_file(hFile, 0, DRM_SEEK_END) != DRM_RESULT_SUCCESS) {
								goto DRM_SEEK_ERROR;
							}
							thumb_size = drm_svc_tell_file(hFile);

							if (drm_svc_seek_file(hFile, 0, DRM_SEEK_SET) != DRM_RESULT_SUCCESS) {
								goto DRM_SEEK_ERROR;
							}
							/* remove thumbnail extract routine in db creating time.
							media_svc_debug("drm thumb size : %d", thumb_size);
							if (thumb_size > 0) {
								unsigned int readSize = 0;

								thumb_buffer = malloc(thumb_size);
								if (drm_svc_read_file(hFile, thumb_buffer,thumb_size, &readSize) != DRM_RESULT_SUCCESS) {
									SAFE_FREE(thumb_buffer);
									goto DRM_SEEK_ERROR;
								}

								__save_thumbnail(thumb_buffer, readSize, 1, content_info);
								SAFE_FREE(thumb_buffer);
								thumb_extracted_from_drm = TRUE;
							}
							*/
							DRM_SEEK_ERROR:
								drm_svc_free_dcf_header_info(&header_info);
								drm_svc_close_file(hFile);
						}
					}
				}
			} else {
				media_svc_debug("unsupported drm format");
				drm_svc_free_dcf_header_info(&header_info);
				title = _media_svc_get_title_from_filepath(content_info->path);
				if (title) {
					//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
					SAFE_FREE(title);
					media_svc_retv_del_if(ret < 0, ret, content_info);

				} else {
					media_svc_error("Can't extract title from filepath");
					return MEDIA_INFO_ERROR_INTERNAL;
				}

				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
/*
				_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
				_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
				_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
				_strncpy_safe(content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.composer));
				_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/
				return MEDIA_INFO_ERROR_NONE;
			}

			if (invalid_file == TRUE) {
				if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_TITLE)) {
					title = _media_svc_get_title_from_filepath(content_info->path);
					if (title) {
						//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
						ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
						SAFE_FREE(title);
						media_svc_retv_del_if(ret < 0, ret, content_info);

						extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
					} else {
						media_svc_error("Can't extract title from filepath");
						drm_svc_free_dcf_header_info(&header_info);
						return MEDIA_INFO_ERROR_INTERNAL;
					}

					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
					media_svc_retv_del_if(ret < 0, ret, content_info);
/*
					_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
					_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
					_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
					_strncpy_safe(content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.composer));
					_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/
				}

				drm_svc_free_dcf_header_info(&header_info);
				return MEDIA_INFO_ERROR_NONE;
			}
		} else if (type == DRM_FILE_TYPE_PLAYREADY) {
			media_svc_debug("drm type is PLAYREADY");
			if (drm_svc_has_valid_ro(content_info->path, DRM_PERMISSION_PLAY) != DRM_RESULT_SUCCESS) {
				media_svc_debug("no valid ro. can't extract meta data");
				title = _media_svc_get_title_from_filepath(content_info->path);
				if (title) {
					ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
					SAFE_FREE(title);
					media_svc_retv_del_if(ret < 0, ret, content_info);
					//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
				} else {
					media_svc_error("Can't extract title from filepath");
					return MEDIA_INFO_ERROR_INTERNAL;
				}

				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
/*
				_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
				_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
				_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
				_strncpy_safe(content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.composer));
				_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/

				return MEDIA_INFO_ERROR_NONE;
			}
		} else {
			media_svc_error("Not supported DRM type");
			title = _media_svc_get_title_from_filepath(content_info->path);
			if (title) {
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
				SAFE_FREE(title);
				media_svc_retv_del_if(ret < 0, ret, content_info);
				//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
			} else {
				media_svc_error("Can't extract title from filepath");
				return MEDIA_INFO_ERROR_INTERNAL;
			}

			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
/*
			_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
			_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
			_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
			_strncpy_safe(content_info->media_meta.author, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.author));
			_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/
			return MEDIA_INFO_ERROR_NONE;
		}
	}
#endif
	/*Get Content attribute ===========*/
	mmf_error = mm_file_create_content_attrs(&content, content_info->path);
	if (mmf_error == MM_ERROR_NONE) {
		/*Common attribute*/
		mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_DURATION, &content_info->media_meta.duration, NULL);
		if (mmf_error != 0) {
			SAFE_FREE(err_attr_name);
			media_svc_debug("fail to get duration attr - err(%x)", mmf_error);
		} else {
			media_svc_debug("duration : %d", content_info->media_meta.duration);
		}

		/*Sound/Music attribute*/
		if((media_type == MEDIA_SVC_MEDIA_TYPE_SOUND) || (media_type == MEDIA_SVC_MEDIA_TYPE_MUSIC)) {

			mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_BITRATE, &content_info->media_meta.bitrate, NULL);
			if (mmf_error != 0) {
				SAFE_FREE(err_attr_name);
				media_svc_debug("fail to get audio bitrate attr - err(%x)", mmf_error);
			} else {
				media_svc_debug("bit rate : %d", content_info->media_meta.bitrate);
			}

			mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_SAMPLERATE, &content_info->media_meta.samplerate, NULL);
			if (mmf_error != 0) {
				SAFE_FREE(err_attr_name);
				media_svc_debug("fail to get sample rate attr - err(%x)", mmf_error);
			} else {
				media_svc_debug("sample rate : %d", content_info->media_meta.samplerate);
			}

			mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_CHANNELS, &content_info->media_meta.channel, NULL);
			if (mmf_error != 0) {
				SAFE_FREE(err_attr_name);
				media_svc_debug("fail to get audio channels attr - err(%x)", mmf_error);
			} else {
				media_svc_debug("channel : %d", content_info->media_meta.channel);
			}
		}else if(media_type == MEDIA_SVC_MEDIA_TYPE_VIDEO)	{	/*Video attribute*/

			mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_VIDEO_WIDTH, &content_info->media_meta.width, NULL);
			if (mmf_error != 0) {
				SAFE_FREE(err_attr_name);
				media_svc_debug("fail to get video width attr - err(%x)", mmf_error);
			} else {
				media_svc_debug("width : %d", content_info->media_meta.width);
			}

			mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_VIDEO_HEIGHT, &content_info->media_meta.height, NULL);
			if (mmf_error != 0) {
				SAFE_FREE(err_attr_name);
				media_svc_debug("fail to get video height attr - err(%x)", mmf_error);
			} else {
				media_svc_debug("height : %d", content_info->media_meta.height);
			}

		} else {
			media_svc_error("Not support type");
			return MEDIA_INFO_ERROR_INVALID_PARAMETER;
		}

		mmf_error = mm_file_destroy_content_attrs(content);
		if (mmf_error != 0) {
			media_svc_debug("fail to free content attr - err(%x)", mmf_error);
		}
	} else {
		media_svc_error("error in mm_file_create_content_attrs [%d]", mmf_error);
	}

	/*Get Content Tag attribute ===========*/
	mmf_error = mm_file_create_tag_attrs(&tag, content_info->path);

	if (mmf_error == MM_ERROR_NONE) {
		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ALBUM, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_ALBUM)) && mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.album, p, sizeof(content_info->media_meta.album));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);

			media_svc_debug("album[%d] : %s", size, content_info->media_meta.album);
		} else {
			SAFE_FREE(err_attr_name);
			media_svc_debug("album - unknown");
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			//_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTIST, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_ARTIST)) && mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.artist, p, sizeof(content_info->media_meta.artist));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			media_svc_debug("artist[%d] : %s", size, content_info->media_meta.artist);
		} else {
			SAFE_FREE(err_attr_name);
			media_svc_debug("artist - unknown");
			//_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_GENRE, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_GENRE)) && mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.genre, p, sizeof(content_info->media_meta.genre));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);

			media_svc_debug("genre : %s", content_info->media_meta.genre);
			if ((strcasecmp("Ringtone", p) == 0) | (strcasecmp("Alert tone", p) == 0)) {
				content_info->media_type = MEDIA_SVC_MEDIA_TYPE_SOUND;
			}
		} else {
			SAFE_FREE(err_attr_name);
			media_svc_debug("genre - unknown");
			//_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_TITLE, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_TITLE)) && mmf_error == 0 && size > 0 && 	(!isspace(*p))) {
			//_strncpy_safe(content_info->media_meta.title, p, sizeof(content_info->media_meta.title));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);

			extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_TITLE;
			media_svc_debug("extract title from content : %s", content_info->media_meta.title);
			media_svc_debug("^^^^^^^^^^^^^^^ path = %s, title = %s, size = %d ^^^^^^^^^^^^^^", content_info->path, content_info->media_meta.title, size);
		} else {
			SAFE_FREE(err_attr_name);
			title = _media_svc_get_title_from_filepath(content_info->path);
			if (title) {
				//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
				SAFE_FREE(title);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			} else {
				media_svc_error("Can't extract title from filepath");
				return MEDIA_INFO_ERROR_INTERNAL;
			}
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_DESCRIPTION, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_DESC)) && mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.description, p, sizeof(content_info->media_meta.description));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.description, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			media_svc_debug("desc : %s", content_info->media_meta.description);
		} else {
			SAFE_FREE(err_attr_name);
			//content_info->media_meta.description = strdup("");
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_AUTHOR, &p, &size, NULL);
		if ((!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_AUTHOR)) && mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.composer, p, sizeof(content_info->media_meta.composer));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);
			extracted_field |= MEDIA_SVC_EXTRACTED_FIELD_AUTHOR;
			media_svc_debug("extract composer from content : %s", content_info->media_meta.composer);
		} else {
			media_svc_debug("composer - unknown");
			SAFE_FREE(err_attr_name);
			//_strncpy_safe(content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.composer));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_TRACK_NUM, &p, &size, NULL);
		if (mmf_error == 0 && size > 0) {
			//_strncpy_safe(content_info->media_meta.track_num, p, sizeof(content_info->media_meta.track_num));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.track_num, p);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		} else {
			SAFE_FREE(err_attr_name);
			//_strncpy_safe(content_info->media_meta.track_num, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.track_num));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.track_num, MEDIA_SVC_TAG_UNKNOWN);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		}
		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_DATE, &p, &size, NULL);
		if (!(extracted_field & MEDIA_SVC_EXTRACTED_FIELD_YEAR)) {
			if (mmf_error == 0 && size > 0) {
				//_strncpy_safe(content_info->media_meta.year, p, sizeof(content_info->media_meta.year));
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, p);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			} else {
				SAFE_FREE(err_attr_name);
				//_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
				ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}
		} else {
			SAFE_FREE(err_attr_name);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_RATING, &p, &size, NULL);
		if (mmf_error == 0 && size > 0) {
			content_info->media_meta.rating = atoi(p);
		} else {
			SAFE_FREE(err_attr_name);
			content_info->media_meta.rating = 0;
		}

		/*Initialize album_id to 0. below code will set the album_id*/
		content_info->album_id = album_id;

		/* extract thumbnail image */
		if(strncmp(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, strlen(MEDIA_SVC_TAG_UNKNOWN))) {
			if(strncmp(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, strlen(MEDIA_SVC_TAG_UNKNOWN))) {

				ret = _media_svc_get_album_id(handle, content_info->media_meta.album, content_info->media_meta.artist, &album_id);

				if (ret != MEDIA_INFO_ERROR_NONE) {
					if (ret == MEDIA_INFO_ERROR_DATABASE_NO_RECORD) {
						media_svc_debug("album does not exist. So start to make album art");
						extract_thumbnail = TRUE;
						append_album = TRUE;
					} else
						return ret;
				} else {
					media_svc_debug("album already exists. don't need to make album art");
					content_info->album_id = album_id;
					ret = _media_svc_get_album_art_by_album_id(handle, album_id, &content_info->thumbnail_path);
					media_svc_debug("content_info->thumbnail_path[%s]", content_info->thumbnail_path);
					media_svc_retv_del_if((ret != MEDIA_INFO_ERROR_NONE) && (ret != MEDIA_INFO_ERROR_DATABASE_NO_RECORD), ret, content_info);
					extract_thumbnail = FALSE;
					append_album = FALSE;
				}
			} else {
				ret = _media_svc_get_album_id(handle, content_info->media_meta.album, content_info->media_meta.artist, &album_id);

				if (ret != MEDIA_INFO_ERROR_NONE) {

					if (ret == MEDIA_INFO_ERROR_DATABASE_NO_RECORD) {
						media_svc_debug("Unknown artist album does not exist.");
						extract_thumbnail = TRUE;
						append_album = TRUE;
					}
					else
						return ret;
				} else {
					media_svc_debug("Unknown artist album already exists.");

					content_info->album_id = album_id;
					extract_thumbnail = TRUE;
					append_album = FALSE;
				}
			}
		} else {
			extract_thumbnail = TRUE;
			append_album = FALSE;
		}

		if ((!thumb_extracted_from_drm) && (extract_thumbnail == TRUE)) {
			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK, &image, &size, NULL);
			if (mmf_error != 0) {
				media_svc_debug("fail to get tag artwork - err(%x)", mmf_error);
				SAFE_FREE(err_attr_name);
			} else {
				media_svc_debug("artwork size1 [%d]", size);
			}

			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK_SIZE, &size, NULL);
			if (mmf_error != 0) {
				media_svc_debug("fail to get artwork size - err(%x)", mmf_error);
				SAFE_FREE(err_attr_name);
			} else {
				media_svc_debug("artwork size2 [%d]", size);
			}
			if (image != NULL && size > 0) {
				bool ret = FALSE;
				int result = MEDIA_INFO_ERROR_NONE;
				char thumb_path[MEDIA_SVC_PATHNAME_SIZE] = "\0";
				int artwork_mime_size = -1;

				mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK_MIME, &p, &artwork_mime_size, NULL);
				if (mmf_error == 0 && artwork_mime_size > 0) {
					ret = _media_svc_get_thumbnail_path(content_info->storage_type, thumb_path, content_info->path, p);
					if (ret == FALSE) {
						media_svc_error("fail to get thumb path..");
						mmf_error = mm_file_destroy_tag_attrs(tag);
						if (mmf_error != 0) {
							media_svc_error("fail to free tag attr - err(%x)", mmf_error);
						}
						return MEDIA_INFO_ERROR_INTERNAL;
					}
				} else {
					SAFE_FREE(err_attr_name);
				}

				if (!strlen(thumb_path)) {
					media_svc_error("fail to get thumb path..");
					mmf_error = mm_file_destroy_tag_attrs(tag);
					if (mmf_error != 0) {
						media_svc_error("fail to free tag attr - err(%x)", mmf_error);
					}
					return MEDIA_INFO_ERROR_INTERNAL;
				}

				result = _media_svc_save_image(image, size, thumb_path);
				if (result != MEDIA_INFO_ERROR_NONE) {
					mmf_error = mm_file_destroy_tag_attrs(tag);
					if (mmf_error != 0) {
						media_svc_error("fail to free tag attr - err(%x)", mmf_error);
					}
					return result;
				}

				//_strncpy_safe(content_info->thumbnail_path, thumb_path, sizeof(content_info->thumbnail_path));
				ret = __media_svc_malloc_and_strncpy(&content_info->thumbnail_path, thumb_path);
				media_svc_retv_del_if(ret < 0, ret, content_info);
			}
		}

		if(append_album == TRUE) {

			if(strncmp(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, strlen(MEDIA_SVC_TAG_UNKNOWN)))
				ret = _media_svc_append_album(handle, content_info->media_meta.album, content_info->media_meta.artist, content_info->thumbnail_path, &album_id);
			else
				ret = _media_svc_append_album(handle, content_info->media_meta.album, content_info->media_meta.artist, NULL, &album_id);

			content_info->album_id = album_id;
		}

		if(media_type == MEDIA_SVC_MEDIA_TYPE_VIDEO) {
			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_LONGITUDE, &gps_value, NULL);
			if (mmf_error == 0) {
				if (gps_value == 0.0)
					content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
				else
					content_info->media_meta.longitude = gps_value;
			} else {
				SAFE_FREE(err_attr_name);
				content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
			}

			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_LATIDUE, &gps_value, NULL);
			if (mmf_error == 0) {
				if (gps_value == 0.0)
					content_info->media_meta.latitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
				else
					content_info->media_meta.latitude = gps_value;
			} else {
				SAFE_FREE(err_attr_name);
				content_info->media_meta.latitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
			}

			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ALTIDUE, &gps_value, NULL);
			if (mmf_error == 0) {
				if (gps_value == 0.0)
					content_info->media_meta.altitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
				else
					content_info->media_meta.altitude = gps_value;
			} else {
				SAFE_FREE(err_attr_name);
				content_info->media_meta.altitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
			}

			if ((!thumb_extracted_from_drm) && (extract_thumbnail == TRUE))
			{
				/* Extracting thumbnail */
				char thumb_path[MEDIA_SVC_PATHNAME_SIZE + 1] = {0, };
				int width = 0;
				int height = 0;

				ret = thumbnail_request_from_db_with_size(content_info->path, thumb_path, sizeof(thumb_path), &width, &height);
				if (ret < 0) {
					media_svc_error("thumbnail_request_from_db failed: %d", ret);
				} else {
					media_svc_debug("thumbnail_request_from_db success: %s", thumb_path);
				}

				ret = __media_svc_malloc_and_strncpy(&content_info->thumbnail_path, thumb_path);
				media_svc_retv_del_if(ret < 0, ret, content_info);

				if (content_info->media_meta.width <= 0) content_info->media_meta.width = width;
				if (content_info->media_meta.height <= 0) content_info->media_meta.height = height;
			}
		}

		mmf_error = mm_file_destroy_tag_attrs(tag);
		if (mmf_error != 0) {
			media_svc_error("fail to free tag attr - err(%x)", mmf_error);
		}
	}	else {
		char *title = NULL;
		media_svc_error("no tag information");

		title = _media_svc_get_title_from_filepath(content_info->path);
		if (title) {
			//_strncpy_safe(content_info->media_meta.title, title, sizeof(content_info->media_meta.title));
			ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.title, title);
			SAFE_FREE(title);
			media_svc_retv_del_if(ret < 0, ret, content_info);
		} else {
			media_svc_error("Can't extract title from filepath");
			return MEDIA_INFO_ERROR_INTERNAL;
		}

		/* in case of file size 0, MMFW Can't parsting tag info but add it to Music DB. */
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN);
		media_svc_retv_del_if(ret < 0, ret, content_info);
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN);
		media_svc_retv_del_if(ret < 0, ret, content_info);
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN);
		media_svc_retv_del_if(ret < 0, ret, content_info);
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN);
		media_svc_retv_del_if(ret < 0, ret, content_info);
		ret = __media_svc_malloc_and_strncpy(&content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN);
		media_svc_retv_del_if(ret < 0, ret, content_info);
/*
		_strncpy_safe(content_info->media_meta.album, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.album));
		_strncpy_safe(content_info->media_meta.artist, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.artist));
		_strncpy_safe(content_info->media_meta.genre, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.genre));
		_strncpy_safe(content_info->media_meta.composer, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.composer));
		_strncpy_safe(content_info->media_meta.year, MEDIA_SVC_TAG_UNKNOWN, sizeof(content_info->media_meta.year));
*/
		content_info->album_id = album_id;
		content_info->media_meta.longitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
		content_info->media_meta.latitude= MEDIA_SVC_DEFAULT_GPS_VALUE;
		content_info->media_meta.altitude = MEDIA_SVC_DEFAULT_GPS_VALUE;
	}

	return MEDIA_INFO_ERROR_NONE;
}

void _media_svc_destroy_content_info(media_svc_content_info_s *content_info)
{
	media_svc_retm_if(content_info == NULL, "content info is NULL");

	/* Delete media_svc_content_info_s */
	if (content_info->media_uuid) {
		SAFE_FREE(content_info->media_uuid);
	}
	if (content_info->path) {
		SAFE_FREE(content_info->path);
	}
	if (content_info->file_name) {
		SAFE_FREE(content_info->file_name);
	}
	if (content_info->mime_type) {
		SAFE_FREE(content_info->mime_type);
	}
	if (content_info->folder_uuid) {
		SAFE_FREE(content_info->folder_uuid);
	}
	if (content_info->thumbnail_path) {
		SAFE_FREE(content_info->thumbnail_path);
	}

	/* Delete media_svc_content_meta_s */
	if (content_info->media_meta.title) {
		SAFE_FREE(content_info->media_meta.title);
	}
	if (content_info->media_meta.album) {
		SAFE_FREE(content_info->media_meta.album);
	}
	if (content_info->media_meta.artist) {
		SAFE_FREE(content_info->media_meta.artist);
	}
	if (content_info->media_meta.genre) {
		SAFE_FREE(content_info->media_meta.genre);
	}
	if (content_info->media_meta.composer) {
		SAFE_FREE(content_info->media_meta.composer);
	}
	if (content_info->media_meta.year) {
		SAFE_FREE(content_info->media_meta.year);
	}
	if (content_info->media_meta.recorded_date) {
		SAFE_FREE(content_info->media_meta.recorded_date);
	}
	if (content_info->media_meta.copyright) {
		SAFE_FREE(content_info->media_meta.copyright);
	}
	if (content_info->media_meta.track_num) {
		SAFE_FREE(content_info->media_meta.track_num);
	}
	if (content_info->media_meta.description) {
		SAFE_FREE(content_info->media_meta.description);
	}
	if (content_info->media_meta.datetaken) {
		SAFE_FREE(content_info->media_meta.datetaken);
	}

	return;
}

int _media_svc_get_store_type_by_path(const char *path, media_svc_storage_type_e *storage_type)
{
	if(path != NULL && strlen(path) > 0)
	{
		if(strncmp(path, MEDIA_SVC_PATH_PHONE, strlen(MEDIA_SVC_PATH_PHONE)) == 0)
		{
			*storage_type = MEDIA_SVC_STORAGE_INTERNAL;
		}
		else if(strncmp (path, MEDIA_SVC_PATH_MMC, strlen(MEDIA_SVC_PATH_MMC)) == 0)
		{
			*storage_type = MEDIA_SVC_STORAGE_EXTERNAL;
		}
	}
	else
	{
		media_svc_error("INVALID parameter");
		return MEDIA_INFO_ERROR_INVALID_PARAMETER;
	}

	return MEDIA_INFO_ERROR_NONE;
}

char *_media_svc_replace_path(char *s, const char *olds, const char *news)
{
  char *result, *sr;
  size_t i, count = 0;
  size_t oldlen = strlen(olds); if (oldlen < 1) return s;
  size_t newlen = strlen(news);

  if (newlen != oldlen) {
    for (i = 0; s[i] != '\0';) {
      if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
      else i++;
    }   
  } else i = strlen(s);


  result = (char *) calloc(1, i + 1 + count * (newlen - oldlen));
  if (result == NULL) return NULL;

  sr = result;
  while (*s) {
    if (memcmp(s, olds, oldlen) == 0) {
      memcpy(sr, news, newlen);
      sr += newlen;
      s  += oldlen;
    } else *sr++ = *s++;
  }

  *sr = '\0';

  return result;
}

