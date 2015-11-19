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


#ifndef _MEDIA_SVC_DB_UTILS_H_
#define _MEDIA_SVC_DB_UTILS_H_

#include <sqlite3.h>
#include <glib.h>

#define SQLITE3_FINALIZE(x)		if (x != NULL) {sqlite3_finalize(x);}
#define SQLITE3_SAFE_FREE(x)	  {if(x != NULL) {sqlite3_free(x);x = NULL;}}

int _media_svc_make_table_query(const char *table_name, media_svc_table_slist_e list, uid_t uid);
int _media_svc_upgrade_table_query(sqlite3 *db_handle, const char *table_name, media_svc_table_slist_e list, uid_t uid);
int _media_svc_init_table_query(const char *event_table_name);
void _media_svc_destroy_table_query();
int _media_svc_create_media_table_with_id(const char *table_id, uid_t uid);
int _media_svc_drop_media_table(const char *storage_id, uid_t uid);
int _media_svc_update_media_view(sqlite3 *db_handle, uid_t uid);
int _media_svc_sql_query(const char *sql_str, uid_t uid);
int _media_svc_get_user_version(sqlite3 *db_handle, int *user_version);
int _media_svc_sql_prepare_to_step(sqlite3 *handle, const char *sql_str, sqlite3_stmt **stmt);
int _media_svc_sql_prepare_to_step_simple(sqlite3 *handle, const char *sql_str, sqlite3_stmt **stmt);
int _media_svc_sql_begin_trans(uid_t uid);
int _media_svc_sql_end_trans(uid_t uid);
int _media_svc_sql_rollback_trans(uid_t uid);
int _media_svc_sql_query_list(GList **query_list, uid_t uid);
void _media_svc_sql_query_add(GList **query_list, char **query);
void _media_svc_sql_query_release(GList **query_list);
int _media_svc_check_db_upgrade(sqlite3 *db_handle, bool *need_full_scan, int user_version, uid_t uid);
int _media_db_check_corrupt(sqlite3 *db_handle);
char *_media_svc_get_path(uid_t uid);


#endif /*_MEDIA_SVC_DB_UTILS_H_*/
