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



#ifndef _MEDIA_SVC_UTIL_H_
#define _MEDIA_SVC_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
typedef struct _handle_table {
	int ref_cnt;
	sqlite3* handle;
} HandleTable;


int
_media_info_get_thread_id();

/* To manage handles in each trhead, use GHashTable */
int
_media_info_init_handle_tbl();

int
_media_info_finalize_handle_tbl();

int
_media_info_insert_handle(int** tid_key, int tid, HandleTable** handle_table);

int
_media_info_remove_handle(int tid);

HandleTable*
_media_info_search_handle(int tid);

sqlite3*
_media_info_get_proper_handle();

void
_media_info_atomic_add_counting(HandleTable *handle_table);

void
_media_info_atomic_sub_counting(HandleTable *handle_table);
#endif

char *_media_info_generate_uuid(void);

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_UTIL_H_*/
