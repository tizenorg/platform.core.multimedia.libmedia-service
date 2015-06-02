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
#include <drm_client.h>
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
int _media_svc_save_image(void *image, int size, char *image_path, uid_t uid);
bool _media_svc_get_thumbnail_path(media_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format, uid_t uid);
bool _media_svc_get_file_ext(const char *file_path, char *file_ext);
int _media_svc_get_file_time(const char *full_path);
int _media_svc_set_media_info(media_svc_content_info_s *content_info, media_svc_storage_type_e storage_type,
			  const char *path, media_svc_media_type_e *media_type, bool refresh, drm_content_info_s **drm_contentInfo);
int _media_svc_extract_image_metadata(media_svc_content_info_s *content_info, media_svc_media_type_e media_type);
int _media_svc_extract_media_metadata(sqlite3 *handle, media_svc_content_info_s *content_info, media_svc_media_type_e media_type, drm_content_info_s *drm_contentInfo, uid_t uid);
int __media_svc_malloc_and_strncpy(char **dst, const char *src);
void _media_svc_destroy_content_info(media_svc_content_info_s *content_info);
int _media_svc_get_store_type_by_path(const char *path, media_svc_storage_type_e *storage_type);
char *_media_svc_replace_path(char *s, const char *olds, const char *news);
char* _media_svc_get_thumb_internal_path(uid_t uid);
char* _media_svc_get_thumb_external_path(uid_t uid);
int _media_svc_error_convert(int error);

/* Define data structures for media type and mime type */
#define MEDIA_SVC_CATEGORY_UNKNOWN	0x00000000	/**< Default */
#define MEDIA_SVC_CATEGORY_ETC		0x00000001	/**< ETC category */
#define MEDIA_SVC_CATEGORY_IMAGE	0x00000002	/**< Image category */
#define MEDIA_SVC_CATEGORY_VIDEO	0x00000004	/**< Video category */
#define MEDIA_SVC_CATEGORY_MUSIC	0x00000008	/**< Music category */
#define MEDIA_SVC_CATEGORY_SOUND	0x00000010	/**< Sound category */

#define CONTENT_TYPE_NUM 4
#define MUSIC_MIME_NUM 29
#define SOUND_MIME_NUM 1
#define MIME_TYPE_LENGTH 255
#define MIME_LENGTH 50
#define _3GP_FILE ".3gp"
#define _MP4_FILE ".mp4"

typedef struct {
	char content_type[15];
	int category_by_mime;
} _media_svc_content_table_s;

static const _media_svc_content_table_s content_category[CONTENT_TYPE_NUM] = {
	{"audio", MEDIA_SVC_CATEGORY_SOUND},
	{"image", MEDIA_SVC_CATEGORY_IMAGE},
	{"video", MEDIA_SVC_CATEGORY_VIDEO},
	{"application", MEDIA_SVC_CATEGORY_ETC},
};

static const char music_mime_table[MUSIC_MIME_NUM][MIME_LENGTH] = {
	/*known mime types of normal files*/
	"mpeg",
	"ogg",
	"x-ms-wma",
	"x-flac",
	"mp4",
	/* known mime types of drm files*/
	"mp3",
	"x-mp3", /*alias of audio/mpeg*/
	"x-mpeg", /*alias of audio/mpeg*/
	"3gpp",
	"x-ogg", /*alias of  audio/ogg*/
	"vnd.ms-playready.media.pya:*.pya", /*playready*/
	"wma",
	"aac",
	"x-m4a", /*alias of audio/mp4*/
	/* below mimes are rare*/
	"x-vorbis+ogg",
	"x-flac+ogg",
	"x-matroska",
	"ac3",
	"mp2",
	"x-ape",
	"x-ms-asx",
	"vnd.rn-realaudio",

	"x-vorbis", /*alias of audio/x-vorbis+ogg*/
	"vorbis", /*alias of audio/x-vorbis+ogg*/
	"x-oggflac",
	"x-mp2", /*alias of audio/mp2*/
	"x-pn-realaudio", /*alias of audio/vnd.rn-realaudio*/
	"vnd.m-realaudio", /*alias of audio/vnd.rn-realaudio*/
	"x-wav",
};

static const char sound_mime_table[SOUND_MIME_NUM][MIME_LENGTH] = {
	"x-smaf",
};

bool _media_svc_is_drm_file(const char *path);
int _media_svc_get_mime_in_drm_info(const char *path, char *mime, drm_content_info_s **drm_contentInfo);
int _media_svc_get_content_type_from_mime(const char * path, const char * mimetype, int * category);
int _media_svc_get_pinyin_str(const char *src_str, char **pinyin_str);
bool _media_svc_check_pinyin_support(void);
int _media_svc_get_mime_type(const char *path, char *mimetype, drm_bool_type_e *is_drm, drm_content_info_s **drm_contentInfo);

int _media_svc_get_media_type(const char *path, const char *mime_type, media_svc_media_type_e *media_type);

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_UTIL_H_*/
