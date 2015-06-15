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
#include "media-svc-util.h"

#define MEDIA_SVC_PLUGIN_ERROR_NONE		0
#define MEDIA_SVC_PLUGIN_ERROR			-1

#define STRING_VALID(str)	\
	((str != NULL && strlen(str) > 0) ? TRUE : FALSE)
#define STORAGE_VALID(storage)\
	(((storage == MEDIA_SVC_STORAGE_INTERNAL) || (storage == MEDIA_SVC_STORAGE_EXTERNAL)) ? TRUE : FALSE)


typedef enum{
	ERR_HANDLE = 1,
	ERR_FILE_PATH,
	ERR_FOLDER_PATH,
	ERR_MIME_TYPE,
	ERR_NOT_MEDIAFILE,
	ERR_STORAGE_TYPE,
	ERR_CHECK_ITEM,
	ERR_MAX,
}media_svc_error_type_e;

static void __set_error_message(int err_type, char ** err_msg);

static void __set_error_message(int err_type, char ** err_msg)
{
	if (err_msg)
		*err_msg = NULL;
	else
		return;

	if(err_type == ERR_HANDLE)
		*err_msg = strdup("invalid handle");
	else if(err_type == ERR_FILE_PATH)
		*err_msg = strdup("invalid file path");
	else if(err_type == ERR_FOLDER_PATH)
		*err_msg = strdup("invalid folder path");
	else if(err_type == ERR_MIME_TYPE)
		*err_msg = strdup("invalid mime type");
	else if(err_type == ERR_NOT_MEDIAFILE)
		*err_msg = strdup("not media content");
	else if(err_type == ERR_STORAGE_TYPE)
		*err_msg = strdup("invalid storage type");
	else if(err_type == ERR_CHECK_ITEM)
		*err_msg = strdup("item does not exist");
	else if(err_type == MS_MEDIA_ERR_DB_CONNECT_FAIL)
		*err_msg = strdup("DB connect error");
	else if(err_type == MS_MEDIA_ERR_DB_DISCONNECT_FAIL)
		*err_msg = strdup("DB disconnect error");
	else if(err_type == MS_MEDIA_ERR_INVALID_PARAMETER)
		*err_msg = strdup("invalid parameter");
	else if(err_type == MS_MEDIA_ERR_DB_INTERNAL)
		*err_msg = strdup("DB internal error");
	else if(err_type == MS_MEDIA_ERR_DB_NO_RECORD)
		*err_msg = strdup("not found in DB");
	else if(err_type == MS_MEDIA_ERR_INTERNAL)
		*err_msg = strdup("media service internal error");
	else if(err_type == MS_MEDIA_ERR_DB_CORRUPT)
		*err_msg = strdup("DB corrupt error");
	else
		*err_msg = strdup("error unknown");

	return;
}

int check_item(const char *file_path, char ** err_msg)
{
	if (!STRING_VALID(file_path)) {
		__set_error_message(ERR_FILE_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int connect_db(void ** handle, uid_t uid, char ** err_msg)
{
	int ret = media_svc_connect(handle,uid);

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int disconnect_db(void * handle, char ** err_msg)
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

	ret = media_svc_check_item_exist_by_path(handle, file_path);
	if(ret == MS_MEDIA_ERR_NONE) {
		return MEDIA_SVC_PLUGIN_ERROR_NONE;	//exist
	}

	__set_error_message(ERR_CHECK_ITEM, err_msg);

	return MEDIA_SVC_PLUGIN_ERROR;		//not exist
}

int insert_item_begin(void * handle, int item_cnt, int with_noti, int from_pid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_insert_item_begin(handle, item_cnt, with_noti, from_pid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item_end(void * handle, uid_t uid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_insert_item_end(handle, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item(void * handle, const char *file_path, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_insert_item_bulk(handle, storage_type, file_path, FALSE, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_item_immediately(void * handle, const char *file_path, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_insert_item_immediately(handle, storage_type, file_path, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int insert_burst_item(void * handle, const char *file_path, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_insert_item_bulk(handle, storage_type, file_path, TRUE, uid);
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

	ret = media_svc_move_item_begin(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int move_item_end(void * handle, uid_t uid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_move_item_end(handle, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int move_item(void * handle, const char *src_path, int src_storage_type, const char *dest_path, int dest_storage_type, uid_t uid, char ** err_msg)
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

	if((!STORAGE_VALID(src_storage_type)) || (!STORAGE_VALID(dest_storage_type))) {
		__set_error_message(ERR_STORAGE_TYPE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_move_item(handle, src_storage_type, src_path, dest_storage_type, dest_path, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_all_storage_items_validity(void * handle, int storage_type, int validity, uid_t uid, char ** err_msg)
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

	ret = media_svc_set_all_storage_items_validity(handle, storage_type, validity, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_folder_item_validity(void * handle, const char * folder_path, int validity, int recursive, uid_t uid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(folder_path)) {
		__set_error_message(ERR_FOLDER_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_set_folder_items_validity(handle, folder_path, validity, recursive, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
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

	ret = media_svc_set_item_validity_begin(handle, item_cnt);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_item_validity_end(void * handle, uid_t uid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_set_item_validity_end(handle, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int set_item_validity(void * handle, const char *file_path, int storage_type, int validity, uid_t uid, char ** err_msg)
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

	ret = media_svc_set_item_validity(handle, file_path, validity, uid);

	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_item(void * handle, const char *file_path, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_check_item_exist_by_path(handle, file_path);
	if(ret == 0) {
		ret = media_svc_delete_item_by_path(handle, "media", file_path, uid);

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

int delete_all_items_in_storage(void * handle, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_delete_all_items_in_storage(handle, storage_type, uid);
	if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_all_invalid_items_in_storage(void * handle, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_delete_invalid_items_in_storage(handle, storage_type, uid);
	if(ret < 0) {
			__set_error_message(ret, err_msg);
			return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_all_invalid_items_in_folder(void * handle, const char *folder_path, uid_t uid, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(folder_path)) {
		__set_error_message(ERR_FOLDER_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_delete_invalid_items_in_folder(handle, folder_path, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int delete_all_items(void * handle, uid_t uid ,char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = delete_all_items_in_storage(handle, MEDIA_SVC_STORAGE_INTERNAL, uid, err_msg);
	if(ret < 0)
		return MEDIA_SVC_PLUGIN_ERROR;

	ret = delete_all_items_in_storage(handle, MEDIA_SVC_STORAGE_EXTERNAL, uid, err_msg);
	if(ret < 0)
		return MEDIA_SVC_PLUGIN_ERROR;

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int refresh_item(void * handle, const char *file_path, int storage_type, uid_t uid, char ** err_msg)
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

	ret = media_svc_refresh_item(handle, storage_type, "media", file_path, uid);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int update_begin(void)
{
	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int update_end(uid_t uid)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	ret = thumbnail_request_extract_all_thumbs(uid);
	if(ret < 0) {
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int send_dir_update_noti(void * handle, const char *dir_path, char **err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if (!STRING_VALID(dir_path)) {
		__set_error_message(ERR_FOLDER_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_send_dir_update_noti(handle, dir_path);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}

int count_delete_items_in_folder(void * handle, const char *folder_path, int *count, char ** err_msg)
{
	int ret = MEDIA_SVC_PLUGIN_ERROR_NONE;

	if(handle == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if(count == NULL) {
		__set_error_message(ERR_HANDLE, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	if (!STRING_VALID(folder_path)) {
		__set_error_message(ERR_FOLDER_PATH, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	ret = media_svc_count_invalid_items_in_folder(handle, folder_path, count);
	if(ret < 0) {
		__set_error_message(ret, err_msg);
		return MEDIA_SVC_PLUGIN_ERROR;
	}

	return MEDIA_SVC_PLUGIN_ERROR_NONE;
}
