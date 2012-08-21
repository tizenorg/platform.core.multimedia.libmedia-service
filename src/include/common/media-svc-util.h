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
#include "media-svc-types.h"
#include "media-svc-env.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#define SAFE_FREE(src)      { if(src) {free(src); src = NULL;}}
#define STRING_VALID(str)	\
	((str != NULL && strlen(str) > 0) ? TRUE : FALSE)

char *_media_info_generate_uuid(void);
char *_media_svc_escape_str(char *input, int len);
void _strncpy_safe(char *x_dst, const char *x_src, int max_len);
unsigned int _media_svc_get_current_time(void);
int _media_svc_rename_file( const char *old_name, const char *new_name);
bool _media_svc_remove_file(const char *path);
int _media_svc_remove_all_files_in_dir(const char *dir_path);
char *_media_svc_get_title_from_filepath (const char *path);
int _media_svc_save_image(void *image, int size, char *image_path);
bool _media_svc_get_thumbnail_path(media_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format);
bool _media_svc_get_file_ext(const char *file_path, char *file_ext);
int _media_svc_get_file_time(const char *full_path);
int _media_svc_set_media_info(media_svc_content_info_s *content_info, media_svc_storage_type_e storage_type, const char *path, const char *mime_type, media_svc_media_type_e media_type, bool refresh);
int _media_svc_extract_image_metadata(media_svc_content_info_s *content_info, media_svc_media_type_e media_type);
int _media_svc_extract_media_metadata(sqlite3 *handle, media_svc_content_info_s *content_info, media_svc_media_type_e media_type);
int __media_svc_malloc_and_strncpy(char **dst, const char *src);
void _media_svc_destroy_content_info(media_svc_content_info_s *content_info);
int _media_svc_get_store_type_by_path(const char *path, media_svc_storage_type_e *storage_type);
char *_media_svc_replace_path(char *s, const char *olds, const char *news);

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_UTIL_H_*/
