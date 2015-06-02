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

int _media_svc_get_folder_id_by_foldername(sqlite3 *handle, const char *folder_name, char *folder_id)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT folder_uuid FROM %s WHERE path = '%q';", MEDIA_SVC_DB_TABLE_FOLDER, folder_name);

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

int _media_svc_append_folder(sqlite3 *handle, media_svc_storage_type_e storage_type,
				    const char *folder_id, const char *path_name, const char *folder_name, int modified_date, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	/*Update Pinyin If Support Pinyin*/
	char *folder_name_pinyin = NULL;
	if(_media_svc_check_pinyin_support())
		_media_svc_get_pinyin_str(folder_name, &folder_name_pinyin);

	char *sql = sqlite3_mprintf("INSERT INTO %s (folder_uuid, path, name, storage_type, modified_time, name_pinyin) values (%Q, %Q, %Q, '%d', '%d', %Q); ",
					     MEDIA_SVC_DB_TABLE_FOLDER, folder_id, path_name, folder_name, storage_type, modified_date, folder_name_pinyin);
	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("failed to insert folder");
		return MS_MEDIA_ERR_DB_INTERNAL;
	}
	
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
		if (ret != SQLITE_OK) {
			media_svc_error("failed to update folder");
			return MS_MEDIA_ERR_DB_INTERNAL;
		}
	} else {
		_media_svc_sql_query_add(&g_media_svc_move_item_query_list, &sql);
	}

	return ret;
}

int _media_svc_get_and_append_folder_id_by_path(sqlite3 *handle, const char *path, media_svc_storage_type_e storage_type, char *folder_id, uid_t uid)
{
	char *path_name = NULL;
	int ret = MS_MEDIA_ERR_NONE;

	path_name = g_path_get_dirname(path);

	ret = _media_svc_get_folder_id_by_foldername(handle, path_name, folder_id);

	if(ret == MS_MEDIA_ERR_DB_NO_RECORD) {
		char *folder_name = NULL;
		int folder_modified_date = 0;
		char *folder_uuid = _media_info_generate_uuid();
		if(folder_uuid == NULL ) {
			media_svc_error("Invalid UUID");
			SAFE_FREE(path_name);
			return MS_MEDIA_ERR_INTERNAL;
		}

		folder_name = g_path_get_basename(path_name);
		folder_modified_date = _media_svc_get_file_time(path_name);

		ret = _media_svc_append_folder(handle, storage_type, folder_uuid, path_name, folder_name, folder_modified_date, uid);
		SAFE_FREE(folder_name);
		_strncpy_safe(folder_id, folder_uuid, MEDIA_SVC_UUID_SIZE+1);
	}

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
	if (ret != SQLITE_OK) {
		media_svc_error("failed to delete folder item");
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}
