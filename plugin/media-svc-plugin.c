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

#include <string.h>
#include <mm_file.h>
#include <media-thumbnail.h>
#include "media-svc.h"
#include "audio-svc.h"
#include "visual-svc.h"

#define MEDIA_SVC_PLUGIN_ERROR_NONE		0
#define MEDIA_SVC_PLUGIN_ERROR			-1
#define MEDIA_SVC_PLUGIN_STORAGE_INTERNAL	0
#define MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL	1

#define STRING_VALID(str)	\
	((str != NULL && strlen(str) > 0) ? TRUE : FALSE)
#define STORAGE_VALID(storage)\
	(((storage == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL) || (storage == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL)) ? TRUE : FALSE)

typedef enum{
	CONTENT_TYPE_SOUND,
	CONTENT_TYPE_MUSIC,
	CONTENT_TYPE_IMAGE,
	CONTENT_TYPE_VIDEO,
	CONTENT_TYPE_MAX,
}media_svc_content_type_e;

typedef enum{
	ERR_FILE_PATH = 1,
	ERR_HANDLE,
	ERR_MIME_TYPE,
	ERR_NOT_MEDIAFILE,
	ERR_STORAGE_TYPE,
	ERR_CHECK_ITEM,
	ERR_MAX,
}media_svc_error_type_e;

#define MS_CATEGORY_UNKNOWN	0x00000000	/**< Default */
#define MS_CATEGORY_ETC		0x00000001	/**< ETC category */
#define MS_CATEGORY_IMAGE		0x00000002	/**< Image category */
#define MS_CATEGORY_VIDEO		0x00000004	/**< Video category */
#define MS_CATEGORY_MUSIC		0x00000008	/**< Music category */
#define MS_CATEGORY_SOUND	0x00000010	/**< Sound category */

#define CONTENT_TYPE_NUM 4
#define MUSIC_MIME_NUM 28
#define SOUND_MIME_NUM 1
#define MIME_TYPE_LENGTH 255
#define MIME_LENGTH 50
#define _3GP_FILE ".3gp"
#define _MP4_FILE ".mp4"


typedef struct {
	char content_type[15];
	int category_by_mime;
} fex_content_table_t;

static const fex_content_table_t content_category[CONTENT_TYPE_NUM] = {
	{"audio", MS_CATEGORY_SOUND},
	{"image", MS_CATEGORY_IMAGE},
	{"video", MS_CATEGORY_VIDEO},
	{"application", MS_CATEGORY_ETC},
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
};

static const char sound_mime_table[SOUND_MIME_NUM][MIME_LENGTH] = {
	"x-smaf",
};

static int __get_content_type_from_mime(const char * path, const char * mimetype, int * category);
static int __get_content_type(const char * file_path, const char * mime_type);
static void __set_error_message(int err_type, char ** err_msg);

static int __get_content_type_from_mime(const char * path, const char * mimetype, int * category)
{
	int i = 0;
	int err = 0;

	*category = MS_CATEGORY_UNKNOWN;

	//MS_DBG("mime type : %s", mimetype);

	/*categorize from mimetype */
	for (i = 0; i < CONTENT_TYPE_NUM; i++) {
		if (strstr(mimetype, content_category[i].content_type) != NULL) {
			*category = (*category | content_category[i].category_by_mime);
			break;
		}
	}

	/*in application type, exitst sound file ex) x-smafs */
	if (*category & MS_CATEGORY_ETC) {
		int prefix_len = strlen(content_category[0].content_type);

		for (i = 0; i < SOUND_MIME_NUM; i++) {
			if (strstr(mimetype + prefix_len, sound_mime_table[i]) != NULL) {
				*category ^= MS_CATEGORY_ETC;
				*category |= MS_CATEGORY_SOUND;
				break;
			}
		}
	}

	/*check music file in soun files. */
	if (*category & MS_CATEGORY_SOUND) {
		int prefix_len = strlen(content_category[0].content_type) + 1;

		//MS_DBG("mime_type : %s", mimetype + prefix_len);

		for (i = 0; i < MUSIC_MIME_NUM; i++) {
			if (strcmp(mimetype + prefix_len, music_mime_table[i]) == 0) {
				*category ^= MS_CATEGORY_SOUND;
				*category |= MS_CATEGORY_MUSIC;
				break;
			}
		}
	} else if (*category & MS_CATEGORY_VIDEO) {
		/*some video files don't have video stream. in this case it is categorize as music. */
		char *ext;
		/*"3gp" and "mp4" must check video stream and then categorize in directly. */
		ext = strrchr(path, '.');
		if (ext != NULL) {
			if ((strncasecmp(ext, _3GP_FILE, 4) == 0) || (strncasecmp(ext, _MP4_FILE, 5) == 0)) {
				int audio = 0;
				int video = 0;

				err = mm_file_get_stream_info(path, &audio, &video);
				if (err == 0) {
					if (audio > 0 && video == 0) {
						*category ^= MS_CATEGORY_VIDEO;
						*category |= MS_CATEGORY_MUSIC;
					}
				}
			}
		}
	}

	//MS_DBG("category_from_ext : %d", *category);

	return err;
}

static int __get_content_type(const char * file_path, const char * mime_type)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;
	int category = 0;

	ret = __get_content_type_from_mime(file_path, mime_type, &category);
	if(ret < 0)
		return ret;

	if (category & MS_CATEGORY_SOUND)		return CONTENT_TYPE_SOUND;
	else if (category & MS_CATEGORY_MUSIC)	return CONTENT_TYPE_MUSIC;
	else if (category & MS_CATEGORY_IMAGE)	return CONTENT_TYPE_IMAGE;
	else if (category & MS_CATEGORY_VIDEO)	return CONTENT_TYPE_VIDEO;
	else	return CONTENT_TYPE_MAX;

}

static void __set_error_message(int err_type, char ** err_msg)
{
	if (err_msg)
		*err_msg = NULL;

	if(err_type == ERR_FILE_PATH)
		*err_msg = strdup("invalid file path");
	if(err_type == ERR_HANDLE)
		*err_msg = strdup("invalid handle");
	else if(err_type == ERR_MIME_TYPE)
		*err_msg = strdup("invalid mime type");
	else if(err_type == ERR_NOT_MEDIAFILE)
		*err_msg = strdup("not media content");
	else if(err_type == ERR_STORAGE_TYPE)
			*err_msg = strdup("invalid storage type");
	else if(err_type == ERR_CHECK_ITEM)
		*err_msg = strdup("item does not exist");
	else if(err_type == MEDIA_INFO_ERROR_DATABASE_CONNECT)
		*err_msg = strdup("DB connect error");
	else if(err_type == MEDIA_INFO_ERROR_DATABASE_DISCONNECT)
		*err_msg = strdup("DB disconnect error");
	else if((err_type == AUDIO_SVC_ERROR_INVALID_PARAMETER) || (err_type == MB_SVC_ERROR_INVALID_PARAMETER))
		*err_msg = strdup("invalid parameter");
	else if((err_type == AUDIO_SVC_ERROR_DB_INTERNAL) ||(err_type == MB_SVC_ERROR_DB_INTERNAL))
		*err_msg = strdup("DB internal error");
	else if((err_type == AUDIO_SVC_ERROR_INTERNAL) || (err_type == MB_SVC_ERROR_INTERNAL))
		*err_msg = strdup("media service internal error");
	else
		*err_msg = strdup("error unknown");

}

int check_item(const char *file_path, const char * mime_type, char ** err_msg)
{
	int content_type = 0;

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(mime_type)) {
		__set_error_message(ERR_MIME_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	content_type = __get_content_type(file_path, mime_type);

	if(content_type == CONTENT_TYPE_MAX) {
		__set_error_message(ERR_NOT_MEDIAFILE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;		//not media file
	}
	else
		return MEDIA_SVC_PLUGIN_ERROR_NONE;	//media file

}

int connect(void ** handle, char ** err_msg)
{
	int ret = media_svc_connect(handle);

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int disconnect(void * handle, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_disconnect(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int check_item_exist(void* handle, const char *file_path, int storage_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_check_item_exist(handle, file_path);
	if(ret == 0)
		return MEDIA_SVC_PLUGIN_ERROR_NONE;	//exist

	ret = minfo_check_item_exist(handle, file_path);
	if(ret == 0)
		return MEDIA_SVC_PLUGIN_ERROR_NONE;	//exist

	__set_error_message(ERR_CHECK_ITEM, err_msg);
	return MEDIA_SVC_PLUGIN_ERROR;		//not exist

}

int insert_item_begin(void * handle, int item_cnt, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_insert_item_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_add_media_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item_end(void * handle, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_insert_item_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_add_media_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item(void * handle, const char *file_path, int storage_type, const char * mime_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(mime_type)) {
		__set_error_message(ERR_MIME_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	media_svc_content_type_e content_type = __get_content_type(file_path, mime_type);

	if(content_type == CONTENT_TYPE_SOUND || content_type == CONTENT_TYPE_MUSIC)	{

		if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL) {

			if(content_type == CONTENT_TYPE_SOUND)
				ret = audio_svc_insert_item(handle, AUDIO_SVC_STORAGE_PHONE, file_path, AUDIO_SVC_CATEGORY_SOUND);
			else
				ret = audio_svc_insert_item(handle, AUDIO_SVC_STORAGE_PHONE, file_path, AUDIO_SVC_CATEGORY_MUSIC);

		} else if (storage_type == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL) {

			if(content_type == CONTENT_TYPE_SOUND)
				ret = audio_svc_insert_item(handle, AUDIO_SVC_STORAGE_MMC, file_path, AUDIO_SVC_CATEGORY_SOUND);
			else
				ret = audio_svc_insert_item(handle, AUDIO_SVC_STORAGE_MMC, file_path, AUDIO_SVC_CATEGORY_MUSIC);

		}

	} else if (content_type == CONTENT_TYPE_IMAGE || content_type == CONTENT_TYPE_VIDEO) {

		if(content_type == CONTENT_TYPE_IMAGE)
			ret = minfo_add_media_batch(handle, file_path, MINFO_ITEM_IMAGE);
		else
			ret = minfo_add_media_batch(handle, file_path, MINFO_ITEM_VIDEO);

	} else {

		__set_error_message(ERR_NOT_MEDIAFILE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;

	}

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item_immediately(void * handle, const char *file_path, int storage_type, const char * mime_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(mime_type)) {
		__set_error_message(ERR_MIME_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	media_svc_content_type_e content_type = __get_content_type(file_path, mime_type);

	if(content_type == CONTENT_TYPE_SOUND || content_type == CONTENT_TYPE_MUSIC)	{

		if(storage_type == 0) {

			if(content_type == CONTENT_TYPE_SOUND)
				ret = audio_svc_insert_item_immediately(handle, AUDIO_SVC_STORAGE_PHONE, file_path, AUDIO_SVC_CATEGORY_SOUND);
			else
				ret = audio_svc_insert_item_immediately(handle, AUDIO_SVC_STORAGE_PHONE, file_path, AUDIO_SVC_CATEGORY_MUSIC);

		} else if (storage_type == 1) {

			if(content_type == CONTENT_TYPE_SOUND)
				ret = audio_svc_insert_item_immediately(handle, AUDIO_SVC_STORAGE_MMC, file_path, AUDIO_SVC_CATEGORY_SOUND);
			else
				ret = audio_svc_insert_item_immediately(handle, AUDIO_SVC_STORAGE_MMC, file_path, AUDIO_SVC_CATEGORY_MUSIC);

		}

	} else if (content_type == CONTENT_TYPE_IMAGE || content_type == CONTENT_TYPE_VIDEO) {

		if(content_type == CONTENT_TYPE_IMAGE)
			ret = minfo_add_media(handle, file_path, MINFO_ITEM_IMAGE);
		else
			ret = minfo_add_media(handle, file_path, MINFO_ITEM_VIDEO);

	} else {

		__set_error_message(ERR_NOT_MEDIAFILE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;

	}

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}


int move_item_begin(void * handle, int item_cnt, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_move_item_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_move_media_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int move_item_end(void * handle, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_move_item_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_move_media_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int move_item(void * handle, const char *src_path, int src_storage_type, const char *dest_path, int dest_storage_type, const char * mime_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if ((!STRING_VALID(src_path)) || (!STRING_VALID(dest_path))) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(mime_type)) {
		__set_error_message(ERR_MIME_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if((!STORAGE_VALID(src_storage_type)) || (!STORAGE_VALID(dest_storage_type))) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	media_svc_content_type_e content_type = __get_content_type(src_path, mime_type);

	if(content_type == CONTENT_TYPE_SOUND || content_type == CONTENT_TYPE_MUSIC)	{

		ret = audio_svc_move_item(handle, src_storage_type, src_path, dest_storage_type, dest_path);

	} else if (content_type == CONTENT_TYPE_IMAGE || content_type == CONTENT_TYPE_VIDEO) {

		if(content_type == CONTENT_TYPE_IMAGE)
			ret = minfo_move_media(handle, src_path, dest_path, MINFO_ITEM_IMAGE);
		else
			ret = minfo_move_media(handle, src_path, dest_path, MINFO_ITEM_VIDEO);

	} else {

		__set_error_message(ERR_NOT_MEDIAFILE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;

	}

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}


int set_all_storage_items_validity(void * handle, int storage_type, int validity, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL) {
		ret = audio_svc_set_db_valid(handle, AUDIO_SVC_STORAGE_PHONE, validity);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

		ret = minfo_set_db_valid(handle, MINFO_PHONE, validity);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

	} else if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL) {

		ret = audio_svc_set_db_valid(handle, AUDIO_SVC_STORAGE_MMC, validity);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

		ret = minfo_set_db_valid(handle, MINFO_MMC, validity);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_item_validity_begin(void * handle, int item_cnt, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_set_item_valid_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_set_item_valid_start(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_item_validity_end(void * handle, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_set_item_valid_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = minfo_set_item_valid_end(handle);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_item_validity(void * handle, const char *file_path, int storage_type, const char * mime_type, int validity, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(mime_type)) {
		__set_error_message(ERR_MIME_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	media_svc_content_type_e content_type = __get_content_type(file_path, mime_type);

	if(content_type == CONTENT_TYPE_SOUND || content_type == CONTENT_TYPE_MUSIC)	{

		ret = audio_svc_set_item_valid(handle, file_path, validity);

	} else if (content_type == CONTENT_TYPE_IMAGE || content_type == CONTENT_TYPE_VIDEO) {

			if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL)
				ret = minfo_set_item_valid(handle, MINFO_PHONE, file_path, validity);
			else if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL)
				ret = minfo_set_item_valid(handle, MINFO_MMC, file_path, validity);

	} else {

		__set_error_message(ERR_NOT_MEDIAFILE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;

	}

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_item(void * handle, const char *file_path, int storage_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = audio_svc_check_item_exist(handle, file_path);
	if(ret == 0) {		/*audio file*/
		ret = audio_svc_delete_item_by_path(handle, file_path);

		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		} else
			return MEDIA_SVC_PLUGIN_ERROR_NONE;
	}

	ret = minfo_check_item_exist(handle, file_path);
	if(ret == 0) {		/*visual file*/
		ret = minfo_delete_media(handle, file_path);

		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}
		else
			return MEDIA_SVC_PLUGIN_ERROR_NONE;
	} 

	__set_error_message(ERR_CHECK_ITEM, err_msg);	//not exist in DB so can't delete item.
	return MEDIA_SVC_PLUGIN_ERROR;
}

int delete_all_items_in_storage(void * handle, int storage_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL) {

		ret = audio_svc_delete_all(handle, AUDIO_SVC_STORAGE_PHONE);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

		ret = minfo_delete_all_media_records(handle, MINFO_PHONE);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

	} else if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL) {

		ret = audio_svc_delete_all(handle,AUDIO_SVC_STORAGE_MMC);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}
		ret = minfo_delete_all_media_records(handle, MINFO_MMC);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_all_invalid_items_in_storage(void * handle, int storage_type, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(!STORAGE_VALID(storage_type)) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_INTERNAL) {

		ret =  audio_svc_delete_invalid_items(handle,AUDIO_SVC_STORAGE_PHONE);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

		ret = minfo_delete_invalid_media_records(handle, MINFO_PHONE);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

	} else if(storage_type == MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL) {

		ret =  audio_svc_delete_invalid_items(handle,AUDIO_SVC_STORAGE_MMC);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}

		ret = minfo_delete_invalid_media_records(handle, MINFO_MMC);
		if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
		}
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_all_items(void * handle, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = delete_all_items_in_storage(handle, MEDIA_SVC_PLUGIN_STORAGE_INTERNAL, err_msg);
	if(ret < 0)
		return MEDIA_SVC_PLUGIN_ERROR;

	ret = delete_all_items_in_storage(handle, MEDIA_SVC_PLUGIN_STORAGE_EXTERNAL, err_msg);
	if(ret < 0)
		return MEDIA_SVC_PLUGIN_ERROR;

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int update_begin(void)
{
	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int update_end(void)
{
	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}
