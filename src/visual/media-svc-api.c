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
#include "media-info-debug.h"
#include "media-svc-api.h"
#include "media-svc-thumb.h"
#include "media-svc-db-util.h"
#include "media-svc-debug.h"
#include "media-svc-util.h"
#include "media-svc-db.h"
#include "media-info-util.h"
#include "minfo-types.h"
#include "media-svc-error.h"
#include <glib.h>
#include <glib-object.h>
#include <drm-service.h>
#include <dirent.h>
#include <sys/types.h>

#ifdef _PERFORMANCE_CHECK_
double g_insertdb = 0;
double g_thumb = 0;
double g_meta = 0;
long start = 0L, end = 0L;
#endif

static __thread GList *g_insert_sql_list = NULL;

const char *mb_svc_media_order[5] = {
	"uuid ASC",
	"display_name COLLATE NOCASE DESC",
	"display_name COLLATE NOCASE ASC",
	"modified_date DESC",
	"modified_date ASC",
};

const char *mb_svc_folder_order[5] = {
	"uuid ASC",
	"folder_name COLLATE NOCASE DESC",
	"folder_name COLLATE NOCASE ASC",
	"modified_date DESC",
	"modified_date ASC",
};

typedef struct {
	DIR* dp;
	struct dirent* entry;
	char dir_full_path[MB_SVC_DIR_PATH_LEN_MAX+1];
} mb_svc_sync_param_s;

#define MB_SVC_DB_DEFAULT_GET_ALL_RECORDS -1	/* get all records, not limit on start position */
#define MB_SVC_DB_GET_UNTIL_LAST_RECORD -1	/* get all records until last item */

static int __mb_svc_folder_by_path_iter_start(char *parent_path, mb_svc_iterator_s *mb_svc_iterator);
static int __mb_svc_get_folder_record_by_path_info(const char *uri, char *display_name, minfo_store_type storage_type, mb_svc_folder_record_s *record);
static int __mb_svc_get_folder_record_by_full_path(const char *folder_full_path, mb_svc_folder_record_s *folder_record);
static int __mb_svc_get_media_id_by_fid_name(const char *folder_id, char *display_name, char *media_id);
static int __mb_svc_get_media_list_by_folder_id(const char *folder_id, GList **p_record_list, int valid);
static int __mb_svc_delete_media_records_list(GList *p_record_list);
static int __mb_svc_delete_tag_by_id(const int tag_id);
static int __mb_svc_update_tag(int tag_id, const char *tag_name);
static int __mb_svc_update_tagmap(int src_tag_id, int dst_tag_id);
static int __mb_svc_update_tagmap_by_media_id(const char *media_id, int src_tag_id, int dst_tag_id);
static int __mb_svc_get_media_cnt_by_tagid(int tag_id);

int mb_svc_insert_items()
{
	mb_svc_debug("");
	int i = 0;
	int length = g_list_length(g_insert_sql_list);

	for (i = 0; i < length; i++) {
		char *sql = (char*)g_list_nth_data(g_insert_sql_list, i);
		mb_svc_query_sql(sql);
	}

	mb_svc_sql_list_release(&g_insert_sql_list);

	return 0;
}


int
mb_svc_insert_file_batch(const char *file_full_path, minfo_file_type content_type)
{
	char dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dir_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	mb_svc_folder_record_s folder_record = {"",};
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_record = {0,};
	mb_svc_video_meta_record_s video_record = {0,};
	int folder_modified_date = 0;
	int ret = 0;
	int store_type = 0;
	bool is_drm = false;
	
	char *folder_sql = NULL;
	char *insert_sql = NULL;
	char *meta_sql = NULL;

	if (file_full_path == NULL ) {
		mb_svc_debug("file_full_path == NULL || thumb_path == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("file_full_path is %s\n", file_full_path);
	/* 1. get file detail */
	_mb_svc_get_file_parent_path(file_full_path, dir_path);
	mb_svc_debug("dir_path is %s\n", dir_path);

	folder_modified_date = _mb_svc_get_file_dir_modified_date(dir_path);
	mb_svc_debug("folder_modified_date is %d\n", folder_modified_date);

	/* 2. insert or update folder table */
	ret = __mb_svc_get_folder_record_by_full_path(dir_path, &folder_record);
	if (ret < 0) {
		mb_svc_debug("no any record in %s", dir_path);
		store_type = _mb_svc_get_store_type_by_full(file_full_path);

		if (store_type == MB_SVC_ERROR_INTERNAL) {
			mb_svc_debug("Failed to get storage type : %s",
				     file_full_path);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
		mb_svc_debug("store_type is %d\n", store_type);

		_mb_svc_get_dir_display_name(dir_path, dir_display_name);
		mb_svc_debug("dir_display_name is %s\n", dir_display_name);

		folder_record.modified_date = folder_modified_date;
		folder_record.storage_type = store_type;
		strncpy(folder_record.uri, dir_path, MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(folder_record.display_name, dir_display_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		folder_record.lock_status = 0;

		mb_svc_debug
		    ("no record in %s, ready insert the folder record into db\n",
		     dir_path);

		ret = mb_svc_insert_record_folder(&folder_record);
		if (ret < 0) {
			mb_svc_debug
				("mb_svc_insert_record_folder failed (%d)\n", ret);
			return ret;
		}
	} else {
		/* there is already matched folder record in folder table, update it if the file is modified */
		mb_svc_debug
		    ("folder_record info: uuid is %s,uri is %s,display_name is %s,modified_date is %d,web_account_id is %s,store_type is %d\n",
		     folder_record.uuid, folder_record.uri,
		     folder_record.display_name, folder_record.modified_date,
		     folder_record.web_account_id, folder_record.storage_type);
		mb_svc_debug
		    ("existing file date in folder record is %d, new file date is %d",
		     folder_record.modified_date, folder_modified_date);

		store_type = folder_record.storage_type;
		if (folder_record.modified_date < folder_modified_date)	{ /* the file is updated */
			mb_svc_debug("directory %s is modified", dir_path);
			folder_record.modified_date = folder_modified_date;
			ret = mb_svc_update_record_folder_sql(&folder_record, &folder_sql);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_update_record_folder_sql fails(%d)",
				     ret);
			}

			mb_svc_sql_list_add(&g_insert_sql_list, &folder_sql);
		}
	}

	/* 3. insert media table, before insert a record, check whether the same record has existed.     */
	mb_svc_debug("ready insert file info into media table\n");
	mb_svc_debug("file_full_path is %s\n", file_full_path);

	strncpy(media_record.path, file_full_path, sizeof(media_record.path));
	_mb_svc_get_file_display_name(file_full_path, file_display_name);
	mb_svc_debug("file_display_name is %s\n", file_display_name);
	strncpy(media_record.display_name, file_display_name,
		MB_SVC_FILE_NAME_LEN_MAX + 1);

	media_record.content_type = content_type;
	media_record.rate = 0;
	mb_svc_debug
	    ("ready get file date for insert file info into media table\n");
	media_record.modified_date =
	    _mb_svc_get_file_dir_modified_date(file_full_path);
	is_drm = (drm_svc_is_drm_file(file_full_path) == DRM_TRUE);

	ret =
		_mb_svc_thumb_generate_hash_name(file_full_path,
								media_record.thumbnail_path,
								MB_SVC_FILE_PATH_LEN_MAX + 1);
	
	if (ret < 0) {
		mb_svc_debug("_mb_svc_thumb_generate_hash_name failed : %d", ret);
		return ret;
	}

	/* 4. if it's image file, insert into image_meta table */
	if (media_record.content_type == MINFO_ITEM_IMAGE)	{ /* it's image file, insert into image_meta table */
		mb_svc_debug
		    ("ready insert file info into media table,file date is %d\n",
		     media_record.modified_date);
		
		bool thumb_done = FALSE;
		ret = mb_svc_get_image_meta(file_full_path, &image_record, &thumb_done);

		if (ret < 0) {
			mb_svc_debug("mb_svc_get_image_meta failed\n");
			return ret;
		}

		strncpy(media_record.folder_uuid, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = mb_svc_insert_record_media_sql(&media_record, store_type, &insert_sql);

		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_media_sql fails(%d)",
				     ret);
			return ret;
		}
		
		mb_svc_sql_list_add(&g_insert_sql_list, &insert_sql);

		strncpy(image_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_image_meta_sql(&image_record, store_type, &meta_sql);

		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_image_meta_sql failed(%d)\n", ret);
			return ret;
		}

		mb_svc_sql_list_add(&g_insert_sql_list, &meta_sql);
	}
	/* 5       if video, insert video_meta and bookmark table */
	else if (media_record.content_type == MINFO_ITEM_VIDEO) {	/* it's video file, insert into vidoe table */
		mb_svc_debug
		    ("ready insert file info into media table,file date is %d\n",
		     media_record.modified_date);

		video_record.last_played_time = 0;
		video_record.latitude = 0.0;
		video_record.longitude = 0.0;

		ret = mb_svc_get_video_meta(file_full_path, &video_record);
		if (ret < 0) {
			mb_svc_debug("mb_svc_get_video_meta failed\n");
			return ret;
		}

		strncpy(media_record.folder_uuid, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = mb_svc_insert_record_media_sql(&media_record, store_type, &insert_sql);
		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_media_sql fails(%d)",
				     ret);
			return ret;
		}
		mb_svc_sql_list_add(&g_insert_sql_list, &insert_sql);

		strncpy(video_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_video_meta_sql(&video_record, store_type, &meta_sql);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_insert_record_video_meta_sql failed(%d)\n", ret);
			return ret;
		}
		mb_svc_sql_list_add(&g_insert_sql_list, &meta_sql);
	}

	return 0;
}

int
mb_svc_insert_file(const char *file_full_path, minfo_file_type content_type)
{
	char dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dir_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	mb_svc_folder_record_s folder_record = {"",};
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_record = {0,};
	mb_svc_video_meta_record_s video_record = {0,};
	int folder_modified_date = 0;
	int ret = 0;
	int store_type = 0;
	bool is_drm = false;
	GList *insert_sql_list = NULL;
	char *folder_sql = NULL;
	char *insert_sql = NULL;
	char *meta_sql = NULL;
	int insert_new_folder = 0;

	if (file_full_path == NULL ) {
		mb_svc_debug("file_full_path == NULL || thumb_path == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("file_full_path is %s\n", file_full_path);
	/* 1. get file detail */
	_mb_svc_get_file_parent_path(file_full_path, dir_path);
	mb_svc_debug("dir_path is %s\n", dir_path);

	folder_modified_date = _mb_svc_get_file_dir_modified_date(dir_path);
	mb_svc_debug("folder_modified_date is %d\n", folder_modified_date);

	/* 2. insert or update folder table */
	ret = __mb_svc_get_folder_record_by_full_path(dir_path, &folder_record);
	if (ret < 0) {
		mb_svc_debug("no any record in %s", dir_path);
		store_type = _mb_svc_get_store_type_by_full(file_full_path);

		if (store_type == MB_SVC_ERROR_INTERNAL) {
			mb_svc_debug("Failed to get storage type : %s",
				     file_full_path);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
		mb_svc_debug("store_type is %d\n", store_type);

		_mb_svc_get_dir_display_name(dir_path, dir_display_name);
		mb_svc_debug("dir_display_name is %s\n", dir_display_name);

		folder_record.modified_date = folder_modified_date;
		folder_record.storage_type = store_type;
		strncpy(folder_record.uri, dir_path, MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(folder_record.display_name, dir_display_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		folder_record.lock_status = 0;

		mb_svc_debug
		    ("no record in %s, ready insert the folder record into db\n",
		     dir_path);

		insert_new_folder = 1;
	} else {
		/* there is already matched folder record in folder table, update it if the file is modified */
		mb_svc_debug
		    ("folder_record info: uuid is %s,uri is %s,display_name is %s,modified_date is %d,web_account_id is %s,store_type is %d\n",
		     folder_record.uuid, folder_record.uri,
		     folder_record.display_name, folder_record.modified_date,
		     folder_record.web_account_id, folder_record.storage_type);
		mb_svc_debug
		    ("existing file date in folder record is %d, new file date is %d",
		     folder_record.modified_date, folder_modified_date);

		store_type = folder_record.storage_type;
		if (folder_record.modified_date < folder_modified_date)	{ /* the file is updated */
			mb_svc_debug("directory %s is modified", dir_path);
			folder_record.modified_date = folder_modified_date;
			ret = mb_svc_update_record_folder_sql(&folder_record, &folder_sql);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_update_record_folder_sql fails(%d)",
				     ret);
			}

			mb_svc_sql_list_add(&insert_sql_list, &folder_sql);
		}
	}

	/* 3. insert media table, before insert a record, check whether the same record has existed.     */
	mb_svc_debug("ready insert file info into media table\n");
	mb_svc_debug("file_full_path is %s\n", file_full_path);

	strncpy(media_record.path, file_full_path, sizeof(media_record.path));
	_mb_svc_get_file_display_name(file_full_path, file_display_name);
	mb_svc_debug("file_display_name is %s\n", file_display_name);
	strncpy(media_record.display_name, file_display_name,
		MB_SVC_FILE_NAME_LEN_MAX + 1);

	media_record.content_type = content_type;
	media_record.rate = 0;
	mb_svc_debug
	    ("ready get file date for insert file info into media table\n");
	media_record.modified_date =
	    _mb_svc_get_file_dir_modified_date(file_full_path);
	is_drm = (drm_svc_is_drm_file(file_full_path) == DRM_TRUE);

	/* 4. if it's image file, insert into image_meta table */
	if (media_record.content_type == MINFO_ITEM_IMAGE)	{ /* it's image file, insert into image_meta table */
		mb_svc_debug
		    ("ready insert file info into media table,file date is %d\n",
		     media_record.modified_date);

#ifdef _PERFORMANCE_CHECK_
		start = mediainfo_get_debug_time();
#endif
		
		bool thumb_done = FALSE;
		ret = mb_svc_get_image_meta(file_full_path, &image_record, &thumb_done);

#ifdef _PERFORMANCE_CHECK_
		end = mediainfo_get_debug_time();

		g_meta += ((double)(end - start) / (double)CLOCKS_PER_SEC);
		mb_svc_debug("Meta : %f", g_meta);
#endif

		if (ret < 0) {
			mb_svc_debug("mb_svc_get_image_meta failed\n");
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}

		ret = mb_svc_sqlite3_begin_trans();
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}

		if (insert_new_folder == 1) {
			
			ret = mb_svc_insert_record_folder_sql(&folder_record, &folder_sql);
			if (ret < 0) {
				mb_svc_debug
					("mb_svc_insert_record_folder_sql failed (%d)\n", ret);
				return ret;
			}

			mb_svc_sql_list_add(&insert_sql_list, &folder_sql);
		}

		strncpy(media_record.folder_uuid, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = mb_svc_insert_record_media_sql(&media_record, store_type, &insert_sql);

		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_media_sql fails(%d)",
				     ret);
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}
		
		mb_svc_sql_list_add(&insert_sql_list, &insert_sql);

		strncpy(image_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_image_meta_sql(&image_record, store_type, &meta_sql);

		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_image_meta_sql failed(%d)\n", ret);
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}

		mb_svc_sql_list_add(&insert_sql_list, &meta_sql);
	}

	/* 5       if video, insert video_meta and bookmark table */
	else if (media_record.content_type == MINFO_ITEM_VIDEO) {	/* it's video file, insert into vidoe table */

		mb_svc_debug
		    ("ready insert file info into media table,file date is %d\n",
		     media_record.modified_date);

		video_record.last_played_time = 0;
		video_record.latitude = 0.0;
		video_record.longitude = 0.0;

		ret = mb_svc_get_video_meta(file_full_path, &video_record);
		if (ret < 0) {
			mb_svc_debug("mb_svc_get_video_meta failed\n");
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}

		ret = mb_svc_sqlite3_begin_trans();
		if (ret < 0) {
			mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}

		if (insert_new_folder == 1) {
			
			ret = mb_svc_insert_record_folder_sql(&folder_record, &folder_sql);
			if (ret < 0) {
				mb_svc_debug
					("mb_svc_insert_record_folder_sql failed (%d)\n", ret);
				return ret;
			}

			mb_svc_sql_list_add(&insert_sql_list, &folder_sql);
		}

		strncpy(media_record.folder_uuid, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret = mb_svc_insert_record_media_sql(&media_record, store_type, &insert_sql);
		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_media_sql fails(%d)",
				     ret);
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}
		mb_svc_sql_list_add(&insert_sql_list, &insert_sql);

		strncpy(video_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_video_meta_sql(&video_record, store_type, &meta_sql);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_insert_record_video_meta_sql failed(%d)\n", ret);
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}
		mb_svc_sql_list_add(&insert_sql_list, &meta_sql);
	}

	/* Start transaction */
	int i = 0;
	int length = g_list_length(insert_sql_list);

#ifdef _PERFORMANCE_CHECK_
	start = mediainfo_get_debug_time();
#endif

	for (i = 0; i < length; i++) {
		char *sql = (char *)g_list_nth_data(insert_sql_list, i);
		ret = mb_svc_query_sql(sql);

		if (ret < 0) {
			mb_svc_debug
				("mb_svc_query_sql failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans();
			mb_svc_sql_list_release(&insert_sql_list);
			return ret;
		}
	}

	ret = mb_svc_sqlite3_commit_trans();
	if (ret < 0) {
		mb_svc_debug
			("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans();
		mb_svc_sql_list_release(&insert_sql_list);
		return ret;
	}

#ifdef _PERFORMANCE_CHECK_
	end = mediainfo_get_debug_time();

	g_insertdb += ((double)(end - start) / (double)CLOCKS_PER_SEC);
	mb_svc_debug("Insert : %f", g_insertdb);
	mb_svc_debug
	("========== End of Performance check of one item ==============\n\n");
#endif

	/* Release sql list */	
	mb_svc_sql_list_release(&insert_sql_list);

	return 0;
}


int mb_svc_delete_file(const char *file_full_path)
{
	char dir_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	mb_svc_folder_record_s folder_record = {"",};
	mb_svc_media_record_s media_record = {"",};
	int ret = -1;
	int media_id = -1;
	int media_record_cnt = 0;
	int folder_modified_date = 0;

	if (file_full_path == NULL) {
		mb_svc_debug("file_full_path == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("Delete start : %s", file_full_path);

	_mb_svc_get_file_parent_path(file_full_path, dir_full_path);
	ret = __mb_svc_get_folder_record_by_full_path(dir_full_path, &folder_record);
	if (ret < 0) {
		mb_svc_debug(" file directory %s doesn't exist ",
			     dir_full_path);
		return MB_SVC_ERROR_DIR_NOT_EXSITED;
	}

	_mb_svc_get_file_display_name(file_full_path, file_display_name);
	/* if the folder has at least one file */
	ret =
	    mb_svc_get_media_record_by_full_path(file_full_path, &media_record);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_get_media_record_by_full_path fail:file_full_path is %s ",
		     file_full_path);
		return ret;
	}

	/* delete the matched file info in media table */
	ret = mb_svc_delete_record_media_by_id(media_record.media_uuid);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_delete_record_media fail:folder id is %s,file name is %s ",
		     media_record.folder_uuid, file_display_name);
		return ret;
	}
	mb_svc_debug
	    ("mb_svc_delete_record_media sccceed,media_id is %s,file type is %d, path is %s\n",
	     media_record.media_uuid, media_record.content_type, media_record.path);

#ifdef DELETE_FOLDER_RECORD_IF_NO_FILE_IN	/* the media file is deleted succeed, then verify if the folder has no left files */
	media_record_cnt = mb_svc_get_folder_content_count_by_folder_id(media_record.folder_uuid);	/* after delete the media file,get the left media file count in the specified folder */
	mb_svc_debug("media_record_cnt after delete the media file is %d\n",
		     media_record_cnt);
	if (media_record_cnt == 0)	{ /* the folder has no left files, so delete the folder record */
		ret = mb_svc_delete_record_folder_by_id(folder_record.uuid);
		if (ret < 0) {
			mb_svc_debug("mb_svc_delete_record_older fail:%s\n",
				     folder_record.uuid);
			return ret;
		}
	} else {		/* update old folder modified date */
		folder_modified_date =
		    _mb_svc_get_file_dir_modified_date(dir_full_path);
		mb_svc_debug("folder_modified_date is %d\n",
			     folder_modified_date);
		if (folder_record.modified_date < folder_modified_date) {
			mb_svc_debug("directory %s is modified", dir_full_path);
			folder_record.modified_date = folder_modified_date;
			mb_svc_update_record_folder(&folder_record);
		}
	}

#endif
	/* delete thumbnail file */
	_mb_svc_thumb_delete(file_full_path);

	/* delete file info in image_meta table & (video_meta table and bookmark table if it's video file) */
	ret =
	    mb_svc_delete_bookmark_meta_by_media_id(media_record.media_uuid,
						    media_record.content_type);
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_delete_record_video_or_image_by_media_id fail:media id is %d\n",
		     media_id);
	}

	return ret;
}

int
mb_svc_rename_file(const char *old_file_full_path,
		   const char *new_file_full_path, minfo_file_type content_type,
		   char *thumb_path)
{
	mb_svc_debug("");

	char old_file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char new_file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char old_dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	mb_svc_media_record_s media_record = {"",};
	mb_svc_folder_record_s folder_record = {"",};
	int ret = 0;
	int folder_modified_date = 0;
	char old_media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	if (old_file_full_path == NULL || new_file_full_path == NULL) {
		mb_svc_debug
		    ("old_file_full_path==NULL || new_file_full_path==NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("old file_full_path is %s, new file full path is %s\n",
		     old_file_full_path, new_file_full_path);

	_mb_svc_get_file_display_name(old_file_full_path,
				      old_file_display_name);
	_mb_svc_get_file_parent_path(old_file_full_path, old_dir_path);

	ret = __mb_svc_get_folder_record_by_full_path(old_dir_path, &folder_record);
	if (ret < 0) {
		mb_svc_debug(" file directory %s doesn't exist ", old_dir_path);
		return MB_SVC_ERROR_DIR_NOT_EXSITED;
	}

	ret =
	    mb_svc_get_media_record_by_fid_name(folder_record.uuid,
						old_file_display_name,
						&media_record);
	if (ret >= 0) {
		strncpy(old_media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		_mb_svc_get_file_display_name(new_file_full_path,
					      new_file_display_name);
		mb_svc_debug("new file_display_name is %s\n",
			     new_file_display_name);

		snprintf(media_record.path, sizeof(media_record.path), "%s/%s",
			 folder_record.uri, new_file_display_name);
		strncpy(media_record.display_name, new_file_display_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		
		media_record.modified_date =
		    _mb_svc_get_file_dir_modified_date(new_file_full_path);

		/*  thumb file rename */
		_mb_svc_thumb_rename(old_file_full_path, new_file_full_path,
				     thumb_path);
		strncpy(media_record.thumbnail_path, thumb_path,
			MB_SVC_FILE_PATH_LEN_MAX + 1);

		mb_svc_update_record_media(&media_record);
	}

	folder_modified_date = _mb_svc_get_file_dir_modified_date(old_dir_path);
	mb_svc_debug("folder_modified_date is %d\n", folder_modified_date);
	if (folder_record.modified_date < folder_modified_date) {
		mb_svc_debug("directory %s is modified", old_dir_path);
		folder_record.modified_date = folder_modified_date;
		mb_svc_update_record_folder(&folder_record);
	}

	return 0;
}

int
mb_svc_move_file(const char *old_file_full_path, const char *new_file_full_path,
		 minfo_file_type content_type, char *thumb_path)
{
	mb_svc_debug("");

	int ret = 0;

	if (old_file_full_path == NULL || new_file_full_path == NULL
	    || thumb_path == NULL) {
		mb_svc_debug
		    ("old_file_full_path==NULL || new_file_full_path==NULL || thumb_path == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	mb_svc_debug("old file_full_path is %s, new file full path is %s\n",
		     old_file_full_path, new_file_full_path);
	
	char old_file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char new_file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char old_dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char new_dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };

	mb_svc_folder_record_s src_folder_record = {"",};
	mb_svc_folder_record_s dst_folder_record = {"",};
	mb_svc_media_record_s media_record = {"",};
	int src_clus_cont_cnt = 0;
	int src_clus_modified_date = 0;
	int dst_clus_modified_date = 0;
	char src_cluster_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	GList *move_sql_list = NULL;
	char *insert_new_folder_sql = NULL;
	char *update_old_folder_sql = NULL;
	char *update_new_folder_sql = NULL;
	char *delete_folder_sql = NULL;
	char *media_sql = NULL;
	int insert_new_folder = 0;
	
	_mb_svc_get_file_display_name(old_file_full_path,
				      old_file_display_name);
	_mb_svc_get_file_parent_path(old_file_full_path, old_dir_path);

	_mb_svc_get_file_display_name(new_file_full_path,
				      new_file_display_name);
	_mb_svc_get_file_parent_path(new_file_full_path, new_dir_path);

	ret = __mb_svc_get_folder_record_by_full_path(new_dir_path, &dst_folder_record);
	if (ret < 0) {
		mb_svc_debug("Directory %s is NOT in DB", new_dir_path);
		mb_svc_debug("Now making new dir %s", new_dir_path);

		int store_type = 0;
		char new_dir_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
		int folder_modified_date = 0;

		store_type = _mb_svc_get_store_type_by_full(new_file_full_path);
		
		if (store_type == MB_SVC_ERROR_INTERNAL) {
			mb_svc_debug("Failed to get storage type : %s",
				     new_file_full_path);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		_mb_svc_get_dir_display_name(new_dir_path, new_dir_display_name);
		mb_svc_debug("dir_display_name is %s\n", new_dir_display_name);

		folder_modified_date = _mb_svc_get_file_dir_modified_date(new_dir_path);

		dst_folder_record.modified_date = folder_modified_date;
		dst_folder_record.storage_type = store_type;
		strncpy(dst_folder_record.uri, new_dir_path, MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(dst_folder_record.display_name, new_dir_display_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(dst_folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		strncpy(dst_folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		dst_folder_record.lock_status = 0;

		insert_new_folder = 1;
	}

	ret = mb_svc_get_media_record_by_full_path(old_file_full_path, &media_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_record_by_full_path fails : %d", ret);
		return ret;
	}
	
	strncpy(src_cluster_uuid, media_record.folder_uuid, MB_SVC_UUID_LEN_MAX + 1);

	strncpy(media_record.folder_uuid, dst_folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);
	strncpy(media_record.path, new_file_full_path, sizeof(media_record.path));
	strncpy(media_record.display_name, new_file_display_name, sizeof(media_record.display_name));
	media_record.modified_date = _mb_svc_get_file_dir_modified_date(new_file_full_path);

	_mb_svc_thumb_move(old_file_full_path, new_file_full_path, thumb_path);

	strncpy(media_record.thumbnail_path, thumb_path, sizeof(media_record.thumbnail_path));

	/*  verify if the old folder has no  files left */
	src_clus_cont_cnt =
	    mb_svc_get_folder_content_count_by_folder_id(src_cluster_uuid);

	if (src_clus_cont_cnt == 1) {

		ret = mb_svc_delete_record_folder_sql(src_cluster_uuid, &delete_folder_sql);
		if (ret < 0) {
			mb_svc_debug("mb_svc_delete_record_folder_sql fail:%d\n",
				     ret);
			return ret;
		}
		mb_svc_sql_list_add(&move_sql_list, &delete_folder_sql);
	} else	{	/* update  modified date */
		src_clus_modified_date =
		    _mb_svc_get_file_dir_modified_date(old_dir_path);
		mb_svc_get_folder_record_by_id(src_cluster_uuid, &src_folder_record);
		mb_svc_debug("src cluster modified_date is %d\n",
			     src_clus_modified_date);
		if (src_folder_record.modified_date < src_clus_modified_date) {
			src_folder_record.modified_date = src_clus_modified_date;

			ret = mb_svc_update_record_folder_sql(&src_folder_record, &update_old_folder_sql);
			if (ret < 0) {
				mb_svc_debug("mb_svc_delete_record_folder_sql fail:%d\n",
						ret);
				return ret;
			}
			
			mb_svc_sql_list_add(&move_sql_list, &update_old_folder_sql);
		}
	}

	ret = mb_svc_sqlite3_begin_trans();
	if (ret < 0) {
		mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
		return ret;
	}

	if (insert_new_folder == 1) {
		ret = mb_svc_insert_record_folder_sql(&dst_folder_record, &insert_new_folder_sql);
		if (ret < 0) {
			mb_svc_debug("mb_svc_insert_record_folder_sql fails : %d", ret);
			return ret;
		}

		mb_svc_sql_list_add(&move_sql_list, &insert_new_folder_sql);

		strncpy(media_record.folder_uuid, dst_folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

	} else {
		dst_clus_modified_date =
			_mb_svc_get_file_dir_modified_date(new_dir_path);
	
		mb_svc_debug("dst cluster modified_date is %d\n",
				dst_clus_modified_date);
		if (dst_folder_record.modified_date < dst_clus_modified_date) {
			dst_folder_record.modified_date = dst_clus_modified_date;

			ret = mb_svc_update_record_folder_sql(&dst_folder_record, &update_new_folder_sql);
			if (ret < 0) {
				mb_svc_debug("mb_svc_delete_record_folder_sql fail:%d\n",
						ret);
				return ret;
			}
			
			mb_svc_sql_list_add(&move_sql_list, &update_new_folder_sql);
		}
	}

	ret = mb_svc_update_record_media_sql(&media_record, &media_sql);
	if (ret < 0) {
		mb_svc_debug("mb_svc_update_record_media fails : %d", ret);
		return ret;
	}

	mb_svc_sql_list_add(&move_sql_list, &media_sql);

	/* Start transaction */
	int i = 0;
	int length = g_list_length(move_sql_list);

	for (i = 0; i < length; i++) {
		char *sql = (char *)g_list_nth_data(move_sql_list, i);
		ret = mb_svc_query_sql(sql);

		if (ret < 0) {
			mb_svc_debug
				("mb_svc_query_sql failed.. Now start to rollback\n");
			mb_svc_sqlite3_rollback_trans();
			mb_svc_sql_list_release(&move_sql_list);
			return ret;
		}
	}

	ret = mb_svc_sqlite3_commit_trans();
	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		mb_svc_sqlite3_rollback_trans();
		return ret;
	}

	return ret;
}

int mb_svc_move_file_by_id(const char *src_media_id, const char *dst_cluster_id)
{
	mb_svc_debug("");

	int ret = 0;
	mb_svc_media_record_s media_record = {"",};
	int src_clus_cont_cnt = 0;
	int src_clus_modified_date = 0;
	char src_clus_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	int dst_clus_modified_date = 0;
	char dst_clus_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	mb_svc_folder_record_s folder_record = {"",};
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char dst_file_full_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char src_cluster_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	mb_svc_debug("minfo_mv_media#src_media_id: %s", src_media_id);
	mb_svc_debug("minfo_mv_media#dst_cluster_id: %s", dst_cluster_id);

	ret = mb_svc_get_media_record_by_id(src_media_id, &media_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_record_by_id failed\n");
		return ret;
	}

	strncpy(src_cluster_uuid, media_record.folder_uuid, MB_SVC_UUID_LEN_MAX + 1);

	mb_svc_get_folder_fullpath_by_folder_id(dst_cluster_id, dst_clus_path,
						sizeof(dst_clus_path));
	snprintf(dst_file_full_path, MB_SVC_FILE_PATH_LEN_MAX + 1, "%s/%s",
		 dst_clus_path, media_record.display_name);

	strncpy(media_record.path, dst_file_full_path, MB_SVC_FILE_PATH_LEN_MAX + 1);

	_mb_svc_thumb_move(media_record.path, dst_file_full_path, thumb_path);

	media_record.modified_date =
	    _mb_svc_get_file_dir_modified_date(dst_file_full_path);
	strncpy(media_record.thumbnail_path, thumb_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	//media_record.folder_id = dst_cluster_id;
	strncpy(media_record.folder_uuid, dst_cluster_id, MB_SVC_UUID_LEN_MAX + 1);

	mb_svc_update_record_media(&media_record);

	/*  verify if the old folder has no  files left */
	src_clus_cont_cnt =
	    mb_svc_get_folder_content_count_by_folder_id(src_cluster_uuid);
	if (src_clus_cont_cnt == 0) {
		ret = mb_svc_delete_record_folder_by_id(src_cluster_uuid);
		if (ret < 0) {
			mb_svc_debug("mb_svc_delete_record_older fail:%d\n",
				     src_clus_cont_cnt);
			return ret;
		}
	} else	{	/* update  modified date */
		mb_svc_get_folder_fullpath_by_folder_id(src_cluster_uuid,
							src_clus_path,
							sizeof(src_clus_path));
		src_clus_modified_date =
		    _mb_svc_get_file_dir_modified_date(src_clus_path);
		mb_svc_get_folder_record_by_id(src_cluster_uuid, &folder_record);
		mb_svc_debug("src cluster modified_date is %d\n",
			     src_clus_modified_date);
		if (folder_record.modified_date < src_clus_modified_date) {
			mb_svc_debug("src cluster directory %s is modified",
				     src_clus_path);
			folder_record.modified_date = src_clus_modified_date;
			mb_svc_update_record_folder(&folder_record);
		}
	}

	mb_svc_get_folder_fullpath_by_folder_id(dst_cluster_id, dst_clus_path,
						sizeof(dst_clus_path));
	dst_clus_modified_date =
	    _mb_svc_get_file_dir_modified_date(dst_clus_path);
	mb_svc_get_folder_record_by_id(dst_cluster_id, &folder_record);
	mb_svc_debug("src cluster modified_date is %d\n",
		     dst_clus_modified_date);
	if (folder_record.modified_date < dst_clus_modified_date) {
		mb_svc_debug("src cluster directory %s is modified",
			     dst_clus_path);
		folder_record.modified_date = dst_clus_modified_date;
		mb_svc_update_record_folder(&folder_record);
	}

	return 0;
}

int
mb_svc_copy_file(const char *old_file_full_path, const char *new_file_full_path,
		 minfo_file_type content_type, char *thumb_path)
{
	char new_file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char new_dir_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dir_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	mb_svc_media_record_s media_record = {"",};
	mb_svc_folder_record_s folder_record = {"",};
	mb_svc_image_meta_record_s image_record = {0,};
	mb_svc_video_meta_record_s video_record = {0,};
	int folder_modified_date = 0;
	int ret = 0;
	int store_type = 0;
	char old_media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	bool is_new_folder = FALSE;

	if (old_file_full_path == NULL || new_file_full_path == NULL
	    || thumb_path == NULL) {
		mb_svc_debug
		    ("old_file_full_path==NULL || new_file_full_path==NULL || thumb_path == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	mb_svc_debug("old file_full_path is %s, new file full path is %s\n",
		     old_file_full_path, new_file_full_path);

	/* 1. copy to dest dir */

	/* 1.1. get old media record info */
	ret =
	    mb_svc_get_media_record_by_full_path(old_file_full_path,
						 &media_record);

	if (ret < 0) {
		mb_svc_debug
		    ("mb_svc_get_media_record_by_fid_name fail: %d\n", ret);
		return ret;
	}

	/* 1.2. check new dir  */
	_mb_svc_get_file_parent_path(new_file_full_path, new_dir_path);
	folder_modified_date = _mb_svc_get_file_dir_modified_date(new_dir_path);
	mb_svc_debug("folder_modified_date is %d\n", folder_modified_date);

	ret = __mb_svc_get_folder_record_by_full_path(new_dir_path, &folder_record);

	if (ret < 0 && ret == MB_SVC_ERROR_DB_INTERNAL) {

		is_new_folder = TRUE;
		mb_svc_debug("no any record in %s", new_dir_path);

		store_type = _mb_svc_get_store_type_by_full(new_file_full_path);
		if (store_type == MB_SVC_ERROR_INTERNAL) {
			mb_svc_debug("Failed to get storage type : %s",
				     new_file_full_path);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
		mb_svc_debug("store_type is %d\n", store_type);

		_mb_svc_get_dir_display_name(new_dir_path, dir_display_name);
		mb_svc_debug("dir_display_name is %s\n", dir_display_name);

		/* _mb_svc_get_rel_path_by_full(new_dir_path, rel_path);
		mb_svc_debug("rel path is %s\n", rel_path);
		_mb_svc_get_dir_parent_path(rel_path, uri); 
		mb_svc_debug("uri is %s\n", uri); */

		folder_record.modified_date = folder_modified_date;
		folder_record.storage_type = store_type;
		strncpy(folder_record.uri, new_dir_path,
			MB_SVC_DIR_PATH_LEN_MAX + 1);
		strncpy(folder_record.display_name, dir_display_name,
			MB_SVC_FILE_NAME_LEN_MAX + 1);
		strncpy(folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
		folder_record.lock_status = 0;

		mb_svc_debug
		    ("no record in %s, ready insert the folder record into db\n",
		     new_dir_path);

		ret = mb_svc_insert_record_folder(&folder_record);
		if (ret < 0) {
			mb_svc_debug
			    ("insert file info into folder table failed. Trying to get folder record again.\n");
			memset(&folder_record, 0x00,
			       sizeof(mb_svc_folder_record_s));
			ret = __mb_svc_get_folder_record_by_full_path(new_dir_path,
								  &folder_record);
			if (ret < 0) {
				mb_svc_debug("__mb_svc_get_folder_record_by_full_path failed again.");
				return ret;
			}
			is_new_folder = FALSE;
		}
		mb_svc_debug("folder record id of new inserted is %s\n",
			     folder_record.uuid);
	} else {
		mb_svc_debug
		    ("folder_record info: uuid is %s,uri is %s,display_name is %s,modified_date is %d,web_account_id is %s,store_type is %d\n",
		     folder_record.uuid, folder_record.uri,
		     folder_record.display_name, folder_record.modified_date,
		     folder_record.web_account_id, folder_record.storage_type);

		if (folder_record.modified_date < folder_modified_date) {
			mb_svc_debug("directory %s is modified", new_dir_path);
			folder_record.modified_date = folder_modified_date;
			mb_svc_update_record_folder(&folder_record);
		}
	}

	strncpy(old_media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

	/* 2.3. set new values, take care of  file rename  */
	_mb_svc_get_file_display_name(new_file_full_path,
				      new_file_display_name);
	mb_svc_debug("new file_display_name is %s\n", new_file_display_name);
	strncpy(media_record.display_name, new_file_display_name,
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(media_record.path, new_file_full_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);

	strncpy(media_record.folder_uuid, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);
	media_record.modified_date =
	    _mb_svc_get_file_dir_modified_date(new_file_full_path);

	_mb_svc_thumb_generate_hash_name(new_file_full_path, thumb_path,
					 MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(media_record.thumbnail_path, thumb_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);

	ret = mb_svc_insert_record_media(&media_record, store_type);
	if (ret < 0) {
		if (is_new_folder == TRUE) {
			ret =
			    mb_svc_delete_record_folder_by_id(folder_record.uuid);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_delete_record_older fail:%s\n",
				     folder_record.uuid);
			}
		}
		return ret;
	}

	/* 2.4. copy  image_meta record or video_meta record */
	if (content_type == MINFO_ITEM_IMAGE) {
		mb_svc_debug("update image record\n");
		ret =
		    mb_svc_get_image_record_by_media_id(old_media_uuid,
							&image_record);
		if (ret < 0) {
			mb_svc_debug("get image record by media id failed\n");
			return ret;
		}

		strncpy(image_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_image_meta(&image_record, store_type);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_insert_record_image_meta failed\n");
			return ret;
		}

	} else if (content_type == MINFO_ITEM_VIDEO) {
		mb_svc_debug("update video record\n");
		ret =
		    mb_svc_get_video_record_by_media_id(old_media_uuid,
							&video_record);
		if (ret < 0) {
			mb_svc_debug("get video record by media id failed\n");
			return ret;
		}

		strncpy(video_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_video_meta(&video_record, store_type);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_insert_record_video_meta failed\n");
			return ret;
		}

		/* Get bookmark list and update them */
		mb_svc_bookmark_record_s bookmark_record = { 0 };
		mb_svc_iterator_s mb_svc_bm_iterator = { 0 };
		int record_cnt = 0;

		ret =
		    mb_svc_bookmark_iter_start(old_media_uuid,
					       &mb_svc_bm_iterator);
		if (ret < 0) {
			mb_svc_debug("mb-svc iterator start failed");
			return MB_SVC_ERROR_DB_INTERNAL;
		}

		while (1) {
			ret =
			    mb_svc_bookmark_iter_next(&mb_svc_bm_iterator,
						      &bookmark_record);

			if (ret == MB_SVC_NO_RECORD_ANY_MORE)
				break;

			if (ret < 0) {
				mb_svc_debug
				    ("mb-svc iterator get next recrod failed");
				mb_svc_iter_finish(&mb_svc_bm_iterator);
				return ret;
			}

			record_cnt++;

			strncpy(bookmark_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			ret = mb_svc_insert_record_bookmark(&bookmark_record);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_insert_record_bookmark failed\n");
				return ret;
			}
		}

		mb_svc_iter_finish(&mb_svc_bm_iterator);
	}

	/* 2.5 copy thumnail file */
	ret =
	    _mb_svc_thumb_copy(old_file_full_path, new_file_full_path,
			       thumb_path);
	if (ret < 0) {
		mb_svc_debug("thumb copy fails [%d]", ret);
		/* return ret; */
	}

	return 0;
}

int mb_svc_copy_file_by_id(const char *src_media_id, const char *dst_cluster_id)
{
	int ret = 0;
	mb_svc_media_record_s media_record = {"",};
	mb_svc_image_meta_record_s image_record = {0,};
	mb_svc_video_meta_record_s video_record = {0,};
	mb_svc_folder_record_s folder_record = {"",};
	char dst_clus_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dst_file_full_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	int dst_clus_modified_date = 0;
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	minfo_store_type store_type = MINFO_SYSTEM;

	ret = mb_svc_get_media_record_by_id(src_media_id, &media_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_get_media_record_by_id failed\n");
		return ret;
	}

	mb_svc_get_folder_fullpath_by_folder_id(dst_cluster_id, dst_clus_path,
						sizeof(dst_clus_path));
	snprintf(dst_file_full_path, MB_SVC_FILE_PATH_LEN_MAX + 1, "%s/%s",
		 dst_clus_path, media_record.display_name);

	store_type = _mb_svc_get_store_type_by_full(dst_file_full_path);
	if (store_type == MB_SVC_ERROR_INTERNAL) {
		mb_svc_debug("Failed to get storage type : %s",
			     dst_file_full_path);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	_mb_svc_thumb_generate_hash_name(dst_file_full_path, thumb_path,
					 MB_SVC_FILE_PATH_LEN_MAX + 1);

	media_record.modified_date =
	    _mb_svc_get_file_dir_modified_date(dst_file_full_path);
	strncpy(media_record.path, dst_file_full_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(media_record.thumbnail_path, thumb_path,
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	//media_record.folder_id = dst_cluster_id;
	strncpy(media_record.folder_uuid, dst_cluster_id, MB_SVC_UUID_LEN_MAX + 1);

	mb_svc_insert_record_media(&media_record, store_type);

	if (media_record.content_type == MINFO_ITEM_IMAGE) {
		mb_svc_debug("update image record\n");
		ret =
		    mb_svc_get_image_record_by_media_id(src_media_id,
							&image_record);
		if (ret < 0) {
			mb_svc_debug("get image record by media id failed\n");
			return ret;
		}

		strncpy(image_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_insert_record_image_meta(&image_record, store_type);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_insert_record_image_meta failed\n");
			return ret;
		}
	} else if (media_record.content_type == MINFO_ITEM_VIDEO) {
		mb_svc_debug("update video record\n");
		ret =
		    mb_svc_get_video_record_by_media_id(src_media_id,
							&video_record);
		if (ret < 0) {
			mb_svc_debug("get video record by media id failed\n");
			return ret;
		}

		strncpy(video_record.media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);
		mb_svc_insert_record_video_meta(&video_record, store_type);
	}

	mb_svc_get_folder_fullpath_by_folder_id(dst_cluster_id, dst_clus_path,
						sizeof(dst_clus_path));
	dst_clus_modified_date =
	    _mb_svc_get_file_dir_modified_date(dst_clus_path);
	mb_svc_get_folder_record_by_id(dst_cluster_id, &folder_record);
	mb_svc_debug("src cluster modified_date is %d\n",
		     dst_clus_modified_date);
	if (folder_record.modified_date < dst_clus_modified_date) {
		mb_svc_debug("src cluster directory %s is modified",
			     dst_clus_path);
		folder_record.modified_date = dst_clus_modified_date;
		mb_svc_update_record_folder(&folder_record);
	}

	ret =
	    _mb_svc_thumb_copy(media_record.path, dst_file_full_path,
			       thumb_path);
	if (ret < 0) {
		mb_svc_debug("thumb copy fails [%d]", ret);
		/* return ret; */
	}

	return 0;
}

int mb_svc_update_cluster_name(const char *cluster_id, const char *new_name)
{
	int ret = -1;
	int len = 0;
	mb_svc_folder_record_s folder_record = {"",};
	char old_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char dir_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char src_parent_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char src_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char dst_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	time_t date;

	if (new_name == NULL) {
		mb_svc_debug(" new name is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("minfo_update_cluster_name#cluster_id: %s", cluster_id);
	mb_svc_debug("minfo_update_cluster_name#new_name: %s", new_name);

	ret = mb_svc_get_folder_record_by_id(cluster_id, &folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_cluster_name: no folder record matched with the folder id\n");
		return ret;
	}

	strncpy(old_name, folder_record.display_name, sizeof(old_name));
	strncpy(folder_record.display_name, new_name,
		sizeof(folder_record.display_name));
	_mb_svc_get_file_parent_path(folder_record.uri, dir_full_path);
	snprintf(src_parent_path, sizeof(src_parent_path), "%s/",
		 folder_record.uri);

	len = g_strlcat(dir_full_path, "/", sizeof(dir_full_path));
	if (len >= sizeof(dir_full_path)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	len = g_strlcat(dir_full_path, new_name, sizeof(dir_full_path));
	if (len >= sizeof(dir_full_path)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	strncpy(folder_record.uri, dir_full_path, sizeof(folder_record.uri));
	mb_svc_debug("uri: %s", folder_record.uri);

	len = g_strlcat(dir_full_path, "/", sizeof(dir_full_path));
	if (len >= sizeof(dir_full_path)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	/* Update all folder record's path, which are matched by old parent path */
	ret = mb_svc_update_record_folder_path(src_parent_path, dir_full_path);
	if (ret < 0) {
		mb_svc_debug("mb_svc_update_record_folder_path failed\n");
		return ret;
	}

	time(&date);
	folder_record.modified_date = date;

	/* Update all folder record's modified date, which are changed above */
	ret =
	    mb_svc_update_folder_modified_date(dir_full_path,
					       folder_record.modified_date);
	if (ret < 0) {
		mb_svc_debug("mb_svc_update_folder_modified_date failed\n");
		return ret;
	}

	ret = mb_svc_update_record_folder(&folder_record);
	if (ret < 0) {
		mb_svc_debug
		    ("minfo_update_cluster_name: update cluster name failed\n");
		return ret;
	}

	mb_svc_folder_record_s matched_folder_record = {"",};
	mb_svc_media_record_s media_record = {"",};
	mb_svc_iterator_s mb_svc_folder_iterator = { 0 };
	mb_svc_iterator_s mb_svc_media_iterator = { 0 };
	minfo_item_filter filter = { 0 };
	filter.favorite = MINFO_MEDIA_FAV_ALL;
	filter.start_pos = -1;
	filter.sort_type = MINFO_MEDIA_SORT_BY_NONE;
	filter.file_type = MINFO_ITEM_ALL;
	char old_media_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	ret = __mb_svc_folder_by_path_iter_start(folder_record.uri, &mb_svc_folder_iterator);

	if (ret < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return ret;
	}

	while (1) {
		ret =
		    mb_svc_folder_iter_next(&mb_svc_folder_iterator,
					    &matched_folder_record);

		if (ret == MB_SVC_NO_RECORD_ANY_MORE) {
			break;
		}
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_folder_iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_folder_iterator);
			return ret;
		}

		ret =
		    mb_svc_media_iter_start_new(matched_folder_record.uuid,
						&filter, MINFO_CLUSTER_TYPE_ALL,
						1, NULL,
						&mb_svc_media_iterator);

		if (ret < 0) {
			mb_svc_debug("mb-svc iterator start failed");
			mb_svc_iter_finish(&mb_svc_folder_iterator);
			return ret;
		}

		while (1) {
			ret =
			    mb_svc_media_iter_next(&mb_svc_media_iterator,
						   &media_record);

			if (ret == MB_SVC_NO_RECORD_ANY_MORE) {
				break;
			}

			if (ret < 0) {
				mb_svc_debug
				    ("mb-svc iterator get next recrod failed");
				mb_svc_iter_finish(&mb_svc_folder_iterator);
				mb_svc_iter_finish(&mb_svc_media_iterator);
				return ret;
			}

			strncpy(old_media_uuid, media_record.media_uuid, MB_SVC_UUID_LEN_MAX + 1);

			snprintf(src_full_path, sizeof(src_full_path), "%s",
				 media_record.path);

			memset(media_record.path, 0x00,
			       sizeof(media_record.path));
			snprintf(media_record.path, sizeof(media_record.path),
				 "%s/%s", matched_folder_record.uri,
				 media_record.display_name);

			snprintf(dst_full_path, sizeof(dst_full_path), "%s",
				 media_record.path);

			mb_svc_debug
			    ("_mb_svc_thumb_move : src[ %s ], dst[ %s ]",
			     src_full_path, dst_full_path);
			ret =
			    _mb_svc_thumb_move(src_full_path, dst_full_path,
					       media_record.thumbnail_path);
			if (ret < 0) {
				mb_svc_debug
				    ("_mb_svc_thumb_move fails.. so use default thumbnail");
				snprintf(media_record.thumbnail_path,
					 sizeof(media_record.thumbnail_path),
					 "%s", DEFAULT_IMAGE_THUMB);
			}

			ret = mb_svc_update_record_media(&media_record);
			if (ret < 0) {
				mb_svc_debug
				    ("Error : mb_svc_update_record_media path : %s",
				     media_record.path);
				mb_svc_iter_finish(&mb_svc_folder_iterator);
				mb_svc_iter_finish(&mb_svc_media_iterator);
				return ret;
			}
		}

		mb_svc_iter_finish(&mb_svc_media_iterator);
	}

	mb_svc_iter_finish(&mb_svc_folder_iterator);

	return 0;
}

EXPORT_API int mb_svc_initialize()
{
	int err = -1;
	int tid = -1;
	mb_svc_debug("mb_svc_initialize-----------enter\n");

	g_type_init();

	err = _media_info_init_handle_tbl();
	if (err < 0) {
		mb_svc_debug("Error:_media_info_init_handle_tbl\n");
		return MB_SVC_ERROR_DB_CONNECT;
	}

	tid = _media_info_get_thread_id();
	mb_svc_debug("Current thread id : %d", tid);

	HandleTable *handle_table = NULL;
	handle_table = _media_info_search_handle(tid);

	if (handle_table == NULL) {
		mb_svc_debug
		    ("A handle in thread [%d] does not exist. So now trying to make connection");
		int *key_tid = NULL;

		err = _media_info_insert_handle(&key_tid, tid, &handle_table);
		if (err < 0) {
			mb_svc_debug("A handle in thread [%d] exists. ", tid);
			if (key_tid)
				g_free(key_tid);
			if (handle_table)
				g_free(handle_table);
			return 0;
		}

		sqlite3 *handle = NULL;
		err = mb_svc_connect_db(&handle);
		if (err < 0 || handle == NULL) {
			mb_svc_debug("Error:failed to initialize DB\n");
			if (key_tid)
				g_free(key_tid);
			if (handle_table)
				g_free(handle_table);
			return MB_SVC_ERROR_DB_CONNECT;
		}

		handle_table->handle = handle;
	} else {
		mb_svc_debug("A handle in thread [%d] exists. ", tid);
		_media_info_atomic_add_counting(handle_table);
	}

	mb_svc_debug("mb_svc_initialize-----------leave\n");
	return 0;
}

EXPORT_API int mb_svc_finalize()
{
	int ret = 0;
	int err = -1;
	int tid = -1;
	mb_svc_debug("mb_svc_finalize-----------enter\n");

	tid = _media_info_get_thread_id();
	mb_svc_debug("Current thread id : %d", tid);

	HandleTable *handle_table = NULL;
	handle_table = _media_info_search_handle(tid);

	if (handle_table == NULL) {
		mb_svc_debug("handle_table is NULL");
		return MB_SVC_ERROR_DB_DISCONNECT;
	} else {
		mb_svc_debug("ref count in thread[%d] is %d", tid,
			     handle_table->ref_cnt);

		if (handle_table->ref_cnt > 1) {
			_media_info_atomic_sub_counting(handle_table);
		} else {

			err = mb_svc_disconnect_db(handle_table->handle);
			if (err < 0) {
				mb_svc_debug("Error:mb_svc_disconnect_db\n");
				ret = MB_SVC_ERROR_DB_DISCONNECT;
			}

			err = _media_info_remove_handle(tid);
			if (err < 0) {
				mb_svc_debug
				    ("Error:_media_info_remove_handle\n");
				return MB_SVC_ERROR_DB_DISCONNECT;
			}

			_media_info_finalize_handle_tbl();

		}
	}

	return ret;
}

/* clock_t */
long mb_svc_get_clock(void)
{
	struct timeval tv;
	long curtime;

	gettimeofday(&tv, NULL);
	curtime = tv.tv_sec * USEC_PER_SEC + tv.tv_usec;

	/* int curtime = time((time_t*)NULL); */
	return curtime;
}

int mb_svc_table_member_count(char *table_name)
{
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	int err = -1;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (table_name == NULL) {
		mb_svc_debug("Error:table_name == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(q_string, sizeof(q_string), MB_SVC_TABLE_COUNT_QUERY_STRING,
		 table_name);

	err =
	    sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	stmt = NULL;
	mb_svc_debug("record count of table %s is %d\n", table_name, count);
	return count;
}

int
mb_svc_geo_media_iter_start(const char *folder_id,
			    minfo_folder_type store_filter,
			    minfo_item_filter *filter,
			    mb_svc_iterator_s *mb_svc_iterator,
			    double min_longitude,
			    double max_longitude,
			    double min_latitude, double max_latitude)
{
	mb_svc_debug("");

	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char query_complete_string[MB_SVC_DEFAULT_QUERY_SIZE * 3 + 1] = { 0 };
	char query_where[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char tmp_str[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	/* mb_svc_debug ("mb_svc_media_iter_start--enter\n"); */

	if (mb_svc_iterator == NULL || filter == NULL) {
		mb_svc_debug("Error:mb_svc_iterator == NULL || filter == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	if (filter->start_pos >= 0 && filter->start_pos > filter->end_pos) {
		mb_svc_debug(" filter->start_pos (%d) > filter->end_pos (%d) = %d\n",
			     filter->start_pos, filter->end_pos);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	strncpy(query_where, "", MB_SVC_DEFAULT_QUERY_SIZE);

	if (store_filter != MINFO_CLUSTER_TYPE_ALL) {
		switch (store_filter) {
		case MINFO_CLUSTER_TYPE_LOCAL_ALL:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and (storage_type = %d or storage_type = %d)",
				 MINFO_PHONE, MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_PHONE:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_PHONE);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_MMC:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_WEB:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_WEB);
			break;
		case MINFO_CLUSTER_TYPE_STREAMING:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ",
				 MINFO_WEB_STREAMING);
			break;
		default:
			break;
		}
	}

	snprintf(query_string, MB_SVC_DEFAULT_QUERY_SIZE + 1,
		 MB_SVC_TABLE_SELECT_GEO_LIST, table_name, min_longitude,
		 max_longitude, min_latitude, max_latitude, min_longitude,
		 max_longitude, min_latitude, max_latitude, tmp_str);

	memset(tmp_str, 0x00, sizeof(tmp_str));

	mb_svc_debug("Query string: %s", query_string);

	if (filter->favorite == MINFO_MEDIA_FAV_ONLY) {
		strncat(query_where, " and a.rating = 1",
			MB_SVC_DEFAULT_QUERY_SIZE + 1);
	} else if (filter->favorite == MINFO_MEDIA_UNFAV_ONLY) {
		strncat(query_where, " and a.rating = 0",
			MB_SVC_DEFAULT_QUERY_SIZE + 1);
	}

	/* set to get only unlocked items */
	strncat(query_where, " and b.lock_status = 0",
		MB_SVC_DEFAULT_QUERY_SIZE + 1);

	if (folder_id != NULL) {
		snprintf(tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1,
			 " and a.folder_uuid = '%s'", folder_id);
		strncat(query_where, tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1);
	}

	snprintf(tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1,
		 " and (a.content_type = 0");
	strncat(query_where, tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1);

	if (filter->file_type & MINFO_ITEM_ALL) {
		filter->file_type = MINFO_ITEM_IMAGE | MINFO_ITEM_VIDEO;
	}

	if (filter->file_type & MINFO_ITEM_IMAGE) {
		snprintf(tmp_str, sizeof(tmp_str), " or a.content_type = %d",
			 MINFO_ITEM_IMAGE);
		strncat(query_where, tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1);
	}

	if (filter->file_type & MINFO_ITEM_VIDEO) {
		snprintf(tmp_str, sizeof(tmp_str), " or a.content_type = %d",
			 MINFO_ITEM_VIDEO);
		strncat(query_where, tmp_str, MB_SVC_DEFAULT_QUERY_SIZE + 1);
	}

	strncat(query_where, ")", MB_SVC_DEFAULT_QUERY_SIZE + 1);

	if (filter->sort_type == MINFO_MEDIA_SORT_BY_NONE)
		filter->sort_type = MINFO_MEDIA_SORT_BY_NAME_ASC;

	snprintf(query_complete_string, MB_SVC_DEFAULT_QUERY_SIZE * 3 + 1,
		 "%s %s ORDER BY a.%s", query_string, query_where,
		 mb_svc_media_order[filter->sort_type]);

	if (filter->start_pos != MB_SVC_DB_DEFAULT_GET_ALL_RECORDS) {	/* -1 get all record */
		int length = filter->end_pos - filter->start_pos + 1;
		if (length <= 0) {
			mb_svc_debug
			    ("start position and end position is invalid ( start:%d, end:%d )",
			     filter->start_pos, filter->end_pos);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		snprintf(tmp_str, sizeof(tmp_str), " LIMIT %d,%d",
			 filter->start_pos, length);
		strncat(query_complete_string, tmp_str,
			sizeof(query_complete_string));
	}

	mb_svc_debug("############### SQL: %s\n", query_complete_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_complete_string,
			       strlen(query_complete_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_complete_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_media_iter_start_new(const char *folder_id, minfo_item_filter *filter,
			    minfo_folder_type folder_type, int valid,
			    GList *p_folder_id_list,
			    mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char query_complete_string[MB_SVC_DEFAULT_QUERY_SIZE * 3 + 1] = { 0 };
	char query_where[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char condition_str[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int len = 0;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_MEDIA);

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (mb_svc_iterator == NULL || filter == NULL) {
		mb_svc_debug("Error:mb_svc_iterator == NULL || filter == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	if (filter->start_pos >= 0 && filter->start_pos > filter->end_pos) {
		mb_svc_debug(" filter->start_pos (%d) > filter->end_pos (%d)",
			     filter->start_pos, filter->end_pos);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(query_string, sizeof(query_string), MB_SVC_SELECT_ALL_MEDIA,
		 table_name);

	if (valid) {
		strncpy(query_where,
			" f.uuid = m.folder_uuid and m.valid=1 and f.valid=1 ",
			sizeof(query_where));
	} else {
		strncpy(query_where,
			" f.uuid = m.folder_uuid and m.valid=0 and f.valid=0 ",
			sizeof(query_where));
	}

	if (filter->favorite == MINFO_MEDIA_FAV_ONLY) {
		len =
		    g_strlcat(query_where, " and m.rating = 1 ",
			      sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	} else if (filter->favorite == MINFO_MEDIA_UNFAV_ONLY) {
		len =
		    g_strlcat(query_where, " and m.rating = 0 ",
			      sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	if (folder_id != NULL) {
		int len =
		    snprintf(condition_str, sizeof(condition_str),
			     " and folder_uuid = '%s' ", folder_id);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}

		len =
		    g_strlcat(query_where, condition_str, sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	} else {
		int len =
		    snprintf(condition_str, sizeof(condition_str),
			     " and lock_status = 0 ");
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}

		len =
		    g_strlcat(query_where, condition_str, sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	if (folder_type != MINFO_CLUSTER_TYPE_ALL) {
		switch (folder_type) {
		case MINFO_CLUSTER_TYPE_LOCAL_ALL:
			snprintf(condition_str, sizeof(condition_str),
				 " and (storage_type = %d or storage_type = %d)",
				 MINFO_PHONE, MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_PHONE:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_PHONE);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_MMC:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_WEB:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_WEB);
			break;
		case MINFO_CLUSTER_TYPE_STREAMING:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ",
				 MINFO_WEB_STREAMING);
			break;
		default:
			break;
		}

		len =
		    g_strlcat(query_where, condition_str, sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	len =
	    snprintf(condition_str, sizeof(condition_str),
		     " and (content_type = 0");
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		condition_str[0] = '\0';
	} else {
		condition_str[len] = '\0';
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	if (filter->file_type & MINFO_ITEM_ALL) {
		filter->file_type = MINFO_ITEM_IMAGE | MINFO_ITEM_VIDEO;
	}

	if (filter->file_type & MINFO_ITEM_IMAGE) {
		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " or content_type = %d", MINFO_ITEM_IMAGE);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	if (filter->file_type & MINFO_ITEM_VIDEO) {
		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " or content_type = %d", MINFO_ITEM_VIDEO);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
	}

	len = g_strlcat(condition_str, ")", sizeof(condition_str));
	if (len >= sizeof(condition_str)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	len =
	    snprintf(query_complete_string, sizeof(query_complete_string),
		     "%s %s ORDER BY m.%s", query_string, query_where,
		     mb_svc_media_order[filter->sort_type]);
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		query_complete_string[0] = '\0';
	} else {
		query_complete_string[len] = '\0';
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	if (filter->start_pos != MB_SVC_DB_DEFAULT_GET_ALL_RECORDS)	{/* -1 get all record */
		int length = filter->end_pos - filter->start_pos + 1;
		if (length <= 0) {
			mb_svc_debug
			    ("start position and end position is invalid ( start:%d, end:%d )",
			     filter->start_pos, filter->end_pos);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " LIMIT %d,%d", filter->start_pos, length);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
		len =
		    g_strlcat(query_complete_string, condition_str,
			      sizeof(query_complete_string));
		if (len >= sizeof(query_complete_string)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	mb_svc_debug("############### SQL: %s\n", query_complete_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_complete_string,
			       strlen(query_complete_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_complete_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int 
mb_svc_media_search_iter_start(minfo_search_field_t search_field, 
								const char *search_str, 
								minfo_folder_type folder_type,
								minfo_item_filter filter, 
								mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char query_complete_string[MB_SVC_DEFAULT_QUERY_SIZE * 3 + 1] = { 0 };
	char query_where[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char condition_str[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int len = 0;
	char *like_str = NULL;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_MEDIA);

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("Error:mb_svc_iterator == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (filter.start_pos >= 0 && filter.start_pos > filter.end_pos) {
		mb_svc_debug(" filter.start_pos (%d) > filter.end_pos (%d) = %d\n",
			     filter.start_pos, filter.end_pos);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(query_string, sizeof(query_string), MB_SVC_SELECT_ALL_MEDIA,
		 table_name);

	strncpy(query_where,
		" f.uuid = m.folder_uuid and m.valid=1 and f.valid=1 ",
		sizeof(query_where));

	if (filter.favorite == MINFO_MEDIA_FAV_ONLY) {
		len =
		    g_strlcat(query_where, " and m.rating = 1 ",
			      sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	} else if (filter.favorite == MINFO_MEDIA_UNFAV_ONLY) {
		len =
		    g_strlcat(query_where, " and m.rating = 0 ",
			      sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	if (folder_type != MINFO_CLUSTER_TYPE_ALL) {
		switch (folder_type) {
		case MINFO_CLUSTER_TYPE_LOCAL_ALL:
			snprintf(condition_str, sizeof(condition_str),
				 " and (storage_type = %d or storage_type = %d)",
				 MINFO_PHONE, MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_PHONE:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_PHONE);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_MMC:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_WEB:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ", MINFO_WEB);
			break;
		case MINFO_CLUSTER_TYPE_STREAMING:
			snprintf(condition_str, sizeof(condition_str),
				 " and storage_type = %d ",
				 MINFO_WEB_STREAMING);
			break;
		default:
			break;
		}

		len =
		    g_strlcat(query_where, condition_str, sizeof(query_where));
		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	len =
	    snprintf(condition_str, sizeof(condition_str),
		     " and (content_type = 0");
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		condition_str[0] = '\0';
	} else {
		condition_str[len] = '\0';
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	if (filter.file_type & MINFO_ITEM_ALL) {
		filter.file_type = MINFO_ITEM_IMAGE | MINFO_ITEM_VIDEO;
	}

	if (filter.file_type & MINFO_ITEM_IMAGE) {
		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " or content_type = %d", MINFO_ITEM_IMAGE);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	if (filter.file_type & MINFO_ITEM_VIDEO) {
		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " or content_type = %d", MINFO_ITEM_VIDEO);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
	}

	len = g_strlcat(condition_str, ") ", sizeof(condition_str));
	if (len >= sizeof(condition_str)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	len = g_strlcat(query_where, condition_str, sizeof(query_where));
	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
/*
	switch (search_field) {
		case MINFO_SEARCH_BY_NAME:
			like_str = sqlite3_mprintf("and display_name like '%%%s%%' ", search_str);
			break;
		case MINFO_SEARCH_BY_PATH:
			like_str = sqlite3_mprintf("and path like '%%%s%%' ", search_str);
			break;
		case MINFO_SEARCH_BY_HTTP_URL:
			like_str = sqlite3_mprintf("and http_url like '%%%s%%' ", search_str);
			break;
		default:
			break;
	}
*/
	if (search_field & MINFO_SEARCH_BY_NAME) {
		like_str = sqlite3_mprintf("and (display_name like '%%%q%%' ", search_str);

		len = g_strlcat(query_where, like_str, sizeof(query_where));
		sqlite3_free(like_str);

		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (search_field & MINFO_SEARCH_BY_PATH) {
		if (search_field & MINFO_SEARCH_BY_NAME) {
			like_str = sqlite3_mprintf("or m.path like '%%%q%%' ", search_str);
		} else {
			like_str = sqlite3_mprintf("and (m.path like '%%%q%%' ", search_str);
		}

		len = g_strlcat(query_where, like_str, sizeof(query_where));
		sqlite3_free(like_str);

		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (search_field & MINFO_SEARCH_BY_HTTP_URL) {
		if ((search_field & MINFO_SEARCH_BY_NAME) || (search_field & MINFO_SEARCH_BY_PATH)) {
			like_str = sqlite3_mprintf("or http_url like '%%%q%%' ", search_str);
		} else {
			like_str = sqlite3_mprintf("and (http_url like '%%%q%%' ", search_str);
		}

		len = g_strlcat(query_where, like_str, sizeof(query_where));
		sqlite3_free(like_str);

		if (len >= sizeof(query_where)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	len = g_strlcat(query_where, ") ", sizeof(query_where));

	if (len >= sizeof(query_where)) {
		mb_svc_debug("strlcat returns failure ( %d )", len);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	
	len =
	    snprintf(query_complete_string, sizeof(query_complete_string),
		     "%s %s ORDER BY m.%s", query_string, query_where,
		     mb_svc_media_order[filter.sort_type]);
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		query_complete_string[0] = '\0';
	} else {
		query_complete_string[len] = '\0';
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	if (filter.start_pos != MB_SVC_DB_DEFAULT_GET_ALL_RECORDS)	{/* -1 get all record */
		int length = filter.end_pos - filter.start_pos + 1;
		if (length <= 0) {
			mb_svc_debug
			    ("start position and end position is invalid ( start:%d, end:%d )",
			     filter.start_pos, filter.end_pos);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		len =
		    snprintf(condition_str, sizeof(condition_str),
			     " LIMIT %d,%d", filter.start_pos, length);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			condition_str[0] = '\0';
		} else {
			condition_str[len] = '\0';
		}
		len =
		    g_strlcat(query_complete_string, condition_str,
			      sizeof(query_complete_string));
		if (len >= sizeof(query_complete_string)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	mb_svc_debug("############### SQL: %s\n", query_complete_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_complete_string,
			       strlen(query_complete_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_complete_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_media_iter_next(mb_svc_iterator_s *mb_svc_iterator,
		       mb_svc_media_record_s *record)
{
	int err = -1;

	/* mb_svc_debug ("mb_svc_media_iter_next--enter\n"); */

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("mb_svc_iterator == NULL || record == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	if (record) {
		err = mb_svc_load_record_media(mb_svc_iterator->stmt, record);
		if (err < 0) {
			mb_svc_debug("failed to load item\n");
			sqlite3_finalize(mb_svc_iterator->stmt);
			mb_svc_iterator->current_position = -1;
			return MB_SVC_ERROR_DB_INTERNAL;
		}
	}
	mb_svc_iterator->current_position++;

	return 0;
}

int mb_svc_iter_finish(mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;

	/* mb_svc_debug ("mb_svc_iter_finish---enter\n"); */

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("Error:mb_svc_iterator == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_finalize(mb_svc_iterator->stmt);
	if (SQLITE_OK != err) {
		mb_svc_debug("failed to clear row\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_iterator->current_position = -1;
	mb_svc_iterator->total_count = -1;
	mb_svc_iterator->stmt = NULL;

	/* mb_svc_debug ("mb_svc_iter_finish---leave\n"); */
	return 0;
}

int
mb_svc_get_video_record_by_media_id(const char *media_id,
				    mb_svc_video_meta_record_s *
				    video_meta_record)
{

	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_VIDEO_META;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *p_Stmt_mb = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (video_meta_record == NULL) {
		mb_svc_debug("folder path is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_VIDEO_BY_MUUID, table_name,
		 media_id);

	mb_svc_debug("Query: %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &p_Stmt_mb, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(p_Stmt_mb);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(p_Stmt_mb);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_video_meta(p_Stmt_mb, video_meta_record);

	sqlite3_finalize(p_Stmt_mb);

	return 0;

}

/*
*
*   get video by media_id from "video_meta" table, 
*   condition: each video_meta record mapped to  media_id one by one
*/
int
mb_svc_get_image_record_by_media_id(const char *media_id,
				    mb_svc_image_meta_record_s *
				    image_meta_record)
{

	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_IMAGE_META;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *stmt = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (image_meta_record == NULL) {
		mb_svc_debug("folder path is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_IMAGE_BY_MUUID, table_name,
		 media_id);

	mb_svc_debug("Query: %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_image_meta(stmt, image_meta_record);

	sqlite3_finalize(stmt);

	return 0;

}

int
mb_svc_get_folder_fullpath_by_folder_id(const char *folder_id, char *folder_fullpath,
					int max_length)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *stmt = NULL;
	char uri[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	int ret_len = 0;
	char *tmp = NULL;

	/* mb_svc_debug ("mb_svc_get_folder_fullpath_by_folder_id--enter\n"); */

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (folder_fullpath == NULL) {
		mb_svc_debug("folder path is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret_len =
	    snprintf(query_string, sizeof(query_string),
		     MB_SVC_TABLE_SELECT_FOLDER_URI_BY_FUUID,
		     table_name, folder_id);

	if (ret_len >= sizeof(query_string)) {
		mb_svc_debug("the query string's length is violation!\n");
		return MB_SVC_ERROR_INTERNAL;
	} else if (ret_len < 0) {
		mb_svc_debug("snprintf failed!\n");
		return MB_SVC_ERROR_INTERNAL;
	}

	mb_svc_debug("Query : %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	/* only one field(uri) selected, so the second para of *_text is int 0, means the first one of the selected result. */
	tmp = (char *)sqlite3_column_text(stmt, 0);
	if (strlen(tmp) >= sizeof(uri)) {
		mb_svc_debug("real uri's length is violation!\n");
		return MB_SVC_ERROR_INTERNAL;
	}

	strncpy(uri, (const char *)tmp, sizeof(uri));	/* get path of folder */
	strncpy(folder_fullpath, uri, max_length);
	mb_svc_debug("Full path : %s", folder_fullpath);

	sqlite3_finalize(stmt);

	return 0;
}

int
mb_svc_get_media_fullpath(const char *folder_id, char *media_display_name,
			  char *media_fullpath)
{
	int err = -1;
	char folder_fullpath[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };

	if (media_display_name == NULL || media_fullpath == NULL) {
		mb_svc_debug("Error: NULL pointer  \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	err =
	    mb_svc_get_folder_fullpath_by_folder_id(folder_id, folder_fullpath,
						    sizeof(folder_fullpath));
	if (err < 0) {
		mb_svc_debug("get folder fullpath error\n");
		return err;
	}

	strncat(folder_fullpath, "/",
		MB_SVC_FILE_PATH_LEN_MAX - strlen(folder_fullpath));
	strncat(folder_fullpath, media_display_name,
		MB_SVC_FILE_PATH_LEN_MAX - strlen(folder_fullpath));
	strncpy(media_fullpath, folder_fullpath, MB_SVC_FILE_PATH_LEN_MAX);

	return 0;
}

int
mb_svc_folder_iter_start(minfo_cluster_filter *cluster_filter,
			 mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char query_where[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char tmp_str[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	mb_svc_debug("mb_svc_folder_iter_start--enter\n");

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (cluster_filter == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug
		    ("cluster_filter == NULL || mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (cluster_filter->start_pos >= 0
	    && cluster_filter->start_pos > cluster_filter->end_pos) {
		mb_svc_debug(" cluster_filter->start_pos (%d) > cluster_filter->end_pos (%d) = %d\n",
			     cluster_filter->start_pos, cluster_filter->end_pos);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	strncpy(query_where, " WHERE valid = 1 ", MB_SVC_DEFAULT_QUERY_SIZE);

	snprintf(query_string, MB_SVC_DEFAULT_QUERY_SIZE,
		 MB_SVC_TABLE_SELECT_FOLDER_ALL_QUERY_STRING, table_name);

	if (cluster_filter->cluster_type != MINFO_CLUSTER_TYPE_ALL) {
		switch (cluster_filter->cluster_type) {
		case MINFO_CLUSTER_TYPE_LOCAL_ALL:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and (storage_type = %d or storage_type = %d)",
				 MINFO_PHONE, MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_PHONE:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_PHONE);
			break;
		case MINFO_CLUSTER_TYPE_LOCAL_MMC:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_MMC);
			break;
		case MINFO_CLUSTER_TYPE_WEB:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ", MINFO_WEB);
			break;
		case MINFO_CLUSTER_TYPE_STREAMING:
			snprintf(tmp_str, sizeof(tmp_str),
				 " and storage_type = %d ",
				 MINFO_WEB_STREAMING);
			break;
		default:
			break;
		}
		strncat(query_where, tmp_str,
			MB_SVC_DEFAULT_QUERY_SIZE - strlen(tmp_str));
	}
	strncat(query_string, query_where,
		MB_SVC_DEFAULT_QUERY_SIZE - strlen(query_string));
	strncat(query_string, " ORDER BY ",
		MB_SVC_DEFAULT_QUERY_SIZE - strlen(query_string));
	strncat(query_string, mb_svc_folder_order[cluster_filter->sort_type],
		MB_SVC_DEFAULT_QUERY_SIZE - strlen(query_string));

	if (cluster_filter->start_pos != MB_SVC_DB_DEFAULT_GET_ALL_RECORDS)	{/* -1 get all record */
		int length =
		    cluster_filter->end_pos - cluster_filter->start_pos + 1;
		if (length <= 0) {
			mb_svc_debug
			    ("start position and end position is invalid ( start:%d, end:%d )",
			     cluster_filter->start_pos,
			     cluster_filter->end_pos);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		snprintf(tmp_str, sizeof(tmp_str), " LIMIT %d,%d",
			 cluster_filter->start_pos, length);
		strcat(query_string, tmp_str);
	}

	mb_svc_debug("############### SQL: %s\n", query_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

static int __mb_svc_folder_by_path_iter_start(char *parent_path, mb_svc_iterator_s *mb_svc_iterator)
{
	mb_svc_debug("");

	if (parent_path == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug("parent_path == NULL || mb_svc_iterator == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	char *query_string = NULL;
	char path_like[MB_SVC_FILE_PATH_LEN_MAX + 1];

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_FOLDER);
	snprintf(path_like, sizeof(path_like), "%s/%%", parent_path);

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_SELECT_FOLDER_BY_PATH, table_name,
			    parent_path, path_like);
	mb_svc_debug("############### SQL: %s\n", query_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_folder_iter_next(mb_svc_iterator_s *mb_svc_iterator,
			mb_svc_folder_record_s *record)
{
	int err = -1;

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("pointer mb_svc_iterator is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	if (record) {
		err = mb_svc_load_record_folder(mb_svc_iterator->stmt, record);
		if (err < 0) {
			mb_svc_debug("failed to load item\n");
			sqlite3_finalize(mb_svc_iterator->stmt);
			mb_svc_iterator->current_position = -1;
			return MB_SVC_ERROR_DB_INTERNAL;
		}
	}

	mb_svc_iterator->current_position++;

	return 0;
}

/*
*
* get folder content count from media table according to specified folder ID
*/
int mb_svc_get_folder_content_count_by_folder_id(const char *folder_id)
{
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	int err = -1;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);


	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_MEDIA);

	snprintf(q_string, sizeof(q_string),
		 MB_SVC_FOLDER_CONTENT_COUNT_BY_FUUID, table_name,
		 folder_id);

	mb_svc_debug("Query : %s", q_string);

	err =
	    sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	stmt = NULL;
	mb_svc_debug("record count of table %s is %d\n", table_name, count);
	return count;
}

/**
*   caller need to provide memory space for storing bookmark_record
*/
int
mb_svc_get_bookmark_record_by_id(int record_id,
				 mb_svc_bookmark_record_s *record)
{
	sqlite3_stmt *stmt = NULL;
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_BOOKMARK;
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (record == NULL) {
		mb_svc_debug(" record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int len =
	    snprintf(q_string, sizeof(q_string),
		     MB_SVC_TABLE_SELECT_BOOKMARK_BY_BID_QUERY_STRING,
		     table_name, record_id);
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		q_string[0] = '\0';
	} else {
		q_string[len] = '\0';
	}

	mb_svc_debug("Query: %s", q_string);

	err =
	    sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_bookmark(stmt, record);
	mb_svc_debug(" bookmark record thumbnail path = %s\n",
		     record->thumbnail_path);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return 0;
}

/**
*   caller need to provide memory space for storing bookmark_record
*/
int mb_svc_get_media_tag_by_id(int _id, mb_svc_tag_record_s *mtag_record)
{
	sqlite3_stmt *stmt = NULL;
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_TAG;
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	if (mtag_record == NULL) {
		mb_svc_debug(" record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(q_string, sizeof(q_string),
		 MB_SVC_TABLE_SELECT_TAG_BY_TID_QUERY_STRING, table_name, _id);
	mb_svc_debug("Query: %s", q_string);

	err =
	    sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", q_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_tag(stmt, mtag_record);
	mb_svc_debug(" tag record tag name = %s and media_id is %s!\n",
		     mtag_record->tag_name, mtag_record->media_uuid);

	sqlite3_finalize(stmt);
	stmt = NULL;

	return 0;
}

int
mb_svc_get_web_album_cluster_record(int sns_type, const char *name, const char *account_id, const char *album_id, mb_svc_folder_record_s *folder_record)
{
	int err = -1;
	sqlite3_stmt *stmt = NULL;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char *query_string = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if(album_id == NULL) {
		query_string = sqlite3_mprintf(MB_SVC_TABLE_SELECT_WEB_CLUSTER_RECORD_QUERY_STRING,
										table_name, sns_type, name, account_id);
	}else {
		query_string = sqlite3_mprintf(MB_SVC_TABLE_SELECT_WEB_ALBUM_CLUSTER_RECORD_QUERY_STRING,
										table_name, sns_type, name, account_id, album_id);
	}
	
	err = sqlite3_prepare_v2(handle, query_string, strlen(query_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = mb_svc_load_record_folder(stmt, folder_record);
	if (err < 0) {
		mb_svc_debug("mb-svc load data failed");
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

int
mb_svc_get_folder_list_by_web_account_id(char *web_account,
					 GList **p_record_list)
{
	int record_cnt = 0;
	mb_svc_folder_record_s *fd_record;
	int err = -1;
	GList *l_record_list = NULL;
	mb_svc_iterator_s mb_svc_iterator;
	minfo_cluster_filter cluster_filter = { 0 };

	if (web_account == NULL || p_record_list == NULL) {
		mb_svc_debug
		    ("Error: web_account == NULL || p_record_list == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	cluster_filter.cluster_type = MINFO_CLUSTER_TYPE_WEB;
	cluster_filter.sort_type = MINFO_CLUSTER_SORT_BY_NONE;
	cluster_filter.start_pos = MB_SVC_DB_DEFAULT_GET_ALL_RECORDS;

	err = mb_svc_folder_iter_start(&cluster_filter, &mb_svc_iterator);

	if (err == MB_SVC_ERROR_DB_NO_RECORD) {
		return err;
	} else if (err < 0) {
		mb_svc_debug("mb-svc iterator start failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		fd_record =
		    (mb_svc_folder_record_s *)
		    malloc(sizeof(mb_svc_folder_record_s));
		if (fd_record == NULL) {
			mb_svc_debug("allocate memory failed\n");
			mb_svc_iter_finish(&mb_svc_iterator);
			return MB_SVC_ERROR_OUT_OF_MEMORY;
		}
		memset(fd_record, 0x00, sizeof(mb_svc_folder_record_s));

		err = mb_svc_folder_iter_next(&mb_svc_iterator, fd_record);
		if (err == MB_SVC_NO_RECORD_ANY_MORE) {
			free(fd_record);
			fd_record = NULL;
			break;
		}

		if (err < 0) {
			mb_svc_debug
			    ("mb-svc iterator get next recrod failed\n");
			mb_svc_iter_finish(&mb_svc_iterator);
			free(fd_record);
			return err;
		}

		if (strcmp(fd_record->web_account_id, web_account)) {
			free(fd_record);
			mb_svc_debug
			    ("mb-svc iterator -------different get next\n ");
			continue;
		}

		record_cnt++;
		l_record_list = g_list_append(l_record_list, fd_record);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	*p_record_list = l_record_list;

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return 0;
}

static int __mb_svc_get_folder_record_by_path_info(const char *uri, char *display_name,
										minfo_store_type storage_type, mb_svc_folder_record_s *record)
{
	int err = -1;
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	char *query_string = NULL;

	sqlite3_stmt *stmt = NULL;
	if (record == NULL || uri == NULL || display_name == NULL) {
		mb_svc_debug("pointer image_meta_record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_FOLDER);

	query_string = sqlite3_mprintf(MB_SVC_TABLE_SELECT_FOLDER_BY_PATH_INFO,
									table_name, uri, display_name, storage_type);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = mb_svc_load_record_folder(stmt, record);
	if (err < 0) {
		mb_svc_debug("mb-svc load data failed");
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

static int __mb_svc_get_folder_record_by_full_path(const char *folder_full_path,
				      mb_svc_folder_record_s *folder_record)
{
	minfo_store_type store_type = MINFO_SYSTEM;
	int err = -1;
	char display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };

	if (folder_full_path == NULL || folder_record == NULL) {
		mb_svc_debug
		    ("Error:folder_full_path == NULL || folder_record == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	store_type = _mb_svc_get_store_type_by_full(folder_full_path);
	if (store_type == MB_SVC_ERROR_INTERNAL) {
		mb_svc_debug("Failed to get storage type : %s",
			     folder_full_path);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	_mb_svc_get_dir_display_name(folder_full_path, display_name);

	err = __mb_svc_get_folder_record_by_path_info(folder_full_path, display_name, store_type, folder_record);
	if (err < 0) {
		mb_svc_debug
		    ("Error:get folder record via uri and display name failed\n");
		return err;
	}

	return 0;
}

int
mb_svc_get_folder_id_by_full_path(const char *folder_full_path, char *folder_id, int max_length)
{
	minfo_store_type store_type = MINFO_SYSTEM;
	int err = -1;
	char display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char rel_path[MB_SVC_FILE_PATH_LEN_MAX + 1] = { 0 };
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char *query_string = NULL;
	sqlite3_stmt *stmt = NULL;

	if (folder_full_path == NULL || folder_id == NULL) {
		mb_svc_debug
		    ("Error:folder_full_path == NULL || folder_id == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	store_type = _mb_svc_get_store_type_by_full(folder_full_path);
	if (store_type == MB_SVC_ERROR_INTERNAL) {
		mb_svc_debug("Failed to get storage type : %s",
			     folder_full_path);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	_mb_svc_get_rel_path_by_full(folder_full_path, rel_path);
	_mb_svc_get_dir_display_name(rel_path, display_name);

	query_string = sqlite3_mprintf(MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_PATH_INFO,
									table_name, folder_full_path, display_name, store_type);

	mb_svc_debug("Query : %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	strncpy(folder_id, (const char *)sqlite3_column_text(stmt, 0), max_length);

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

int
mb_svc_get_folder_id_by_web_album_id(const char *web_album_id, char *folder_id)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char *query_string = NULL;
	sqlite3_stmt *stmt = NULL;

	if (web_album_id == NULL || folder_id == NULL) {
		mb_svc_debug
		    ("Error:web_album_id == NULL || folder_id == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string = sqlite3_mprintf(MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_WEB_ALBUM_ID_QUERY_STRING,
									table_name, web_album_id);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	strncpy(folder_id, (const char *)sqlite3_column_text(stmt, 0), MB_SVC_UUID_LEN_MAX + 1);

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);
	return 0;
}

static int __mb_svc_get_media_id_by_fid_name(const char *folder_id, char *display_name, char *media_id)
{
	int err = -1;
	char *query_string = NULL;
	sqlite3_stmt *stmt = NULL;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string = sqlite3_mprintf(MB_SVC_SELECT_MEDIA_ID_BY_FOLDER_UUID_AND_DISPLAY_NAME,
								table_name, folder_id, display_name);

	mb_svc_debug("Query: %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	strncpy(media_id, (const char *)sqlite3_column_text(stmt, 0), MB_SVC_UUID_LEN_MAX + 1);

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

int
mb_svc_get_media_record_by_fid_name(const char *folder_id, const char *display_name,
				    mb_svc_media_record_s *m_record)
{
	int err = -1;
	char *query_string = NULL;
	sqlite3_stmt *stmt = NULL;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string = sqlite3_mprintf(MB_SVC_SELECT_MEDIA_RECORD_BY_FOLDER_ID_AND_DISPLAY_NAME,
									table_name, folder_id, display_name);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_media(stmt, m_record);

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

int mb_svc_update_favorite_by_media_id(const char *media_id, int favorite)
{
	int err = mb_svc_update_favorite_by_id(media_id, favorite);

	return err;
}

int
mb_svc_get_media_record_by_id(const char *media_id,
			      mb_svc_media_record_s *media_record)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *p_Stmt_mb = NULL;

	if (media_id == NULL || media_record == NULL) {
		mb_svc_debug("media_id == NULL || media_record == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_MEDIA_BY_MEDIA_UUID, table_name, media_id);

	mb_svc_debug("Query: %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &p_Stmt_mb, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(p_Stmt_mb);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(p_Stmt_mb);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_media(p_Stmt_mb, media_record);

	mb_svc_debug("Path : %s", media_record->path);
	mb_svc_debug("Thumb : %s", media_record->thumbnail_path);
	sqlite3_finalize(p_Stmt_mb);
	return 0;
}

int
mb_svc_get_folder_name_by_id(const char *folder_id, char *folder_name, int max_length)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	sqlite3_stmt *stmt = NULL;

	if (folder_id == NULL) {
		mb_svc_debug("folder_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (folder_name == NULL) {
		mb_svc_debug("pointer folder_name is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_FOLDER_NAME_BY_UUID, table_name, folder_id);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	err = mb_svc_load_record_folder_name(stmt, folder_name, max_length);

	if (err < 0) {
		mb_svc_debug("mb-svc load data failed");
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_finalize(stmt);

	return 0;

}

int
mb_svc_get_folder_record_by_id(const char *folder_id,
			       mb_svc_folder_record_s *folder_record)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *stmt = NULL;

	if (folder_record == NULL) {
		mb_svc_debug("pointer folder_record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_FOLDER_RECORD_BY_UUID, table_name,
		 folder_id);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = mb_svc_load_record_folder(stmt, folder_record);
	if (err < 0) {
		mb_svc_debug("mb-svc load data failed");
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_finalize(stmt);

	return 0;

}

int
mb_svc_get_web_streaming_record_by_id(int webstreaming_id,
				      mb_svc_web_streaming_record_s *
				      webstreaming_record)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_WEB_STREAMING;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	sqlite3_stmt *stmt = NULL;
	if (webstreaming_record == NULL) {
		mb_svc_debug("pointer webstreaming_record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_WEBSTREAMING_RECORD_BY_ID, table_name,
		 webstreaming_id);
	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = mb_svc_load_record_web_streaming(stmt, webstreaming_record);
	if (err < 0) {
		mb_svc_debug("mb-svc load data failed");
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_finalize(stmt);

	return 0;

}

int mb_svc_webstreaming_iter_start(mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_FOLDER_ALL_QUERY_STRING, table_name);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_webstreaming_iter_next(mb_svc_iterator_s *mb_svc_iterator,
			      mb_svc_web_streaming_record_s *
			      webstreaming_record)
{
	int err = -1;

	if (webstreaming_record == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug
		    ("webstreaming_record == NULL || mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
/*
	if (mb_svc_iterator->current_position < -1 || mb_svc_iterator->current_position >= mb_svc_iterator->total_count - 1) {
		mb_svc_debug ("iteration is not started: %d\n", mb_svc_iterator->current_position);
		return MB_SVC_ERROR_DB_OUT_OF_RANGE;
	}
*/
	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	memset(webstreaming_record, 0, sizeof(mb_svc_web_streaming_record_s));
	err =
	    mb_svc_load_record_web_streaming(mb_svc_iterator->stmt,
					     webstreaming_record);
	if (err < 0) {
		mb_svc_debug("failed to load item\n");
		sqlite3_finalize(mb_svc_iterator->stmt);
		mb_svc_iterator->current_position = -1;
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_iterator->current_position++;

	return 0;
}

static int __mb_svc_get_media_list_by_folder_id(const char *folder_id, GList **p_record_list, int valid)
{
	int record_cnt = 0;
	mb_svc_media_record_s *md_record;
	int err = -1;
	GList *l_record_list = NULL;
	mb_svc_iterator_s mb_svc_iterator = { 0 };

	if (p_record_list == NULL) {
		mb_svc_debug("p_record_list is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	minfo_item_filter filter = { 0 };
	filter.favorite = MINFO_MEDIA_FAV_ALL;
	filter.start_pos = MB_SVC_DB_DEFAULT_GET_ALL_RECORDS;
	filter.sort_type = MINFO_MEDIA_SORT_BY_NONE;
	filter.file_type = MINFO_ITEM_IMAGE | MINFO_ITEM_VIDEO;

	err =
	    mb_svc_media_iter_start_new(folder_id, &filter,
					MINFO_CLUSTER_TYPE_ALL, valid, NULL,
					&mb_svc_iterator);

	if (err < 0) {
		mb_svc_debug("mb-svc iterator start failed");
		return err;
	}

	while (1) {
		md_record =
		    (mb_svc_media_record_s *)
		    malloc(sizeof(mb_svc_media_record_s));
		if (md_record == NULL) {
			mb_svc_debug("Error: memory allocation failed\n");
			mb_svc_iter_finish(&mb_svc_iterator);
			_mb_svc_glist_free(&l_record_list, true);
			return MB_SVC_ERROR_OUT_OF_MEMORY;
		}
		memset(md_record, 0x00, sizeof(mb_svc_media_record_s));

		err = mb_svc_media_iter_next(&mb_svc_iterator, md_record);

		if (err == MB_SVC_NO_RECORD_ANY_MORE) {
			if (md_record)
				free(md_record);
			md_record = NULL;
			break;
		}

		if (err < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			if (md_record)
				free(md_record);
			md_record = NULL;
			_mb_svc_glist_free(&l_record_list, true);
			return err;
		}

		record_cnt++;

		l_record_list = g_list_append(l_record_list, md_record);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	*p_record_list = l_record_list;

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return 0;
}

static int __mb_svc_get_invalid_media_list(const minfo_store_type storage_type, GList **p_record_list)
{
	int record_cnt = 0;
	char *sql = NULL;
	mb_svc_media_record_s *md_record;
	int err = -1;
	GList *l_record_list = NULL;
	mb_svc_iterator_s mb_svc_iterator = { 0 };

	if (p_record_list == NULL) {
		mb_svc_debug("p_record_list is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sql = sqlite3_mprintf(MB_SVC_SELECT_INVALID_MEDIA_LIST, storage_type);
	err =
	    sqlite3_prepare_v2(handle, sql, strlen(sql), &mb_svc_iterator.stmt, NULL);

	sqlite3_free(sql);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	while (1) {
		md_record =
		    (mb_svc_media_record_s *)
		    malloc(sizeof(mb_svc_media_record_s));
		if (md_record == NULL) {
			mb_svc_debug("Error: memory allocation failed\n");
			mb_svc_iter_finish(&mb_svc_iterator);
			_mb_svc_glist_free(&l_record_list, true);
			return MB_SVC_ERROR_OUT_OF_MEMORY;
		}
		memset(md_record, 0x00, sizeof(mb_svc_media_record_s));

		err = mb_svc_media_iter_next(&mb_svc_iterator, md_record);

		if (err == MB_SVC_NO_RECORD_ANY_MORE) {
			if (md_record)
				free(md_record);
			md_record = NULL;
			break;
		}

		if (err < 0) {
			mb_svc_debug("mb-svc iterator get next recrod failed");
			mb_svc_iter_finish(&mb_svc_iterator);
			if (md_record)
				free(md_record);
			md_record = NULL;
			_mb_svc_glist_free(&l_record_list, true);
			return err;
		}

		record_cnt++;

		l_record_list = g_list_append(l_record_list, md_record);
	}

	mb_svc_iter_finish(&mb_svc_iterator);
	*p_record_list = l_record_list;

	if (record_cnt == 0)
		return MB_SVC_ERROR_DB_NO_RECORD;
	else
		return 0;
}

static int __mb_svc_delete_media_records_list(GList *p_record_list)
{
	int ret = -1;
	mb_svc_media_record_s *m_data = NULL;
	int i = 0;

	if (p_record_list != NULL) {
		char *sql = NULL;
		GList *delete_sql_list = NULL;
		int trans_count = 30;
		int cur_trans_count = 0;
		int length = g_list_length(p_record_list);

		for (i = 0; i < length; i++) {
			
			m_data =
			    (mb_svc_media_record_s *)
			    g_list_nth_data(p_record_list, i);

			ret = mb_svc_delete_record_media_sql(m_data->media_uuid, &sql);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_delete_record_media_sql failed\n");
			}

			mb_svc_sql_list_add(&delete_sql_list, &sql);

			if (m_data->content_type == MINFO_ITEM_IMAGE) {
				ret = mb_svc_delete_record_image_meta_sql(m_data->media_uuid, &sql);
				if (ret < 0) {
					mb_svc_debug
						("mb_svc_delete_record_image_meta_sql failed\n");
				}

				mb_svc_sql_list_add(&delete_sql_list, &sql);

			} else if (m_data->content_type == MINFO_ITEM_VIDEO) {
				ret = mb_svc_delete_record_video_meta_sql(m_data->media_uuid, &sql);
				if (ret < 0) {
					mb_svc_debug
						("mb_svc_delete_record_video_meta_sql failed\n");
				}

				mb_svc_sql_list_add(&delete_sql_list, &sql);

				ret =
					mb_svc_delete_bookmark_meta_by_media_id_sql(m_data->media_uuid, &sql);
				if (ret < 0) {
					mb_svc_debug
						("mb_svc_delete_bookmark_meta_by_media_id_sql failed\n");
				}

				mb_svc_sql_list_add(&delete_sql_list, &sql);
			}

			ret =
			    mb_svc_delete_tagmap_by_media_id_sql(m_data->media_uuid, &sql);
			if (ret < 0) {
				mb_svc_debug
				    ("mb_svc_delete_tagmap_by_media_id_sql failed (%d)\n", ret);
			}

			mb_svc_sql_list_add(&delete_sql_list, &sql);

			/* delete thumbnail file directly */
			if (strlen(m_data->http_url) == 0) {
				ret = _mb_svc_thumb_rm(m_data->thumbnail_path);
			}
			if (ret < 0) {
				mb_svc_debug
				    ("_mb_svc_thumb_delete fail:file is %s\n",
				     m_data->thumbnail_path);
			}

			cur_trans_count++;

			if (cur_trans_count >= trans_count) {
				cur_trans_count = 0;
				
				/* Start transaction */
				int i = 0;
				int length = g_list_length(delete_sql_list);
			
				ret = mb_svc_sqlite3_begin_trans();
				if (ret < 0) {
					mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
					mb_svc_sql_list_release(&delete_sql_list);
					return ret;
				}
			
				for (i = 0; i < length; i++) {
					char *sql = (char *)g_list_nth_data(delete_sql_list, i);
					ret = mb_svc_query_sql(sql);
			
					if (ret < 0) {
						mb_svc_debug
							("mb_svc_query_sql failed.. Now start to rollback\n");
						mb_svc_sqlite3_rollback_trans();
						mb_svc_sql_list_release(&delete_sql_list);
						return ret;
					}
				}
			
				ret = mb_svc_sqlite3_commit_trans();
				if (ret < 0) {
					mb_svc_debug
						("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
					mb_svc_sqlite3_rollback_trans();
					mb_svc_sql_list_release(&delete_sql_list);
					return ret;
				}
			}
		}

		if (cur_trans_count > 0) {
			cur_trans_count = 0;
			
			/* Start transaction */
			int i = 0;
			int length = g_list_length(delete_sql_list);
		
			ret = mb_svc_sqlite3_begin_trans();
			if (ret < 0) {
				mb_svc_debug("mb_svc_sqlite3_begin_trans failed\n");
				mb_svc_sql_list_release(&delete_sql_list);
				return ret;
			}
		
			for (i = 0; i < length; i++) {
				char *sql = (char *)g_list_nth_data(delete_sql_list, i);
				ret = mb_svc_query_sql(sql);
		
				if (ret < 0) {
					mb_svc_debug
						("mb_svc_query_sql failed.. Now start to rollback\n");
					mb_svc_sqlite3_rollback_trans();
					mb_svc_sql_list_release(&delete_sql_list);
					return ret;
				}
			}
		
			ret = mb_svc_sqlite3_commit_trans();
			if (ret < 0) {
				mb_svc_debug
					("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
				mb_svc_sqlite3_rollback_trans();
				mb_svc_sql_list_release(&delete_sql_list);
				return ret;
			}
		}
		
		mb_svc_sql_list_release(&delete_sql_list);
	}

	return 0;
}

int mb_svc_delete_folder(const char *folder_id, minfo_store_type storage_type)
{
	int ret = 0;
	GList *p_record_list = NULL;

	if (folder_id == NULL) {
		mb_svc_debug("folder_id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = __mb_svc_get_media_list_by_folder_id(folder_id, &p_record_list, TRUE);

	if (ret == MB_SVC_ERROR_DB_NO_RECORD) {
		mb_svc_debug("There's no item in the folder %s", folder_id);
		goto DELETE_FOLDER;
	} else if (ret < 0) {
		mb_svc_debug("minfo_delete_cluster, __mb_svc_get_media_list_by_folder_id failed\n");
		return ret;
	}

	ret = __mb_svc_delete_media_records_list(p_record_list);
	_mb_svc_glist_free(&p_record_list, true);

	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_cluster, delete media related records by folder_id failed\n");
		return ret;
	}

 DELETE_FOLDER:
	ret = mb_svc_delete_record_folder_by_id(folder_id);

	if (ret < 0) {
		mb_svc_debug
		    ("minfo_delete_cluster, delete matched folder record failed\n");
		return ret;
	}

	return 0;
}

/*
*
* caller need to provide the local statement--stmt
*/
int
mb_svc_bookmark_iter_start(const char *media_id, mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_BOOKMARK;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_BOOKMARK_ALL_BY_MUUID, table_name,
		 media_id);

	mb_svc_debug("Query : %s", query_string);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int
mb_svc_bookmark_iter_next(mb_svc_iterator_s *mb_svc_iterator,
			  mb_svc_bookmark_record_s *record)
{
	int err = -1;

	if (record == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug("pointer record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	err = mb_svc_load_record_bookmark(mb_svc_iterator->stmt, record);
	if (err < 0) {
		mb_svc_debug("failed to load item\n");
		sqlite3_finalize(mb_svc_iterator->stmt);
		mb_svc_iterator->current_position = -1;
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_iterator->current_position++;

	return 0;
}

/*
*
* caller need to provide the local statement--stmt
*/
int
mb_svc_tag_iter_start(const char *tag_name, const char *media_id,
		      mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char query_string_with_lock_status[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int len = 0;

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (tag_name != NULL) {
		/* set to get only unlocked items */
		len =
		    snprintf(query_string_with_lock_status,
			     sizeof(query_string_with_lock_status),
			     MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_TAG_NAME_WITH_LOCK_STATUS,
			     tag_name, 0);
	} else if (media_id == NULL) {
		/* set to get only unlocked items */
		len =
		    snprintf(query_string_with_lock_status,
			     sizeof(query_string_with_lock_status),
			     MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_WITH_LOCK_STATUS,
			     0);
	} else {
		/* set to get only unlocked items */
		len =
		    snprintf(query_string_with_lock_status,
			     sizeof(query_string_with_lock_status),
			     MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_MEDIA_ID_WITH_LOCK_STATUS,
			     media_id, 0);
	}

	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		query_string_with_lock_status[0] = '\0';
	} else {
		query_string_with_lock_status[len] = '\0';
	}

	mb_svc_debug("Query : %s", query_string_with_lock_status);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string_with_lock_status,
			       strlen(query_string_with_lock_status),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n",
			     query_string_with_lock_status);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int
mb_svc_tag_iter_with_filter_start(const char *tag_name, minfo_tag_filter filter,
				  mb_svc_iterator_s *mb_svc_iterator)
{
	int err = -1;
	char query_string_with_lock_status[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char condition_str[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int len = 0;

	if (mb_svc_iterator == NULL) {
		mb_svc_debug("mb_svc_iterator == NULL\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	memset(condition_str, 0x00, sizeof(condition_str));
	if (!(filter.file_type & MINFO_ITEM_ALL)) {
		char content_str[MB_SVC_DEFAULT_QUERY_SIZE] = { 0 };

		len =
		    snprintf(content_str, sizeof(content_str),
			     " and ( m.content_type=%d ", 0);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			content_str[0] = '\0';
		} else {
			content_str[len] = '\0';
		}

		len =
		    g_strlcat(condition_str, content_str,
			      sizeof(condition_str));
		if (len >= sizeof(condition_str)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		if (filter.file_type & MINFO_ITEM_IMAGE) {
			len =
			    snprintf(content_str, sizeof(content_str),
				     " or m.content_type=%d ",
				     MINFO_ITEM_IMAGE);
			if (len < 0) {
				mb_svc_debug("snprintf returns failure ( %d )",
					     len);
				content_str[0] = '\0';
			} else {
				content_str[len] = '\0';
			}

			len =
			    g_strlcat(condition_str, content_str,
				      sizeof(condition_str));
			if (len >= sizeof(condition_str)) {
				mb_svc_debug("strlcat returns failure ( %d )",
					     len);
				return MB_SVC_ERROR_INVALID_PARAMETER;
			}
		}

		if (filter.file_type & MINFO_ITEM_VIDEO) {
			len =
			    snprintf(content_str, sizeof(content_str),
				     " or m.content_type=%d ",
				     MINFO_ITEM_VIDEO);
			if (len < 0) {
				mb_svc_debug("snprintf returns failure ( %d )",
					     len);
				content_str[0] = '\0';
			} else {
				content_str[len] = '\0';
			}

			len =
			    g_strlcat(condition_str, content_str,
				      sizeof(condition_str));
			if (len >= sizeof(condition_str)) {
				mb_svc_debug("strlcat returns failure ( %d )",
					     len);
				return MB_SVC_ERROR_INVALID_PARAMETER;
			}
		}

		len = g_strlcat(condition_str, ") ", sizeof(condition_str));
		if (len >= sizeof(condition_str)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	if (filter.start_pos != MB_SVC_DB_DEFAULT_GET_ALL_RECORDS) {	/* -1 get all record */
		int length = filter.end_pos - filter.start_pos + 1;
		if (length <= 0) {
			mb_svc_debug
			    ("start position and end position is invalid ( start:%d, end:%d )",
			     filter.start_pos, filter.end_pos);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}

		char limit_str[MB_SVC_DEFAULT_QUERY_SIZE] = { 0 };
		len =
		    snprintf(limit_str, sizeof(limit_str), " LIMIT %d,%d",
			     filter.start_pos, length);
		if (len < 0) {
			mb_svc_debug("snprintf returns failure ( %d )", len);
			limit_str[0] = '\0';
		} else {
			limit_str[len] = '\0';
		}

		len =
		    g_strlcat(condition_str, limit_str, sizeof(condition_str));
		if (len >= sizeof(condition_str)) {
			mb_svc_debug("strlcat returns failure ( %d )", len);
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	/* set to get only unlocked items */
	len =
	    snprintf(query_string_with_lock_status,
		     sizeof(query_string_with_lock_status),
		     MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_TAG_NAME_WITH_LOCK_STATUS_AND_FILTER,
		     tag_name, 0, condition_str);

	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		query_string_with_lock_status[0] = '\0';
	} else {
		query_string_with_lock_status[len] = '\0';
	}

	mb_svc_debug("Query : %s", query_string_with_lock_status);

	mb_svc_iterator->current_position = 0;

	err =
	    sqlite3_prepare_v2(handle, query_string_with_lock_status,
			       strlen(query_string_with_lock_status),
			       &mb_svc_iterator->stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n",
			     query_string_with_lock_status);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

static int __mb_svc_delete_tag_by_id(const int tag_id)
{
	int err = -1;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG;

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_DELETE_TAG_BY_TAGID, table_name,
			    tag_id);

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("failed to delete tagmap\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_delete_tagmap_by_media_id(const char *media_id)
{
	int err = -1;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG_MAP;

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_DELETE_TAG_MAP_BY_MEDIA_UUID, table_name,
			    media_id);

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("failed to delete tagmap\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_delete_record_tag(const char *tag_name, const char *media_id)
{
	int err = -1;
	int tag_id = 0;
	int count = 0;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG_MAP;

	if (tag_name == NULL) {
		mb_svc_debug("tag_name pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	tag_id = mb_svc_get_tagid_by_tagname(tag_name);
	if (tag_id <= 0) {
		mb_svc_debug("There's no tag %s in the table");

		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (media_id == NULL) {
		query_string =
		    sqlite3_mprintf(MB_SVC_TABLE_DELETE_TAG_MAP_BY_TAGNAME,
				    table_name, tag_id);
	} else {
		query_string =
		    sqlite3_mprintf
		    (MB_SVC_TABLE_DELETE_TAG_MAP_BY_TAGNAME_MEDIA_UUID,
		     table_name, media_id, tag_id);
	}

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("failed to delete tagmap\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	count = __mb_svc_get_media_cnt_by_tagid(tag_id);

	if (count <= 0) {
		err = __mb_svc_delete_tag_by_id(tag_id);
		if (err < 0) {
			mb_svc_debug("__mb_svc_delete_tag_by_id : %d", tag_id);
			return err;
		}
	}

	return err;
}

int
mb_svc_media_id_list_by_tag_iter_next(mb_svc_iterator_s *mb_svc_iterator,
				      mb_svc_tag_record_s *record)
{
	int err = -1;

	if (record == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug("pointer record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	record->_id = sqlite3_column_int(mb_svc_iterator->stmt, 0);
	//record->media_id = sqlite3_column_int(mb_svc_iterator->stmt, 1);
	strncpy(record->media_uuid, (const char *)sqlite3_column_text(mb_svc_iterator->stmt, 1), MB_SVC_UUID_LEN_MAX + 1);
	strncpy(record->tag_name, "", MB_SVC_ARRAY_LEN_MAX + 1);

	mb_svc_iterator->current_position++;
	return 0;
}

int
mb_svc_tag_iter_next(mb_svc_iterator_s *mb_svc_iterator,
		     mb_svc_tag_record_s *record)
{
	int err = -1;

	if (record == NULL || mb_svc_iterator == NULL) {
		mb_svc_debug("pointer record is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	err = sqlite3_step(mb_svc_iterator->stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of iteration : count = %d\n",
			     mb_svc_iterator->current_position);
		return MB_SVC_NO_RECORD_ANY_MORE;
	}

	err = mb_svc_load_record_tag(mb_svc_iterator->stmt, record);
	if (err < 0) {
		mb_svc_debug("failed to load item\n");
		sqlite3_finalize(mb_svc_iterator->stmt);
		mb_svc_iterator->current_position = -1;
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_iterator->current_position++;
	return 0;
}

int mb_svc_rename_record_tag(const char *src_tagname, const char *dst_tag_name)
{
	int err = 0;

	int src_tag_id = mb_svc_get_tagid_by_tagname(src_tagname);

	if (src_tag_id <= 0) {
		mb_svc_debug("there's no tag %s ", src_tagname);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int dst_tag_id = mb_svc_get_tagid_by_tagname(dst_tag_name);

	if (dst_tag_id > 0) {
		err = __mb_svc_update_tagmap(src_tag_id, dst_tag_id);
	} else {
		err = __mb_svc_update_tag(src_tag_id, dst_tag_name);
	}

	return err;
}

int
mb_svc_rename_record_tag_by_id(const char *media_id, const char *src_tagname,
			       const char *dst_tag_name)
{
	int err = 0;
	mb_svc_tag_record_s tag_record = { 0 };

	int src_tag_id = mb_svc_get_tagid_by_tagname(src_tagname);

	if (src_tag_id <= 0) {
		mb_svc_debug("there's no tag %s ", src_tagname);
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int dst_tag_id = mb_svc_get_tagid_by_tagname(dst_tag_name);

	if (dst_tag_id > 0) {
		err = __mb_svc_update_tagmap_by_media_id(media_id, src_tag_id, dst_tag_id);
	} else {

		strncpy(tag_record.tag_name, dst_tag_name,
			MB_SVC_ARRAY_LEN_MAX + 1);

		err = mb_svc_insert_record_tag(&tag_record);
		if (err < 0) {
			mb_svc_debug("mb_svc_insert_record_tag fail\n");
			return err;
		}

		err = __mb_svc_update_tagmap_by_media_id(media_id, src_tag_id, tag_record._id);
	}

	return err;
}

static int __mb_svc_update_tag(int tag_id, const char *tag_name)
{
	mb_svc_debug("");

	int err = -1;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG;

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_UPDATE_TAG_NAME_QUERY_STRING_BY_TAG_ID,
			    table_name, tag_name, tag_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("__mb_svc_update_tag failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

static int __mb_svc_update_tagmap(int src_tag_id, int dst_tag_id)
{
	mb_svc_debug("");

	int err = -1;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG_MAP;

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_UPDATE_TAG_MAP_QUERY_STRING_BY_TAG_ID,
			    table_name, dst_tag_id, src_tag_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("mb_svc_update_tag failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

static int __mb_svc_update_tagmap_by_media_id(const char *media_id, int src_tag_id, int dst_tag_id)
{
	mb_svc_debug("");

	int err = -1;
	char *query_string = NULL;
	char *table_name = MB_SVC_TBL_NAME_TAG_MAP;

	query_string =
	    sqlite3_mprintf
	    (MB_SVC_TABLE_UPDATE_TAG_MAP_QUERY_STRING_BY_TAG_ID_AND_MEDIA_ID,
	     table_name, dst_tag_id, media_id, src_tag_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("mb_svc_update_tag failed\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_get_tagid_by_tagname(const char *tag_name)
{
	mb_svc_debug("");
	char *table_name = MB_SVC_TBL_NAME_TAG;
	char *query_string = NULL;

	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	int return_id = 0;
	int err = -1;

	if (tag_name == NULL) {
		mb_svc_debug("Error:tag_name == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_SELECT_TAG_COUNT_QUERY_STRING,
			    table_name, tag_name);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	sqlite3_free(query_string);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		return_id = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	stmt = NULL;

	return return_id;
}

static int __mb_svc_get_media_cnt_by_tagid(int tag_id)
{
	int rc = 0;
	sqlite3_stmt *stmt = NULL;
	int count = 0;
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_TAG_MAP;
	char *query_string = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_SELECT_MEDIA_CNT_BY_TAGID, table_name,
			    tag_id);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	sqlite3_free(query_string);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		count = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);

	stmt = NULL;
	mb_svc_debug("record count is %d\n", count);
	return count;
}

int mb_svc_add_web_streaming_folder(char *folder_id)
{
	mb_svc_folder_record_s folder_record = {"",};
	int ret = 0;

	folder_record.modified_date = 0;
	folder_record.sns_type = 0;
	folder_record.storage_type = MINFO_WEB_STREAMING;
	strncpy(folder_record.uri, "", MB_SVC_DIR_PATH_LEN_MAX + 1);
	strncpy(folder_record.display_name, "", MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(folder_record.web_account_id, "", MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(folder_record.web_album_id, "", MB_SVC_ARRAY_LEN_MAX + 1);

	ret = mb_svc_insert_record_folder(&folder_record);
	if (ret < 0) {
		mb_svc_debug("mb_svc_insert_record_folder failed\n");
		return ret;
	}

	strncpy(folder_id, folder_record.uuid, MB_SVC_UUID_LEN_MAX + 1);

	return 0;
}

int mb_svc_get_web_streaming_folder_uuid(char *folder_uuid, int max_length)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_FOLDER;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	sqlite3_stmt *stmt = NULL;
	int folder_id = 0;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_WEB_STREAMING, table_name,
		 MINFO_WEB_STREAMING);
	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_finalize(stmt);
		folder_id = -1;
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	strncpy(folder_uuid, (const char *)sqlite3_column_text(stmt, 0), max_length);

	sqlite3_finalize(stmt);

	return 0;
}

int mb_svc_get_media_id_by_full_path(const char *file_full_path, char *media_id)
{
	int err = 0;
	int folder_id = 0;
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};
	char file_display_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };
	char dir_full_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };

	_mb_svc_get_file_parent_path(file_full_path, dir_full_path);
	_mb_svc_get_file_display_name(file_full_path, file_display_name);

	err = mb_svc_get_folder_id_by_full_path(dir_full_path, folder_uuid, sizeof(folder_uuid));
	if (err < 0) {
		mb_svc_debug("mb_svc_get_folder_id_by_full_path fails:%s",
			     dir_full_path);
		return err;
	}

	_mb_svc_get_file_display_name(file_full_path, file_display_name);
	err = __mb_svc_get_media_id_by_fid_name(folder_uuid, file_display_name, media_id);
	if (err < 0) {
		mb_svc_debug("__mb_svc_get_media_id_by_fid_name fails:%d,%s",
			     folder_id, file_display_name);
		return err;
	}

	return 0;
}

int mb_svc_get_media_id_by_http_url(const char *http_url, char *media_id)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;
	char *query_string;
	sqlite3_stmt *stmt = NULL;

	if (http_url == NULL) {
		mb_svc_debug("Error: http_url == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_SELECT_MEDIA_ID_BY_HTTP_URL,
			    table_name, http_url);
	mb_svc_debug("Query : %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);
	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		sqlite3_finalize(stmt);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	strncpy(media_id, (const char *)sqlite3_column_text(stmt,0),
		MB_SVC_UUID_LEN_MAX + 1);

	sqlite3_free(query_string);
	sqlite3_finalize(stmt);

	return 0;
}

int
mb_svc_get_media_record_by_full_path(const char *file_full_path,
				     mb_svc_media_record_s *record)
{
	int err = -1;
	char *table_name = MB_SVC_TBL_NAME_MEDIA;
	char *query_string = NULL;
	sqlite3_stmt *p_Stmt_mb = NULL;

	if (record == NULL) {
		mb_svc_debug("media_record is null \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_TABLE_SELECT_MEDIA_BY_PATH, table_name,
			    file_full_path);

	mb_svc_debug("Query : %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &p_Stmt_mb, NULL);
	sqlite3_free(query_string);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	err = sqlite3_step(p_Stmt_mb);
	if (err != SQLITE_ROW) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(p_Stmt_mb);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	mb_svc_load_record_media(p_Stmt_mb, record);

	sqlite3_finalize(p_Stmt_mb);

	return 0;
}

int mb_svc_delete_invalid_media_records(const minfo_store_type storage_type)
{
	mb_svc_debug("storage_type: %d", storage_type);

	sqlite3_stmt *stmt = NULL;

	int err = -1;
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	char *sql = NULL;
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	GList *invalid_media_list = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	err = __mb_svc_get_invalid_media_list(storage_type, &invalid_media_list);
	if (err == MB_SVC_ERROR_DB_NO_RECORD) {
		mb_svc_debug("There is no invalid media");
		return 0;
	} else if (err < 0) {
		mb_svc_debug("__mb_svc_get_invalid_media_list failed : %d", err);
		mb_svc_debug("Keep going to remove invalid folder..");
	} else {
		err = __mb_svc_delete_media_records_list(invalid_media_list);
		if (err < 0) {
			_mb_svc_glist_free(&invalid_media_list, true);
			mb_svc_debug
				("__mb_svc_delete_media_records_list failed : %d", err);
			return err;
		}
	}

	/* Get folder list to remove, which is invalid */
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_FOLDER);

	sql =
	    sqlite3_mprintf("select uuid from %s where storage_type = %d and valid=0;",
			    table_name, storage_type);
	err = sqlite3_prepare_v2(handle, sql, strlen(sql), &stmt, NULL);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", sql);
		sqlite3_free(sql);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	sqlite3_free(sql);

	for (;;) {
		err = sqlite3_step(stmt);
		if (err != SQLITE_ROW) {
			break;
		}

		strncpy(folder_uuid, (const char *)sqlite3_column_text(stmt, 0), MB_SVC_UUID_LEN_MAX + 1);

		err = mb_svc_delete_record_folder_by_id(folder_uuid);
		if (err < 0) {
			mb_svc_debug("mb_svc_delete_record_folder_by_id fail:%d\n", err);
			return err;
		}
	}

	err = sqlite3_finalize(stmt);
	if (SQLITE_OK != err) {
		mb_svc_debug("failed to clear row\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int mb_svc_get_all_item_count(int *cnt)
{
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int rc = 0;
	sqlite3_stmt *stmt = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	int len =
	    snprintf(q_string, sizeof(q_string), MB_SVC_SELECT_ALL_ITEM_COUNT,
		     MB_SVC_TBL_NAME_MEDIA);
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		q_string[0] = '\0';
	} else {
		q_string[len] = '\0';
	}

	rc = sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt,
				NULL);
	if (SQLITE_OK != rc) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		*cnt = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);
	stmt = NULL;

	return 0;
}

int mb_svc_get_media_count_by_tagname(const char *tagname, int *count)
{
	char q_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int rc = 0;
	sqlite3_stmt *stmt = NULL;

	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	char *tag_name_with_escape = NULL;
	tag_name_with_escape = sqlite3_mprintf("%q", tagname);

	int len =
	    snprintf(q_string, sizeof(q_string),
		     MB_SVC_TABLE_SELECT_COUNT_BY_TAG_NAME_WITH_LOCK_STATUS,
		     tag_name_with_escape, 0);
	if (len < 0) {
		mb_svc_debug("snprintf returns failure ( %d )", len);
		q_string[0] = '\0';
	} else {
		q_string[len] = '\0';
	}

	sqlite3_free(tag_name_with_escape);

	rc = sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt,
				NULL);
	if (SQLITE_OK != rc) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		*count = sqlite3_column_int(stmt, 0);
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);
	stmt = NULL;

	return 0;
}

