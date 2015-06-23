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

#include <glib/gstdio.h>
#include <media-util-err.h>
#include "media-svc-media-folder.h"
#include "media-svc-debug.h"
#include "media-svc-env.h"
#include "media-svc-util.h"
#include "media-svc-db-utils.h"

extern __thread GList *g_media_svc_move_item_query_list;

int _media_svc_get_folder_id_by_foldername(sqlite3 *handle, const char *storage_id, const char *folder_name, char *folder_id)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = NULL;

	if(STRING_VALID(storage_id))
		sql = sqlite3_mprintf("SELECT folder_uuid FROM '%s' WHERE storage_uuid = '%q' AND path = '%q';", MEDIA_SVC_DB_TABLE_FOLDER, storage_id, folder_name);
	else
		sql = sqlite3_mprintf("SELECT folder_uuid FROM '%s' WHERE path = '%q';", MEDIA_SVC_DB_TABLE_FOLDER, folder_name);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		if(ret == MS_MEDIA_ERR_DB_NO_RECORD) {
			media_svc_debug("there is no folder.");
		}
		else {
			media_svc_error("error when _media_svc_get_folder_id_by_foldername. err = [%d]", ret);
		}
		return ret;
	}

	_strncpy_safe(folder_id, (const char *)sqlite3_column_text(sql_stmt, 0), MEDIA_SVC_UUID_SIZE+1);

	SQLITE3_FINALIZE(sql_stmt);

	return ret;
}

int _media_svc_append_folder(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type,
				    const char *folder_id, const char *path_name, const char *folder_name, int modified_date, const char *parent_folder_uuid, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	/*Update Pinyin If Support Pinyin*/
	char *folder_name_pinyin = NULL;
	if(_media_svc_check_pinyin_support())
		_media_svc_get_pinyin_str(folder_name, &folder_name_pinyin);

	char *sql = sqlite3_mprintf("INSERT INTO %s (folder_uuid, path, name, storage_uuid, storage_type, modified_time, name_pinyin, parent_folder_uuid) \
							values (%Q, %Q, %Q, %Q, '%d', '%d', %Q, %Q); ",
						     MEDIA_SVC_DB_TABLE_FOLDER, folder_id, path_name, folder_name, storage_id, storage_type, modified_date, folder_name_pinyin, parent_folder_uuid);
	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	SAFE_FREE(folder_name_pinyin);

	return ret;
}

int _media_svc_update_folder_modified_time_by_folder_uuid(sqlite3 *handle, const char *folder_uuid, const char *folder_path, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	int modified_time = 0;

	modified_time = _media_svc_get_file_time(folder_path);

	char *sql = sqlite3_mprintf("UPDATE %s SET modified_time=%d WHERE folder_uuid=%Q;", MEDIA_SVC_DB_TABLE_FOLDER, modified_time, folder_uuid);

	if(!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
	} else {
		_media_svc_sql_query_add(&g_media_svc_move_item_query_list, &sql);
	}

	return ret;
}

int __media_svc_get_and_append_parent_folder(sqlite3 *handle, const char *storage_id, const char *path, media_svc_storage_type_e storage_type, char *folder_id, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	unsigned int next_pos;
	char *next = NULL;
	char *dir_path = NULL;
	char *token = "/";
	char *folder_uuid = NULL;
	char *folder_name = NULL;
	int folder_modified_date = 0;
	char parent_folder_uuid[MEDIA_SVC_UUID_SIZE+1] = {0,};
	bool folder_search_end = FALSE;

	memset(parent_folder_uuid, 0, sizeof(parent_folder_uuid));

	if(strncmp(path, _media_svc_get_path(uid), strlen(_media_svc_get_path(uid))) == 0)
		next_pos = strlen(_media_svc_get_path(uid));
	else if (strncmp(path, MEDIA_ROOT_PATH_SDCARD, strlen(MEDIA_ROOT_PATH_SDCARD)) == 0)
		next_pos = strlen(MEDIA_ROOT_PATH_SDCARD);
	else
	{
		media_svc_error("Invalid Path");
		return MS_MEDIA_ERR_INTERNAL;
	}

	while (!folder_search_end)
	{
		next = strstr(path + next_pos , token);
		if (next != NULL)
		{
			next_pos = (next - path);
			dir_path = strndup(path, next_pos);
			next_pos++;
		}
		else
		{
			media_svc_error("End Path");
			dir_path = strndup(path, strlen(path));
			folder_search_end = TRUE;
		}

		ret = _media_svc_get_folder_id_by_foldername(handle, storage_id, dir_path, parent_folder_uuid);
		if (ret == MS_MEDIA_ERR_DB_NO_RECORD)
		{
			media_svc_error("NOT EXIST dir path : %s", dir_path);

			folder_uuid = _media_info_generate_uuid();
			if(folder_uuid == NULL )
			{
			media_svc_error("Invalid UUID");
				SAFE_FREE(dir_path);
			return MS_MEDIA_ERR_INTERNAL;
		}

			folder_name = g_path_get_basename(dir_path);
			folder_modified_date = _media_svc_get_file_time(dir_path);

			ret = _media_svc_append_folder(handle, storage_id, storage_type, folder_uuid, dir_path, folder_name, folder_modified_date, parent_folder_uuid, uid);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("_media_svc_append_folder is failed");
		}

			_strncpy_safe(parent_folder_uuid, folder_uuid, MEDIA_SVC_UUID_SIZE+1);

		SAFE_FREE(folder_name);
		}
		else
		{
			media_svc_error("EXIST dir path : %s\n", dir_path);
		}

		SAFE_FREE(dir_path);
	}

		_strncpy_safe(folder_id, folder_uuid, MEDIA_SVC_UUID_SIZE+1);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_and_append_folder(sqlite3 *handle, const char *storage_id, const char *path, media_svc_storage_type_e storage_type, char *folder_id, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	ret = _media_svc_get_folder_id_by_foldername(handle, storage_id, path, folder_id);

	if(ret == MS_MEDIA_ERR_DB_NO_RECORD)
	{
		ret = __media_svc_get_and_append_parent_folder(handle, storage_id, path, storage_type, folder_id, uid);
	}

	return ret;
}

int _media_svc_get_and_append_folder_id_by_path(sqlite3 *handle, const char *storage_id, const char *path, media_svc_storage_type_e storage_type, char *folder_id, uid_t uid)
{
	char *path_name = NULL;
	int ret = MS_MEDIA_ERR_NONE;

	path_name = g_path_get_dirname(path);

	ret =  _media_svc_get_and_append_folder(handle, storage_id, path_name, storage_type, folder_id, uid);

	SAFE_FREE(path_name);

	return ret;
}

int _media_svc_update_folder_table(sqlite3 *handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	sql = sqlite3_mprintf("DELETE FROM %s WHERE folder_uuid IN (SELECT folder_uuid FROM %s WHERE folder_uuid NOT IN (SELECT folder_uuid FROM %s))",
	     MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_TABLE_MEDIA);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

static int __media_svc_count_all_folders(sqlite3 *handle, char* start_path, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT count(*) FROM %s WHERE path LIKE '%q%%'", MEDIA_SVC_DB_TABLE_FOLDER, start_path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_sql_prepare_to_step. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_all_folders(sqlite3 *handle, char *start_path, char ***folder_list, time_t **modified_time_list, int **item_num_list, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	int idx = 0;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = NULL;
	int cnt =0;
	char **folder_uuid = NULL;
	int i =0;

	ret  = __media_svc_count_all_folders(handle, start_path, &cnt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when __media_svc_count_all_folders. err = [%d]", ret);
		return ret;
	}

	if (cnt > 0) {
		sql = sqlite3_mprintf("SELECT path, modified_time, folder_uuid FROM %s WHERE path LIKE '%q%%'", MEDIA_SVC_DB_TABLE_FOLDER, start_path);
	} else {
		*folder_list = NULL;
		*modified_time_list = NULL;
		*item_num_list = NULL;
		return MS_MEDIA_ERR_NONE;
	}

	*folder_list = malloc(sizeof(char *) * cnt);
	*modified_time_list = malloc(sizeof(int) * cnt);
	*item_num_list = malloc(sizeof(int) * cnt);
	folder_uuid = malloc(sizeof(char *) * cnt);

	if((*folder_list == NULL) || (*modified_time_list == NULL) || (*item_num_list == NULL) ||(folder_uuid == NULL)) {
		media_svc_error("Out of memory");
		goto ERROR;
	}
	memset(folder_uuid, 0x0, sizeof(char *) * cnt);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		goto ERROR;
	}

	media_svc_debug("QEURY OK");

	while (1) {
		if(STRING_VALID((char *)sqlite3_column_text(sql_stmt, 0)))
			(*folder_list)[idx] = strdup((char *)sqlite3_column_text(sql_stmt, 0));

		(*modified_time_list)[idx] = (int)sqlite3_column_int(sql_stmt, 1);

		/* get the folder's id */
		if(STRING_VALID((char *)sqlite3_column_text(sql_stmt, 2)))
			folder_uuid[idx] = strdup((char *)sqlite3_column_text(sql_stmt, 2));

		idx++;

		if(sqlite3_step(sql_stmt) != SQLITE_ROW)
			break;
	}
	SQLITE3_FINALIZE(sql_stmt);

	/*get the numbder of item in the folder by using folder's id */
	for (i = 0; i < idx; i ++) {
		if(STRING_VALID(folder_uuid[i])) {
			sql = sqlite3_mprintf("SELECT COUNT(*) FROM %s WHERE (folder_uuid='%q' AND validity = 1)", MEDIA_SVC_DB_TABLE_MEDIA, folder_uuid[i]);
			ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
			if (ret != MS_MEDIA_ERR_NONE) {
				media_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
				goto ERROR;
			}

			(*item_num_list)[i] = (int)sqlite3_column_int(sql_stmt, 0);

			SQLITE3_FINALIZE(sql_stmt);
		} else
		{
			media_svc_error("Invalid Folder Id");
		}
	}

	if (cnt == idx) {
		*count = cnt;
		media_svc_debug("Get Folder is OK");
	} else {
		media_svc_error("Fail to get folder");
		ret = MS_MEDIA_ERR_INTERNAL;
		goto ERROR;
	}

	/* free all data */
	for (i  = 0; i < idx; i ++) {
		SAFE_FREE(folder_uuid[i]);
	}
	SAFE_FREE(folder_uuid);

	return ret;

ERROR:

	/* free all data */
	for (i  = 0; i < idx; i ++) {
		SAFE_FREE((*folder_list)[i]);
		SAFE_FREE(folder_uuid[i]);
	}
	SAFE_FREE(*folder_list);
	SAFE_FREE(*modified_time_list);
	SAFE_FREE(*item_num_list);
	SAFE_FREE(folder_uuid);

	*count = 0;

	return ret;
}

int _media_svc_get_folder_info_by_foldername(sqlite3 *handle, const char *folder_name, char *folder_id, time_t *modified_time)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT folder_uuid, modified_time FROM %s WHERE path = '%q';", MEDIA_SVC_DB_TABLE_FOLDER, folder_name);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		if(ret == MS_MEDIA_ERR_DB_NO_RECORD) {
			media_svc_debug("there is no folder.");
		}
		else {
			media_svc_error("error when _media_svc_get_folder_id_by_foldername. err = [%d]", ret);
		}
		return ret;
	}

	_strncpy_safe(folder_id, (const char *)sqlite3_column_text(sql_stmt, 0), MEDIA_SVC_UUID_SIZE+1);
	*modified_time = (int)sqlite3_column_int(sql_stmt, 1);

	SQLITE3_FINALIZE(sql_stmt);

	return ret;
}
