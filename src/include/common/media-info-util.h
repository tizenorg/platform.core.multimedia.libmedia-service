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



#ifndef _MEDIA_INFO_UTIL_H_
#define _MEDIA_INFO_UTIL_H_

#include <string.h>
#include <alloca.h>
#include <sqlite3.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "media-info-env.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIAINFO_CATEGORY_UNKNOWN    0x00000000  /**< Default */
#define MEDIAINFO_CATEGORY_ETC        0x00000001  /**< ETC category */
#define MEDIAINFO_CATEGORY_IMAGE      0x00000002  /**< Image category */
#define MEDIAINFO_CATEGORY_VIDEO      0x00000004  /**< Video category */
#define MEDIAINFO_CATEGORY_MUSIC      0x00000008  /**< Music category */
#define MEDIAINFO_CATEGORY_SOUND      0x00000010  /**< Sound category */
#define MEDIAINFO_CATEGORY_DRM        0x00000020  /**< DRM category */

typedef enum {
	MEDIAINFO_PHONE,  /**< Phone storage */
	MEDIAINFO_MMC     /**< MMC storage */
} mediainfo_store_type;

#define MEDIAINFO_PHONE_ROOT_PATH         "/opt/media"
#define MEDIAINFO_MMC_ROOT_PATH           "/opt/storage/sdcard"


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

char *_media_info_generate_uuid();

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_INFO_UTIL_H_*/
