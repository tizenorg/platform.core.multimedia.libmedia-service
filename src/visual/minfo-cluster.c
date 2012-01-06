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

#include "minfo-cluster.h"
#include "media-svc-api.h"
#include "media-svc-util.h"
#include "media-svc-error.h"
#include <string.h>

static void _minfo_mcluster_init(Mcluster *mcluster);

int minfo_mcluster_load(Mcluster *mcluster)
{
	mb_svc_folder_record_s fd_record;
	int ret = 0;
	int length = 0;

	ret = mb_svc_get_folder_record_by_id(mcluster->uuid, &fd_record);
	if (ret < 0) {
		return ret;
	}

	mcluster->thumb_url = NULL;
	mcluster->mtime = (time_t) fd_record.modified_date;
	mcluster->type = fd_record.storage_type;

	mcluster->display_name = (char *)malloc(MB_SVC_FILE_NAME_LEN_MAX + 1);
	if (mcluster->display_name == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	strncpy(mcluster->display_name, fd_record.display_name,
		MB_SVC_FILE_NAME_LEN_MAX);

	mcluster->count =
	    mb_svc_get_folder_content_count_by_folder_id(mcluster->uuid);
	mcluster->sns_type = fd_record.sns_type;

	length = strlen(fd_record.web_account_id) + 1;
	mcluster->account_id = (char *)malloc(length);
	if (mcluster->account_id == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	strncpy(mcluster->account_id, fd_record.web_account_id, length);

	mcluster->lock_status = fd_record.lock_status;

	length = strlen(fd_record.web_album_id) + 1;
	mcluster->web_album_id = (char *)malloc(length);
	if (mcluster->web_album_id == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	strncpy(mcluster->web_album_id, fd_record.web_album_id, length);

	mcluster->_reserved = NULL;

	return 0;
}

Mcluster *minfo_mcluster_new(const char *uuid)
{
	Mcluster *mcluster = NULL;
	int ret = 0;

	mcluster = (Mcluster *) malloc(sizeof(Mcluster));
	if (mcluster == NULL) {
		return NULL;
	}

	_minfo_mcluster_init(mcluster);

	if (uuid != NULL) {
		mcluster->uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);
		strncpy(mcluster->uuid, uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = minfo_mcluster_load(mcluster);
		if (ret < 0) {
			minfo_mcluster_destroy(mcluster);
			return NULL;
		}
	}

	return mcluster;
}

void minfo_mcluster_destroy(Mcluster *mcluster)
{
	if (mcluster != NULL) {
		if (mcluster->uuid) {
			free(mcluster->uuid);
		}
		if (mcluster->display_name) {
			free(mcluster->display_name);
		}
		if (mcluster->account_id) {
			free(mcluster->account_id);
		}
		if (mcluster->web_album_id) {
			free(mcluster->web_album_id);
		}
		free(mcluster);
		mcluster = NULL;
	}
}

static void _minfo_mcluster_init(Mcluster *mcluster)
{
	mcluster->gtype = MINFO_TYPE_MCLUSTER;

	mcluster->uuid = NULL;
	mcluster->thumb_url = NULL;
	mcluster->mtime = 0;
	mcluster->type = 0;
	mcluster->display_name = NULL;
	mcluster->count = 0;
	mcluster->sns_type = 0;
	mcluster->account_id = NULL;
	mcluster->web_album_id = NULL;
	mcluster->_reserved = NULL;
}

