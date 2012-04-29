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

#include <string.h>
#include "media-svc-debug.h"
#include "visual-svc-types.h"
#include "visual-svc.h"
#include "visual-svc-error.h"
#include "media-svc-db.h"
#include "media-svc-api.h"
#include "visual-svc-util.h"
#include "media-svc-thumb.h"
#include "visual-svc-debug.h"
#include "minfo-cluster.h"
#include "minfo-item.h"
#include "minfo-tag.h"
#include "minfo-meta.h"
#include "minfo-bookmark.h"
#include "minfo-streaming.h"
#include "media-svc-db-util.h"

static __thread int g_trans_valid_cnt = 1;
static __thread int g_cur_trans_valid_cnt = 0;
static __thread int g_trans_insert_cnt = 1;
static __thread int g_cur_trans_insert_cnt = 0;
static __thread int g_trans_move_cnt = 1;
static __thread int g_cur_trans_move_cnt = 0;

EXPORT_API int
minfo_get_item_list(MediaSvcHandle *mb_svc_handle,
			const char *cluster_id,
			const minfo_item_filter filter,
			minfo_item_ite_cb func,
			void *user_data)
{
	int record_cnt = 0;
	int ret = -1;
	mb_svc_media_record_s md_record = {"",};
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	Mitem *mitem = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	start = mediainfo_get_debug_time();
#endif

	minfo_item_filter mb_filter;
	memcpy(&mb_filter, &filter, sizeof(minfo_item_filter));

	mb_svc_debug("minfo_get_item_list--enter\n");

	mb_svc_debug("minfo_get_item_list#filter.file_type: %d",
		     filter.file_type);
	mb_svc_debug("minfo_get_item_list#filter.sort_type: %d",
		     filter.sort_type);
	mb_svc_debug("minfo_get_item_list#filter.start_pos: %d",
		     filter.start_pos);
	mb_svc_debug("minfo_get_item_list#filter.end_pos: %d", filter.end_pos);
	mb_svc_debug("minfo_get_item_list#filter.with_meta: %d",
		     filter.with_meta);
	mb_svc_debug("minfo_get_item_list#filter.favorite: %d",
		     filter.favorite);

	ret =
	    mb_svc_media_iter_start_new(mb_svc_handle, cluster_id, &mb_filter,
					MINFO_CLUSTER_TYPE_ALL, TRUE, NULL,
					&mb_svc_iterator);

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	record_cnt = 0;

	while (1) {
		/* improve the performance of getting meida list. */
		ret = mb_svc_media_iter_next(&mb_svc_iterator, &md_record);
		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, NULL, &md_record);
		if (filter.with_meta && mitem) {
			mitem->meta_info =
			    minfo_mmeta_new(mb_svc_handle, mitem->uuid, &md_record);
		}

		func(mitem, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	mb_svc_debug("minfo_get_item_list--leave\n");

#ifdef _PERFORMANCE_CHECK_
	end = mediainfo_get_debug_time();

	char msg[255];
	snprintf(msg, sizeof(msg), "LIST count : %d", record_cnt);
	mediainfo_print_debug_time_ex(start, end, __FUNCTION__, msg);
#endif

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_item_list_search(MediaSvcHandle *mb_svc_handle,
								minfo_search_field_t search_field,
								const char *search_str,
								minfo_folder_type folder_type,
								const minfo_item_filter filter,
								minfo_item_ite_cb func,
								void *user_data)
{
	mb_svc_debug("");
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	int ret = -1;
	int record_cnt = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (search_str == NULL) {
		mb_svc_debug("search string is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("User func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (search_field < MINFO_SEARCH_BY_NAME || search_field >= MINFO_SEARCH_MAX) {
		mb_svc_debug("search field is wrong");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (folder_type < MINFO_CLUSTER_TYPE_ALL || folder_type >= MINFO_CLUSTER_TYPE_MAX) {
		mb_svc_debug("folder type is wrong");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_media_record_s md_record = {"",};
	Mitem *mitem = NULL;

	mb_svc_debug("minfo_get_item_list_search--enter\n");

	ret =
	    mb_svc_media_search_iter_start(mb_svc_handle, search_field, search_str, folder_type, filter, &mb_svc_iterator);

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		/* improve the performance of getting meida list. */
		ret = mb_svc_media_iter_next(&mb_svc_iterator, &md_record);
		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, NULL, &md_record);

		if (filter.with_meta && mitem) {
			mitem->meta_info =
			    minfo_mmeta_new(mb_svc_handle, mitem->uuid, &md_record);
		}

		func(mitem, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_all_item_list(MediaSvcHandle *mb_svc_handle,
			const minfo_folder_type cluster_type,
			const minfo_item_filter filter,
			minfo_item_ite_cb func,
			void *user_data)
{
	mb_svc_debug("");

	mb_svc_iterator_s mb_svc_iterator = { 0 };
	int ret = -1;
	int record_cnt = 0;
	GList *p_list = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_media_record_s md_record = {"",};
	Mitem *mitem = NULL;
	minfo_item_filter mb_item_filter = { 0 };
	minfo_cluster_filter mb_filter = { 0 };

	mb_filter.cluster_type = cluster_type;
	mb_filter.sort_type = MINFO_CLUSTER_SORT_BY_NAME_ASC;
	mb_filter.start_pos = -1;
	mb_filter.end_pos = -1;

#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	start = mediainfo_get_debug_time();
#endif
	memcpy(&mb_item_filter, &filter, sizeof(minfo_item_filter));

	mb_svc_debug("minfo_get_all_item_list--enter\n");

	ret =
	    mb_svc_media_iter_start_new(mb_svc_handle, NULL, &mb_item_filter, cluster_type, TRUE,
					p_list, &mb_svc_iterator);

	if (p_list) {
		_mb_svc_glist_free(&p_list, FALSE);
	}

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		/* improve the performance of getting meida list. */
		ret = mb_svc_media_iter_next(&mb_svc_iterator, &md_record);
		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, NULL, &md_record);

		if (mitem != NULL) {
			if (filter.with_meta) {
				mitem->meta_info =
					minfo_mmeta_new(mb_svc_handle, mitem->uuid, &md_record);
			}

			func(mitem, user_data);
		} else {
			mb_svc_debug("Making media item failed");
		}
	}

	mb_svc_iter_finish(&mb_svc_iterator);

#ifdef _PERFORMANCE_CHECK_
	end = mediainfo_get_debug_time();

	char msg[255];
	snprintf(msg, sizeof(msg), "LIST count : %d", record_cnt);
	mediainfo_print_debug_time_ex(start, end, __FUNCTION__, msg);
#endif

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

DEPRECATED_API int minfo_get_all_item_cnt(MediaSvcHandle *mb_svc_handle, int *cnt)
{
	int ret = -1;

	mb_svc_debug("");

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cnt == NULL) {
		mb_svc_debug("cnt == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_all_item_count(mb_svc_handle, MINFO_CLUSTER_TYPE_ALL, MINFO_ITEM_ALL, MINFO_MEDIA_FAV_ALL, cnt);
	if (ret < 0) {
		mb_svc_debug("Error: get image full pathfull failed\n");
		return ret;
	}

	mb_svc_debug("record count = %d", *cnt);
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_all_item_count(
						MediaSvcHandle *mb_svc_handle,
						minfo_folder_type folder_type,
						minfo_file_type file_type,
						minfo_media_favorite_type fav_type,
						int *cnt)
{
	int ret = -1;

	mb_svc_debug("");

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cnt == NULL) {
		mb_svc_debug("cnt == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_all_item_count(mb_svc_handle, folder_type, file_type, fav_type, cnt);
	if (ret < 0) {
		mb_svc_debug("Error: mb_svc_get_all_item_count failed\n");
		return ret;
	}

	mb_svc_debug("record count = %d", *cnt);
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_cluster_cover(MediaSvcHandle *mb_svc_handle, 
					const char *cluster_id,
					const int img_cnt,
					minfo_cover_ite_cb func,
					void *user_data)
{
	int record_cnt = 0;
	int ret = -1;
	mb_svc_media_record_s md_record = {"",};
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	minfo_item_filter mb_filter = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_item_list--enter\n");

	mb_svc_debug("minfo_get_cluster_cover#cluster_id: %s", cluster_id);
	mb_svc_debug("minfo_get_cluster_cover#img_cnt: %d", img_cnt);

	if (func == NULL) {
		mb_svc_debug("func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_filter.start_pos = 0;
	mb_filter.end_pos = img_cnt - 1;
	mb_filter.sort_type = MINFO_MEDIA_SORT_BY_DATE_ASC;
	mb_filter.file_type = MINFO_ITEM_ALL;
	mb_filter.favorite = MINFO_MEDIA_FAV_ALL;
	mb_filter.with_meta = FALSE;

	ret =
	    mb_svc_media_iter_start_new(mb_svc_handle, cluster_id, &mb_filter,
					MINFO_CLUSTER_TYPE_ALL, TRUE, NULL,
					&mb_svc_iterator);

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret = mb_svc_media_iter_next(&mb_svc_iterator, &md_record);
		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb_svc_media_iter_next failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;
		func(md_record.thumbnail_path, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	mb_svc_debug("minfo_get_item_list--leave\n");

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_item_cnt(MediaSvcHandle *mb_svc_handle,
				const char *cluster_id,
				const minfo_item_filter filter,
				int *cnt)
{
	int ret = -1;
	int record_cnt = 0;
	mb_svc_iterator_s mb_svc_iterator = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	minfo_item_filter mb_filter;
	memcpy(&mb_filter, &filter, sizeof(minfo_item_filter));

	mb_svc_debug("minfo_get_item_cnt--enter\n");
	if (cnt == NULL) {
		mb_svc_debug("cnt == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	
	mb_svc_debug("minfo_get_item_cnt#filter.file_type: %d",
		     filter.file_type);
	mb_svc_debug("minfo_get_item_cnt#filter.sort_type: %d",
		     filter.sort_type);
	mb_svc_debug("minfo_get_item_cnt#filter.start_pos: %d",
		     filter.start_pos);
	mb_svc_debug("minfo_get_item_cnt#filter.end_pos: %d", filter.end_pos);
	mb_svc_debug("minfo_get_item_cnt#filter.with_meta: %d",
		     filter.with_meta);
	mb_svc_debug("minfo_get_item_cnt#filter.favorite: %d", filter.favorite);

	ret =
	    mb_svc_media_iter_start_new(mb_svc_handle, cluster_id, &mb_filter,
					MINFO_CLUSTER_TYPE_ALL, TRUE, NULL,
					&mb_svc_iterator);

	if (ret < 0) {
		mb_svc_debug("mb_svc_media_iter_start failed");
		*cnt = -1;
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret = mb_svc_media_iter_next(&mb_svc_iterator, NULL);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	*cnt = record_cnt;

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_cluster_cnt(MediaSvcHandle *mb_svc_handle, const minfo_cluster_filter filter, int *cnt)
{
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	int ret = -1;
	int record_cnt = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	minfo_cluster_filter mb_filter;

	memcpy(&mb_filter, &filter, sizeof(minfo_cluster_filter));

	mb_svc_debug("minfo_get_cluster_list enter\n");
	if (cnt == NULL) {
		mb_svc_debug("Error: cnt == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_cluster_list#filter.file_type: %d",
		     filter.cluster_type);
	mb_svc_debug("minfo_get_cluster_list#filter.sort_type: %d",
		     filter.sort_type);
	mb_svc_debug("minfo_get_cluster_list#filter.start_pos: %d",
		     filter.start_pos);
	mb_svc_debug("minfo_get_cluster_list#filter.end_pos: %d",
		     filter.end_pos);

	ret = mb_svc_folder_iter_start(mb_svc_handle, &mb_filter, &mb_svc_iterator);
	if (ret < 0) {
		mb_svc_debug("mb_svc_folder_iter_start failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret = mb_svc_folder_iter_next(&mb_svc_iterator, NULL);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;
	}

	*cnt = record_cnt;
	mb_svc_debug("record count = %d\n", *cnt);

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_cluster_list(MediaSvcHandle *mb_svc_handle,
				const minfo_cluster_filter filter,
				minfo_cluster_ite_cb func,
				void *user_data)
{
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	mb_svc_folder_record_s fd_record = {"",};
	int ret = -1;
	int record_cnt = 0;
	Mcluster *cluster = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	minfo_cluster_filter mb_filter;

	memcpy(&mb_filter, &filter, sizeof(minfo_cluster_filter));

	mb_svc_debug("minfo_get_cluster_list enter\n");

	mb_svc_debug("minfo_get_cluster_list#filter.file_type: %d",
		     filter.cluster_type);
	mb_svc_debug("minfo_get_cluster_list#filter.sort_type: %d",
		     filter.sort_type);
	mb_svc_debug("minfo_get_cluster_list#filter.start_pos: %d",
		     filter.start_pos);
	mb_svc_debug("minfo_get_cluster_list#filter.end_pos: %d",
		     filter.end_pos);

	ret = mb_svc_folder_iter_start(mb_svc_handle, &mb_filter, &mb_svc_iterator);
	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret = mb_svc_folder_iter_next(&mb_svc_iterator, &fd_record);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug
			    ("mb-svc iterator get next recrod failed\n");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		cluster = minfo_mcluster_new(mb_svc_handle, fd_record.uuid);
		func(cluster, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_web_cluster_by_web_account_id(MediaSvcHandle *mb_svc_handle,
					const char *web_account_id,
					minfo_cluster_ite_cb func,
					void *user_data)
{
	mb_svc_folder_record_s *fd_record;
	int ret = -1;
	int record_cnt = 0;
	Mcluster *cluster = NULL;
	GList *p_web_cluster_list = NULL;
	char _web_account_id[MB_SVC_ARRAY_LEN_MAX + 1] = { 0 };
	int i = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	strncpy(_web_account_id, web_account_id, MB_SVC_ARRAY_LEN_MAX + 1);
	_web_account_id[MB_SVC_ARRAY_LEN_MAX] = '\0';

	ret =
	    mb_svc_get_folder_list_by_web_account_id(mb_svc_handle, _web_account_id,
						     &p_web_cluster_list);

	if (ret < 0) {
		mb_svc_debug
		    ("minfo_get_web_cluster_by_web_account_id failed\n");
		return ret;
	}

	record_cnt = g_list_length(p_web_cluster_list);
	for (; i < record_cnt; i++) {
		fd_record = g_list_nth_data(p_web_cluster_list, i);
		cluster = minfo_mcluster_new(mb_svc_handle, fd_record->uuid);
		func(cluster, user_data);
	}

	_mb_svc_glist_free(&p_web_cluster_list, TRUE);

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_bookmark_list(MediaSvcHandle *mb_svc_handle,
					const char *media_id,
					minfo_bm_ite_cb func,
					void *user_data)
{
	mb_svc_debug("");
	int ret = -1;
	int record_cnt = 0;

	mb_svc_bookmark_record_s bookmark_record = { 0, };
	mb_svc_iterator_s mb_svc_iterator = { 0, };
	Mbookmark *mbookmark = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_bookmark_list#media_id: %s", media_id);

	ret = mb_svc_bookmark_iter_start(mb_svc_handle, media_id, &mb_svc_iterator);
	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret =
		    mb_svc_bookmark_iter_next(&mb_svc_iterator, &bookmark_record);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;
		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mbookmark = minfo_mbookmark_new(mb_svc_handle, bookmark_record._id);
		func(mbookmark, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	mb_svc_debug("minfo_get_bookmark by media_id--leave\n");
	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_geo_item_list(MediaSvcHandle *mb_svc_handle,
			const char *cluster_id,
			minfo_folder_type store_filter,
			minfo_item_filter filter,
			double min_longitude,
			double max_longitude,
			double min_latitude,
			double max_latitude,
			minfo_item_ite_cb func, void *user_data)
{
	/* list of Mitem cluster_id == -1, All ?? cluster_id > 0, specified */
	int record_cnt = 0;
	int ret = -1;
	mb_svc_media_record_s md_record = {"",};
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	Mitem *mitem = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

/*
	mb_svc_debug ("minfo_get_geo_item_list--enter\n");    
	mb_svc_debug("minfo_get_geo_item_list#filter.file_type: %d", filter.file_type);
	mb_svc_debug("minfo_get_geo_item_list#filter.sort_type: %d", filter.sort_type);
	mb_svc_debug("minfo_get_geo_item_list#filter.start_pos: %d", filter.start_pos);
	mb_svc_debug("minfo_get_geo_item_list#filter.end_pos: %d", filter.end_pos);
	mb_svc_debug("minfo_get_geo_item_list#filter.with_meta: %d", filter.with_meta);
	mb_svc_debug("minfo_get_geo_item_list#filter.favorite: %d", filter.favorite);
	mb_svc_debug("minfo_get_geo_item_list#min_longitude: %f",
		     min_longitude);
	mb_svc_debug("minfo_get_geo_item_list#max_longitude: %f",
		     max_longitude);
	mb_svc_debug("minfo_get_geo_item_list#min_latitude: %f", min_latitude);
	mb_svc_debug("minfo_get_geo_item_list#max_latitude: %f", max_latitude);
*/

	if ((max_longitude < min_longitude) || (max_latitude < min_latitude)) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((max_longitude < -180.0) || (min_longitude > 180.0)) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((max_latitude < -90.0) || (min_latitude > 90.0)) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret =
	    mb_svc_geo_media_iter_start(mb_svc_handle, cluster_id, store_filter, &filter,
					&mb_svc_iterator, min_longitude,
					max_longitude, min_latitude,
					max_latitude);

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		ret = mb_svc_media_iter_next(&mb_svc_iterator, &md_record);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, md_record.media_uuid, &md_record);
		if (filter.with_meta && mitem) {
			mitem->meta_info =
			    minfo_mmeta_new(mb_svc_handle, mitem->uuid, &md_record);
		}

		func(mitem, user_data);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	mb_svc_debug("minfo_get_geo_item_list--leave\n");

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_streaming_list(MediaSvcHandle *mb_svc_handle, GList **p_list)
{
	Mstreaming *mstreaming = NULL;
	int record_cnt = 0;
	mb_svc_web_streaming_record_s webstreaming_record = { 0 };
	int ret = -1;
	mb_svc_iterator_s mb_svc_iterator = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (p_list == NULL) {
		mb_svc_debug("Error:p_list == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_webstreaming_iter_start(mb_svc_handle, &mb_svc_iterator);
	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		memset(&webstreaming_record, 0x00,
		       sizeof(mb_svc_web_streaming_record_s));
		ret =
		    mb_svc_webstreaming_iter_next(&mb_svc_iterator,
						  &webstreaming_record);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return ret;
		}

		record_cnt++;

		mstreaming = minfo_mstreaming_new(webstreaming_record._id);
		*p_list = g_list_append(*p_list, mstreaming);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	mb_svc_debug("minfo_get_streaming_list--leave");

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_meta_info(MediaSvcHandle *mb_svc_handle, const char *media_id, Mmeta **meta)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_meta_info#media_id: %s", media_id);
	Mmeta *mmeta = NULL;

	mmeta = minfo_mmeta_new(mb_svc_handle, media_id, NULL);
	if (mmeta == NULL) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	*meta = mmeta;
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_update_video_meta_info_int(MediaSvcHandle *mb_svc_handle,
				const char *media_id,
				minfo_video_meta_field_t meta_field,
				const int updated_value)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_video_meta_info_int#media_id: %s", media_id);
	int ret = -1;
	mb_svc_video_meta_record_s video_meta_record = {0,};

	ret = mb_svc_get_video_record_by_media_id(mb_svc_handle, media_id, &video_meta_record);

	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_video_meta_info_int--load video meta fails! err%d",
		     ret);
		return ret;
	}
	switch (meta_field) {
	case MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED:
		video_meta_record.last_played_time = updated_value;
		break;
	default:
		break;
	}
	ret = mb_svc_update_record_video_meta(mb_svc_handle, &video_meta_record);

	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_video_meta_info_int--update video meta fails! err%d",
		     ret);
		return ret;
	}

	return ret;
}

EXPORT_API int
minfo_update_image_meta_info_int(MediaSvcHandle *mb_svc_handle,
				const char *media_id,
				minfo_image_meta_field_t meta_field,
				const int updated_value,
				...)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_image_meta_info_int#media_id: %s", media_id);
	int ret = -1;
	mb_svc_image_meta_record_s image_meta_record = {0,};

	ret = mb_svc_get_image_record_by_media_id(mb_svc_handle, media_id, &image_meta_record);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_get_image_record_by_media_id fails! err%d", ret);
		return ret;
	}

	va_list var_args;
	minfo_image_meta_field_t field;
	int value = 0;
	field = meta_field;
	value = updated_value;

	va_start(var_args, updated_value);

	do {
		if (field == meta_field) {
			value = updated_value;
		} else {
			value = va_arg((var_args), int);
		}

		mb_svc_debug("Field:%d, Value:%d", field, value);

		switch (field) {
			case MINFO_IMAGE_META_WIDTH:
				image_meta_record.width = value;
				break;
			case MINFO_IMAGE_META_HEIGHT:
				image_meta_record.height = value;
				break;
			case MINFO_IMAGE_META_ORIENTATION:
				image_meta_record.orientation = value;
				break;
			case MINFO_VIDEO_META_DATE_TAKEN:
				image_meta_record.datetaken = value;
				break;
			default:
				break;
		}

		/* next field */
		field = va_arg(var_args, int);
	} while (field > 0 && field < 5);

	va_end(var_args);

	ret = mb_svc_update_record_image_meta(mb_svc_handle, &image_meta_record);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_update_record_image_meta fails! err%d", ret);
		return ret;
	}

	return ret;
}

EXPORT_API int
minfo_add_media_start(MediaSvcHandle *mb_svc_handle, int trans_count)
{
	mb_svc_debug("Transaction count : %d", trans_count);

	if (trans_count <= 1) {
		mb_svc_debug("Trans count should be bigger than 1");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	g_trans_insert_cnt = trans_count;
	g_cur_trans_insert_cnt = 0;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_add_media_end(MediaSvcHandle *mb_svc_handle)
{
	mb_svc_debug("");

	if (g_cur_trans_insert_cnt > 0) {
		int ret = -1;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");

			g_cur_trans_insert_cnt = 0;
			g_trans_insert_cnt = 1;

			return ret;
		}

		ret = mb_svc_insert_items(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_insert_items failed...");
			return ret;
		}

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);

			g_cur_trans_insert_cnt = 0;
			g_trans_insert_cnt = 1;

			return ret;
		}
	}

	g_cur_trans_insert_cnt = 0;
	g_trans_insert_cnt = 1;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_add_media_batch(MediaSvcHandle *mb_svc_handle, const char *file_url, minfo_file_type content_type)
{
	int err = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL) {
		mb_svc_debug("File URL is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("file_full_path is %s\n", file_url);

	if (g_cur_trans_insert_cnt < g_trans_insert_cnt) {
		err = mb_svc_insert_file_batch(mb_svc_handle, file_url, content_type);

		if (err < 0) {
			mb_svc_debug("mb_svc_insert_file_batch failed\n");
			return err;
		}

		g_cur_trans_insert_cnt++;

		return MB_SVC_ERROR_NONE;
	}

	if (g_cur_trans_insert_cnt == g_trans_insert_cnt) {
		err = mb_svc_insert_file_batch(mb_svc_handle, file_url, content_type);
		if (err < 0) {
			mb_svc_debug("mb_svc_insert_file_batch failed\n");
			return err;
		}

		g_cur_trans_insert_cnt = 0;

		err = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (err < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			return err;
		}

		err = mb_svc_insert_items(mb_svc_handle);
		if (err < 0) {
			mb_svc_debug
				("mb_svc_insert_items failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return err;
		}

		err = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (err < 0) {
			mb_svc_debug
				("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return err;
		}
 	}

	return err;
}

EXPORT_API int minfo_add_media(MediaSvcHandle *mb_svc_handle, const char *file_url, minfo_file_type content_type)
{
	int err = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL) {
		mb_svc_debug("File URL is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("file_full_path is %s\n", file_url);

	err = mb_svc_insert_file(mb_svc_handle, file_url, content_type);

	if (err < 0) {
		mb_svc_debug("mb_svc_insert_file failed (%d) ", err);
	}

	return err;
}

EXPORT_API int minfo_delete_media(MediaSvcHandle *mb_svc_handle, const char *file_url)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL) {
		mb_svc_debug("File URL is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_delete_file(mb_svc_handle, file_url);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo delete media, delete media file info failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);

		return ret;
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_delete_media_id(MediaSvcHandle *mb_svc_handle, const char *media_id)
{
	int ret = -1;
	mb_svc_media_record_s media_record = {"",};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_media_record_by_id(mb_svc_handle, media_id, &media_record);
	if (ret < 0) {
		mb_svc_debug("minfo delete media, get media record failed\n");
		return ret;
	}

	mb_svc_debug("media uuid : %s", media_record.media_uuid);
	mb_svc_debug("media folder uuid : %s", media_record.folder_uuid);
	mb_svc_debug("media path : %s", media_record.path);

	/* handle web image case. */
	if (strncmp(media_record.http_url, "", 1) != 0) {
		ret = mb_svc_delete_record_media_by_id(mb_svc_handle, media_record.media_uuid);
		if (ret < 0) {
			mb_svc_debug
			    ("minfo delete media, delete media record by media_id failed\n");
			return ret;
		}

		/* delete file info in image_meta table & (video_meta table and bookmark table if it's video file) */
		ret =
		    mb_svc_delete_bookmark_meta_by_media_id(mb_svc_handle, 
								media_record.media_uuid,
								media_record.
								content_type);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_delete_record_video_or_image_by_media_id fail:media id is %d\n",
			     media_id);
		}
		return ret;
	}

	return minfo_delete_media(mb_svc_handle, media_record.path);
}

DEPRECATED_API int
minfo_add_web_media(MediaSvcHandle *mb_svc_handle,
				const char *cluster_id,
				const char *http_url,
				const char *file_name,
				const char *thumb_path)
{
	int ret = -1;
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_meta_record = {0,};
	mb_svc_video_meta_record_s video_meta_record = {0,};
	mb_svc_folder_record_s folder_record = {"",};
	minfo_file_type content_type;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_add_web_media#cluster_uuid: %s", cluster_id);
	mb_svc_debug("minfo_add_web_media#http_url: %s", http_url);
	mb_svc_debug("minfo_add_web_media#file_name: %s", file_name);

	ret = mb_svc_get_folder_record_by_id(mb_svc_handle, cluster_id, &folder_record);
	if ((ret < 0) || (folder_record.storage_type != MINFO_WEB)) {
		mb_svc_debug
		    ("minfo_add_web_media, get web folder record by id failed\n");
		return ret;
	}

	ret =
	    mb_svc_get_media_record_by_fid_name(mb_svc_handle, cluster_id, file_name,
						&media_record);
	if (ret < 0) {
		strncpy(media_record.folder_uuid, cluster_id, MB_SVC_UUID_LEN_MAX + 1);

		strncpy(media_record.http_url, http_url,
			MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(media_record.display_name, file_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(media_record.thumbnail_path, thumb_path,
			MB_SVC_FILE_PATH_LEN_MAX + 1);

		/* need to get the web media modified date */
		content_type = _mb_svc_get_file_type(file_name);

		mb_svc_debug("content_type is %d\n", content_type);

		media_record.content_type = content_type;
		media_record.rate = MB_SVC_DEFAULT;

		ret = mb_svc_insert_record_media(mb_svc_handle, &media_record, MINFO_WEB);
		if (ret < 0) {
			mb_svc_debug
			    ("minfo_add_web_media, insert new media record failed\n");
			return ret;
		}

		if (content_type == MINFO_ITEM_IMAGE) {
			strncpy(image_meta_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			/* Couldn't extract meta information from an web image, which hasn't downloaded yet */
			image_meta_record.longitude = MINFO_DEFAULT_GPS;
			image_meta_record.latitude = MINFO_DEFAULT_GPS;

			ret =
			    mb_svc_insert_record_image_meta(mb_svc_handle, &image_meta_record,
							    MINFO_WEB);
			if (ret < 0) {
				mb_svc_debug
				    ("minfo_add_web_media, insert new image_meta record failed\n");
				return ret;
			}
		}

		else if (content_type == MINFO_ITEM_VIDEO) {
			strncpy(video_meta_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			ret =
			    mb_svc_insert_record_video_meta(mb_svc_handle, &video_meta_record,
							    MINFO_WEB);
			if (ret < 0) {
				mb_svc_debug
				    ("minfo_add_web_media, insert new video_meta record failed\n");
				return ret;
			}
		}
		return MB_SVC_ERROR_NONE;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_add_web_media_with_type(MediaSvcHandle *mb_svc_handle,
					const char *cluster_id,
					const char *http_url,
					const char *file_name,
					minfo_file_type content_type,
					const char *thumb_path)
{
	int ret = -1;
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_meta_record = {0,};
	mb_svc_video_meta_record_s video_meta_record = {0,};
	mb_svc_folder_record_s folder_record = {"",};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (http_url == NULL || file_name == NULL
	    || thumb_path == NULL || content_type <= 0
	    || content_type > MINFO_ITEM_VIDEO) {
		mb_svc_debug
		    ("http_url == NULL || file_name == NULL || thumb_path == NULL || content_type <= 0 || content_type > MINFO_ITEM_VIDEO");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("cluster_uuid: %s", cluster_id);
	mb_svc_debug("http_url: %s", http_url);
	mb_svc_debug("file_name: %s", file_name);

	ret = mb_svc_get_folder_record_by_id(mb_svc_handle, cluster_id, &folder_record);
	if ((ret < 0) || (folder_record.storage_type != MINFO_WEB)) {
		mb_svc_debug("get web folder record by id failed\n");
		return ret;
	}

	ret =
	    mb_svc_get_media_record_by_fid_name(mb_svc_handle, cluster_id, file_name,
						&media_record);
	if (ret < 0) {
		strncpy(media_record.folder_uuid, cluster_id, MB_SVC_UUID_LEN_MAX + 1);

		strncpy(media_record.http_url, http_url,
			MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(media_record.display_name, file_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(media_record.thumbnail_path, thumb_path,
			MB_SVC_FILE_PATH_LEN_MAX + 1);
		media_record.content_type = content_type;
		media_record.rate = MB_SVC_DEFAULT;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			return ret;
		}

		ret = mb_svc_insert_record_media(mb_svc_handle, &media_record, MINFO_WEB);
		if (ret < 0) {
			mb_svc_debug
			    ("insert new media record failed..Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}

		if (content_type == MINFO_ITEM_IMAGE) {
			strncpy(image_meta_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			/* Couldn't extract meta information from an web image, which hasn't downloaded yet */
			image_meta_record.longitude = MINFO_DEFAULT_GPS;
			image_meta_record.latitude = MINFO_DEFAULT_GPS;

			ret =
			    mb_svc_insert_record_image_meta(mb_svc_handle, &image_meta_record,
							    MINFO_WEB);
			if (ret < 0) {
				mb_svc_debug
				    ("minfo_add_web_media, insert new image_meta record failed..Now start to rollback\n");
				mb_svc_sqlite3_rollback_trans(mb_svc_handle);
				return ret;
			}
		}

		else if (content_type == MINFO_ITEM_VIDEO) {
			strncpy(video_meta_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			ret =
			    mb_svc_insert_record_video_meta(mb_svc_handle, &video_meta_record,
							    MINFO_WEB);

			/* Couldn't extract meta information from an web video, which hasn't downloaded yet */
			video_meta_record.longitude = MINFO_DEFAULT_GPS;
			video_meta_record.latitude = MINFO_DEFAULT_GPS;

			if (ret < 0) {
				mb_svc_debug
				    ("minfo_add_web_media, insert new video_meta record failed..Now start to rollback\n");
				mb_svc_sqlite3_rollback_trans(mb_svc_handle);
				return ret;
			}
		}

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}

		return MB_SVC_ERROR_NONE;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_move_media_start(MediaSvcHandle *mb_svc_handle, int trans_count)
{
	mb_svc_debug("Transaction count : %d", trans_count);

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (trans_count <= 1) {
		mb_svc_debug("Trans count should be bigger than 1");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	g_trans_move_cnt = trans_count;
	g_cur_trans_move_cnt = 0;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_move_media_end(MediaSvcHandle *mb_svc_handle)
{
	mb_svc_debug("");

	if (g_cur_trans_move_cnt > 0) {
		int ret = -1;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");

			g_cur_trans_move_cnt = 0;
			g_trans_move_cnt = 1;

			return ret;
		}

		ret = mb_svc_move_items(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_move_items failed...");
			return ret;
		}

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);

			g_cur_trans_move_cnt = 0;
			g_trans_move_cnt = 1;

			return ret;
		}
	}

	g_cur_trans_move_cnt = 0;
	g_trans_move_cnt = 1;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_move_media(MediaSvcHandle *mb_svc_handle,
			const char *old_file_url,
			const char *new_file_url,
			minfo_file_type content_type)
{
	mb_svc_debug("");

	int ret = -1;
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char old_dir_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char new_dir_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char old_file_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char new_file_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	bool is_renamed = false;
	bool is_moved = false;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (old_file_url == NULL || new_file_url == NULL) {
		mb_svc_debug("old_file_url == NULL || new_file_url == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	_mb_svc_get_file_parent_path(old_file_url, old_dir_full_path);
	_mb_svc_get_file_parent_path(new_file_url, new_dir_full_path);

	_mb_svc_get_file_display_name(old_file_url, old_file_name);
	_mb_svc_get_file_display_name(new_file_url, new_file_name);

	if ((strncmp
	     (old_dir_full_path, new_dir_full_path,
	      MB_SVC_FILE_PATH_LEN_MAX) == 0)
	    && (strncmp(old_file_name, new_file_name, MB_SVC_FILE_NAME_LEN_MAX)
		!= 0)) {
		is_renamed = true;
	} else
	    if ((strncmp
		 (old_dir_full_path, new_dir_full_path,
		  MB_SVC_FILE_NAME_LEN_MAX) != 0)
		&&
		(strncmp(old_file_name, new_file_name, MB_SVC_FILE_NAME_LEN_MAX)
		 == 0)) {
		is_moved = true;
	} else {
	    if ((strncmp(old_dir_full_path, new_dir_full_path, MB_SVC_FILE_PATH_LEN_MAX) != 0)
			&&  (strncmp(old_file_name, new_file_name, MB_SVC_FILE_NAME_LEN_MAX) != 0)) {
			is_moved = true;
		} else {
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (g_trans_move_cnt == 1) {
		if (is_renamed) {
			ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
			if (ret < 0) {
				mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
				return ret;
			}

			ret =
				mb_svc_rename_file(mb_svc_handle, old_file_url, new_file_url, content_type, thumb_path);

			if (ret < 0) {
				mb_svc_debug
					("file rename failed.. Now start to rollback\n");
				mb_svc_sqlite3_rollback_trans(mb_svc_handle);
				return ret;
			}

			ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
			if (ret < 0) {
				mb_svc_debug
					("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
				mb_svc_sqlite3_rollback_trans(mb_svc_handle);
				return ret;
			}

			return MB_SVC_ERROR_NONE;
		}
	
		if (is_moved) {
			ret = mb_svc_move_file(mb_svc_handle, old_file_url, new_file_url, content_type, thumb_path);
			if (ret < 0) {
				mb_svc_debug
					("file move failed.. Now start to rollback\n");
				return ret;
			} else {
				mb_svc_debug("move success");
				return MB_SVC_ERROR_NONE;
			}
		}
	}

	if (g_trans_move_cnt == 1 && is_renamed) {
		mb_svc_debug("Move batch job doesn't support renaming file");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_cur_trans_move_cnt < g_trans_move_cnt) {
		ret = mb_svc_move_file_batch(mb_svc_handle, old_file_url, new_file_url, content_type, thumb_path);
		if (ret < 0) {
			mb_svc_debug("mb_svc_move_file_batch failed : %d\n", ret);
			return ret;
		}

		g_cur_trans_move_cnt++;
	}

	if (g_cur_trans_move_cnt == g_trans_move_cnt) {
		ret = mb_svc_move_file_batch(mb_svc_handle, old_file_url, new_file_url, content_type, thumb_path);
		if (ret < 0) {
			mb_svc_debug("mb_svc_move_file_batch failed : %d\n", ret);
			return ret;
		}

		g_cur_trans_move_cnt = 0;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			return ret;
		}

		ret = mb_svc_move_items(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_move_items failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_copy_media(MediaSvcHandle *mb_svc_handle,
			const char *old_file_url,
			const char *new_file_url,
			minfo_file_type content_type)
{
	int ret = -1;
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (old_file_url == NULL || new_file_url == NULL) {
		mb_svc_debug("old_file_url == NULL || new_file_url == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_copy_file(mb_svc_handle, old_file_url, new_file_url, content_type, thumb_path);
	if (ret < 0) {
		mb_svc_debug("file copy failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return ret;
}

EXPORT_API int minfo_update_media_name(MediaSvcHandle *mb_svc_handle, const char *media_id, const char *new_name)
{
	int ret = -1;
	mb_svc_media_record_s media_record = {"",};
	mb_svc_folder_record_s folder_record = {"",};
	char old_file_full_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char new_file_full_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char dest_thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (new_name == NULL) {
		mb_svc_debug(" new name is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_media_record_by_id(mb_svc_handle, media_id, &media_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_record_by_id failed: %s\n",
			     media_id);
		return ret;
	}

	ret =
	    mb_svc_get_folder_record_by_id(mb_svc_handle, media_record.folder_uuid,
					   &folder_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_folder_record_by_id failed: %s\n",
			     media_id);
		return ret;
	}

	if (strlen(folder_record.uri) > MB_SVC_FILE_PATH_LEN_MAX) {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(old_file_full_path, sizeof(old_file_full_path), "%s/",
		 folder_record.uri);
	strncpy(new_file_full_path, old_file_full_path,
		sizeof(new_file_full_path));

	strncat(old_file_full_path, media_record.display_name,
		MB_SVC_FILE_PATH_LEN_MAX - strlen(old_file_full_path));
	strncat(new_file_full_path, new_name,
		MB_SVC_FILE_PATH_LEN_MAX - strlen(new_file_full_path));

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret =
	    mb_svc_rename_file(mb_svc_handle, old_file_full_path, new_file_full_path,
			       media_record.content_type, dest_thumb_path);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_rename_file fails.. Now start to rollback");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}


EXPORT_API int
minfo_update_media_thumb(MediaSvcHandle *mb_svc_handle, const char *media_id, const char *thumb_path)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (thumb_path == NULL) {
		mb_svc_debug("thumb path is invalid");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_media_thumb: %s", thumb_path);

	ret = mb_svc_update_thumb_path_by_id(mb_svc_handle, media_id, thumb_path);
	if (ret < 0) {
		mb_svc_debug("mb_svc_update_thumb_by_media_id failed\n");
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_update_media_favorite(MediaSvcHandle *mb_svc_handle, const char *media_id, const int favorite_level)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (favorite_level != 0 && favorite_level != 1) {
		mb_svc_debug("favorite_level is invalid");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_update_favorite_by_media_id(mb_svc_handle, media_id, favorite_level);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_media_favorite, update media record failed\n");
		return ret;
	}
	return MB_SVC_ERROR_NONE;
}

int minfo_update_media_date(MediaSvcHandle *mb_svc_handle, const char *media_id, time_t modified_date)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		mb_svc_debug("media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_media_date: %s", media_id);
	mb_svc_debug("minfo_update_media_date: %d", modified_date);

	ret = mb_svc_update_date_by_id(mb_svc_handle, media_id, modified_date);
	if (ret < 0) {
		mb_svc_debug("mb_svc_update_date_by_media_id failed\n");
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_cp_media(MediaSvcHandle *mb_svc_handle, const char *src_media_id, const char *dst_cluster_id)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (src_media_id == NULL || dst_cluster_id == NULL) {
		mb_svc_debug("src_media_id == NULL || dst_cluster_id == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_copy_file_by_id(mb_svc_handle, src_media_id, dst_cluster_id);

	if (ret < 0) {
		mb_svc_debug("file copy failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return ret;
}

EXPORT_API int minfo_mv_media(MediaSvcHandle *mb_svc_handle, const char *src_media_id, const char *dst_cluster_id)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (src_media_id == NULL || dst_cluster_id == NULL) {
		mb_svc_debug("src_media_id == NULL || dst_cluster_id == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_move_file_by_id(mb_svc_handle, src_media_id, dst_cluster_id);
	if (ret < 0) {
		mb_svc_debug("file move failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return ret;
}

EXPORT_API int minfo_add_cluster(MediaSvcHandle *mb_svc_handle, const char *cluster_url, char *id, int max_length)	/* only for local folder path */
{
	int ret = -1;
	mb_svc_folder_record_s folder_record = {"",};
	char parent_dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dir_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	int folder_modified_date = 0;
	int store_type = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_url == NULL || id == NULL) {
		mb_svc_debug("cluster_url == NULL || id == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	/* <Example> cluster_url:/opt/media/tmp */
	mb_svc_debug("minfo_add_cluster#cluster_url:%s\n", cluster_url);

	store_type = _mb_svc_get_store_type_by_full(cluster_url);

	if (store_type == MB_SVC_ERROR_INTERNAL) {
		mb_svc_debug("Failed to get storage type : %s", cluster_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	/* dir_display_name : tmp */
	_mb_svc_get_dir_display_name(cluster_url, dir_display_name);

	folder_modified_date = _mb_svc_get_file_dir_modified_date(cluster_url);

	mb_svc_debug("store_type is %d\n", store_type);
	mb_svc_debug("dir_display_name is %s\n", dir_display_name);
	mb_svc_debug("parent_dir_path is %s\n", parent_dir_path);
	mb_svc_debug("folder_modified_date is %d\n", folder_modified_date);

	folder_record.modified_date = folder_modified_date;
	folder_record.storage_type = (minfo_store_type) store_type;
	strncpy(folder_record.uri, cluster_url,
		MB_SVC_DIR_PATH_LEN_MAX + 1);
	strncpy(folder_record.display_name, dir_display_name,
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
	folder_record.sns_type = 0;

	mb_svc_debug("folder_record.uri is %s\n", folder_record.uri);
	mb_svc_debug
	    ("no record in %s, ready insert the folder record into db\n",
	     cluster_url);

	ret = mb_svc_insert_record_folder(mb_svc_handle, &folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("insert file info into folder table failed\n");
		return ret;
	}

	strncpy(id, folder_record.uuid, max_length);

	return MB_SVC_ERROR_NONE;
}

DEPRECATED_API int
minfo_add_web_cluster(MediaSvcHandle *mb_svc_handle,
				int sns_type,
				const char *name,
				const char *account_id,
				char *id, int max_length)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (name == NULL || account_id == NULL || id == 0) {
		mb_svc_debug("the parameters are invalid!\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_folder_record_s folder_record = {"",};

	folder_record.sns_type = sns_type;
	folder_record.storage_type = MINFO_WEB;
	strncpy(folder_record.web_account_id, account_id, MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(folder_record.display_name, name, MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(folder_record.uri, "", MB_SVC_DIR_PATH_LEN_MAX + 1);
	strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
	folder_record.modified_date = 0;

	/* first, check whether the same web cluster has existed. */
	ret = mb_svc_get_web_album_cluster_record(mb_svc_handle, sns_type, name, account_id, NULL, &folder_record);

	if (ret == 0) {
		strncpy(id, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);
		return MB_SVC_ERROR_NONE;
	}

	ret = mb_svc_insert_record_folder(mb_svc_handle, &folder_record);
	if (ret < 0) {
		mb_svc_debug("insert record into folder table failed\n");
		return ret;
	}

	strncpy(id, folder_record.uuid, max_length);

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_add_web_cluster_album_id(MediaSvcHandle *mb_svc_handle,
						int sns_type,
						const char *name,
						const char *account_id,
						const char *album_id,
						char *id, int max_length)
{
	int ret = -1;
	mb_svc_folder_record_s folder_record = {"",};

	if (name == NULL || account_id == NULL || album_id == NULL || id == NULL) {
		mb_svc_debug("the parameters are invalid!\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	folder_record.sns_type = sns_type;
	folder_record.storage_type = MINFO_WEB;
	strncpy(folder_record.web_account_id, account_id, MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(folder_record.display_name, name, MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(folder_record.uri, "", MB_SVC_DIR_PATH_LEN_MAX + 1);
	strncpy(folder_record.web_album_id, album_id, MB_SVC_ARRAY_LEN_MAX + 1);
	folder_record.modified_date = 0;

	/* first, check whether the same web cluster has existed. */
	ret = mb_svc_get_web_album_cluster_record(mb_svc_handle, sns_type, name, account_id, album_id, &folder_record);

	if (ret == 0) {
		strncpy(id, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);
		return MB_SVC_ERROR_NONE;
	}

	ret = mb_svc_insert_record_folder(mb_svc_handle, &folder_record);
	if (ret < 0) {
		mb_svc_debug("insert record into folder table failed\n");
		return ret;
	}

	strncpy(id, folder_record.uuid, max_length);
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_delete_web_cluster(MediaSvcHandle *mb_svc_handle, const char *cluster_id)
{
	/* delete id & all media items */
	int ret = -1;
	mb_svc_folder_record_s folder_record = {"",};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_delete_web_cluster#cluster_id: %s", cluster_id);

	ret = mb_svc_get_folder_record_by_id(mb_svc_handle, cluster_id, &folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_web_cluster: get folder record by id failed\n");
		return ret;
	}

	if (folder_record.storage_type == MINFO_WEB) {

		ret = mb_svc_delete_folder(mb_svc_handle, cluster_id, MINFO_WEB);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_delete_folder: delete web cluster failed..Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}

	} else {
		mb_svc_debug
		    ("minfo_delete_web_cluster: the folder is not web folder\n");
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_web_cluster_web_album_id(MediaSvcHandle *mb_svc_handle,
					const char *web_album_id,
					Mcluster **mcluster)
{
	int ret = 0;
	int folder_id = 0;
	Mcluster *cluster = NULL;
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (web_album_id == NULL || *mcluster != NULL) {
		mb_svc_debug("web_album_id == NULL || *mcluster != NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_web_cluster_web_album_id#album_id: %s",
		     web_album_id);
	ret = mb_svc_get_folder_id_by_web_album_id(mb_svc_handle, web_album_id, folder_uuid);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_folder_id_by_web_album_id fails:%s\n",
			     web_album_id);
		return ret;
	}

	cluster = minfo_mcluster_new(mb_svc_handle, folder_uuid);
	if (cluster == NULL) {
		mb_svc_debug("minfo_mcluster_new: %d\n", folder_id);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	*mcluster = cluster;
	return MB_SVC_ERROR_NONE;

}

EXPORT_API int minfo_delete_cluster(MediaSvcHandle *mb_svc_handle, const char *cluster_id)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_delete_cluster#cluster_id: %s", cluster_id);

	return mb_svc_delete_folder(mb_svc_handle, cluster_id, MINFO_SYSTEM);
}

EXPORT_API int
minfo_update_cluster_name(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const char *new_name)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL || new_name == NULL) {
		mb_svc_debug("cluster_id == NULL || new_name == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_update_cluster_name(mb_svc_handle, cluster_id, new_name);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_update_cluster_name failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return ret;
}

EXPORT_API int
minfo_update_cluster_date(MediaSvcHandle *mb_svc_handle, const char *cluster_id, time_t modified_date)
{
	int ret = -1;
	mb_svc_folder_record_s folder_record;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_cluster_date#cluster_id: %s", cluster_id);
	mb_svc_debug("minfo_update_cluster_date#modified_date: %d",
		     modified_date);

	ret = mb_svc_get_folder_record_by_id(mb_svc_handle, cluster_id, &folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_cluster_date: no folder record matched with the folder id\n");
		return ret;
	}

	folder_record.modified_date = modified_date;

	ret = mb_svc_update_record_folder(mb_svc_handle, &folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_cluster_date: update cluster date failed\n");
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_add_bookmark(MediaSvcHandle *mb_svc_handle, const char *media_id, const int position,
		   const char *thumb_path)
{
	int ret = -1;
	mb_svc_bookmark_record_s bookmark_record = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (thumb_path == NULL || media_id == NULL) {
		mb_svc_debug("Thumb path or media_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_add_bookmark#media_id: %s", media_id);
	mb_svc_debug("minfo_add_bookmark#position: %d", position);
	mb_svc_debug("minfo_add_bookmark#thumb_path: %s", thumb_path);

	//bookmark_record.media_id = media_id;
	strncpy(bookmark_record.media_uuid, media_id, MB_SVC_UUID_LEN_MAX + 1);
	bookmark_record.marked_time = position;
	strncpy(bookmark_record.thumbnail_path, thumb_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	ret = mb_svc_insert_record_bookmark(mb_svc_handle, &bookmark_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_insert_record_bookmark fail\n");
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_delete_bookmark(MediaSvcHandle *mb_svc_handle, const int bookmark_id)
{
	int ret = -1;
	mb_svc_debug("minfo_delete_bookmark#bookmark_id: %d", bookmark_id);

	ret = mb_svc_delete_record_bookmark_by_id(mb_svc_handle, bookmark_id);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_bookmark: delete matched bookmark record by id failed\n");
		return ret;
	}
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_add_streaming(MediaSvcHandle *mb_svc_handle, const char *title, const char *url, uint duration,
		    const char *thumb_path, int *id)
{
	/* "> 0 : SuccessOthers : fail" */
	int ret = -1;
	mb_svc_web_streaming_record_s webstreaming_record = { 0 };
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_add_streaming#title: %s", title);
	mb_svc_debug("minfo_add_streaming#url: %s", url);
	mb_svc_debug("minfo_add_streaming#duration: %d", duration);
	mb_svc_debug("minfo_add_streaming#thumb_path: %s", thumb_path);

	ret = mb_svc_get_web_streaming_folder_uuid(mb_svc_handle, folder_uuid, sizeof(folder_uuid));
	if (ret < 0) {
		mb_svc_debug("not add web streaming foler yet, so insert it.");
		ret = mb_svc_add_web_streaming_folder(mb_svc_handle, folder_uuid);
		if (ret < 0) {
			mb_svc_debug("mb_svc_add_web_streaming_folder failed : %d", ret);
			return ret;
		}
	}

	//webstreaming_record.folder_id = folder_id;
	strncpy(webstreaming_record.folder_uuid, folder_uuid, MB_SVC_UUID_LEN_MAX + 1);
	webstreaming_record.duration = duration;
	strncpy(webstreaming_record.title, title, MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(webstreaming_record.url, url,  MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(webstreaming_record.thumb_path, thumb_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	ret = mb_svc_insert_record_web_streaming(mb_svc_handle, &webstreaming_record);
	if (ret < 0) {
		return ret;
	}

	*id = webstreaming_record._id;
	mb_svc_debug("minfo_add_streaming: new webstreaming record id is %d\n",
		     webstreaming_record._id);

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_delete_streaming(MediaSvcHandle *mb_svc_handle, int streaming_id)
{
	int ret = -1;
	mb_svc_web_streaming_record_s webstreaming_record = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_delete_streaming#streaming_id: %d", streaming_id);

	ret =
	    mb_svc_get_web_streaming_record_by_id(mb_svc_handle, streaming_id,
						  &webstreaming_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_streaming, get webstreaming record failed\n");
		return ret;
	}
	/* delete webstreaming record */
	ret = mb_svc_delete_record_web_streaming_by_id(mb_svc_handle, webstreaming_record._id);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_streaming, delete webstreaming record by _id failed\n");
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_thumb_path(MediaSvcHandle *mb_svc_handle, const char *file_url, char *thumb_path,
		     size_t max_thumb_path)
{
	int err = -1;
	char media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	Mitem *item = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL || thumb_path == NULL || max_thumb_path <= 0) {
		mb_svc_debug
		    ("file_url == NULL || thumb_path == NULL || max_thumb_path <= 0 \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = mb_svc_get_media_id_by_full_path(mb_svc_handle, file_url, media_uuid);

	if (err < 0) {
		mb_svc_debug("There is no ( %s ) file in DB", file_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	item = minfo_media_item_new(mb_svc_handle, media_uuid, NULL);

	if (item == NULL) {
		mb_svc_debug("minfo_media_item_new fails: %s\n", file_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	} else {
		mb_svc_debug("thumb path :%s", item->thumb_url);
		if (strlen(item->thumb_url) == 0) {
			thumb_path[0] = '\0';
		} else {
			strncpy(thumb_path, item->thumb_url, max_thumb_path);
		}
	}

	minfo_destroy_mtype_item(item);

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_thumb_path_for_video(MediaSvcHandle *mb_svc_handle, const char *file_url, char *thumb_path,
			       size_t max_thumb_path)
{
	int err = -1;
	char media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	Mitem *item = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL || thumb_path == NULL || max_thumb_path <= 0) {
		mb_svc_debug
		    ("file_url == NULL || thumb_path == NULL || max_thumb_path <= 0 \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = mb_svc_get_media_id_by_full_path(mb_svc_handle, file_url, media_uuid);

	if (err < 0) {
		mb_svc_debug("There is no ( %s ) file in DB", file_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	item = minfo_media_item_new(mb_svc_handle, media_uuid, NULL);

	if (item == NULL) {
		mb_svc_debug("minfo_media_item_new fails: %s\n", file_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	} else {
		mb_svc_debug("thumb path :%s", item->thumb_url);
		strncpy(thumb_path, item->thumb_url, max_thumb_path);
	}

	minfo_destroy_mtype_item(item);
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_destroy_mtype_item(void *item)
{
	if (item == NULL) {
		mb_svc_debug("item == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (IS_MINFO_MITEM(item)) {
		minfo_mitem_destroy((Mitem *) item);
	} else if (IS_MINFO_MMETA(item)) {
		minfo_mmeta_destroy((Mmeta *) item);
	} else if (IS_MINFO_MBOOKMARK(item)) {
		minfo_mbookmark_destroy((Mbookmark *) item);
	} else if (IS_MINFO_MCLUSTER(item)) {
		minfo_mcluster_destroy((Mcluster *) item);
	} else if (IS_MINFO_MSTREAMING(item)) {
		minfo_mstreaming_destroy((Mstreaming *) item);
	} else if (IS_MINFO_MTAG(item)) {
		minfo_media_tag_destroy((Mtag *) item);
	} else {
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_check_cluster_exist(MediaSvcHandle *mb_svc_handle, const char *path)
{
	mb_svc_debug("");
	int err = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (path == NULL) {
		mb_svc_debug("path is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = mb_svc_check_exist_by_path(mb_svc_handle, path, MB_SVC_TBL_NAME_FOLDER);
	if (err < 0) {
		mb_svc_debug("mb_svc_check_exist_by_path failed : %d", err);
		return err;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_check_item_exist(MediaSvcHandle *mb_svc_handle, const char *path)
{
	mb_svc_debug("");
	int err = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (path == NULL) {
		mb_svc_debug("path is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = mb_svc_check_exist_by_path(mb_svc_handle, path, MB_SVC_TBL_NAME_MEDIA);
	if (err < 0) {
		mb_svc_debug("mb_svc_check_exist_by_path failed : %d", err);
		return err;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_item_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, Mitem **mitem)
{
	Mitem *item = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	item = minfo_media_item_new(mb_svc_handle, media_id, NULL);
	if (item == NULL) {
		mb_svc_debug("minfo_mitem_new: %s\n", media_id);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	*mitem = item;
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_item(MediaSvcHandle *mb_svc_handle, const char *file_url, Mitem ** mitem)
{
	int ret = 0;
	Mitem *item = NULL;
	char _media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (file_url == NULL) {
		mb_svc_debug("file_url == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	mb_svc_debug("minfo_get_item#file_url: %s", file_url);
	ret = mb_svc_get_media_id_by_full_path(mb_svc_handle, file_url, _media_uuid);

	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_id_by_full_path fails:%s\n",
			     file_url);
		return ret;
	}

	item = minfo_media_item_new(mb_svc_handle, _media_uuid, NULL);

	if (item == NULL) {
		mb_svc_debug("minfo_mitem_new: %s\n", file_url);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	*mitem = item;
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_item_by_http_url(MediaSvcHandle *mb_svc_handle, const char *http_url, Mitem ** mitem)
{
	int ret = 0;
	char media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	Mitem *item = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (http_url == NULL) {
		mb_svc_debug("http_url == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_get_item_by_http_url#http_url: %s", http_url);

	ret = mb_svc_get_media_id_by_http_url(mb_svc_handle, http_url, media_uuid);

	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_id_by_http_url fails:%s\n",
			     http_url);
		return ret;
	}

	item = minfo_media_item_new(mb_svc_handle, media_uuid, NULL);

	if (item == NULL) {
		mb_svc_debug("minfo_mitem_new: %s\n", http_url);
		return MB_SVC_ERROR_INTERNAL;
	}

	*mitem = item;
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_cluster(MediaSvcHandle *mb_svc_handle,
			const char *cluster_url,
			const char *cluster_id,
			Mcluster **mcluster)
{
	int ret = 0;
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	char *_uuid = NULL;
	Mcluster *cluster = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (*mcluster != NULL) {
		mb_svc_debug("cluster_url == NULL || *mcluster != NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_url != NULL) {
		mb_svc_debug("minfo_get_item#file_url: %d", cluster_url);
		ret =
		    mb_svc_get_folder_id_by_full_path(mb_svc_handle, cluster_url, folder_uuid, sizeof(folder_uuid));
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_get_folder_id_by_full_path fails:%s\n",
			     cluster_url);
			return ret;
		}

		_uuid = folder_uuid;
	} else if (cluster_id != NULL) {
		strncpy(folder_uuid, cluster_id, MB_SVC_UUID_LEN_MAX + 1);
		_uuid = folder_uuid;
	}

	cluster = minfo_mcluster_new(mb_svc_handle, _uuid);
	if (cluster == NULL) {
		mb_svc_debug("minfo_mcluster_new: %s\n", folder_uuid);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	*mcluster = cluster;
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_get_cluster_id_by_url(MediaSvcHandle *mb_svc_handle, const char *url, char *cluster_id, int max_length)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	return mb_svc_get_folder_id_by_full_path(mb_svc_handle, url, cluster_id, max_length);
}

EXPORT_API int
minfo_get_cluster_name_by_id(MediaSvcHandle *mb_svc_handle, const char *cluster_id, char *cluster_name, int max_length)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	return mb_svc_get_folder_name_by_id(mb_svc_handle, cluster_id, cluster_name,
					    max_length);
}

EXPORT_API int
minfo_get_cluster_fullpath_by_id(MediaSvcHandle *mb_svc_handle, const char *cluster_id, char *folder_path,
				 int max_length)
{
	int ret = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (folder_path == NULL) {
		mb_svc_debug("folder_path: NULL!\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret =
	    mb_svc_get_folder_fullpath_by_folder_id(mb_svc_handle, cluster_id, folder_path,
						    max_length);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_get_folder_fullpath_by_folder_id fail %d\n", ret);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_set_cluster_lock_status(MediaSvcHandle *mb_svc_handle, const char *cluster_id, int lock_status)
{
	mb_svc_debug("");
	int ret = 0;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret =
	    mb_svc_update_album_lock_status(mb_svc_handle, cluster_id, lock_status,
					    MINFO_PHONE);
	return ret;
}

EXPORT_API int
minfo_get_cluster_lock_status(MediaSvcHandle *mb_svc_handle, const char *cluster_id, int *lock_status)
{
	mb_svc_debug("");
	int ret = 0;
	mb_svc_folder_record_s record;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_id == NULL) {
		mb_svc_debug("cluster_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (lock_status == NULL) {
		mb_svc_debug("password is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_folder_record_by_id(mb_svc_handle, cluster_id, &record);

	if (ret != 0) {
		mb_svc_debug("minfo_get_cluster_lock_status fails");
		return ret;
	}

	*lock_status = record.lock_status;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_media_path(
			minfo_store_type storage_type,
			char *media_path,
			size_t max_media_path)
{
	mb_svc_debug("");
	int len = 0;

	if (media_path == NULL) {
		mb_svc_debug("Passed media path is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type == MINFO_PHONE) {
		if (max_media_path < strlen(MB_SVC_PATH_PHONE) + 1) {
			mb_svc_debug("max_media_path is too short");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	} else if (storage_type == MINFO_MMC) {
		if (max_media_path < strlen(MB_SVC_PATH_MMC) + 1) {
			mb_svc_debug("max_media_path is too short");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (storage_type == MINFO_PHONE) {
		len = strlen(MB_SVC_PATH_PHONE) + 1;
		if (len > max_media_path) {
			mb_svc_debug("String length violation");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		strncpy(media_path, MB_SVC_PATH_PHONE, max_media_path);
		media_path[max_media_path - 1] = '\0';
	} else if (storage_type == MINFO_MMC) {
		len = strlen(MB_SVC_PATH_MMC) + 1;
		if (len > max_media_path) {
			mb_svc_debug("String length violation");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		strncpy(media_path, MB_SVC_PATH_MMC, max_media_path);
		media_path[max_media_path - 1] = '\0';
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_set_db_valid(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type, int valid)
{
	mb_svc_debug("storage:%d", storage_type);
	int ret;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != MINFO_PHONE && storage_type != MINFO_MMC) {
		mb_svc_debug("storage type should be phone or mmc");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (valid != 0 && valid != 1) {
		mb_svc_debug("valid should be 0 or 1");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_set_media_records_as_valid(mb_svc_handle, storage_type, valid);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_set_media_records_as_valid failed..Now Start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_set_item_valid_start(MediaSvcHandle *mb_svc_handle, int trans_count)
{
	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("Transaction count : %d", trans_count);
	g_trans_valid_cnt = trans_count;
	g_cur_trans_valid_cnt = 0;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_set_item_valid_end(MediaSvcHandle *mb_svc_handle)
{
	mb_svc_debug("");

	if (g_cur_trans_valid_cnt > 0) {
		int ret = -1;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");

			g_cur_trans_valid_cnt = 0;
			g_trans_valid_cnt = 1;

			return ret;
		}

		ret = mb_svc_set_item_as_valid(mb_svc_handle);

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);

			g_cur_trans_valid_cnt = 0;
			g_trans_valid_cnt = 1;

			return ret;
		}
	}

	g_cur_trans_valid_cnt = 0;
	g_trans_valid_cnt = 1;

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_set_item_valid(MediaSvcHandle *mb_svc_handle,
				const minfo_store_type storage_type,
				const char *full_path,
				int valid)
{
	mb_svc_debug("storage:%d", storage_type);
	int ret;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != MINFO_PHONE && storage_type != MINFO_MMC) {
		mb_svc_debug("storage type should be phone or mmc");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (g_cur_trans_valid_cnt < g_trans_valid_cnt) {
		ret = mb_svc_set_item_as_valid_sql_add(mb_svc_handle, full_path, valid);
		if (ret < 0) {
			mb_svc_debug("mb_svc_set_item_as_valid_sql_add failed\n");
			return ret;
		}

		g_cur_trans_valid_cnt++;

		return MB_SVC_ERROR_NONE;
	}

	if (g_cur_trans_valid_cnt == g_trans_valid_cnt) {
		ret = mb_svc_set_item_as_valid_sql_add(mb_svc_handle, full_path, valid);
		if (ret < 0) {
			mb_svc_debug("mb_svc_set_item_as_valid_sql_add failed\n");
			return ret;
		}

		g_cur_trans_valid_cnt = 0;

		ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			return ret;
		}

		ret = mb_svc_set_item_as_valid(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_set_item_as_valid failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}

		ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans(mb_svc_handle);
			return ret;
		}
 	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_delete_all_media_records(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type)
{
	mb_svc_debug("storage:%d", storage_type);
	int ret;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != MINFO_PHONE && storage_type != MINFO_MMC) {
		mb_svc_debug("storage type should be phone or mmc");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	start = mediainfo_get_debug_time();
#endif

	ret = mb_svc_delete_all_media_records(mb_svc_handle, storage_type);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_delete_all_media_records failed..: %d", ret);
		return ret;
	}

#ifdef _PERFORMANCE_CHECK_
	end = mediainfo_get_debug_time();
	mediainfo_print_debug_time_ex(start, end, __FUNCTION__, "time");
#endif

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_delete_invalid_media_records(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type)
{
	mb_svc_debug("storage:%d", storage_type);
	int ret;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (storage_type != MINFO_PHONE && storage_type != MINFO_MMC) {
		mb_svc_debug("storage type should be phone or mmc");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

#ifdef _PERFORMANCE_CHECK_
	long start = 0L, end = 0L;
	start = mediainfo_get_debug_time();
#endif

	ret = mb_svc_delete_invalid_media_records(mb_svc_handle, storage_type);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_delete_invalid_media_records failed.: %d", ret);
		return ret;
	}

#ifdef _PERFORMANCE_CHECK_
	end = mediainfo_get_debug_time();
	mediainfo_print_debug_time_ex(start, end, __FUNCTION__, "time");
#endif

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_delete_tag(MediaSvcHandle *mb_svc_handle, const char *media_id, const char *tag_name)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_delete_tag#tag_name: %s!", tag_name);
#ifdef _PERFORMANCE_CHECK_
	long start = mediainfo_get_debug_time();
#endif
	ret = mb_svc_sqlite3_begin_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	ret = mb_svc_delete_record_tag(mb_svc_handle, tag_name, media_id);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_delete_record_tag fail..Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

	ret = mb_svc_sqlite3_commit_trans(mb_svc_handle);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans(mb_svc_handle);
		return ret;
	}

#ifdef _PERFORMANCE_CHECK_
	long end = mediainfo_get_debug_time();

	double tag = ((double)(end - start) / (double)CLOCKS_PER_SEC);
	mb_svc_debug("Delete Tag : %f", tag);
#endif


	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_rename_tag(MediaSvcHandle *mb_svc_handle, const char *src_tagname, const char *dst_tag_name)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (src_tagname == NULL || dst_tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (strlen(src_tagname) > MB_SVC_ARRAY_LEN_MAX
	    || strlen(dst_tag_name) > MB_SVC_ARRAY_LEN_MAX) {
		mb_svc_debug("tag_name length is violation!");
		return MB_SVC_ERROR_INTERNAL;
	}

	char *src_tag_escape = NULL;
	char *dst_tag_escape = NULL;

	src_tag_escape = sqlite3_mprintf("%q", src_tagname);
	dst_tag_escape = sqlite3_mprintf("%q", dst_tag_name);

	mb_svc_debug("minfo_rename_tag#src_tagname: %s!", src_tagname);
	mb_svc_debug("minfo_rename_tag#dst_tag_name: %s!", dst_tag_name);

	ret = mb_svc_rename_record_tag(mb_svc_handle, src_tagname, dst_tag_name);

	sqlite3_free(src_tag_escape);
	sqlite3_free(dst_tag_escape);

	if (ret < 0) {
		mb_svc_debug("mb_svc_rename_record_tag fail: %d!\n", ret);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_rename_tag_by_id(MediaSvcHandle *mb_svc_handle,
				const char *media_id,
				const char *src_tagname,
				const char *dst_tag_name)
{
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (src_tagname == NULL || dst_tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (strlen(src_tagname) > MB_SVC_ARRAY_LEN_MAX
	    || strlen(dst_tag_name) > MB_SVC_ARRAY_LEN_MAX) {
		mb_svc_debug("tag_name length is violation!");
		return MB_SVC_ERROR_INTERNAL;
	}

	char *src_tag_escape = NULL;
	char *dst_tag_escape = NULL;
	src_tag_escape = sqlite3_mprintf("%q", src_tagname);
	dst_tag_escape = sqlite3_mprintf("%q", dst_tag_name);

	mb_svc_debug("src_tagname: %s!", src_tagname);
	mb_svc_debug("dst_tag_name: %s!", dst_tag_name);

	ret =
	    mb_svc_rename_record_tag_by_id(mb_svc_handle, media_id, src_tagname, dst_tag_name);

	sqlite3_free(src_tag_escape);
	sqlite3_free(dst_tag_escape);

	if (ret < 0) {
		mb_svc_debug("mb_svc_rename_record_tag fail: %d!\n", ret);
		return ret;
	}

	return MB_SVC_ERROR_NONE;
}

EXPORT_API int minfo_add_tag(MediaSvcHandle *mb_svc_handle, const char *media_id, const char *tag_name)
{
	int ret = -1;
	bool tag_exist = FALSE;
	mb_svc_tag_record_s tag_record = { 0 };

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

#ifdef _PERFORMANCE_CHECK_
	long start = mediainfo_get_debug_time();
#endif

	mb_svc_debug("minfo_add_tag#media_id: %s!", media_id);
	mb_svc_debug("minfo_add_tag#tag_name: %s!", tag_name);

	if ((tag_record._id = mb_svc_get_tagid_by_tagname(mb_svc_handle, tag_name)) > 0) {
		mb_svc_debug("This tagname %s is exist");
		tag_exist = TRUE;
	}

	strncpy(tag_record.tag_name, tag_name, MB_SVC_ARRAY_LEN_MAX + 1);

	if (!tag_exist) {
		ret = mb_svc_insert_record_tag(mb_svc_handle, &tag_record);
		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_tag fail\n");
			return ret;
		}
	}

	if (media_id != NULL) {
		ret =
		    mb_svc_insert_record_tag_map(mb_svc_handle, media_id,
						 tag_record._id);
		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_tag_map fail\n");
			return ret;
		}
	}

#ifdef _PERFORMANCE_CHECK_
		long end = mediainfo_get_debug_time();

		double tag = ((double)(end - start) / (double)CLOCKS_PER_SEC);
		mb_svc_debug("Insert Tag : %f", tag);
#endif
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_media_list_by_tagname(MediaSvcHandle *mb_svc_handle,
				const char *tag_name,
				bool with_meta,
				minfo_item_ite_cb func,
				void *user_data)
{
	mb_svc_debug("");
	int err = 0;
	int record_cnt = 0;
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	mb_svc_tag_record_s tag_record = { 0 };
	Mitem *mitem = NULL;
	char *tag_name_escape_char = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("tag_name:%s", tag_name);
	tag_name_escape_char = sqlite3_mprintf("%q", tag_name);
	err = mb_svc_tag_iter_start(mb_svc_handle, tag_name_escape_char, NULL, &mb_svc_iterator);
	sqlite3_free(tag_name_escape_char);

	if (err < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		/* improve the performance of getting meida list. */
		err =
		    mb_svc_media_id_list_by_tag_iter_next(&mb_svc_iterator,
							  &tag_record);

		if (err == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (err < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return err;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, tag_record.media_uuid, NULL);
		if (with_meta && mitem) {
			mitem->meta_info = minfo_mmeta_new(mb_svc_handle, mitem->uuid, NULL);
		}

		if (mitem) {
			func(mitem, user_data);
		}
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_media_list_by_tagname_with_filter(MediaSvcHandle *mb_svc_handle,
						const char *tag_name,
						minfo_tag_filter filter,
						minfo_item_ite_cb func,
						void *user_data)
{
	mb_svc_debug("");
	int err = 0;
	int record_cnt = 0;
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	mb_svc_tag_record_s tag_record = { 0 };
	Mitem *mitem = NULL;
	char *tag_name_escape_char = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tag_name == NULL) {
		mb_svc_debug("tag_name is NULL!");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("tag_name:%s", tag_name);
	tag_name_escape_char = sqlite3_mprintf("%q", tag_name);
	err =
	    mb_svc_tag_iter_with_filter_start(mb_svc_handle, tag_name_escape_char, filter,
					      &mb_svc_iterator);
	sqlite3_free(tag_name_escape_char);

	if (err < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		/* improve the performance of getting meida list. */
		err =
		    mb_svc_media_id_list_by_tag_iter_next(&mb_svc_iterator,
							  &tag_record);

		if (err == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (err < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return err;
		}

		record_cnt++;

		mitem = minfo_media_item_new(mb_svc_handle, tag_record.media_uuid, NULL);
		if (filter.with_meta && mitem) {
			mitem->meta_info = minfo_mmeta_new(mb_svc_handle, mitem->uuid, NULL);
		}

		if (mitem) {
			func(mitem, user_data);
		}
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_media_count_by_tagname(MediaSvcHandle *mb_svc_handle, const char *tag_name, int *count)
{
	mb_svc_debug("");
	int ret = -1;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tag_name == NULL || count == NULL) {
		mb_svc_debug("tag_name == NULL || count == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_media_count_by_tagname(mb_svc_handle, tag_name, count);
	if (ret < 0) {
		mb_svc_debug
		    ("Error: mb_svc_get_media_count_by_tagname failed\n");
		return ret;
	}

	mb_svc_debug("record count = %d", *count);
	return MB_SVC_ERROR_NONE;
}

EXPORT_API int
minfo_get_tag_list_by_media_id(MediaSvcHandle *mb_svc_handle,
					const char *media_id,
					minfo_tag_ite_cb func,
					void *user_data)
{
	mb_svc_debug("");
	int err = 0;
	int record_cnt = 0;
	mb_svc_iterator_s mb_svc_iterator = { 0 };
	mb_svc_tag_record_s tag_record = { 0 };
	Mtag *i_tag = NULL;

	if (mb_svc_handle == NULL) {
		mb_svc_debug("media service handle is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (func == NULL) {
		mb_svc_debug("Func is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = mb_svc_tag_iter_start(mb_svc_handle, NULL, media_id, &mb_svc_iterator);

	if (err < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		/* improve the performance of getting meida list. */
		err = mb_svc_tag_iter_next(&mb_svc_iterator, &tag_record);
		if (err == MB_SVC_NO_RECORD_ANY_MORE)
			break;

		if (err < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			return err;
		}

		record_cnt++;

		i_tag = minfo_media_tag_new(mb_svc_handle, -1, &tag_record);
		if (i_tag) {
			err =
			    mb_svc_get_media_count_by_tagname(mb_svc_handle, i_tag->tag_name,
							      &(i_tag->count));
			if (err < 0) {
				mb_svc_debug
				    ("mb_svc_get_media_count_by_tagname fails : %s",
				     i_tag->tag_name);
				minfo_destroy_mtype_item( i_tag );
			} else {
				mb_svc_debug("Tagname : %s  [%d]",
					     i_tag->tag_name, i_tag->count);
				func(i_tag, user_data);
			}
		}
	}

	mb_svc_iter_finish(&mb_svc_iterator);

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return MB_SVC_ERROR_NONE;
}
