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
#include <asm/unistd.h>
#include <pthread.h>
#include <string.h>

#include "media-info-util.h"
#include "media-info-error.h"
#include "media-info-debug.h"
#include "media-svc.h"
#include "audio-svc.h"
#include "audio-svc-error.h"
#include "uuid.h"

static GHashTable *g_handle_table = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int _media_info_get_thread_id()
{
	return syscall(__NR_gettid);
}

int _media_info_init_handle_tbl()
{
	pthread_mutex_lock(&mutex);

	if (g_handle_table == NULL)
		g_handle_table = g_hash_table_new(g_int_hash, g_int_equal);

	pthread_mutex_unlock(&mutex);

	if (g_handle_table == NULL)
		return -1;
	else
		return 0;
}

int _media_info_finalize_handle_tbl()
{
	pthread_mutex_lock(&mutex);

	if (g_handle_table != NULL) {
		int size = g_hash_table_size(g_handle_table);
		if (size == 0) {
			g_hash_table_destroy(g_handle_table);
			g_handle_table = NULL;
		} else
			mediainfo_dbg("handle table is not empty");
	}

	pthread_mutex_unlock(&mutex);

	return 0;
}

int
_media_info_insert_handle(int **tid_key, int tid, HandleTable ** handle_table)
{
	*tid_key = g_malloc(sizeof **tid_key);
	**tid_key = tid;
	*handle_table = g_malloc(sizeof **handle_table);

	pthread_mutex_lock(&mutex);

	g_hash_table_insert(g_handle_table, (gpointer) * tid_key,
			    (gpointer) * handle_table);

	(*handle_table)->ref_cnt = 1;

	pthread_mutex_unlock(&mutex);

	return 0;
}

int _media_info_remove_handle(int tid)
{
	int *key = NULL;
	HandleTable *val = NULL;

	pthread_mutex_lock(&mutex);

	if (!g_hash_table_lookup_extended
	    (g_handle_table, &tid, (gpointer) & key, (gpointer) & val)) {
		pthread_mutex_unlock(&mutex);
		return -1;
	} else {
		if (g_hash_table_remove(g_handle_table, (gpointer) & tid))
			mediainfo_dbg("g_hash_table_remove done");
		else
			mediainfo_dbg("g_hash_table_remove fails");

		if (key)
			g_free(key);
		if (val)
			g_free(val);

		pthread_mutex_unlock(&mutex);
		return 0;
	}
}

HandleTable *_media_info_search_handle(int tid)
{
	pthread_mutex_lock(&mutex);

	HandleTable *value =
	    (HandleTable *) g_hash_table_lookup(g_handle_table,
						(gpointer) & tid);

	pthread_mutex_unlock(&mutex);

	return value;
}

sqlite3 *_media_info_get_proper_handle()
{
	int tid = _media_info_get_thread_id();
	HandleTable *value =
	    (HandleTable *) g_hash_table_lookup(g_handle_table,
						(gpointer) & tid);

	if (value == NULL) {
		return NULL;
	} else {
		return value->handle;
	}
}
void _media_info_atomic_add_counting(HandleTable *handle_table)
{
	pthread_mutex_lock(&mutex);
	handle_table->ref_cnt++;
	pthread_mutex_unlock(&mutex);
}


void _media_info_atomic_sub_counting(HandleTable *handle_table)
{
	pthread_mutex_lock(&mutex);
	handle_table->ref_cnt--;
	pthread_mutex_unlock(&mutex);
}

char *_media_info_generate_uuid()
{
	uuid_t uuid_value;
	static char uuid_unparsed[50];	

	uuid_generate(uuid_value);
	uuid_unparse(uuid_value, uuid_unparsed);

	//mediainfo_dbg("UUID : %s", uuid_unparsed);
	return uuid_unparsed;
}

