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

#include "minfo-bookmark.h"
#include "media-svc-api.h"
#include "visual-svc-debug.h"
#include "visual-svc-error.h"
#include <string.h>

static void _minfo_mbookmark_init(Mbookmark *mbookmark);

int minfo_mbookmark_load(MediaSvcHandle *mb_svc_handle, Mbookmark *mbookmark)
{
	mb_svc_bookmark_record_s bookmark_record = { 0 };
	int ret = 0;
	int length = 0;

	ret =
	    mb_svc_get_bookmark_record_by_id(mb_svc_handle, mbookmark->_id, &bookmark_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_bookmark_record_by_id failed");
		return ret;
	}

	mbookmark->_id = bookmark_record._id;
	mbookmark->position = bookmark_record.marked_time;

	/* length = strlen(bookmark_record.thumbnail_path) + 1; */
	length = MB_SVC_FILE_PATH_LEN_MAX + 1;
	mbookmark->thumb_url = (char *)malloc(length);
	if (mbookmark->thumb_url == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mbookmark->thumb_url, 0x00, length);
	strncpy(mbookmark->thumb_url, bookmark_record.thumbnail_path, length);

	mbookmark->media_uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);
	if (mbookmark->media_uuid == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}

	memset(mbookmark->media_uuid, 0x00, MB_SVC_UUID_LEN_MAX + 1);
	strncpy(mbookmark->media_uuid, bookmark_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

	return 0;
}

Mbookmark *minfo_mbookmark_new(MediaSvcHandle *mb_svc_handle, int id)
{
	Mbookmark *bookmark = NULL;
	int ret = 0;

	bookmark = malloc(sizeof(Mbookmark));
	if (bookmark == NULL) {
		return NULL;
	}

	_minfo_mbookmark_init(bookmark);
	if (id != -1) {
		bookmark->_id = id;
		ret = minfo_mbookmark_load(mb_svc_handle, bookmark);
		if (ret < 0) {
			free(bookmark);
			return NULL;
		}
	}

	return bookmark;
}

void minfo_mbookmark_destroy(Mbookmark *mbookmark)
{
	if (mbookmark != NULL && IS_MINFO_MBOOKMARK(mbookmark)) {
		mb_svc_debug("do free resource\n");
		if (mbookmark->media_uuid) {
			free(mbookmark->media_uuid);
		}
		if (mbookmark->thumb_url) {
			free(mbookmark->thumb_url);
		}
		free(mbookmark);
		mbookmark = NULL;
	}

}

static void _minfo_mbookmark_init(Mbookmark *mbookmark)
{
	mbookmark->gtype = MINFO_TYPE_MBOOKMARK;

	mbookmark->_id = -1;
	mbookmark->media_uuid = NULL;
	mbookmark->position = 0;
	mbookmark->thumb_url = NULL;
}

