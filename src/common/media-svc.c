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
#include <sqlite3.h>
#include <db-util.h>
#include "media-svc.h"
#include "media-svc-error.h"
#include "media-svc-debug.h"
#include "media-svc-env.h"


static int __media_svc_busy_handler(void *pData, int count);
static int __media_svc_connect_db_with_handle(sqlite3 **db_handle);
static int __media_svc_disconnect_db_with_handle(sqlite3 *db_handle);

static int __media_svc_busy_handler(void *pData, int count)
{
	usleep(50000);
	mediainfo_dbg("mb_svc_busy_handler called : %d\n", count);

	return 100 - count;
}

static int __media_svc_connect_db_with_handle(sqlite3 **db_handle)
{
	mediainfo_dbg("");
	int err = -1;
	err = db_util_open(MEDIA_INFO_DATABASE_NAME, db_handle,
			 DB_UTIL_REGISTER_HOOK_METHOD);

	if (SQLITE_OK != err) {
		*db_handle = NULL;

		return MEDIA_INFO_ERROR_DATABASE_CONNECT;
	}

	/*Register busy handler*/
	err = sqlite3_busy_handler(*db_handle, __media_svc_busy_handler, NULL);
	if (SQLITE_OK != err) {
		if (*db_handle) mediainfo_dbg("[sqlite] %s\n", sqlite3_errmsg(*db_handle));

		db_util_close(*db_handle);
		*db_handle = NULL;

		return MEDIA_INFO_ERROR_DATABASE_CONNECT;
	}

	return MEDIA_INFO_ERROR_NONE;
}

static int __media_svc_disconnect_db_with_handle(sqlite3 *db_handle)
{
	mediainfo_dbg("");
	int err = -1;
	err = db_util_close(db_handle);

	if (SQLITE_OK != err) {
		db_handle = NULL;

		return MEDIA_INFO_ERROR_DATABASE_DISCONNECT;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int media_svc_connect(MediaSvcHandle **handle)
{
	int ret = -1;
	sqlite3 * db_handle = NULL;

	ret = __media_svc_connect_db_with_handle(&db_handle);

	if ( ret < 0) {
		return -1;
	} else {
		*handle = db_handle;
		return 0;
	}
}

int media_svc_disconnect(MediaSvcHandle *handle)
{
	sqlite3 * db_handle = (sqlite3 *)handle;

	return __media_svc_disconnect_db_with_handle(db_handle);
}

