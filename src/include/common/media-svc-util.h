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



#ifndef _MEDIA_SVC_UTIL_H_
#define _MEDIA_SVC_UTIL_H_

#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <iniparser.h>
#include "media-svc-types.h"
#include "media-svc-env.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#define SAFE_FREE(src)	{ if (src) {free(src); src = NULL;}}
#define G_SAFE_FREE(src)	{ if (src) {g_free(src); src = NULL;}}
#define STRING_VALID(str)	\
	((str != NULL && strlen(str) > 0) ? TRUE : FALSE)

char *_media_info_generate_uuid(void);
void _strncpy_safe(char *x_dst, const char *x_src, int max_len);
int _media_svc_rename_file(const char *old_name, const char *new_name);
int _media_svc_remove_file(const char *path);
int _media_svc_remove_all_files_in_dir(const char *dir_path);
int _media_svc_get_thumbnail_path(media_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format, uid_t uid);
int _media_svc_get_file_time(const char *full_path);
int _media_svc_set_default_value(media_svc_content_info_s *content_info, bool refresh);
int _media_svc_set_media_info(media_svc_content_info_s *content_info, const char *storage_id, media_svc_storage_type_e storage_type,
		const char *path, media_svc_media_type_e *media_type, bool refresh);
int _media_svc_extract_image_metadata(sqlite3 *handle, media_svc_content_info_s *content_info);
int _media_svc_extract_media_metadata(sqlite3 *handle, media_svc_content_info_s *content_info, uid_t uid);
int __media_svc_malloc_and_strncpy(char **dst, const char *src);
time_t __media_svc_get_timeline_from_str(const char *timstr);
void _media_svc_destroy_content_info(media_svc_content_info_s *content_info);
int _media_svc_get_storage_type_by_path(const char *path, media_svc_storage_type_e *storage_type, uid_t uid);
char *_media_svc_replace_path(char *s, const char *olds, const char *news);
char *_media_svc_get_thumb_internal_path(uid_t uid);
char *_media_svc_get_thumb_external_path(uid_t uid);
bool _media_svc_is_drm_file(const char *path);
int _media_svc_request_thumbnail_with_origin_size(const char *path, char *thumb_path, int max_length, int *origin_width, int *origin_height, uid_t uid);
int _media_svc_get_pinyin_str(const char *src_str, char **pinyin_str);
bool _media_svc_check_pinyin_support(void);
int _media_svc_extract_music_metadata_for_update(sqlite3 *handle, media_svc_content_info_s *content_info, media_svc_media_type_e media_type);
int _media_svc_get_ini_value();
char *_media_svc_get_title_from_path(const char *path);
void _media_svc_print_stderror(void);

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_UTIL_H_*/
