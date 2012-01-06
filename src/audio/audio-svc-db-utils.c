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
 * This file defines structure and functions related to database.
 *
 * @file       	audio-svc-db-utils.c
 * @version 	0.1
 * @brief     	This file defines sqlite utilities for Audio Service.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "audio-svc-debug.h"
#include "audio-svc-error.h"
#include "audio-svc-db-utils.h"
#include "media-info-util.h"

int _audio_svc_sql_busy_handler(void *pData, int count)
{
	usleep(50000);
	
	printf("_audio_svc_sql_busy_handler called : %d\n", count);
	audio_svc_debug("_audio_svc_sql_busy_handler called : %d\n", count);

	return 100 - count;
}

int _audio_svc_sql_query(const char *sql_str)
{
	int err = -1;
	char *zErrMsg = NULL;
	
	audio_svc_debug("SQL = [%s]", sql_str);

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_error("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	
	err = sqlite3_exec(handle, sql_str, NULL, NULL, &zErrMsg);

	if (SQLITE_OK != err) {
		audio_svc_error("failed to execute [%s], err[%d]", zErrMsg, err);
	} else {
		audio_svc_debug("query success");
	}

	if (zErrMsg)
		sqlite3_free (zErrMsg);

	return err;
}

int _audio_svc_sql_query_list(GList **query_list)
{
	int i = 0;
	int length = g_list_length(*query_list);
	int err = -1;
	char *sql = NULL;
	
	audio_svc_debug("query list length : [%d]", length);

	for (i = 0; i < length; i++) {
		sql = (char*)g_list_nth_data(*query_list, i);
		if(sql != NULL) {
			err = _audio_svc_sql_query(sql);
			sqlite3_free(sql);
			sql = NULL;
			if (err != SQLITE_OK) {
				return AUDIO_SVC_ERROR_DB_INTERNAL;
			}
		}
	}

	_audio_svc_sql_query_release(query_list);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_sql_prepare_to_step(const char *sql_str, sqlite3_stmt** stmt)
{
	int err = -1;

	audio_svc_debug("[SQL query] : %s", sql_str);

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_error("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	
	err = sqlite3_prepare_v2(handle, sql_str, -1, stmt, NULL);
	sqlite3_free((char *)sql_str);
	
	if (err != SQLITE_OK) {
		audio_svc_error ("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	err = sqlite3_step(*stmt);
	if (err != SQLITE_ROW) {
		audio_svc_error("Item not found. end of row [%s]", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(*stmt);
		return AUDIO_SVC_ERROR_DB_NO_RECORD;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_sql_begin_trans(void)
{
	char *err_msg = NULL;

	audio_svc_debug("========_audio_svc_sql_begin_trans");
	
	sqlite3 *handle = _media_info_get_proper_handle();

	if (handle == NULL) {
		audio_svc_error("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (SQLITE_OK != sqlite3_exec(handle, "BEGIN IMMEDIATE;", NULL, NULL, &err_msg)) {
		audio_svc_error("Error:failed to begin transaction: error=%s", err_msg);
		sqlite3_free(err_msg);
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	
	sqlite3_free(err_msg);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_sql_end_trans(void)
{
	char *err_msg = NULL;

	audio_svc_debug("========_audio_svc_sql_end_trans");
	
	sqlite3 *handle = _media_info_get_proper_handle();
	
	if (handle == NULL) {
		audio_svc_error("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (SQLITE_OK != sqlite3_exec(handle, "COMMIT;", NULL, NULL, &err_msg)) {
		audio_svc_error("Error:failed to end transaction: error=%s", err_msg);
		sqlite3_free(err_msg);
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	
	sqlite3_free(err_msg);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_sql_rollback_trans(void)
{
	char *err_msg = NULL;

	audio_svc_debug("========_audio_svc_sql_rollback_trans");
	
	sqlite3 *handle = _media_info_get_proper_handle();

	if (handle == NULL) {
		audio_svc_error("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (SQLITE_OK != sqlite3_exec(handle, "ROLLBACK;", NULL, NULL, &err_msg)) {
		audio_svc_error("Error:failed to rollback transaction: error=%s", err_msg);
		sqlite3_free(err_msg);
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_free(err_msg);

	return AUDIO_SVC_ERROR_NONE;
}

void _audio_svc_sql_query_add(GList **query_list, char **query)
{
	*query_list = g_list_append( *query_list, *query);
}

void _audio_svc_sql_query_release(GList **query_list)
{
	if (*query_list) {
		audio_svc_debug("_audio_svc_sql_query_release");
		g_list_free(*query_list);
		*query_list = NULL;
	}
}

