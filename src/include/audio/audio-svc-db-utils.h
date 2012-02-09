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



#ifndef _AUDIO_SVC_DB_UTILS_H_
#define _AUDIO_SVC_DB_UTILS_H_


/**
 * @file       	audio-svc-db-utils.h
 * @version 	0.1
 * @brief     	This file defines sqlite utilities for Audio Service.
 */

#include <sqlite3.h>
#include <glib.h>

#define SQLITE3_FINALIZE(x)       if(x  != NULL) {sqlite3_finalize(x);}

int _audio_svc_sql_busy_handler(void *pData, int count);
int _audio_svc_sql_query(sqlite3 *handle, const char *sql_str);
int _audio_svc_sql_query_list(sqlite3 *handle, GList **query_list);
int _audio_svc_sql_prepare_to_step(sqlite3 *handle, const char *sql_str, sqlite3_stmt** stmt);
int _audio_svc_sql_begin_trans(sqlite3 *handle);
int _audio_svc_sql_end_trans(sqlite3 *handle);
int _audio_svc_sql_rollback_trans(sqlite3 *handle);
void _audio_svc_sql_query_add(GList **query_list, char **query);
void _audio_svc_sql_query_release(GList **query_list);



#endif /*_AUDIO_SVC_DB_UTILS_H_*/

