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
#include "media-svc-media.h"
#include "media-svc-media-folder.h"
#include "media-svc-error.h"
#include "media-svc-debug.h"
#include "media-svc-util.h"
#include "media-svc-db-utils.h"

typedef struct{
	char thumbnail_path[MEDIA_SVC_PATHNAME_SIZE];
}media_svc_thumbnailpath_s;

static __thread GList *g_media_svc_item_validity_query_list = NULL;
static __thread GList *g_media_svc_insert_item_query_list = NULL;
__thread GList *g_media_svc_move_item_query_list = NULL;

static int __media_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, media_svc_storage_type_e storage_type, int *count);
static int __media_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, media_svc_storage_type_e storage_type,
							int count, media_svc_thumbnailpath_s * thumb_path);

static int __media_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, media_svc_storage_type_e storage_type, int *count)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT count(*) FROM %s WHERE validity=0 AND storage_type=%d AND thumbnail_path IS NOT NULL",
					MEDIA_SVC_DB_TABLE_MEDIA, storage_type);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MEDIA_INFO_ERROR_NONE) {
		media_svc_error("error when __media_svc_count_invalid_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;

}

static int __media_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, media_svc_storage_type_e storage_type,
							int count, media_svc_thumbnailpath_s * thumb_path)
{
	int err = -1;
	int idx = 0;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("select thumbnail_path from %s WHERE validity=0 AND storage_type=%d AND thumbnail_path IS NOT NULL",
					MEDIA_SVC_DB_TABLE_MEDIA, storage_type);

	media_svc_debug("[SQL query] : %s", sql);

	err = sqlite3_prepare_v2(handle, sql, -1, &sql_stmt, NULL);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(thumb_path[idx].thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), sizeof(thumb_path[idx]));
		media_svc_debug("thumb_path[%d]=[%s]", idx, thumb_path[idx].thumbnail_path);
		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_count_record_with_path(sqlite3 *handle, const char *path, int *count)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT count(*) FROM %s WHERE path='%q'", MEDIA_SVC_DB_TABLE_MEDIA, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	media_svc_retv_if(ret != MEDIA_INFO_ERROR_NONE, ret);

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_insert_item_with_data(sqlite3 *handle, media_svc_content_info_s *content_info, bool stack_query)
{
	int err = -1;

	char * db_fields = "media_uuid, path, file_name, media_type, mime_type, size, added_time, modified_time, folder_uuid, \
					thumbnail_path, title, album_id, album, artist, genre, composer, year, recorded_date, copyright, track_num, description,\
					bitrate, samplerate, channel, duration, longitude, latitude, altitude, width, height, datetaken, orientation,\
					rating, is_drm, storage_type";

	char *sql = sqlite3_mprintf("INSERT INTO %s (%s) VALUES (%Q, %Q, %Q, %d, %Q, %lld, %d, %d, %Q, \
													%Q, %Q, %d, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, \
													%d, %d, %d, %d, %f, %f, %f, %d, %d, %Q, %d, \
													%d, %d, %d);",
		MEDIA_SVC_DB_TABLE_MEDIA, db_fields,
		content_info->media_uuid,
		content_info->path,
		content_info->file_name,
		content_info->media_type,
		content_info->mime_type,
		content_info->size,
		content_info->added_time,
		content_info->modified_time,
		content_info->folder_uuid,
		content_info->thumbnail_path,		//
		content_info->media_meta.title,
		content_info->album_id,
		content_info->media_meta.album,
		content_info->media_meta.artist,
		content_info->media_meta.genre,
		content_info->media_meta.composer,
		content_info->media_meta.year,
		content_info->media_meta.recorded_date,
		content_info->media_meta.copyright,
		content_info->media_meta.track_num,
		content_info->media_meta.description,	//
		content_info->media_meta.bitrate,
		content_info->media_meta.samplerate,
		content_info->media_meta.channel,
		content_info->media_meta.duration,
		content_info->media_meta.longitude,
		content_info->media_meta.latitude,
		content_info->media_meta.altitude,
		content_info->media_meta.width,
		content_info->media_meta.height,
		content_info->media_meta.datetaken,
		content_info->media_meta.orientation,
		content_info->media_meta.rating,
		content_info->is_drm,
		content_info->storage_type);

	media_svc_debug("query : %s", sql);

	if(!stack_query) {
		err = _media_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			media_svc_error("failed to insert item");

			return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
		}
	} else {
		_media_svc_sql_query_add(&g_media_svc_insert_item_query_list, &sql);
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_update_item_with_data(sqlite3 *handle, media_svc_content_info_s *content_info)
{
	int err = -1;

	char *sql = sqlite3_mprintf("UPDATE %s SET \
		size=%lld, modified_time=%d, thumbnail_path=%Q, title=%Q, album_id=%d, album=%Q, artist=%Q, genre=%Q, \
		composer=%Q, year=%Q, recorded_date=%Q, copyright=%Q, track_num=%Q, description=%Q, \
		bitrate=%d, samplerate=%d, channel=%d, duration=%d, longitude=%f, latitude=%f, altitude=%f, width=%d, height=%d, datetaken=%Q, \
													orientation=%d WHERE path=%Q",
		MEDIA_SVC_DB_TABLE_MEDIA,
		content_info->size,
		content_info->modified_time,
		content_info->thumbnail_path,
		content_info->media_meta.title,
		content_info->album_id,
		content_info->media_meta.album,
		content_info->media_meta.artist,
		content_info->media_meta.genre,
		content_info->media_meta.composer,
		content_info->media_meta.year,
		content_info->media_meta.recorded_date,
		content_info->media_meta.copyright,
		content_info->media_meta.track_num,
		content_info->media_meta.description,
		content_info->media_meta.bitrate,
		content_info->media_meta.samplerate,
		content_info->media_meta.channel,
		content_info->media_meta.duration,
		content_info->media_meta.longitude,
		content_info->media_meta.latitude,
		content_info->media_meta.altitude,
		content_info->media_meta.width,
		content_info->media_meta.height,
		content_info->media_meta.datetaken,
		content_info->media_meta.orientation,
		content_info->path
		);

	media_svc_debug("query : %s", sql);

	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("failed to update item");

		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;
}
int _media_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *path, char *thumbnail_path)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT thumbnail_path FROM %s WHERE path='%q'", MEDIA_SVC_DB_TABLE_MEDIA, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MEDIA_INFO_ERROR_NONE) {
		if(ret == MEDIA_INFO_ERROR_DATABASE_NO_RECORD) {
			media_svc_debug("there is no thumbnail.");
		}
		else {
			media_svc_error("error when _media_svc_get_thumbnail_path_by_path. err = [%d]", ret);
		}
		return ret;
	}

	_strncpy_safe(thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), MEDIA_SVC_PATHNAME_SIZE);

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_get_media_type_by_path(sqlite3 *handle, const char *path, int *media_type)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT media_type FROM %s WHERE path='%q'", MEDIA_SVC_DB_TABLE_MEDIA, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MEDIA_INFO_ERROR_NONE) {
		media_svc_error("error when _media_svc_get_media_type_by_path. err = [%d]", ret);
		return ret;
	}

	*media_type = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_delete_item_by_path(sqlite3 *handle, const char *path)
{
	int err = -1;
	char *sql = sqlite3_mprintf("DELETE FROM %s WHERE validity=1 AND path='%q'", MEDIA_SVC_DB_TABLE_MEDIA, path);

	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("It failed to delete item (%d)", err);
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_truncate_table(sqlite3 *handle, media_svc_storage_type_e storage_type)
{
	int err = -1;
	char *sql = sqlite3_mprintf("DELETE FROM %s WHERE storage_type=%d", MEDIA_SVC_DB_TABLE_MEDIA, storage_type);

	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("It failed to truncate table (%d)", err);
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;

}

int _media_svc_delete_invalid_items(sqlite3 *handle, media_svc_storage_type_e storage_type)
{
	int idx = 0;
	media_svc_thumbnailpath_s *thumbpath_record = NULL;
	int err = -1;
	int invalid_count = 0;
	int ret = MEDIA_INFO_ERROR_NONE;

	ret = __media_svc_count_invalid_records_with_thumbnail(handle, storage_type, &invalid_count);
	media_svc_retv_if(ret != MEDIA_INFO_ERROR_NONE, ret);

	media_svc_debug("invalid count: %d\n", invalid_count);

	if (invalid_count > 0) {
		thumbpath_record = (media_svc_thumbnailpath_s *)calloc( invalid_count, sizeof(media_svc_thumbnailpath_s));
		if (thumbpath_record == NULL) {
			media_svc_error("fail to memory allocation");
			return MEDIA_INFO_ERROR_OUT_OF_MEMORY;
		}

		ret = __media_svc_get_invalid_records_with_thumbnail(handle, storage_type, invalid_count, thumbpath_record);
		if (ret != MEDIA_INFO_ERROR_NONE) {
			media_svc_error("error when get thumbnail record");
			SAFE_FREE(thumbpath_record);
			return ret;
		}
	} else {
		media_svc_debug("There is no item with thumbnail");
	}

	char *sql = sqlite3_mprintf("DELETE FROM %s WHERE validity = 0 AND storage_type=%d", MEDIA_SVC_DB_TABLE_MEDIA, storage_type);
	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("To delete invalid items is failed(%d)", err);
		SAFE_FREE(thumbpath_record);
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	/*Delete thumbnails*/
	for (idx = 0; idx < invalid_count; idx++) {
		if (strlen(thumbpath_record[idx].thumbnail_path) > 0) {
			if (_media_svc_remove_file(thumbpath_record[idx].thumbnail_path) == FALSE) {
				media_svc_error("fail to remove thumbnail file.");
				SAFE_FREE(thumbpath_record);
				return MEDIA_INFO_ERROR_INTERNAL;
			}
		}
	}

	SAFE_FREE(thumbpath_record);

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_update_item_validity(sqlite3 *handle, const char *path, int validity, bool stack_query)
{
	int err = -1;

	char *sql = sqlite3_mprintf("UPDATE %s SET validity=%d WHERE path= '%q'", MEDIA_SVC_DB_TABLE_MEDIA, validity, path);

	if(!stack_query) {
		err = _media_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			media_svc_error("To update item as valid is failed(%d)", err);
			return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
		}
	} else {
		_media_svc_sql_query_add(&g_media_svc_item_validity_query_list, &sql);
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_update_thumbnail_path(sqlite3 *handle, const char *path, const char *thumb_path)
{
	int err = -1;

	char *sql = sqlite3_mprintf("UPDATE %s SET thumbnail_path=%Q WHERE path= %Q", MEDIA_SVC_DB_TABLE_MEDIA, thumb_path, path);

	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("To update thumb path failed(%d)", err);
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_update_valid_of_music_records(sqlite3 *handle, media_svc_storage_type_e storage_type, int validity)
{
	int err = -1;
	char *sql = sqlite3_mprintf("UPDATE %s SET validity=%d WHERE storage_type=%d", MEDIA_SVC_DB_TABLE_MEDIA, validity, storage_type);
	err = _media_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		media_svc_error("To update item as valid is failed(%d)", err);
		return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_update_item_by_path(sqlite3 *handle, const char *src_path, media_svc_storage_type_e dest_storage, const char *dest_path,
				const char *file_name, int modified_time, const char *folder_uuid, const char *thumb_path, bool stack_query)
{
	/* update path, filename, modified_time, folder_uuid, thumbnail_path, */
	/* played_count, last_played_time, last_played_position, favourite, storaget_type*/

	int err = -1;
	char *sql = NULL;

	if(thumb_path != NULL) {
		sql = sqlite3_mprintf("UPDATE %s SET \
					path=%Q, file_name=%Q, modified_time=%d, folder_uuid=%Q, thumbnail_path=%Q, storage_type=%d, \
					played_count=0, last_played_time=0, last_played_position=0, favourite=0 \
					WHERE path=%Q",
					MEDIA_SVC_DB_TABLE_MEDIA, dest_path, file_name, modified_time, folder_uuid, thumb_path, dest_storage, src_path);
	} else {
		sql = sqlite3_mprintf("UPDATE %s SET \
					path=%Q, file_name=%Q, modified_time=%d, folder_uuid=%Q, storage_type=%d, \
					played_count=0, last_played_time=0, last_played_position=0, favourite=0 \
					WHERE path=%Q",
					MEDIA_SVC_DB_TABLE_MEDIA, dest_path, file_name, modified_time, folder_uuid, dest_storage, src_path);
	}

	if(!stack_query) {
		err = _media_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			media_svc_error("It failed to update metadata (%d)", err);
			return MEDIA_INFO_ERROR_DATABASE_INTERNAL;
		}
	} else {
		_media_svc_sql_query_add(&g_media_svc_move_item_query_list, &sql);
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_list_query_do(sqlite3 *handle, media_svc_query_type_e query_type)
{
	int ret = MEDIA_INFO_ERROR_NONE;

	ret = _media_svc_sql_begin_trans(handle);
	media_svc_retv_if(ret != MEDIA_INFO_ERROR_NONE, ret);

	if (query_type == MEDIA_SVC_QUERY_SET_ITEM_VALIDITY)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_item_validity_query_list);
	else if (query_type == MEDIA_SVC_QUERY_MOVE_ITEM)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_move_item_query_list);
	else if (query_type == MEDIA_SVC_QUERY_INSERT_ITEM)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_insert_item_query_list);
	else
		ret = MEDIA_INFO_ERROR_INVALID_PARAMETER;

	if (ret != MEDIA_INFO_ERROR_NONE) {
		media_svc_error("_media_svc_list_query_do failed. start rollback");
		_media_svc_sql_rollback_trans(handle);
		return ret;
	}

	ret = _media_svc_sql_end_trans(handle);
	if (ret != MEDIA_INFO_ERROR_NONE) {
		media_svc_error("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		_media_svc_sql_rollback_trans(handle);
		return ret;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int _media_svc_get_media_id_by_path(sqlite3 *handle, const char *path, char *media_uuid, int max_length)
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT media_uuid FROM %s WHERE validity=1 AND path='%q'",
					MEDIA_SVC_DB_TABLE_MEDIA, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MEDIA_INFO_ERROR_NONE) {
		media_svc_error("error when __media_svc_count_invalid_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	strncpy(media_uuid, (const char*)sqlite3_column_text(sql_stmt, 0), max_length);
	media_uuid[max_length - 1] = '\0';

	SQLITE3_FINALIZE(sql_stmt);

	return MEDIA_INFO_ERROR_NONE;

}
