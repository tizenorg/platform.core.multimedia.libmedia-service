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

#include "minfo-item.h"
#include "minfo-meta.h"
#include "media-svc-api.h"
#include "media-svc-util.h"
#include "media-svc-debug.h"
#include "media-svc-error.h"
#include <string.h>

static void _minfo_mitem_init(Mitem *mitem);

static int minfo_mitem_load(Mitem *mitem, mb_svc_media_record_s * p_md_record)
{
	mb_svc_media_record_s md_record = {"",};
	int ret = 0;
	int length = 0;

	if (p_md_record == NULL) {
		ret = mb_svc_get_media_record_by_id(mitem->uuid, &md_record);
	} else {
		md_record = *p_md_record;
	}

	if (ret < 0) {
		return ret;
	}

	//mitem->cluster_id = md_record.folder_id;
	if (mitem->cluster_uuid == NULL) 
		mitem->cluster_uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);

	strncpy(mitem->cluster_uuid, md_record.folder_uuid, MB_SVC_UUID_LEN_MAX + 1);

	if (strlen(md_record.http_url) != 0) {
		length = MB_SVC_DIR_PATH_LEN_MAX + 1;
		mitem->file_url = (char *)malloc(length);
		if (mitem->file_url == NULL) {
			return MB_SVC_ERROR_OUT_OF_MEMORY;
		}
		strncpy(mitem->file_url, md_record.http_url, length);
		mitem->file_url[length - 1] = '\0';
	} else {
		length = strlen(md_record.path) + 1;
		mitem->file_url = (char *)malloc(length);
		if (mitem->file_url == NULL) {
			return MB_SVC_ERROR_OUT_OF_MEMORY;
		}
		strncpy(mitem->file_url, md_record.path, length);
		mitem->file_url[length - 1] = '\0';
	}

	length = MB_SVC_FILE_PATH_LEN_MAX + 1;
	mitem->thumb_url = (char *)malloc(length);
	if (mitem->thumb_url == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mitem->thumb_url, 0x00, length);
	strncpy(mitem->thumb_url, md_record.thumbnail_path, length);
	mitem->thumb_url[length - 1] = '\0';

	mitem->mtime = (time_t) md_record.modified_date;

	length = MB_SVC_FILE_EXT_LEN_MAX + 1;
	mitem->ext = (char *)malloc(length);
	if (mitem->ext == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mitem->ext, 0x00, length);
	_mb_svc_get_file_ext(md_record.display_name, mitem->ext);

	mitem->type = md_record.content_type;
	length = MB_SVC_FILE_NAME_LEN_MAX + 1;
	mitem->display_name = (char *)malloc(length);
	if (mitem->display_name == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mitem->display_name, 0x00, length);
	strncpy(mitem->display_name, md_record.display_name, length);

	mitem->rate = md_record.rate;
	mitem->_reserved = NULL;

	return 0;
}

Mitem *minfo_mitem_new(const char *uuid)
{
	return minfo_media_item_new(uuid, NULL);
}

Mitem *minfo_media_item_new(const char *uuid, mb_svc_media_record_s * p_md_record)
{
	Mitem *mitem = NULL;
	int ret = 0;

	mitem = (Mitem *) malloc(sizeof(Mitem));
	if (mitem == NULL) {
		return NULL;
	}

	_minfo_mitem_init(mitem);

	if (p_md_record) {
		mitem->uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);
		strncpy(mitem->uuid, p_md_record->media_uuid, MB_SVC_UUID_LEN_MAX + 1);
		ret = minfo_mitem_load(mitem, p_md_record);
	} else if (uuid != NULL) {
		mitem->uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);
		strncpy(mitem->uuid, uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = minfo_mitem_load(mitem, NULL);
		if (ret < 0) {
			free(mitem);
			return NULL;
		}
	}

	return mitem;
}

void minfo_mitem_destroy(Mitem *mitem)
{
	if (mitem != NULL && IS_MINFO_MITEM(mitem)) {
		mb_svc_debug("do free resource %s\n", mitem->file_url);
		if (mitem->uuid) {
			free(mitem->uuid);
		}
		if (mitem->cluster_uuid) {
			free(mitem->cluster_uuid);
		}
		if (mitem->file_url) {
			free(mitem->file_url);
		}
		if (mitem->thumb_url) {
			free(mitem->thumb_url);
		}
		if (mitem->ext) {
			free(mitem->ext);
		}
		if (mitem->display_name) {
			free(mitem->display_name);
		}
		if (mitem->meta_info) {
			minfo_mmeta_destroy(mitem->meta_info);
		}
		mitem->gtype = 0;
		free(mitem);

		mitem = NULL;
	}
}

static void _minfo_mitem_init(Mitem *mitem)
{
	mitem->gtype = MINFO_TYPE_MITEM;

	//mitem->_id = -1;
	mitem->uuid = NULL;
	mitem->file_url = NULL;
	mitem->thumb_url = NULL;;
	mitem->mtime = 0;
	mitem->ext = NULL;
	mitem->type = 0;
	//mitem->cluster_id = 0;
	mitem->cluster_uuid = NULL;
	mitem->display_name = NULL;
	mitem->rate = 0;
	mitem->meta_info = NULL;
	mitem->_reserved = NULL;
}
