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

#include "minfo-meta.h"
#include "minfo-api.h"
#include "minfo-bookmark.h"
#include "media-svc-api.h"
#include "media-svc-util.h"
#include "media-svc-debug.h"
#include "media-svc-error.h"
#include <string.h>

static void _minfo_mmeta_init(Mmeta *mmeta);

int minfo_mmeta_load(Mmeta *mmeta, mb_svc_media_record_s *p_md_record)
{
	int ret = -1;
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_meta_record = {0,};
	mb_svc_video_meta_record_s video_meta_record = {0,};

	if (p_md_record == NULL) {
		ret =
		    mb_svc_get_media_record_by_id(mmeta->media_uuid,
						  &media_record);
		if (ret < 0) {
			mb_svc_debug
			    ("minfo_get_meta_info:get media record by id failed\n");
			return ret;
		}
	} else {
		media_record = *p_md_record;
	}

	if (media_record.content_type == MINFO_ITEM_IMAGE) {
		ret =
		    mb_svc_get_image_record_by_media_id(mmeta->media_uuid,
							&image_meta_record);
		if (ret < 0) {
			return ret;
		}

		mmeta->longitude = image_meta_record.longitude;
		mmeta->latitude = image_meta_record.latitude;
		mmeta->type = media_record.content_type;
		mmeta->description = NULL;

		mmeta->width = image_meta_record.width;
		mmeta->height = image_meta_record.height;
		mmeta->datetaken = image_meta_record.datetaken;

		mmeta->image_info = minfo_mimage_new(NULL);
		if (mmeta->image_info == NULL) {
			return MB_SVC_ERROR_INTERNAL;
		}
		mmeta->image_info->orientation = image_meta_record.orientation;
	}

	else if (media_record.content_type == MINFO_ITEM_VIDEO) {
		ret =
		    mb_svc_get_video_record_by_media_id(mmeta->media_uuid,
							&video_meta_record);
		if (ret < 0) {
			return ret;
		}

		mmeta->type = media_record.content_type;
		mmeta->description = NULL;
		mmeta->longitude = video_meta_record.longitude;
		mmeta->latitude = video_meta_record.latitude;

		mmeta->width = video_meta_record.width;
		mmeta->height = video_meta_record.height;
		mmeta->datetaken = video_meta_record.datetaken;

		mmeta->video_info = minfo_mvideo_new(mmeta->media_uuid);

		if (mmeta->video_info == NULL) {
			return MB_SVC_ERROR_INTERNAL;
		}
	}

	return 0;
}

static void _minfo_mmeta_init(Mmeta *mmeta)
{
	mmeta->gtype = MINFO_TYPE_MMETA;

	mmeta->media_uuid = NULL;
	mmeta->type = 0;
	mmeta->description = NULL;
	mmeta->image_info = NULL;
	mmeta->video_info = NULL;
	mmeta->_reserved = NULL;
	mmeta->longitude = 0.0f;
	mmeta->latitude = 0.0f;
}

Mmeta *minfo_mmeta_new(const char *media_uuid, mb_svc_media_record_s *p_md_record)
{
	Mmeta *mmeta = NULL;
	int ret = 0;

	mmeta = (Mmeta *) malloc(sizeof(Mmeta));
	if (mmeta == NULL) {
		return NULL;
	}

	_minfo_mmeta_init(mmeta);

	mmeta->media_uuid = (char *)malloc(MB_SVC_UUID_LEN_MAX + 1);

	if (p_md_record) {
		strncpy(mmeta->media_uuid, p_md_record->media_uuid, MB_SVC_UUID_LEN_MAX + 1);
		ret = minfo_mmeta_load(mmeta, p_md_record);
	} else if (media_uuid != NULL) {
		strncpy(mmeta->media_uuid, media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = minfo_mmeta_load(mmeta, NULL);
		if (ret < 0) {
			free(mmeta);
			return NULL;
		}
	} else {
		free(mmeta);
		return NULL;
	}

	return mmeta;
}

void minfo_mmeta_destroy(Mmeta *mmeta)
{
	if (mmeta != NULL && IS_MINFO_MMETA(mmeta)) {
		if (mmeta->type == MINFO_ITEM_IMAGE && mmeta->image_info) {
			minfo_mimage_destroy(mmeta->image_info);
		}
		if (mmeta->type == MINFO_ITEM_VIDEO && mmeta->video_info) {
			minfo_mvideo_destroy(mmeta->video_info);
		}
		if (mmeta->media_uuid) {
			free(mmeta->media_uuid);
		}
		if (mmeta->description) {
			free(mmeta->description);
		}
		free(mmeta);
		mmeta = NULL;
	}
}

/*--------------------------mvideo-----------------------------*/
static void _minfo_mvideo_init(Mvideo *mvideo);

static int _minfo_bm_ite_fn(Mbookmark *bookmark, void *user_data)
{
	GList **list = (GList **) user_data;
	*list = g_list_append(*list, bookmark);

	return 0;
}

int minfo_mvideo_load(const char *media_id, Mvideo *mvideo)
{
	mb_svc_video_meta_record_s video_meta_record = { 0 };
	int ret = 0;
	int length = 0;

	ret = mb_svc_get_video_record_by_media_id(media_id, &video_meta_record);
	if (ret < 0) {
		return ret;
	}

	length = strlen(video_meta_record.album) + 1;
	mvideo->album_name = (char *)malloc(length);
	if (mvideo->album_name == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mvideo->album_name, 0x00, length);
	strncpy(mvideo->album_name, video_meta_record.album, length);

	length = strlen(video_meta_record.artist) + 1;
	mvideo->artist_name = (char *)malloc(length);
	if (mvideo->artist_name == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mvideo->artist_name, 0x00, length);
	strncpy(mvideo->artist_name, video_meta_record.artist, length);

	length = strlen(video_meta_record.title) + 1;
	mvideo->title = (char *)malloc(length);
	if (mvideo->title == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mvideo->title, 0x00, length);
	strncpy(mvideo->title, video_meta_record.title, length);

	mvideo->last_played_pos = video_meta_record.last_played_time;
	mvideo->duration = video_meta_record.duration;

	length = strlen(video_meta_record.youtube_category) + 1;
	mvideo->web_category = (char *)malloc(length);
	if (mvideo->web_category == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	memset(mvideo->web_category, 0x00, length);

	strncpy(mvideo->web_category, video_meta_record.youtube_category,
		length);

	GList *tmp_list = NULL;
	minfo_get_bookmark_list(media_id, _minfo_bm_ite_fn, &tmp_list);

	mvideo->bookmarks = tmp_list;

	mvideo->_reserved = NULL;

	return 0;
}

static void _minfo_mvideo_init(Mvideo *mvideo)
{
	mvideo->gtype = MINFO_TYPE_MVIDEO;

	mvideo->album_name = NULL;
	mvideo->artist_name = NULL;
	mvideo->title = NULL;
	mvideo->last_played_pos = 0;
	mvideo->duration = 0;
	mvideo->web_category = NULL;
	mvideo->bookmarks = NULL;
	mvideo->_reserved = NULL;
}

Mvideo *minfo_mvideo_new(const char *id)
{
	Mvideo *mvideo = NULL;
	int ret = 0;

	mvideo = (Mvideo *) malloc(sizeof(Mvideo));
	if (mvideo == NULL) {
		return NULL;
	}

	if (id == NULL) {
		_minfo_mvideo_init(mvideo);
	} else {
		ret = minfo_mvideo_load(id, mvideo);
		if (ret < 0) {
			minfo_mvideo_destroy(mvideo);
			return NULL;
		}
	}

	return mvideo;
}

void minfo_mvideo_destroy(Mvideo *mvideo)
{
	int i = 0;
	Mbookmark *bookmark;
	if (mvideo != NULL && IS_MINFO_MVIDEO(mvideo)) {
		if (mvideo->album_name) {
			free(mvideo->album_name);
		}
		if (mvideo->artist_name) {
			free(mvideo->artist_name);
		}
		if (mvideo->title) {
			free(mvideo->title);
		}
		if (mvideo->web_category) {
			free(mvideo->web_category);
		}
		if (mvideo->bookmarks) {
			for (i = 0;
			     i < g_list_length((GList *) mvideo->bookmarks);
			     i++) {
				bookmark =
				    (Mbookmark *) g_list_nth_data((GList *)
								  mvideo->
								  bookmarks, i);
				if (bookmark)
					minfo_mbookmark_destroy(bookmark);
			}
			g_list_free(mvideo->bookmarks);
		}
		free(mvideo);
		mvideo = NULL;
	}

}

/*-------------------------mimage--------------------------*/
static void _minfo_mimage_init(Mimage *mimage);

int minfo_mimage_load(const char *media_id, Mimage *mimage)
{
	mb_svc_image_meta_record_s image_meta_record = { 0 };
	int ret = 0;

	ret = mb_svc_get_image_record_by_media_id(media_id, &image_meta_record);
	if (ret < 0) {
		return ret;
	}

	mimage->orientation = image_meta_record.orientation;
	mimage->_reserved = NULL;

	return 0;
}

Mimage *minfo_mimage_new(const char *id)
{
	Mimage *mimage = NULL;
	int ret = 0;

	mimage = (Mimage *) malloc(sizeof(Mimage));
	if (mimage == NULL) {
		return NULL;
	}

	if (id == NULL) {
		_minfo_mimage_init(mimage);
	} else {
		ret = minfo_mimage_load(id, mimage);
		if (ret < 0) {
			free(mimage);
			return NULL;
		}
	}

	return mimage;
}

void minfo_mimage_destroy(Mimage *mimage)
{
	if (mimage != NULL && IS_MINFO_MIMAGE(mimage)) {
		free(mimage);
		mimage = NULL;
	}
}

static void _minfo_mimage_init(Mimage *mimage)
{
	mimage->gtype = MINFO_TYPE_MIMAGE;

	mimage->orientation = 0;
	mimage->_reserved = NULL;
}
