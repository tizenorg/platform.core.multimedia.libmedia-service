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
 * This file defines structure and functions related to database.
 *
 * @file       	audio-svc-utils.c
 * @version 	0.1
 * @brief     	This file defines utilities for Audio Service.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <ctype.h>

#include <dirent.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <drm-service-types.h>
#include <drm-service.h>

#include <mm_file.h>
#include "media-info-util.h"
#include "audio-svc-debug.h"
#include "audio-svc-error.h"
#include "audio-svc-utils.h"
#include "audio-svc.h"
#include "audio-svc-types-priv.h"
#include "md5_hash.h"

#define AUDIO_SVC_PARENTAL_RATING_LEN		20
#define AUDIO_SVC_FILE_EXT_LEN_MAX				6			/**<  Maximum file ext lenth*/

typedef enum {
	AUDIO_SVC_EXTRACTED_FIELD_NONE 			= 0x00000001,
	AUDIO_SVC_EXTRACTED_FIELD_TITLE 			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 1,
	AUDIO_SVC_EXTRACTED_FIELD_DESC 			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 2,
	AUDIO_SVC_EXTRACTED_FIELD_COPYRIGHT		= AUDIO_SVC_EXTRACTED_FIELD_NONE << 3,
	AUDIO_SVC_EXTRACTED_FIELD_AUTHOR		= AUDIO_SVC_EXTRACTED_FIELD_NONE << 4,
	AUDIO_SVC_EXTRACTED_FIELD_ARTIST			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 5,
	AUDIO_SVC_EXTRACTED_FIELD_GENRE			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 6,
	AUDIO_SVC_EXTRACTED_FIELD_ALBUM			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 7,
	AUDIO_SVC_EXTRACTED_FIELD_TRACKNUM		= AUDIO_SVC_EXTRACTED_FIELD_NONE << 8,
	AUDIO_SVC_EXTRACTED_FIELD_YEAR			= AUDIO_SVC_EXTRACTED_FIELD_NONE << 9,
	AUDIO_SVC_EXTRACTED_FIELD_CATEGORY		= AUDIO_SVC_EXTRACTED_FIELD_NONE << 10,
} audio_svc_extracted_field_e;

static bool __audio_svc_get_file_ext (const char *file_path, char *file_ext);
static int __save_thumbnail(void *image, int size, char *thumb_path);

static bool __audio_svc_get_file_ext (const char *file_path, char *file_ext)
{
	int i = 0;

	for (i = strlen(file_path); i >= 0; i--) {
		if (file_path[i] == '.') {
			_strncpy_safe(file_ext, &file_path[i+1], AUDIO_SVC_FILE_EXT_LEN_MAX);
			return true;
		}

		if (file_path[i] == '/') {
			return false;
		}
	}
	return false;
}

static int __save_thumbnail(void *image, int size, char *thumb_path)
{
	audio_svc_debug("start save thumbnail, path: %s", thumb_path);
	if (!image) {
		audio_svc_error("invalid image..");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	struct statfs fs;
	if (-1 == statfs(AUDIO_SVC_THUMB_PATH_PREFIX, &fs)) {
		audio_svc_error("error in statfs");
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	long bsize_kbytes = fs.f_bsize >> 10;

	if ((bsize_kbytes * fs.f_bavail) < 1024) {
		audio_svc_error("not enought space...");
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	FILE *fp = NULL;
	int nwrite = -1;
	if (image != NULL && size > 0) {
		fp = fopen(thumb_path, "w");

		if (fp == NULL) {
			audio_svc_error("failed to open file");
			return AUDIO_SVC_ERROR_INTERNAL;
		}
		audio_svc_debug("image size = [%d]",  size);

		nwrite = fwrite(image, 1, size, fp);
		if (nwrite != size) {
			audio_svc_error("failed to write thumbnail");
			fclose(fp);
			return AUDIO_SVC_ERROR_INTERNAL;
		}
		fclose(fp);
	}

	audio_svc_debug("save thumbnail success!!");

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_extract_metadata_audio(audio_svc_storage_type_e storage_type, const char *path, audio_svc_audio_item_s *item)
{
	MMHandleType content = 0;
	MMHandleType tag = 0;
	char *p = NULL;
	void *image = NULL;
	int size = -1;
	int extracted_field = AUDIO_SVC_EXTRACTED_FIELD_NONE;
	int mmf_error = -1;
	bool thumb_extracted_from_drm = FALSE;
	char *err_attr_name = NULL;
	char *title = NULL;
#if 0
	bool extract_thumbnail = FALSE;
	int album_id = -1;
	char *thumbnail_path = NULL;
#endif
	int artwork_mime_size = -1;
	
	_strncpy_safe(item->pathname, path, sizeof(item->pathname));
	item->storage_type = storage_type;

	if (drm_svc_is_drm_file(item->pathname)) {
		bool invalid_file = FALSE;

		DRM_FILE_TYPE type = drm_svc_get_drm_type(item->pathname);

		if (type == DRM_FILE_TYPE_OMA) {
			drm_dcf_header_t header_info;
			memset(&header_info, 0, sizeof(drm_dcf_header_t));
			audio_svc_debug("drm type is OMA");

			if (drm_svc_get_dcf_header_info(item->pathname, &header_info) != DRM_RESULT_SUCCESS) {
				audio_svc_debug("cannot get dcf header info. just get the title");		
				title = _audio_svc_get_title_from_filepath(item->pathname);
				if (title) {
					_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
					SAFE_FREE(title);
				} else {
					audio_svc_error("Can't extract title from filepath");
					return AUDIO_SVC_ERROR_INTERNAL;
				}

				_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
				_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
				_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
				_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
				_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
				
				return AUDIO_SVC_ERROR_NONE;
			}

			if (drm_svc_has_valid_ro(item->pathname, DRM_PERMISSION_PLAY) != DRM_RESULT_SUCCESS) {
				audio_svc_debug("no valid ro. can't extract meta data");
				invalid_file = TRUE;
			}

			if (header_info.version == DRM_OMA_DRMV1_RIGHTS) {
				audio_svc_debug("DRM V1");
				if (invalid_file) {

					if (strlen(header_info.headerUnion.headerV1.contentName) > 0) {
						_strncpy_safe(item->audio.title, header_info.headerUnion.headerV1.contentName, sizeof(item->audio.title));
						extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_TITLE;
						audio_svc_debug("extract title from DCF");
					}

					if (strlen(header_info.headerUnion.headerV1.contentDescription) > 0) {
						_strncpy_safe(item->audio.description, header_info.headerUnion.headerV1.contentDescription, sizeof(item->audio.description));
						extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_DESC;
						audio_svc_debug("extract description from DCF");
					}
				}
			} else if (header_info.version == DRM_OMA_DRMV2_RIGHTS) {
				drm_user_data_common_t metadata;
				int type_index = -1;

				audio_svc_debug("DRM V2");
				
				if (drm_svc_get_user_data_box_info(item->audio.title, DRM_UDTA_TITLE, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.title, metadata.subBox.title.str, sizeof(item->audio.title));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_TITLE;
					audio_svc_debug("extract title from odf");
				}

				if (drm_svc_get_user_data_box_info(item->audio.description, DRM_UDTA_DESCRIPTION, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.description, metadata.subBox.desc.str, sizeof(item->audio.description));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_DESC;
				}

				if (drm_svc_get_user_data_box_info(item->audio.copyright, DRM_UDTA_COPYRIGHT, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.copyright, metadata.subBox.copyright.str, sizeof(item->audio.copyright));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_COPYRIGHT;
				}

				if (drm_svc_get_user_data_box_info(item->audio.author, DRM_UDTA_AUTHOR, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.author, metadata.subBox.author.str, sizeof(item->audio.author));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_AUTHOR;
				}

				if (drm_svc_get_user_data_box_info(item->audio.artist, DRM_UDTA_PERFORMER, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.artist, metadata.subBox.performer.str, sizeof(item->audio.artist));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_ARTIST;
				}

				if (drm_svc_get_user_data_box_info(item->audio.genre, DRM_UDTA_GENRE, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.genre, metadata.subBox.genre.str, sizeof(item->audio.genre));
					audio_svc_debug("genre : %s", item->audio.genre);
					if ((strcasecmp("Ringtone", metadata.subBox.genre.str) == 0) | (strcasecmp("Alert tone", metadata.subBox.genre.str) == 0)) {
						item->category = AUDIO_SVC_CATEGORY_SOUND;
					}
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_GENRE;
				}

				if (drm_svc_get_user_data_box_info(item->audio.album, DRM_UDTA_ALBUM, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.album, metadata.subBox.album.albumTitle, sizeof(item->audio.album));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_ALBUM;
					item->audio.track =  (int)metadata.subBox.album.trackNum;
				}

				if (drm_svc_get_user_data_box_info(item->audio.year, DRM_UDTA_RECODINGYEAR, &metadata) == DRM_RESULT_SUCCESS) {
					_strncpy_safe(item->audio.year, _year_2_str(metadata.subBox.recodingYear.recodingYear), sizeof(item->audio.year));
					extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_YEAR;
				}

				if (drm_svc_get_index_of_relative_contents(item->thumbname, DRM_CONTENTS_INDEX_ALBUMJACKET, &type_index) == DRM_RESULT_SUCCESS) {
					char thumb_path[AUDIO_SVC_PATHNAME_SIZE+1] = {0};

					if (drm_svc_make_multipart_drm_full_path(item->pathname, type_index, AUDIO_SVC_PATHNAME_SIZE, thumb_path) == DRM_TRUE) {

						DRM_FILE_HANDLE hFile = DRM_HANDLE_NULL;

						audio_svc_debug("drm image path : %s", thumb_path);

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
							audio_svc_debug("drm thumb size : %d", thumb_size);
							if (thumb_size > 0) {
								unsigned int readSize = 0;

								thumb_buffer = malloc(thumb_size);
								if (drm_svc_read_file(hFile, thumb_buffer,thumb_size, &readSize) != DRM_RESULT_SUCCESS) {
									SAFE_FREE(thumb_buffer);
									goto DRM_SEEK_ERROR;
								}

								__save_thumbnail(thumb_buffer, readSize, 1, item);
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
				audio_svc_debug("unsupported drm format");
				drm_svc_free_dcf_header_info(&header_info);
				title = _audio_svc_get_title_from_filepath(item->pathname);
				if (title) {
					_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
					SAFE_FREE(title);
				} else {
					audio_svc_error("Can't extract title from filepath");
					return AUDIO_SVC_ERROR_INTERNAL;
				}

				_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
				_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
				_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
				_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
				_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
				
				return AUDIO_SVC_ERROR_NONE;
			}

			if (invalid_file == TRUE) {
				if (!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_TITLE)) {
					title = _audio_svc_get_title_from_filepath(item->pathname);
					if (title) {
						_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
						extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_TITLE;
						SAFE_FREE(title);
					} else {
						audio_svc_error("Can't extract title from filepath");
						drm_svc_free_dcf_header_info(&header_info);
						return AUDIO_SVC_ERROR_INTERNAL;
					}

					_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
					_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
					_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
					_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
					_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
				}
				
				drm_svc_free_dcf_header_info(&header_info);
				return AUDIO_SVC_ERROR_NONE;
			}
		} else if (type == DRM_FILE_TYPE_PLAYREADY) {
			audio_svc_debug("drm type is PLAYREADY");
			if (drm_svc_has_valid_ro(item->pathname, DRM_PERMISSION_PLAY) != DRM_RESULT_SUCCESS) {
				audio_svc_debug("no valid ro. can't extract meta data");
				title = _audio_svc_get_title_from_filepath(item->pathname);
				if (title) {
					_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
					SAFE_FREE(title);
				} else {
					audio_svc_error("Can't extract title from filepath");
					return AUDIO_SVC_ERROR_INTERNAL;
				}

				_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
				_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
				_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
				_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
				_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
				
				return AUDIO_SVC_ERROR_NONE;
			}
		} else {
			audio_svc_error("Not supported DRM type");
			title = _audio_svc_get_title_from_filepath(item->pathname);
			if (title) {
				_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
				SAFE_FREE(title);
			} else {
				audio_svc_error("Can't extract title from filepath");
				return AUDIO_SVC_ERROR_INTERNAL;
			}

			_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
			_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
			_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
			_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
			_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
			
			return AUDIO_SVC_ERROR_NONE;
		}
	}

	mmf_error = mm_file_create_content_attrs(&content, item->pathname);

	if (mmf_error == 0) {
		int sample_rate = 0;
		int channels = 0;

		mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_BITRATE, &item->audio.bitrate, NULL);
		if (mmf_error != 0) {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("fail to get audio bitrate attr - err(%x)", mmf_error);
		} else {
			audio_svc_debug("bit rate : %d", item->audio.bitrate);
		}
		mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_DURATION, &item->audio.duration, NULL);
		if (mmf_error != 0) {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("fail to get duration attr - err(%x)", mmf_error);
		} else {
			audio_svc_debug("duration : %d", item->audio.duration);
		}
		mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_SAMPLERATE, &sample_rate, NULL);
		if (mmf_error != 0) {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("fail to get sample rate attr - err(%x)", mmf_error);
		} else {
			audio_svc_debug("sample rate : %d", sample_rate);
		}
		mmf_error = mm_file_get_attrs(content, &err_attr_name, MM_FILE_CONTENT_AUDIO_CHANNELS, &channels, NULL);
		if (mmf_error != 0) {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("fail to get audio channels attr - err(%x)", mmf_error);
		} else {
			audio_svc_debug("channel : %d", channels);
		}

		/*
		 *	Set format string
		 *		example : 3GPP stereo HE-AAC 48kbps 44.1kHz
		 *		example : 128kbps 44.1kHz 2 Channels
		 *		example : 128kpbs 44.1kHz 2ch
		 */

		if (item->audio.bitrate > 0 && sample_rate > 0) {
			snprintf(item->audio.format, sizeof(item->audio.format) - 1,
					"%dkbps %.1fkHz %dch",
					item->audio.bitrate / 1000, sample_rate / 1000.0, channels);
		} else if (item->audio.bitrate > 0) {
			snprintf(item->audio.format, sizeof(item->audio.format) - 1,
					"%dkbps %dch",
					item->audio.bitrate / 1000, channels);
		} else if (sample_rate > 0) {
			snprintf(item->audio.format, sizeof(item->audio.format) - 1,
					"%.1fkHz %dch",
					sample_rate / 1000.0, channels);
		}

		audio_svc_debug("format : %s", item->audio.format);
		mmf_error = mm_file_destroy_content_attrs(content);
		if (mmf_error != 0) {
			audio_svc_debug("fail to free content attr - err(%x)", mmf_error);
		}
	} else {
		audio_svc_debug("no contents information");
	}

	mmf_error = mm_file_create_tag_attrs(&tag, item->pathname);

	if (mmf_error == 0) {
		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ALBUM, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_ALBUM)) && mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.album, p, sizeof(item->audio.album));
			audio_svc_debug("album[%d] : %s", size, item->audio.album);
		} else {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("album - unknown");
			_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTIST, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_ARTIST)) && mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.artist, p, sizeof(item->audio.artist));
			audio_svc_debug("artist[%d] : %s", size, item->audio.artist);
		} else {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("artist - unknown");
			_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_GENRE, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_GENRE)) && mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.genre, p, sizeof(item->audio.genre));
			audio_svc_debug("genre : %s", item->audio.genre);
			if ((strcasecmp("Ringtone", p) == 0) | (strcasecmp("Alert tone", p) == 0)) {
				item->category = AUDIO_SVC_CATEGORY_SOUND;
			}
		} else {
			SAFE_FREE(err_attr_name);
			audio_svc_debug("genre - unknown");
			_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_TITLE, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_TITLE)) && mmf_error == 0 && size > 0 && 	(!isspace(*p))) {
			_strncpy_safe(item->audio.title, p, sizeof(item->audio.title));
			extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_TITLE;
			audio_svc_debug("extract title from content : %s", item->audio.title);
			audio_svc_debug("^^^^^^^^^^^^^^^ path = %s, title = %s, size = %d ^^^^^^^^^^^^^^", path, item->audio.title, size);
		} else {
			SAFE_FREE(err_attr_name);
			char *title = NULL;		
			title = _audio_svc_get_title_from_filepath(item->pathname);
			if (title) {
				_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
				SAFE_FREE(title);
			} else {
				audio_svc_error("Can't extract title from filepath");
				return AUDIO_SVC_ERROR_INTERNAL;
			}
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_DESCRIPTION, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_DESC)) && mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.description, p, sizeof(item->audio.description));
			audio_svc_debug("desc : %s", item->audio.description);
		} else {
			SAFE_FREE(err_attr_name);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_AUTHOR, &p, &size, NULL);
		if ((!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_AUTHOR)) && mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.author, p, sizeof(item->audio.author));
			extracted_field |= AUDIO_SVC_EXTRACTED_FIELD_AUTHOR;
			audio_svc_debug("extract author from content : %s", item->audio.author);
		} else {
			audio_svc_debug("author - unknown");
			SAFE_FREE(err_attr_name);
			_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_TRACK_NUM, &p, &size, NULL);
		if (mmf_error == 0 && size > 0) {
			item->audio.track = atoi(p);
		} else {
			SAFE_FREE(err_attr_name);
			item->audio.track = -1;
		}
		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_DATE, &p, &size, NULL);
		if (!(extracted_field & AUDIO_SVC_EXTRACTED_FIELD_YEAR)) {
			if (mmf_error == 0 && size > 0) {
				_strncpy_safe(item->audio.year, p, sizeof(item->audio.year));
			} else {
				SAFE_FREE(err_attr_name);
				_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
			}
		} else {
			SAFE_FREE(err_attr_name);
		}

		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_RATING, &p, &size, NULL);
		if (mmf_error == 0 && size > 0) {
			_strncpy_safe(item->audio.parental_rating, p, sizeof(item->audio.parental_rating));
		} else {
			SAFE_FREE(err_attr_name);
		}

		/* extract thumbnail image */
		/*hm2007.kim 100122 - remove thumbnail extract routine while db creating.*/
#if 0
		album_id = _audio_svc_get_album_id(item->audio.album);
		if (album_id < 0) {
			audio_svc_debug("album does not exist. So start to make album art");
			extract_thumbnail = TRUE;
		} else {
			audio_svc_debug("album already exists. don't need to make album art");
			thumbnail_path = _audio_svc_get_thumbnail_path_by_album_id(album_id);
			_strncpy_safe(item->thumbname, thumbnail_path, sizeof(item->thumbname));
			SAFE_FREE(thumbnail_path);
		}
#endif
		if ((!thumb_extracted_from_drm)/* && (extract_thumbnail == TRUE)*/) {
			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK, &image, &size, NULL);
			if (mmf_error != 0) {
				audio_svc_debug("fail to get tag artwork - err(%x)", mmf_error);
				SAFE_FREE(err_attr_name);
			} else {
				audio_svc_debug("artwork size1 [%d]", size);
			}

			mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK_SIZE, &size, NULL);
			if (mmf_error != 0) {
				audio_svc_debug("fail to get artwork size - err(%x)", mmf_error);
				SAFE_FREE(err_attr_name);
			} else {
				audio_svc_debug("artwork size2 [%d]", size);
			}
			if (image != NULL && size > 0) {
				bool ret = FALSE;
				int result = AUDIO_SVC_ERROR_NONE;
				char thumb_path[AUDIO_SVC_PATHNAME_SIZE] = "\0";
				mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_ARTWORK_MIME, &p, &artwork_mime_size, NULL);
				if (mmf_error == 0 && artwork_mime_size > 0) {
					ret = _audio_svc_get_thumbnail_path(storage_type, thumb_path, item->pathname, p);
					if (ret == FALSE) {
						audio_svc_error("fail to get thumb path..");
						mmf_error = mm_file_destroy_tag_attrs(tag);
						if (mmf_error != 0) {
							audio_svc_error("fail to free tag attr - err(%x)", mmf_error);
						}
					return AUDIO_SVC_ERROR_INTERNAL;
					}	
				} else {
					SAFE_FREE(err_attr_name);
				}

				if (!strlen(thumb_path)) {
					audio_svc_error("fail to get thumb path..");
					mmf_error = mm_file_destroy_tag_attrs(tag);
					if (mmf_error != 0) {
						audio_svc_error("fail to free tag attr - err(%x)", mmf_error);
					}
					return AUDIO_SVC_ERROR_INTERNAL;
				}

				result = __save_thumbnail(image, size, thumb_path);
				if (result != AUDIO_SVC_ERROR_NONE) {
					mmf_error = mm_file_destroy_tag_attrs(tag);
					if (mmf_error != 0) {
						audio_svc_error("fail to free tag attr - err(%x)", mmf_error);
					}
					return result;
				}

				_strncpy_safe(item->thumbname, thumb_path, sizeof(item->thumbname));
			}
		}

		mmf_error = mm_file_destroy_tag_attrs(tag);
		if (mmf_error != 0) {
			audio_svc_error("fail to free tag attr - err(%x)", mmf_error);
		}
	} else {
		char *title = NULL;
		audio_svc_error("no tag information");
		
		title = _audio_svc_get_title_from_filepath(item->pathname);
		if (title) {
			_strncpy_safe(item->audio.title, title, sizeof(item->audio.title));
			SAFE_FREE(title);
		} else {
			audio_svc_error("Can't extract title from filepath");
			return AUDIO_SVC_ERROR_INTERNAL;
		}
		
		/* hjkim, 101112, in case of file size 0, MMFW Can't parsting tag info but add it to Music DB. */
		_strncpy_safe(item->audio.album, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.album));
		_strncpy_safe(item->audio.artist, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.artist));
		_strncpy_safe(item->audio.genre, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.genre));
		_strncpy_safe(item->audio.author, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.author));
		_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN, sizeof(item->audio.year));
	}
	
	return AUDIO_SVC_ERROR_NONE;
}

bool _audio_svc_possible_to_extract_title_from_file(const char *path)
{
	MMHandleType tag = 0;
	int err = -1;
	int size = 0;
	char *p = NULL;
	char *err_attr_name = NULL;

	if (drm_svc_is_drm_file(path)) {
		DRM_FILE_TYPE type = drm_svc_get_drm_type(path);

		if (type == DRM_FILE_TYPE_OMA) {
			drm_dcf_header_t header_info;
			bool ret = FALSE;
			memset(&header_info, 0, sizeof(drm_dcf_header_t));

			if (drm_svc_get_dcf_header_info(path, &header_info) == DRM_RESULT_SUCCESS) {
				if (header_info.version == DRM_OMA_DRMV1_RIGHTS) {

					if (drm_svc_has_valid_ro(path, DRM_PERMISSION_ANY) != DRM_RESULT_SUCCESS) {
						if (strlen(header_info.headerUnion.headerV1.contentName) > 0) {
							ret =  TRUE;
						} else {
							ret = FALSE;
						}
					} else {
						ret = FALSE;
					}
				} else if (header_info.version == DRM_OMA_DRMV2_RIGHTS) {

					drm_user_data_common_t metadata;

					if (drm_svc_get_user_data_box_info(path, DRM_UDTA_TITLE, &metadata) == DRM_RESULT_SUCCESS) {
						ret = TRUE;
					} else {
						ret = FALSE;
					}					
				} else {
					audio_svc_debug("unsupported drm format");
					ret = FALSE;					
				}
				drm_svc_free_dcf_header_info(&header_info);
				return ret;
			}
		} else if (type == DRM_FILE_TYPE_PLAYREADY) {
			audio_svc_debug("drm type is PLAYREADY");
			if (drm_svc_has_valid_ro(path, DRM_PERMISSION_PLAY) != DRM_RESULT_SUCCESS) {
				audio_svc_debug("no valid ro. can't extract meta data");
			}
		} else {
			audio_svc_error("Not supported DRM type");
			return FALSE;
		}
	}

	err = mm_file_create_tag_attrs(&tag, path);

	if (err == 0) {
		err = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_TITLE, &p, &size, NULL);
		if (err == 0 && size > 0) {
			mm_file_destroy_tag_attrs(tag);
			return TRUE;
		} else {
			SAFE_FREE(err_attr_name);
			mm_file_destroy_tag_attrs(tag);
		}
	}

	return FALSE;
}

char *_audio_svc_get_title_from_filepath (const char *path)
{
	char *filename = NULL;
	char *title = NULL;
	char	*ext = NULL;
	int filename_len = -1;
	int new_title_len = -1;

	audio_svc_debug("title tag doesn't exist, so get from file path");
	
	if (!path) {
		audio_svc_error("path is NULL");
		return NULL;
	}
	
	filename = g_path_get_basename(path);
	if ((filename == NULL) || (strlen(filename) < 1)) {
		audio_svc_error("wrong file name");
		SAFE_FREE(filename);
		return NULL;
	}
	
	filename_len = strlen(filename);

	ext = g_strrstr(filename, ".");
	if (!ext) {
		audio_svc_error("there is no file extention");
		return filename;
	}

	new_title_len = filename_len - strlen(ext);
	if (new_title_len < 1) {
		audio_svc_error("title length is zero");
		SAFE_FREE(filename);
		return NULL;
	}
	
	title = g_strndup(filename, new_title_len < AUDIO_SVC_PATHNAME_SIZE ? new_title_len : AUDIO_SVC_PATHNAME_SIZE-1);

	SAFE_FREE(filename);

	audio_svc_debug("extract title is [%s]", title);
	
	return title;
}

void _audio_svc_get_parental_rating(const char *path, char *parental_rating)
{
	MMHandleType tag = 0;
	char *p = NULL;
	int size = 0;
	int mmf_error = -1;
	char *err_attr_name = NULL;

	if (mm_file_create_tag_attrs(&tag, path) == 0) {
		mmf_error = mm_file_get_attrs(tag, &err_attr_name, MM_FILE_TAG_RATING, &p, &size, NULL);
		if (mmf_error == 0 && size > 1) {
			_strncpy_safe(parental_rating, p, AUDIO_SVC_PARENTAL_RATING_LEN);
			audio_svc_debug("parental rating : %s, %d", parental_rating, size);
		} else {
			SAFE_FREE(err_attr_name);
		}

		mm_file_destroy_tag_attrs(tag);
	}

}

int _audio_svc_remove_all_files_in_dir(const char *dir_path)
{
	struct dirent *entry = NULL;
	struct stat st;
	char filename[AUDIO_SVC_PATHNAME_SIZE] = {0};
	DIR *dir = NULL;

	dir = opendir(dir_path);
	if (dir == NULL) {
		audio_svc_error("%s is not exist", dir_path);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
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
			audio_svc_error("failed to remove : %s", filename);
			closedir(dir);
			return AUDIO_SVC_ERROR_INTERNAL;
		}
	}

	closedir(dir);
	return AUDIO_SVC_ERROR_NONE;
}

bool _audio_svc_get_thumbnail_path(audio_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format)
{
	char savename[AUDIO_SVC_PATHNAME_SIZE] = {0};
	char file_ext[AUDIO_SVC_FILE_EXT_LEN_MAX + 1] = {0};
	char *thumb_dir = NULL;
	char hash[255 + 1];
	char *thumbfile_ext = NULL;
	
	thumb_dir = (storage_type == AUDIO_SVC_STORAGE_PHONE) ? AUDIO_SVC_THUMB_PHONE_PATH : AUDIO_SVC_THUMB_MMC_PATH;

	memset(file_ext, 0, sizeof(file_ext));
	if (!__audio_svc_get_file_ext(pathname, file_ext)) {
		audio_svc_error("get file ext fail");
		return FALSE;
	}

	int err = -1;
	err = mb_svc_generate_hash_code(pathname, hash, sizeof(hash));
	if (err < 0) {
		audio_svc_error("mb_svc_generate_hash_code failed : %d", err);
		return FALSE;
	}
	
	audio_svc_debug("img format is [%s]", img_format);

	if((strstr(img_format, "jpeg") != NULL) || (strstr(img_format, "jpg") != NULL)) {
		thumbfile_ext = "jpg";
	} else if(strstr(img_format, "png") != NULL) {
		thumbfile_ext = "png";
	} else if(strstr(img_format, "gif") != NULL) {
		thumbfile_ext = "gif";
	} else if(strstr(img_format, "bmp") != NULL) {
		thumbfile_ext = "bmp";
	} else {
		audio_svc_error("Not proper img format");
		return FALSE;
	}
	
	snprintf(savename, sizeof(savename), "%s/.%s-%s.%s", thumb_dir, file_ext, hash, thumbfile_ext);
	_strncpy_safe(thumb_path, savename, AUDIO_SVC_PATHNAME_SIZE);
	audio_svc_debug("thumb_path is [%s]", thumb_path);
	
	return TRUE;
}

int _audio_svc_get_drm_mime_type(const char *path, char *mime_type)
{
	drm_content_info_t contentInfo;

	if (path == NULL) {
		audio_svc_error("path is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (drm_svc_get_content_info(path, &contentInfo) == DRM_RESULT_SUCCESS) {
		if (strlen(contentInfo.contentType) == 0) {
			audio_svc_error("contentType is NULL");
			return AUDIO_SVC_ERROR_INVALID_MEDIA;
		} else {
			snprintf(mime_type, sizeof(contentInfo.contentType), "%s", (char *)contentInfo.contentType);
		}
	} else {
		audio_svc_error("Error in drm_service");
		return AUDIO_SVC_ERROR_INVALID_MEDIA;
	}
	
	return AUDIO_SVC_ERROR_NONE;
}

char *_year_2_str(int year)
{

	static char ret[AUDIO_SVC_METADATA_LEN_MAX];
	if (year == -1 || year == 0) {
		_strncpy_safe(ret, AUDIO_SVC_TAG_UNKNOWN, AUDIO_SVC_METADATA_LEN_MAX);
	} else {
		snprintf(ret, AUDIO_SVC_METADATA_LEN_MAX - 1, "%d", year);
	}
	return ret;
}

void _strncpy_safe(char *x_dst, const char *x_src, int max_len)
{
	if (!x_src || strlen(x_src) == 0) {
		audio_svc_error("x_src is NULL");
		return;
	}

	if (max_len < 1) {
		audio_svc_error("length is Wrong");
		return;
	}
	
    strncpy(x_dst, x_src, max_len-1);
	x_dst[max_len-1] = '\0';
}

void _strlcat_safe(char *x_dst, char *x_src, int max_len)
{
	if (!x_src || strlen(x_src) == 0) {
		audio_svc_error("x_src is NULL");
		return;
	}

	if (max_len < 1) {
		audio_svc_error("length is Wrong");
		return;
	}
	
    g_strlcat(x_dst, x_src, max_len-1);
	x_dst[max_len-1] = '\0';
}

bool _audio_svc_copy_file(const char *srcPath, const char *destPath)
{
	FILE *fs = NULL;
	FILE *fd = NULL;
	char buff[4096] = {0};
	int n = 0;
	int m = 0;
	bool result = false;
	bool remove_dest = false;

	fs = fopen(srcPath, "rb");
	if (fs == NULL) {
		audio_svc_error("failed to open source file: %s", srcPath);
		return false;
	}


	fd = fopen(destPath, "wb");
	if (fd == NULL) {
		audio_svc_error("failed to open dest file: %s", destPath);

		fclose(fs);
		return false;
	}


	while (1) {
		result = feof(fs);
		if (!result) {
			n = fread(buff, sizeof(char), sizeof(buff), fs);
			if (n > 0 && n <= sizeof(buff)) {
				m = fwrite(buff, sizeof(char), n, fd);
				if (m <= 0) {
					audio_svc_debug("fwrite = %d \n", m);

					result = false;
					remove_dest = true;
					goto CATCH;
				}
			} else {
				audio_svc_debug("fread = %d \n", n);
				result = true;
				goto CATCH;
			}
		} else {
			result = true;
			goto CATCH;
		}
	}

CATCH:
	fflush(fd);
	fsync(fileno(fd));
	fclose(fd);
	fclose(fs);

	if (remove_dest) {
		remove(destPath);
		sync();
	}

	audio_svc_debug("copying file is successful");
	return result;
}

bool _audio_svc_make_file(const char *path)
{
	FILE *fd = NULL;

	fd = fopen(path, "w");
	if (fd == NULL) {
		audio_svc_error("failed to open file: [%s]", path);
		return FALSE;
	}

	fclose(fd);
	
	return TRUE;
}

bool _audio_svc_remove_file(const char *path)
{
	int result = -1;
	
	result = remove(path);
	if (result == 0) {
		audio_svc_debug("success to remove file");
		return TRUE;
	} else {
		audio_svc_error("fail to remove file result = %d", result);
		return FALSE;
	}
}

bool _audio_svc_make_directory(const char *path)
{
	int result = -1;
	
	/* Returns : 0 if the directory already exists, or was successfully created. Returns -1 if an error occurred, with errno set.  */
	result = g_mkdir_with_parents(path, 0755);	
	if (result == 0) {
		audio_svc_debug("success to make directory");
		return TRUE;
	} else {
		audio_svc_error("fail to make directory result = %d", result);
		return FALSE;
	}
}

unsigned int _audio_svc_print_elapse_time(int start_time, const char *log_msg)
{
	struct timeval	t;
	unsigned int tval = 0;
	gettimeofday(&t, NULL);
	
	tval = t.tv_sec*1000000L + t.tv_usec;

	if (start_time == 0) {
		printf("[%s] start [%u]\n", log_msg, tval);
	} else {
		printf("[%s] elapsed time [%u]\n", log_msg, tval - start_time);
	}

	return tval;
}

int _audio_svc_get_order_field_str(audio_svc_search_order_e order_field,
									char* output_str,
									int len)
{
	if (output_str == NULL) {
		audio_svc_debug("output str is NULL");
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	switch(order_field) {
		case AUDIO_SVC_ORDER_BY_TITLE_DESC:
			_strncpy_safe(output_str, "title COLLATE NOCASE DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_TITLE_ASC:
			_strncpy_safe(output_str, "title COLLATE NOCASE ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ALBUM_DESC:
			_strncpy_safe(output_str, "album COLLATE NOCASE DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ALBUM_ASC:
			_strncpy_safe(output_str, "album COLLATE NOCASE ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ARTIST_DESC:
			_strncpy_safe(output_str, "artist COLLATE NOCASE DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ARTIST_ASC:
			_strncpy_safe(output_str, "artist COLLATE NOCASE ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_GENRE_DESC:
			_strncpy_safe(output_str, "genre COLLATE NOCASE DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_GENRE_ASC:
			_strncpy_safe(output_str, "genre COLLATE NOCASE ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_AUTHOR_DESC:
			_strncpy_safe(output_str, "author COLLATE NOCASE DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_AUTHOR_ASC:
			_strncpy_safe(output_str, "author COLLATE NOCASE ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_PLAY_COUNT_DESC:
			_strncpy_safe(output_str, "played_count DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_PLAY_COUNT_ASC:
			_strncpy_safe(output_str, "played_count ASC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ADDED_TIME_DESC:
			_strncpy_safe(output_str, "added_time DESC", len);
			break;
		case AUDIO_SVC_ORDER_BY_ADDED_TIME_ASC:
			_strncpy_safe(output_str, "added_time ASC", len);
			break;
		default: break;
	}

	return AUDIO_SVC_ERROR_NONE;
}
 
int _audio_svc_get_file_dir_modified_date(const char *full_path)
{
	struct stat statbuf = { 0 };
	int fd = 0;
 
	fd = stat(full_path, &statbuf);
	if (fd == -1) {
		 audio_svc_debug("stat(%s) fails.", full_path);
		 return AUDIO_SVC_ERROR_INTERNAL;
	 }
 
	 return statbuf.st_mtime;
}

