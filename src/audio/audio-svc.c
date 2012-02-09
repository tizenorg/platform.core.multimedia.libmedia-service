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
#include <stdarg.h>
#include <db-util.h>

#include "media-svc-env.h"
#include "media-svc-util.h"
#include "audio-svc.h"
#include "audio-svc-error.h"
#include "audio-svc-debug.h"
#include "audio-svc-utils.h"
#include "audio-svc-music-table.h"
#include "audio-svc-playlist-table.h"
#include "audio-svc-types-priv.h"
#include "audio-svc-db-utils.h"

#define AUDIO_SVC_DATABASE_NAME	"/opt/dbspace/.media.db"

static __thread int g_audio_svc_item_valid_data_cnt = 1;
static __thread int g_audio_svc_item_valid_cur_data_cnt = 0;

static __thread int g_audio_svc_move_item_data_cnt = 1;
static __thread int g_audio_svc_move_item_cur_data_cnt = 0;

static __thread int g_audio_svc_insert_item_data_cnt = 1;
static __thread int g_audio_svc_insert_item_cur_data_cnt = 0;

#if 0
/**
 *	audio_svc_open:\n
 *	Open audio service library. This is the function that an user who wants to use music-service calls first.
 * 	This function connects with the music database and initialize efreet mime libary.
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-types.h' to know the exact meaning of the error.
 *	@see		audio_svc_close
 *	@pre		None.
 *	@post		call audio_svc_close() to close music database
 *	@remark	The database name is "/opt/dbspace/.music.db".
 * 	@par example
 * 	@code

#include <audio-svc.h>

void open_music_db()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// open music database
	ret = audio_svc_open();
	// open failed
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_open(void);


/**
 *    audio_svc_close:\n
 *	Close audio service library. This is the function need to call before close the application.
 *	This function disconnects with the music database and shutdown the efreet mime libary.
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-types.h' to know the exact meaning of the error.
 *	@see		audio_svc_open
 *	@pre		music database already is opened.
 *	@post 		None
 *	@remark	memory free before you call this function to close database.
 * 	@par example
 * 	@code

#include <audio-svc.h>

void close_music_db()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// close music database
	ret = audio_svc_close();
	// close failed
	if (ret < 0)
	{
		printf( "unable to close music db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_close(void);

int audio_svc_open(void)
{
	int err = -1;
	int tid = -1;

	audio_svc_debug("");

	err = _media_info_init_handle_tbl();
	if (err < 0) {
		audio_svc_debug("Error:_media_info_init_handle_tbl\n");
		return AUDIO_SVC_ERROR_DB_CONNECT;
	}

	tid = _media_info_get_thread_id();
	audio_svc_debug("Current thread id : %d", tid);

	HandleTable *handle_table = NULL;
	handle_table = _media_info_search_handle(tid);

	if (handle_table == NULL) {
		audio_svc_debug("A handle in thread [%d] does not exist. So now trying to make connection");
		int *key_tid = NULL;

		err = _media_info_insert_handle(&key_tid, tid, &handle_table);
		if (err < 0) {
			audio_svc_error("Fail to insert handle");
			if (key_tid)	g_free(key_tid);
			if (handle_table)	g_free(handle_table);
			return AUDIO_SVC_ERROR_DB_CONNECT;
		}

		sqlite3 *handle = NULL;

		if (db_util_open(MEDIA_INFO_DATABASE_NAME, &handle, DB_UTIL_REGISTER_HOOK_METHOD) != SQLITE_OK) {
			audio_svc_error("Unable to open database");
			if (handle) audio_svc_error("[sqlite] %s\n", sqlite3_errmsg(handle));
			if (key_tid)	g_free(key_tid);
			if (handle_table)	g_free(handle_table);
			return AUDIO_SVC_ERROR_DB_CONNECT;
		}

		handle_table->handle = handle;

		/* Register Busy handler */
		err = sqlite3_busy_handler(handle, _audio_svc_sql_busy_handler, NULL);
		if (SQLITE_OK != err) {
			audio_svc_error("Fail to register busy handler\n");	
			if (handle) audio_svc_error("[sqlite] %s\n", sqlite3_errmsg(handle));	

			db_util_close(handle);
			handle = NULL;

			return AUDIO_SVC_ERROR_DB_CONNECT;
		}

	} else {
		audio_svc_debug("A handle in thread [%d] exists. ");
		_media_info_atomic_add_counting(handle_table);
	}

	audio_svc_debug("audio_svc_open succeed");
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_close(void)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int err = -1;
	int tid = -1;
	audio_svc_debug("");

	tid = _media_info_get_thread_id();
	audio_svc_debug("Current thread id : %d", tid);

	HandleTable *handle_table = NULL;
	handle_table = _media_info_search_handle(tid);

	if (handle_table == NULL) {
		audio_svc_error("handle_table is NULL");
		return AUDIO_SVC_ERROR_DB_DISCONNECT;
	} else {
		audio_svc_debug("ref count in thread[%d] is %d", tid, handle_table->ref_cnt);

		if (handle_table->ref_cnt > 1) {
			_media_info_atomic_sub_counting(handle_table);
		} else {
			if (db_util_close(handle_table->handle) != SQLITE_OK) {
				audio_svc_error("error closing database: %s\n", sqlite3_errmsg(handle_table->handle));
				ret = AUDIO_SVC_ERROR_DB_DISCONNECT;
			}

			err = _media_info_remove_handle(tid);
			if (err < 0) {
				audio_svc_error
				    ("Error:_media_info_remove_handle\n");
				return AUDIO_SVC_ERROR_DB_DISCONNECT;
			}

			_media_info_finalize_handle_tbl();
		}
	}

	audio_svc_debug("audio_svc_close succeed");
	return ret;
}
#endif
int audio_svc_create_table(MediaSvcHandle *handle)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = _audio_svc_create_music_table(db_handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_create_playlist_table(db_handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_create_folder_table(db_handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_delete_all(MediaSvcHandle *handle, audio_svc_storage_type_e storage_type)
{
	char * dirpath = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != AUDIO_SVC_STORAGE_PHONE
	    && storage_type != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_error("storage type should be phone or mmc");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = _audio_svc_truncate_music_table(db_handle, storage_type);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	/* 20111110, make each thumbnail */
	/* 20110428, thumbnail path depends on th album ID. So Don't remove all file in special directory. */
	dirpath = (storage_type == AUDIO_SVC_STORAGE_PHONE) ? AUDIO_SVC_THUMB_PHONE_PATH : AUDIO_SVC_THUMB_MMC_PATH;

	/* remove thumbnails */
	audio_svc_debug("dirpath [%s]", dirpath);
	ret = _audio_svc_remove_all_files_in_dir(dirpath);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	/* update folder table */
	ret = _audio_svc_delete_folder(db_handle, storage_type, NULL);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

#if 0
	ret = _audio_svc_check_and_update_albums_table(NULL);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
#endif

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_insert_item_start(MediaSvcHandle *handle, int data_cnt)
{
	audio_svc_debug("Transaction data count : [%d]", data_cnt);	

	if(handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if(data_cnt < 1) {
		audio_svc_error("data_cnt shuld be bigger than 1. data_cnt : [%d]", data_cnt);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	g_audio_svc_insert_item_data_cnt  = data_cnt;
	g_audio_svc_insert_item_cur_data_cnt  = 0;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_insert_item_end(MediaSvcHandle *handle)
{
	audio_svc_debug_func();

	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_audio_svc_insert_item_cur_data_cnt  > 0) {
		
		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_INSERT_ITEM);
	}

	g_audio_svc_insert_item_data_cnt  = 1;
	g_audio_svc_insert_item_cur_data_cnt  = 0;

	return ret;
}

int audio_svc_insert_item(MediaSvcHandle *handle, audio_svc_storage_type_e storage_type,
			  const char *path, audio_svc_category_type_e category)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != AUDIO_SVC_STORAGE_PHONE
	    && storage_type != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_error("storage type is incorrect(%d)", storage_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("path is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("storage[%d], path[%s], category[%d]", storage_type, path, category);

	if ((category != AUDIO_SVC_CATEGORY_MUSIC)
	    && (category != AUDIO_SVC_CATEGORY_SOUND)) {
		audio_svc_error("invalid category condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_audio_item_s item;
	memset(&item, 0, sizeof(audio_svc_audio_item_s));

	item.category = category;
	item.time_added = time(NULL);

	ret = _audio_svc_extract_metadata_audio(storage_type, path, &item);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	if (g_audio_svc_insert_item_data_cnt == 1) {

		return _audio_svc_insert_item_with_data(db_handle, &item, FALSE);

	}
	else if(g_audio_svc_insert_item_cur_data_cnt  < (g_audio_svc_insert_item_data_cnt  - 1)) {

		ret = _audio_svc_insert_item_with_data(db_handle, &item, TRUE);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
		g_audio_svc_insert_item_cur_data_cnt ++;

	}
	else if (g_audio_svc_insert_item_cur_data_cnt  == (g_audio_svc_insert_item_data_cnt  - 1)) {

		ret = _audio_svc_insert_item_with_data(db_handle, &item, TRUE);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_INSERT_ITEM);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
		
		g_audio_svc_insert_item_cur_data_cnt = 0;
		
	}
	else {
		audio_svc_debug("Error in audio_svc_insert_item");
		return AUDIO_SVC_ERROR_INTERNAL;
 	}

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_insert_item_immediately(MediaSvcHandle *handle, audio_svc_storage_type_e storage_type,
			  const char *path, audio_svc_category_type_e category)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != AUDIO_SVC_STORAGE_PHONE
	    && storage_type != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_error("storage type is incorrect(%d)", storage_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("path is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("storage[%d], path[%s], category[%d]", storage_type, path, category);

	if ((category != AUDIO_SVC_CATEGORY_MUSIC)
	    && (category != AUDIO_SVC_CATEGORY_SOUND)) {
		audio_svc_error("invalid category condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_audio_item_s item;
	memset(&item, 0, sizeof(audio_svc_audio_item_s));

	item.category = category;
	item.time_added = time(NULL);

	ret = _audio_svc_extract_metadata_audio(storage_type, path, &item);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return _audio_svc_insert_item_with_data(db_handle, &item, FALSE);
}

int audio_svc_get_item_by_path(MediaSvcHandle *handle, const char *path, AudioHandleType * item_handle)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("file path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) item_handle;
	if (!item) {
		audio_svc_error("item is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(item, 0, sizeof(audio_svc_audio_item_s));

	return _audio_svc_select_music_record_by_path(db_handle, path, item);
}

int audio_svc_get_item_by_audio_id(MediaSvcHandle *handle, const char *audio_id, AudioHandleType *item_handle)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) item_handle;

	if (!item) {
		audio_svc_error("item is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(item, 0, sizeof(audio_svc_audio_item_s));

	return _audio_svc_select_music_record_by_audio_id(db_handle, audio_id, item);
}

int audio_svc_delete_item_by_path(MediaSvcHandle *handle, const char *path)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("file path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_audio_item_s item;
	memset(&item, 0, sizeof(audio_svc_audio_item_s));

	ret = _audio_svc_select_music_record_by_path(db_handle, path, &item);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_delete_music_record_by_audio_id(db_handle, item.audio_uuid);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_delete_playlist_item_records_by_audio_id(db_handle, item.audio_uuid);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_check_and_update_folder_table(db_handle, path);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

#if 0
	ret = _audio_svc_check_and_update_albums_table(item.audio.album);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
#endif
#if 0
	if (strlen(item.thumbname) > 0) {
		ret = _audio_svc_check_and_remove_thumbnail(item.thumbname);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	}
#endif
	if (strlen(item.thumbname) > 0) {
		if (_audio_svc_remove_file(item.thumbname) == FALSE) {
			audio_svc_error("fail to remove thumbnail file.");
			return AUDIO_SVC_ERROR_INTERNAL;
		}
	}
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_move_item_start(MediaSvcHandle *handle, int data_cnt)
{
	audio_svc_debug("Transaction data count : [%d]", data_cnt);

	if(handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if(data_cnt < 1) {
		audio_svc_error("data_cnt shuld be bigger than 1. data_cnt : [%d]", data_cnt);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	g_audio_svc_move_item_data_cnt  = data_cnt;
	g_audio_svc_move_item_cur_data_cnt  = 0;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_move_item_end(MediaSvcHandle *handle)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug_func();

	if (g_audio_svc_move_item_cur_data_cnt  > 0) {

		g_audio_svc_move_item_data_cnt  = 1;
		g_audio_svc_move_item_cur_data_cnt  = 0;

		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_MOVE_ITEM);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	}

	g_audio_svc_move_item_data_cnt  = 1;
	g_audio_svc_move_item_cur_data_cnt  = 0;

	ret = _audio_svc_update_folder_table(db_handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	
	return AUDIO_SVC_ERROR_NONE;
}

//old db was separated into phone and mmc table.
//so when src_storage and dest_storage is not same, then need to remove old one and insert new one 
//since audio_id in mmc table is started with over 5000001 . but don't need to do like this anymore.
int audio_svc_move_item(MediaSvcHandle *handle, audio_svc_storage_type_e src_storage,
			const char *src_path,
			audio_svc_storage_type_e dest_storage,
			const char *dest_path)
{
	char folder_id[AUDIO_SVC_UUID_SIZE+1] = {0,};
	int ret = AUDIO_SVC_ERROR_NONE;

	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(src_path)) {
		audio_svc_error("src_path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(dest_path)) {
		audio_svc_error("dest_path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (src_storage != AUDIO_SVC_STORAGE_PHONE
	    && src_storage != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_error("storage type should be phone or mmc");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (dest_storage != AUDIO_SVC_STORAGE_PHONE
	    && dest_storage != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_error("storage type should be phone or mmc");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("g_audio_svc_move_item_data_cnt =[%d], g_audio_svc_move_item_cur_data_cnt =[%d]", 
				g_audio_svc_move_item_data_cnt , g_audio_svc_move_item_cur_data_cnt );

	if (g_audio_svc_move_item_data_cnt == 1) {
		/* update path and storage type*/
		ret = _audio_svc_update_path_and_storage_in_music_record(db_handle, src_path, dest_path, dest_storage);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		/* get folder_id */
		ret = _audio_svc_get_and_append_folder_id_by_path(db_handle, dest_path, dest_storage, folder_id);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		/* update folder_id */
		ret = _audio_svc_update_folder_id_in_music_record(db_handle, dest_path, folder_id);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		/* remove old folder path */
		ret = _audio_svc_check_and_update_folder_table(db_handle, src_path);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	}
	else if (g_audio_svc_move_item_cur_data_cnt  < (g_audio_svc_move_item_data_cnt  - 1)) {

		ret = _audio_svc_get_and_append_folder_id_by_path(db_handle, dest_path, dest_storage, folder_id);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
		
		ret = _audio_svc_move_item_query_add(db_handle, src_path, dest_path, dest_storage, folder_id);

		g_audio_svc_move_item_cur_data_cnt ++;
	}
	else if (g_audio_svc_move_item_cur_data_cnt  == (g_audio_svc_move_item_data_cnt  - 1)) {

		ret = _audio_svc_get_and_append_folder_id_by_path(db_handle, dest_path, dest_storage, folder_id);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
		
		ret = _audio_svc_move_item_query_add(db_handle, src_path, dest_path, dest_storage, folder_id);

		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_MOVE_ITEM);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		g_audio_svc_move_item_cur_data_cnt = 0;
	}
	else {
		audio_svc_debug("Error in audio_svc_move_item");
		return AUDIO_SVC_ERROR_INTERNAL;
 	}
	
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_refresh_metadata(MediaSvcHandle *handle, const char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	audio_svc_audio_item_s item;
	memset(&item, 0, sizeof(audio_svc_audio_item_s));

	ret = _audio_svc_select_music_record_by_audio_id(db_handle, audio_id, &item);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	if (item.audio.duration > 0) {
		audio_svc_debug("The item has already valid metadata");
		return AUDIO_SVC_ERROR_NONE;
	}

	ret = _audio_svc_extract_metadata_audio(item.storage_type, item.pathname, &item);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return _audio_svc_update_metadata_in_music_record(db_handle, audio_id, &item);
}

int audio_svc_count_group_item(MediaSvcHandle *handle, audio_svc_group_type_e group_type,
			       const char *limit_string1,
			       const char *limit_string2,
			       const char *filter_string,
			       const char *filter_string2, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (group_type < AUDIO_SVC_GROUP_BY_ALBUM
	    || group_type > AUDIO_SVC_GROUP_BY_COMPOSER) {
		audio_svc_error("group type is wrong : %d", group_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((group_type == AUDIO_SVC_GROUP_BY_ARTIST_ALBUM ||
	     group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST ||
	     group_type == AUDIO_SVC_GROUP_BY_GENRE_ALBUM) &&
	    /* && (!limit_string1 || strlen(limit_string1) == 0)) */
	    (!limit_string1)) {
		audio_svc_error("limit string1 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM) &&
	    /* &&    (!limit_string2 || strlen(limit_string2) == 0)) */
	    (!limit_string2)) {
		audio_svc_error("limit_string2 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_count_music_group_records(db_handle, group_type, limit_string1,
						    limit_string2,
						    filter_string,
						    filter_string2, count);
}

int audio_svc_get_group_item(MediaSvcHandle *handle, audio_svc_group_type_e group_type,
			     const char *limit_string1,
			     const char *limit_string2,
			     const char *filter_string,
			     const char *filter_string2, int offset, int rows,
			     AudioHandleType *result_records)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_group_item_s *result_groups =
	    (audio_svc_group_item_s *) result_records;

	if (group_type < AUDIO_SVC_GROUP_BY_ALBUM
	    || group_type > AUDIO_SVC_GROUP_BY_COMPOSER) {
		audio_svc_error("group type is wrong : %d", group_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((group_type == AUDIO_SVC_GROUP_BY_ARTIST_ALBUM ||
	     group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST ||
	     group_type == AUDIO_SVC_GROUP_BY_GENRE_ALBUM ||
	     group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM) &&
	    /* && (!limit_string1 || strlen(limit_string1) == 0)) */
	    (!limit_string1)) {
		audio_svc_error("limit_string1 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM) &&
	    /* &&    (!limit_string2 || strlen(limit_string2) == 0)) */
	    (!limit_string2)) {
		audio_svc_error("limit_string2 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (offset < 0 || rows <= 0) {
		audio_svc_error("offset(%d) or rows value(%d) is wrong", offset,
				rows);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!result_records) {
		audio_svc_error
		    ("The memory for search records should be allocated ");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_get_music_group_records(db_handle, group_type, limit_string1,
						  limit_string2, filter_string,
						  filter_string2, offset, rows,
						  result_groups);

}

int audio_svc_count_list_item(MediaSvcHandle *handle, audio_svc_track_type_e item_type,
			      const char *type_string, const char *type_string2,
			      const char *filter_string,
			      const char *filter_string2, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (item_type < AUDIO_SVC_TRACK_ALL
	    || item_type > AUDIO_SVC_TRACK_BY_PLAYLIST) {
		audio_svc_error("item type is wrong : %d", item_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (((item_type >= AUDIO_SVC_TRACK_BY_ALBUM)
	     && (item_type <= AUDIO_SVC_TRACK_BY_COMPOSER)) && (!type_string)) {
		audio_svc_error("type_string should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (((item_type == AUDIO_SVC_TRACK_BY_ARTIST_GENRE)
	     || (item_type == AUDIO_SVC_TRACK_BY_ARTIST_ALBUM))
	    && (!type_string2)) {
		audio_svc_error("type_string2 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_count_music_track_records(db_handle, item_type, type_string,
						    type_string2, filter_string,
						    filter_string2, count);
}

int audio_svc_get_list_item(MediaSvcHandle *handle, audio_svc_track_type_e item_type,
			    const char *type_string, const char *type_string2,
			    const char *filter_string,
			    const char *filter_string2, int offset, int rows,
			    AudioHandleType *track)
{
	audio_svc_list_item_s *result_track = (audio_svc_list_item_s *) track;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (item_type < AUDIO_SVC_TRACK_ALL
	    || item_type > AUDIO_SVC_TRACK_BY_PLAYLIST) {
		audio_svc_error("track type is wrong : %d", item_type);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (((item_type >= AUDIO_SVC_TRACK_BY_ALBUM)
	     && (item_type <= AUDIO_SVC_TRACK_BY_COMPOSER)) && (!type_string)) {
		audio_svc_error("type_string should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (((item_type == AUDIO_SVC_TRACK_BY_ARTIST_GENRE)
	     || (item_type == AUDIO_SVC_TRACK_BY_ARTIST_ALBUM))
	    && (!type_string2)) {
		audio_svc_error("type_string2 should be entered");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (offset < 0 || rows <= 0) {
		audio_svc_error("offset(%d) or row value(%d) is wrong", offset,
				rows);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!track) {
		audio_svc_error
		    ("The memory for search records should be allocated ");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(result_track, 0, sizeof(audio_svc_list_item_s) * rows);

	return _audio_svc_get_music_track_records(db_handle, item_type, type_string,
						  type_string2, filter_string,
						  filter_string2, offset, rows,
						  result_track);

}

int audio_svc_get_audio_id_by_path(MediaSvcHandle *handle, const char *path, char *audio_id, size_t max_audio_id_length)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("file path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (audio_id == NULL) {
		audio_svc_error("invalid audio_id condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_search_audio_id_by_path(db_handle, path, audio_id);
}

int audio_svc_get_thumbnail_path_by_path(MediaSvcHandle *handle, const char *path, char *thumb_path,
					 size_t max_thumb_path_length)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("file path is null");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_get_thumbnail_path_by_path(db_handle, path, thumb_path);
}

int audio_svc_add_playlist(MediaSvcHandle *handle, const char *playlist_name, int *playlist_id)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(playlist_name)) {
		audio_svc_error("invalid playlist_name");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id == NULL) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_insert_playlist_record(db_handle, playlist_name, playlist_id);
}

int audio_svc_delete_playlist(MediaSvcHandle *handle, int playlist_id)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_delete_playlist_record(db_handle, playlist_id);
}

int audio_svc_update_playlist_name(MediaSvcHandle *handle, int playlist_id,
				   const char *new_playlist_name)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(new_playlist_name)) {
		audio_svc_error("invalid playlist_name");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_update_playlist_record_by_name(db_handle, playlist_id,
							 new_playlist_name);
}

int audio_svc_count_playlist(MediaSvcHandle *handle, const char *filter_string,
			     const char *filter_string2, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_count_playlist_records(db_handle, filter_string, filter_string2,
						 count);
}

int audio_svc_get_playlist(MediaSvcHandle *handle, const char *filter_string,
			   const char *filter_string2, int offset, int rows,
			   AudioHandleType *playlists)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_playlist_s *ret_playlists = (audio_svc_playlist_s *) playlists;
	if (offset < 0 || rows <= 0) {
		audio_svc_error("offset(%d) or row value(%d) is wrong", offset,
				rows);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!playlists) {
		audio_svc_error
		    ("The memory for search records should be allocated ");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_get_playlist_records(db_handle, offset, rows, filter_string,
					       filter_string2, ret_playlists);
}

int audio_svc_count_playlist_item(MediaSvcHandle *handle, int playlist_id, const char *filter_string,
				  const char *filter_string2, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_count_playlist_item_records(db_handle, playlist_id,
						      filter_string,
						      filter_string2, count);
}

int audio_svc_get_playlist_item(MediaSvcHandle *handle, int playlist_id, const char *filter_string,
				const char *filter_string2, int offset,
				int rows, AudioHandleType *playlist_item)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_playlist_item_s *ret_playlist_item = (audio_svc_playlist_item_s *) playlist_item;

	if (offset < 0 || rows <= 0) {
		audio_svc_error("offset(%d) or row value(%d) is wrong", offset,
				rows);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!playlist_item) {
		audio_svc_error
		    ("The memory for search records should be allocated ");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(ret_playlist_item, 0, sizeof(audio_svc_playlist_item_s) * rows);

	return _audio_svc_get_playlist_item_records(db_handle, playlist_id, filter_string,
						    filter_string2, offset,
						    rows, ret_playlist_item);
}

int audio_svc_get_playlist_id_by_playlist_name(MediaSvcHandle *handle, const char *playlist_name,
					       int *playlist_id)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(playlist_name)) {
		audio_svc_error("invalid playlist_name");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id == NULL) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_get_playlist_id_by_name(db_handle, playlist_name, playlist_id);
}

int audio_svc_get_playlist_name_by_playlist_id(MediaSvcHandle *handle, int playlist_id,
					       char *playlist_name,
					       size_t max_playlist_name_length)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_get_playlist_name_by_playlist_id(db_handle, playlist_id,
							   playlist_name);
}

int audio_svc_count_playlist_by_name(MediaSvcHandle *handle, const char *playlist_name, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(playlist_name)) {
		audio_svc_error("invalid playlist_name");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_count_playlist_records_by_name(db_handle, playlist_name, count);
}

int audio_svc_get_unique_playlist_name(MediaSvcHandle *handle, const char *orig_name, char *unique_name,
				       size_t max_unique_name_length)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;

	char playlist_name[AUDIO_SVC_PLAYLIST_NAME_SIZE] = { 0 };
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(orig_name)) {
		audio_svc_error("orig_name is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(playlist_name, sizeof(playlist_name), "%s_001", orig_name);
	ret = audio_svc_count_playlist_by_name(db_handle, playlist_name, &count);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	if (count > 0) {
		int i = 1;
		while (i < 1000) {
			count = -1;
			snprintf(unique_name, AUDIO_SVC_PLAYLIST_NAME_SIZE,
				 "%s_%.3d", orig_name, i + 1);
			ret = audio_svc_count_playlist_by_name(db_handle, unique_name, &count);
			audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

			if (count == 0) {
				return AUDIO_SVC_ERROR_NONE;
			} else {
				i++;
			}
		}

		return AUDIO_SVC_ERROR_MAKE_PLAYLIST_NAME_FAILED;

	} else {
		snprintf(unique_name, AUDIO_SVC_PLAYLIST_NAME_SIZE, "%s_%.3d", orig_name, 1);
		return AUDIO_SVC_ERROR_NONE;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_add_item_to_playlist(MediaSvcHandle *handle, int playlist_id, const char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist idx");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	ret = _audio_svc_insert_playlist_item_record(db_handle, playlist_id, audio_id);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	
	if (playlist_id == AUDIO_SVC_FAVORITE_LIST_ID) {
		ret = _audio_svc_update_favourite_in_music_record(db_handle, audio_id, 1);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);		
	}

	return ret;
}

int audio_svc_check_duplicate_insertion_in_playlist(MediaSvcHandle *handle, int playlist_id,
						    const char *audio_id, int *count)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist idx");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (count == NULL) {
		audio_svc_error("invalid count condition");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_check_duplication_records_in_playlist(db_handle, playlist_id, audio_id, count);
}

int audio_svc_update_playlist_item_play_order(MediaSvcHandle *handle, int playlist_id, int uid,
					      int new_play_order)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (new_play_order < 0) {
		audio_svc_error("invalid play_order idx");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	if (uid <= 0) {
		audio_svc_error("invalid uid");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_update_item_play_order(db_handle, playlist_id, uid, new_play_order);
}

int audio_svc_remove_item_from_playlist_by_uid(MediaSvcHandle *handle, int playlist_id, int uid)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	char audio_id[AUDIO_SVC_UUID_SIZE + 1] = {0, };
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist idx");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (uid <= 0) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id == AUDIO_SVC_FAVORITE_LIST_ID) {

		ret = _audio_svc_get_audio_id_by_uid(db_handle, uid, audio_id);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		ret = _audio_svc_update_favourite_in_music_record(db_handle, audio_id, 0);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	}

	ret = _audio_svc_delete_playlist_item_record_from_playlist_by_uid(db_handle, playlist_id, uid);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return ret;
}

int audio_svc_remove_item_from_playlist_by_audio_id(MediaSvcHandle *handle, int playlist_id,
						    const char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (playlist_id < 0) {
		audio_svc_error("invalid playlist idx");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = _audio_svc_delete_playlist_item_record_from_playlist_by_audio_id(db_handle, playlist_id, audio_id);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	if (playlist_id == AUDIO_SVC_FAVORITE_LIST_ID) {
		ret = _audio_svc_update_favourite_in_music_record(db_handle, audio_id, 0);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
	}

	return ret;
}

int audio_svc_set_db_valid(MediaSvcHandle *handle, audio_svc_storage_type_e storage_type, int valid)
{
	audio_svc_debug("storage:%d", storage_type);
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != AUDIO_SVC_STORAGE_PHONE
	    && storage_type != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_debug("storage type should be phone or mmc");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return _audio_svc_update_valid_of_music_records(db_handle, storage_type, valid);
}

int audio_svc_delete_invalid_items(MediaSvcHandle *handle, audio_svc_storage_type_e storage_type)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != AUDIO_SVC_STORAGE_PHONE 
		&& storage_type != AUDIO_SVC_STORAGE_MMC) {
		audio_svc_debug("storage type should be phone or mmc");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	ret = _audio_svc_delete_invalid_music_records(db_handle, storage_type);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = _audio_svc_update_folder_table(db_handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

#if 0
	ret = _audio_svc_check_and_update_albums_table(NULL);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);
#endif

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_set_item_valid_start(MediaSvcHandle *handle, int data_cnt)
{
	audio_svc_debug("Transaction data count : [%d]", data_cnt);

	if(handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if(data_cnt < 1) {
		audio_svc_error("data_cnt shuld be bigger than 1. data_cnt : [%d]", data_cnt);
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	g_audio_svc_item_valid_data_cnt  = data_cnt;
	g_audio_svc_item_valid_cur_data_cnt  = 0;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_set_item_valid_end(MediaSvcHandle *handle)
{
	audio_svc_debug_func();

	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_audio_svc_item_valid_cur_data_cnt  > 0) {
		
		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_SET_ITEM_VALID);

	}

	g_audio_svc_item_valid_data_cnt  = 1;
	g_audio_svc_item_valid_cur_data_cnt  = 0;

	return ret;
}

int audio_svc_set_item_valid(MediaSvcHandle *handle, const char *path, int valid)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("path=[%s], valid=[%d]", path, valid);

	if (!STRING_VALID(path)) {
		audio_svc_error("path is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
#if 0	//original code
	return _audio_svc_update_valid_in_music_record(path, valid);

#else	//stack up querys and commit it at once when query counts are same as g_audio_svc_item_valid_data_cnt

	audio_svc_debug("g_audio_svc_item_valid_data_cnt =[%d], g_audio_svc_item_valid_cur_data_cnt =[%d]", 
			g_audio_svc_item_valid_data_cnt , g_audio_svc_item_valid_cur_data_cnt );

	if (g_audio_svc_item_valid_data_cnt  == 1) {
		
		return _audio_svc_update_valid_in_music_record(db_handle, path, valid);
		
	} else if (g_audio_svc_item_valid_cur_data_cnt  < (g_audio_svc_item_valid_data_cnt  - 1)) {

		ret = _audio_svc_update_valid_in_music_record_query_add(db_handle, path, valid);

		g_audio_svc_item_valid_cur_data_cnt ++;	
		
	} else if (g_audio_svc_item_valid_cur_data_cnt  == (g_audio_svc_item_valid_data_cnt  - 1)) {
	
		ret = _audio_svc_update_valid_in_music_record_query_add(db_handle, path, valid);

		ret = _audio_svc_list_query_do(db_handle, AUDIO_SVC_QUERY_SET_ITEM_VALID);
		audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

		g_audio_svc_item_valid_cur_data_cnt  = 0;
		
 	} else {
 	
		audio_svc_debug("Error in audio_svc_set_item_valid");
		return AUDIO_SVC_ERROR_INTERNAL;
 	}

	return AUDIO_SVC_ERROR_NONE;
#endif
}

int audio_svc_get_path_by_audio_id(MediaSvcHandle *handle, const char *audio_id, char *path,
				   size_t max_path_length)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	if (!path) {
		audio_svc_error("path must be allocated...");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = _audio_svc_get_path(db_handle, audio_id, path);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_group_item_new(AudioHandleType **record, int count)
{
	if (count < 1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("count is [%d]", count);
	audio_svc_group_item_s *grp_item =
	    (audio_svc_group_item_s *) malloc(count *
					      sizeof(audio_svc_group_item_s));
	if (grp_item == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(grp_item, 0, count * sizeof(audio_svc_group_item_s));

	*record = (AudioHandleType *) grp_item;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_group_item_free(AudioHandleType *record)
{
	audio_svc_group_item_s *item = (audio_svc_group_item_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug_func();
	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_group_item_get_val(AudioHandleType *record, int index,
				 audio_svc_group_item_type_e first_field_name,
				 ...)
{
	audio_svc_group_item_s *item = (audio_svc_group_item_s *) record;
	va_list var_args;
	int ret = AUDIO_SVC_ERROR_NONE;
	int field_name = -1;

	if (!record) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (index < 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_GROUP_ITEM_MAIN_INFO:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].maininfo) == 0) {
					audio_svc_debug("maininfo is NULL");
					*size = 0;
				} else {
					*val = item[index].maininfo;
					*size = strlen(item[index].maininfo);
				}
				break;
			}
		case AUDIO_SVC_GROUP_ITEM_SUB_INFO:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].subinfo) == 0) {
					audio_svc_debug("subinfo is NULL");
					*size = 0;
				} else {
					*val = item[index].subinfo;
					*size = strlen(item[index].subinfo);
				}

				break;
			}
		case AUDIO_SVC_GROUP_ITEM_RATING:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].album_rating;
				break;
			}
		case AUDIO_SVC_GROUP_ITEM_THUMBNAIL_PATH:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].thumbnail_path) == 0) {
					audio_svc_debug("thumb path is NULL");
					*size = 0;
				} else {
					*val = item[index].thumbnail_path;
					*size =
					    strlen(item[index].thumbnail_path);
				}

				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return ret;
}

int audio_svc_group_item_get(AudioHandleType *record, int index, AudioHandleType **item)
{
	audio_svc_group_item_s *item_arr = (audio_svc_group_item_s *) record;

	if (!item_arr) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	*item = (AudioHandleType *) &(item_arr[index]);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_list_item_new(AudioHandleType **record, int count)
{
	if (count < 1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("count is [%d]", count);
	audio_svc_list_item_s *list_item =
	    (audio_svc_list_item_s *) malloc(count *
					     sizeof(audio_svc_list_item_s));
	if (list_item == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(list_item, 0, count * sizeof(audio_svc_list_item_s));

	*record = (AudioHandleType *) list_item;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_list_item_free(AudioHandleType *record)
{
	audio_svc_list_item_s *item = (audio_svc_list_item_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug_func();
	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_list_item_get_val(AudioHandleType *record, int index,
				audio_svc_list_item_type_e first_field_name,
				...)
{
	audio_svc_list_item_s *item = (audio_svc_list_item_s *) record;
	va_list var_args;
	int ret = AUDIO_SVC_ERROR_NONE;
	int field_name = -1;

	if (!record) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (index < 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		switch (field_name) {
		case AUDIO_SVC_LIST_ITEM_AUDIO_ID:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].audio_uuid) == 0) {
					audio_svc_error("audio_id is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*val = item[index].audio_uuid;
					*size = strlen(item[index].audio_uuid);
				}
				break;
			}
		case AUDIO_SVC_LIST_ITEM_PATHNAME:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].pathname) == 0) {
					audio_svc_debug("path is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*val = item[index].pathname;
					*size = strlen(item[index].pathname);
				}

				break;
			}
		case AUDIO_SVC_LIST_ITEM_DURATION:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].duration;
				break;
			}
		case AUDIO_SVC_LIST_ITEM_RATING:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].rating;
				break;
			}
		case AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].thumbnail_path) == 0) {
					audio_svc_debug("thumb path is NULL");
					*size = 0;
				} else {
					*val = item[index].thumbnail_path;
					*size =
					    strlen(item[index].thumbnail_path);
				}

				break;
			}
		case AUDIO_SVC_LIST_ITEM_TITLE:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].title) == 0) {
					audio_svc_debug("title is NULL");
					*size = 0;
				} else {
					*val = item[index].title;
					*size = strlen(item[index].title);
				}
				break;
			}
		case AUDIO_SVC_LIST_ITEM_ARTIST:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].artist) == 0) {
					audio_svc_debug("artist is NULL");
					*size = 0;
				} else {
					*val = item[index].artist;
					*size = strlen(item[index].artist);
				}

				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return ret;
}

int audio_svc_list_item_get(AudioHandleType *record, int index, AudioHandleType **item)
{
	audio_svc_list_item_s *item_arr = (audio_svc_list_item_s *) record;

	if (!item_arr) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	*item = (AudioHandleType *) &(item_arr[index]);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_new(AudioHandleType **record, int count)
{
	if (count < 1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug("");

	audio_svc_playlist_s *playlist =
	    (audio_svc_playlist_s *) malloc(count *
					    sizeof(audio_svc_playlist_s));
	if (playlist == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(playlist, 0, count * sizeof(audio_svc_playlist_s));

	*record = (AudioHandleType *) playlist;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_free(AudioHandleType *record)
{
	audio_svc_playlist_s *item = (audio_svc_playlist_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;

}

int audio_svc_playlist_get_val(AudioHandleType *playlists, int index,
			       audio_svc_playlist_e first_field_name, ...)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	va_list var_args;
	int field_name;
	audio_svc_playlist_s *item = (audio_svc_playlist_s *) playlists;

	if (!playlists) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (index < 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_PLAYLIST_ID:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].playlist_id;
				break;
			}
		case AUDIO_SVC_PLAYLIST_NAME:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].name) == 0) {
					audio_svc_error("name is NULL");
					*size = 0;
				} else {
					*val = item[index].name;
					/* *size = AUDIO_SVC_PLAYLIST_NAME_SIZE; */
					*size = strlen(item[index].name);
				}
				break;
			}
		case AUDIO_SVC_PLAYLIST_THUMBNAIL_PATH:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].thumbnail_path) == 0) {
					audio_svc_error
					    ("thumbnail_path is NULL");
					*size = 0;
				} else {
					*val = item[index].thumbnail_path;
					*size =
					    strlen(item[index].thumbnail_path);
				}
				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_set_val(AudioHandleType *playlists, int index,
			       audio_svc_playlist_e first_field_name, ...)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	va_list var_args;
	int field_name;
	audio_svc_playlist_s *item = (audio_svc_playlist_s *) playlists;

	if (!playlists) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (index < 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_PLAYLIST_NAME:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				_strncpy_safe(item[index].name, val,
					      min(size + 1,
						  sizeof(item[index].name)));
				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_get_item(AudioHandleType *record, int index, AudioHandleType **plst)
{
	if (!record) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_playlist_s *plst_arr = (audio_svc_playlist_s *) record;
	*plst = (AudioHandleType *) &(plst_arr[index]);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_item_new(AudioHandleType **record, int count)
{
	if (count < 1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug("");

	audio_svc_playlist_item_s *plst_item =
	    (audio_svc_playlist_item_s *) malloc(count *
						 sizeof
						 (audio_svc_playlist_item_s));
	if (plst_item == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(plst_item, 0, count * sizeof(audio_svc_playlist_item_s));

	*record = (AudioHandleType *) plst_item;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_item_free(AudioHandleType *record)
{
	audio_svc_playlist_item_s *item = (audio_svc_playlist_item_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug_func();
	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_playlist_item_get_val(AudioHandleType *record, int index,
				    audio_svc_playlist_item_type_e
				    first_field_name, ...)
{
	audio_svc_playlist_item_s *item = (audio_svc_playlist_item_s *) record;
	va_list var_args;
	int ret = AUDIO_SVC_ERROR_NONE;
	int field_name = -1;

	if (!record) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (index < 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_PLAYLIST_ITEM_UID:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].u_id;
				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].audio_uuid) == 0) {
					audio_svc_error("audio_id is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*val = item[index].audio_uuid;
					*size = strlen(item[index].audio_uuid);
				}
				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_PATHNAME:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].pathname) == 0) {
					audio_svc_debug("path is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*val = item[index].pathname;
					*size = strlen(item[index].pathname);
				}

				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_DURATION:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].duration;
				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_RATING:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].rating;
				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_THUMBNAIL_PATH:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].thumbnail_path) == 0) {
					audio_svc_debug("thumb path is NULL");
					*size = 0;
				} else {
					*val = item[index].thumbnail_path;
					*size =
					    strlen(item[index].thumbnail_path);
				}

				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_TITLE:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].title) == 0) {
					audio_svc_debug("title is NULL");
					*size = 0;
				} else {
					*val = item[index].title;
					*size = strlen(item[index].title);
				}
				break;
			}

		case AUDIO_SVC_PLAYLIST_ITEM_ARTIST:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item[index].artist) == 0) {
					audio_svc_debug("artist is NULL");
					*size = 0;
				} else {
					*val = item[index].artist;
					*size = strlen(item[index].artist);
				}

				break;
			}
		case AUDIO_SVC_PLAYLIST_ITEM_PLAY_ORDER:
			{
				int *val = va_arg((var_args), int *);
				*val = item[index].play_order;
				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return ret;
}

int audio_svc_playlist_item_get(AudioHandleType *record, int index, AudioHandleType **item)
{
	audio_svc_playlist_item_s *item_arr = (audio_svc_playlist_item_s *) record;

	if (!item_arr) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	*item = (AudioHandleType *) &(item_arr[index]);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_item_new(AudioHandleType **record)
{
	int count = 1;

	audio_svc_debug("count is [%d]", count);
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) malloc(count * sizeof(audio_svc_audio_item_s));
	if (item == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(item, 0, count * sizeof(audio_svc_audio_item_s));

	*record = (AudioHandleType *) item;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_item_free(AudioHandleType *record)
{
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug_func();
	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_item_get_val(AudioHandleType *record,
			   audio_svc_track_data_type_e first_field_name, ...)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	va_list var_args;
	int field_name;
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) record;

	audio_svc_debug_func();
	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_TRACK_DATA_STORAGE:
			{
				int *val = va_arg((var_args), int *);
				*val = item->storage_type;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_AUDIO_ID:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio_uuid) == 0) {
					audio_svc_error("audio_id is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*size = strlen(item->audio_uuid);
					*val = item->audio_uuid;
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_PLAYED_COUNT:
			{
				int *val = va_arg((var_args), int *);
				*val = item->played_count;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_PLAYED_TIME:
			{
				int *val = va_arg((var_args), int *);
				*val = item->time_played;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ADDED_TIME:
			{
				int *val = va_arg((var_args), int *);
				*val = item->time_added;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_RATING:
			{
				int *val = va_arg((var_args), int *);
				*val = item->rating;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_CATEGORY:
			{
				int *val = va_arg((var_args), int *);
				*val = item->category;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_PATHNAME:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->pathname) == 0) {
					audio_svc_error("pah is NULL");
					*size = 0;
					ret = AUDIO_SVC_ERROR_DB_NO_RECORD;
				} else {
					*size = strlen(item->pathname);
					*val = item->pathname;
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->thumbname) == 0) {
					audio_svc_debug("thumbname is NULL");
					*size = 0;
				} else {
					*size = strlen(item->thumbname);
					*val = item->thumbname;
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_TITLE:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.title) == 0) {
					audio_svc_debug("title is NULL");
					*size = 0;
				} else {
					*val = item->audio.title;
					*size = strlen(item->audio.title);
					audio_svc_debug("title = [%s][%d]",
							*val, *size);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ARTIST:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.artist) == 0) {
					audio_svc_debug("artist is NULL");
					*size = 0;
				} else {
					*val = item->audio.artist;
					*size = strlen(item->audio.artist);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ALBUM:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.album) == 0) {
					audio_svc_debug("album is NULL");
					*size = 0;
				} else {
					*val = item->audio.album;
					*size = strlen(item->audio.album);
					audio_svc_debug("album = %s", *val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_GENRE:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.genre) == 0) {
					audio_svc_debug("genre is NULL");
					*size = 0;
				} else {
					*val = item->audio.genre;
					*size = strlen(item->audio.genre);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_AUTHOR:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.author) == 0) {
					audio_svc_debug("author is NULL");
					*size = 0;
				} else {
					*val = item->audio.author;
					*size = strlen(item->audio.author);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_COPYRIGHT:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.copyright) == 0) {
					audio_svc_debug("copyright is NULL");
					*size = 0;
				} else {
					*val = item->audio.copyright;
					*size = strlen(item->audio.copyright);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_DESCRIPTION:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.description) == 0) {
					audio_svc_debug("description is NULL");
					*size = 0;
				} else {
					*val = item->audio.description;
					*size = strlen(item->audio.description);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_FORMAT:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.format) == 0) {
					audio_svc_debug("format is NULL");
					*size = 0;
				} else {
					*val = item->audio.format;
					*size = strlen(item->audio.format);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_DURATION:
			{
				int *val = va_arg((var_args), int *);
				*val = item->audio.duration;
				break;
			}

		case AUDIO_SVC_TRACK_DATA_BITRATE:
			{
				int *val = va_arg((var_args), int *);
				*val = item->audio.bitrate;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_YEAR:
			{
				char **val = va_arg((var_args), char **);
				int *size = va_arg((var_args), int *);
				if (strlen(item->audio.year) == 0) {
					audio_svc_debug("year is NULL");
					*size = 0;
				} else {
					*val = item->audio.year;
					*size = strlen(item->audio.year);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_TRACK_NUM:
			{
				int *val = va_arg((var_args), int *);
				*val = item->audio.track;
				break;
			}
		case AUDIO_SVC_TRACK_DATA_FAVOURATE:
			{
				int *val = va_arg((var_args), int *);
				*val = item->favourate;
				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error occured");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_search_item_new(AudioHandleType **record, int count)
{
	audio_svc_debug("count is [%d]", count);
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) malloc(count * sizeof(audio_svc_audio_item_s));

	if (item == NULL) {
		return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
	}

	memset(item, 0, count * sizeof(audio_svc_audio_item_s));

	*record = (AudioHandleType *) item;

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_search_item_get(AudioHandleType *record, int index, AudioHandleType **item)
{
	audio_svc_audio_item_s *item_arr = (audio_svc_audio_item_s *) record;

	if (!item_arr) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	*item = (AudioHandleType *) &(item_arr[index]);
	if (*item == NULL) {
		audio_svc_error("Index is invalid");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_search_item_free(AudioHandleType *record)
{
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *) record;

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	audio_svc_debug_func();
	SAFE_FREE(item);

	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_update_item_metadata(MediaSvcHandle *handle, const char *audio_id,
				   audio_svc_track_data_type_e first_field_name,
				   ...)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	va_list var_args;
	int field_name = -1;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(audio_id)) {
		audio_svc_error("invalid audio_id");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}
	
	if (first_field_name == -1) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		switch (field_name) {
		case AUDIO_SVC_TRACK_DATA_PLAYED_COUNT:
			{
				int val = va_arg((var_args), int);
				if (val < 0) {
					audio_svc_error("play count should be positive value");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_playcount_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_PLAYED_TIME:
			{
				int val = va_arg((var_args), int);
				if (val < 0) {
					audio_svc_error("play time should be positive value");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_playtime_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ADDED_TIME:
			{
				int val = va_arg((var_args), int);
				if (val < 0) {
					audio_svc_error("added time should be positive value");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_addtime_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_RATING:
			{
				int val = va_arg((var_args), int);
				if (val < AUDIO_SVC_RATING_NONE
				    || val > AUDIO_SVC_RATING_5) {
					audio_svc_error("rating value should be between 0 and 5");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_rating_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_TITLE:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				/* title can not be NULL by UX */
				if (val == NULL) {
					audio_svc_error("title can not be NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else if (size >
					   AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_title_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ARTIST:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				if (size > AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					if (val == NULL) {
						val = AUDIO_SVC_TAG_UNKNOWN;
					}
					ret =_audio_svc_update_artist_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ALBUM:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				if (size > AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					if (val == NULL) {
						val = AUDIO_SVC_TAG_UNKNOWN;
					}
					ret = _audio_svc_update_album_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_GENRE:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				if (size > AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					if (val == NULL) {
						val = AUDIO_SVC_TAG_UNKNOWN;
					}
					ret = _audio_svc_update_genre_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_AUTHOR:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				if (size > AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					if (val == NULL) {
						val = AUDIO_SVC_TAG_UNKNOWN;
					}
					ret = _audio_svc_update_author_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_DESCRIPTION:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);
				if (size > AUDIO_SVC_METADATA_LEN_MAX - 1) {
					audio_svc_error("text size should be shorter than AUDIO_SVC_METADATA_LEN_MAX");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_description_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_YEAR:
			{
				int val = va_arg((var_args), int);
				if (val < 0) {
					audio_svc_error("year should be positive value");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_year_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_TRACK_NUM:
			{
				int val = va_arg((var_args), int);
				if (val < 0) {
					audio_svc_error("track number should be positive value");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_track_num_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		case AUDIO_SVC_TRACK_DATA_ALBUM_RATING:
			{
				int val = va_arg((var_args), int);
				if (val < AUDIO_SVC_RATING_NONE
				    || val > AUDIO_SVC_RATING_5) {
					audio_svc_error("rating value should be between 0 and 5");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					ret = _audio_svc_update_album_rating_in_music_record(db_handle, audio_id, val);
				}
				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("invalid parameter");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	return AUDIO_SVC_ERROR_NONE;
}

int audio_svc_check_item_exist(MediaSvcHandle *handle, const char *path)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!STRING_VALID(path)) {
		audio_svc_error("path is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (_audio_svc_count_record_with_path(db_handle, path) > 0) {
		audio_svc_debug("item is exist in database");
		return AUDIO_SVC_ERROR_NONE;
	} else {
		audio_svc_debug("item is not exist in database");
		return AUDIO_SVC_ERROR_DB_NO_RECORD;
	}

}

int audio_svc_list_by_search(MediaSvcHandle *handle, AudioHandleType *record,
							audio_svc_search_order_e order_field,
							int offset,
							int count,
							audio_svc_serch_field_e first_field_name,
							...)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	va_list var_args;
	int field_name = -1;
	int len = 0;
	char query_where[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char search_str[AUDIO_SVC_METADATA_LEN_MAX] = { 0 };
	char *condition_str = NULL;
	
	audio_svc_audio_item_s *item = (audio_svc_audio_item_s *)record;
	sqlite3 * db_handle = (sqlite3 *)handle;

	if(db_handle == NULL) {
		audio_svc_error("Handle is NULL");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!item) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (offset < 0 || count <= 0) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (order_field < AUDIO_SVC_ORDER_BY_TITLE_DESC ||
			order_field > AUDIO_SVC_ORDER_BY_ADDED_TIME_ASC) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if (first_field_name < AUDIO_SVC_SEARCH_TITLE || 
			first_field_name > AUDIO_SVC_SEARCH_AUTHOR) {
		audio_svc_error("Invalid arguments");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(query_where, 0x00, sizeof(query_where));
	snprintf(query_where, sizeof(query_where), "valid=1 and ( 0 ");

	field_name = first_field_name;
	va_start(var_args, first_field_name);

	while (field_name >= 0) {
		audio_svc_debug("field name = %d", field_name);
		memset(search_str, 0x00, sizeof(search_str));

		switch (field_name) {
		case AUDIO_SVC_SEARCH_TITLE:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				if (val == NULL) {
					audio_svc_error("title is NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					_strncpy_safe(search_str, val,
						      min(size + 1,
							  sizeof(search_str)));
				}

				condition_str = sqlite3_mprintf(" or title like '%%%q%%' ", search_str);

				len =
					g_strlcat(query_where, condition_str,
						sizeof(query_where));
				if (len >= sizeof(query_where)) {
					sqlite3_free(condition_str);
					audio_svc_error("strlcat returns failure ( %d )", len);
					return AUDIO_SVC_ERROR_INTERNAL;
				}
				sqlite3_free(condition_str);

				break;
			}
		case AUDIO_SVC_SEARCH_ARTIST:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				if (val == NULL) {
					audio_svc_error("artist is NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					_strncpy_safe(search_str, val,
						      min(size + 1,
							  sizeof(search_str)));
				}

				condition_str = sqlite3_mprintf(" or artist like '%%%q%%' ", search_str);

				len =
					g_strlcat(query_where, condition_str,
						sizeof(query_where));
				if (len >= sizeof(query_where)) {
					sqlite3_free(condition_str);
					audio_svc_error("strlcat returns failure ( %d )", len);
					return AUDIO_SVC_ERROR_INTERNAL;
				}
				sqlite3_free(condition_str);

				break;
			}
		case AUDIO_SVC_SEARCH_ALBUM:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				if (val == NULL) {
					audio_svc_error("album is NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					_strncpy_safe(search_str, val,
						      min(size + 1,
							  sizeof(search_str)));
				}

				condition_str = sqlite3_mprintf(" or album like '%%%q%%' ", search_str);

				len =
					g_strlcat(query_where, condition_str,
						sizeof(query_where));
				if (len >= sizeof(query_where)) {
					sqlite3_free(condition_str);
					audio_svc_error("strlcat returns failure ( %d )", len);
					return AUDIO_SVC_ERROR_INTERNAL;
				}
				sqlite3_free(condition_str);

				break;
			}
		case AUDIO_SVC_SEARCH_GENRE:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				if (val == NULL) {
					audio_svc_error("genre is NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					_strncpy_safe(search_str, val,
						      min(size + 1,
							  sizeof(search_str)));
				}

				condition_str = sqlite3_mprintf(" or genre like '%%%q%%' ", search_str);

				len =
					g_strlcat(query_where, condition_str,
						sizeof(query_where));
				if (len >= sizeof(query_where)) {
					audio_svc_error("strlcat returns failure ( %d )", len);
					return AUDIO_SVC_ERROR_INTERNAL;
				}

				break;
			}
		case AUDIO_SVC_SEARCH_AUTHOR:
			{
				char *val = va_arg((var_args), char *);
				int size = va_arg((var_args), int);

				if (val == NULL) {
					audio_svc_error("author is NULL");
					ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				} else {
					_strncpy_safe(search_str, val,
						      min(size + 1,
							  sizeof(search_str)));
				}

				condition_str = sqlite3_mprintf(" or author like '%%%q%%' ", search_str);

				len =
					g_strlcat(query_where, condition_str,
						sizeof(query_where));
				if (len >= sizeof(query_where)) {
					sqlite3_free(condition_str);
					audio_svc_error("strlcat returns failure ( %d )", len);
					return AUDIO_SVC_ERROR_INTERNAL;
				}
				sqlite3_free(condition_str);

				break;
			}
		default:
			{
				audio_svc_error("Invalid arguments");
				ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
				break;
			}
		}

		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("invalid parameter");
			va_end(var_args);
			return ret;
		}
		/* next field */
		field_name = va_arg(var_args, int);
	}

	va_end(var_args);
	len = g_strlcat(query_where, ") ", sizeof(query_where));
	if (len >= sizeof(query_where)) {
		audio_svc_error("strlcat returns failure ( %d )", len);
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	return _audio_svc_list_search(db_handle, item, query_where, order_field, offset, count);
}

