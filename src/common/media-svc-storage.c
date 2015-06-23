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

#include "media-util-err.h"
#include "media-svc-debug.h"
#include "media-svc-env.h"
#include "media-svc-db-utils.h"
#include "media-svc-util.h"
#include "media-svc-storage.h"

int _media_svc_init_storage(sqlite3 *handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;
	sqlite3_stmt *sql_stmt = NULL;
	int storage_cnt = 0;

	/*Add Internal storage*/
	sql = sqlite3_mprintf("SELECT COUNT(*) FROM '%s' WHERE storage_uuid='%s'", MEDIA_SVC_DB_TABLE_STORAGE, MEDIA_SVC_DB_TABLE_MEDIA);
	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	storage_cnt = sqlite3_column_int(sql_stmt, 0);
	SQLITE3_FINALIZE(sql_stmt);

	if(storage_cnt == 0)
	{
		sql = sqlite3_mprintf("INSERT INTO %s (storage_uuid, storage_name, storage_path, storage_type) VALUES ('%s', '%s', '%s', 0);",
			MEDIA_SVC_DB_TABLE_STORAGE, MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_MEDIA, _media_svc_get_path(uid));

		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
		media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	}

	return ret;
}

int _media_svc_append_storage(sqlite3 *handle, const char *storage_id, const char *storage_name, const char *storage_path, const char *storage_account, media_svc_storage_type_e storage_type, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = sqlite3_mprintf("INSERT INTO %s (storage_uuid, storage_name, storage_path, storage_account, storage_type) values (%Q, %Q, %Q, %Q, %d); ",
							 MEDIA_SVC_DB_TABLE_STORAGE, storage_id, storage_name, storage_path, storage_account, storage_type);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_delete_storage(sqlite3 *handle, const char *storage_id, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = sqlite3_mprintf("DELETE FROM '%s' WHERE storage_uuid=%Q", MEDIA_SVC_DB_TABLE_STORAGE, storage_id);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_storage_validity(sqlite3 *handle, const char *storage_id, int validity, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	if (storage_id == NULL) {
		sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE storage_uuid != 'media';", MEDIA_SVC_DB_TABLE_STORAGE, validity);
	} else {
		sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE storage_uuid=%Q;", MEDIA_SVC_DB_TABLE_STORAGE, validity, storage_id);
	}

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}
