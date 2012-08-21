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
*	common API
*/

#include <sqlite3.h>
#include "media-svc-util.h"
#include "visual-svc-util.h"
#include "visual-svc-db-util.h"
#include "visual-svc-debug.h"
#include "visual-svc-error.h"

int mb_svc_query_sql_gstring(MediaSvcHandle *mb_svc_handle, GString *query_string)
{
	int err = -1;
	char *err_msg;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug("SQL = %s\n", query_string->str);

	err = sqlite3_exec(handle, query_string->str, NULL, NULL, &err_msg);
	if (SQLITE_OK != err) {
		if (err_msg) {
			mb_svc_debug("failed to query[%s]", err_msg);
			sqlite3_free(err_msg);
		}
		mb_svc_debug("Query fails : query_string[%s]",
			     query_string->str);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (err_msg)
		sqlite3_free(err_msg);
	mb_svc_debug("query success\n");

	return err;
}

int mb_svc_query_sql(MediaSvcHandle *mb_svc_handle, char *query_str)
{
	int err = -1;
	char *err_msg;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug("SQL = %s\n", query_str);

	err = sqlite3_exec(handle, query_str, NULL, NULL, &err_msg);
	if (SQLITE_OK != err) {
		if (err_msg) {
			mb_svc_debug("failed to query[%s]", err_msg);
			sqlite3_free(err_msg);
		}
		mb_svc_debug("Query fails : query_string[%s]", query_str);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (err_msg)
		sqlite3_free(err_msg);
	mb_svc_debug("query success\n");

	return err;
}

int mb_svc_sqlite3_begin_trans(MediaSvcHandle *mb_svc_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug("mb_svc_sqlite3_begin_trans enter\n");
	if (SQLITE_OK !=
	    sqlite3_exec(handle, "BEGIN IMMEDIATE;", NULL, NULL, &err_msg)) {
		mb_svc_debug("Error:failed to begin transaction: error=%s\n",
			     err_msg);
		sqlite3_free(err_msg);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	sqlite3_free(err_msg);
	mb_svc_debug("mb_svc_sqlite3_begin_trans leave\n");
	return 0;
}

int mb_svc_sqlite3_commit_trans(MediaSvcHandle *mb_svc_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug("mb_svc_sqlite3_commit_trans enter\n");
	if (SQLITE_OK != sqlite3_exec(handle, "COMMIT;", NULL, NULL, &err_msg)) {
		mb_svc_debug("Error:failed to end transaction: error=%s\n",
			     err_msg);
		sqlite3_free(err_msg);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	sqlite3_free(err_msg);
	mb_svc_debug("mb_svc_sqlite3_commit_trans leave\n");
	return 0;
}

int mb_svc_sqlite3_rollback_trans(MediaSvcHandle *mb_svc_handle)
{
	char *err_msg = NULL;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug("mb_svc_sqlite3_rollback_trans enter\n");
	if (SQLITE_OK !=
	    sqlite3_exec(handle, "ROLLBACK;", NULL, NULL, &err_msg)) {
		mb_svc_debug("Error:failed to rollback transaction: error=%s\n",
			     err_msg);
		sqlite3_free(err_msg);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	sqlite3_free(err_msg);
	mb_svc_debug("mb_svc_sqlite3_rollback_trans leave\n");
	return 0;
}

void mb_svc_sql_list_add(GList **list, char **sql)
{
	*list = g_list_append( *list, *sql );
}

void mb_svc_sql_list_release(GList **sql_list)
{
	if (*sql_list) {
		int i = 0;
		char *sql = NULL;
		for (i = 0; i < g_list_length(*sql_list); i++) {
			sql = (char*)g_list_nth_data(*sql_list, i);
			if (sql) sqlite3_free(sql);
			sql = NULL;
		}

		g_list_free(*sql_list);
		*sql_list = NULL;
	}
}
