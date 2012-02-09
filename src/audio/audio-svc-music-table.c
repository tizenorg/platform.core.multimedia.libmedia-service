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

/**
 * This file defines structure and functions related to database for managing music item.
 *
 * @file       	audio-svc-music-table.c
 * @version 	0.1
 */

#include <glib.h>
#include <sys/stat.h>
#include "media-svc-util.h"
#include "audio-svc-error.h"
#include "audio-svc-music-table.h"
#include "audio-svc-playlist-table.h"
#include "audio-svc-debug.h"
#include "audio-svc-types-priv.h"
#include "audio-svc-utils.h"
#include <drm-service.h>
#include "audio-svc-db-utils.h"


typedef enum {
	AUDIO_SVC_AUDIO_INFO_AUDIO_ID,
	AUDIO_SVC_AUDIO_INFO_PATH,
	AUDIO_SVC_AUDIO_INFO_THUMBNAIL_PATH,
	AUDIO_SVC_AUDIO_INFO_TITLE,
	AUDIO_SVC_AUDIO_INFO_ALBUM,
	AUDIO_SVC_AUDIO_INFO_ARTIST,
	AUDIO_SVC_AUDIO_INFO_GENRE,
	AUDIO_SVC_AUDIO_INFO_AUTHOR,
	AUDIO_SVC_AUDIO_INFO_YEAR,
	AUDIO_SVC_AUDIO_INFO_COPYRIGHT,
	AUDIO_SVC_AUDIO_INFO_DESCRIPTION,
	AUDIO_SVC_AUDIO_INFO_FORMAT,
	AUDIO_SVC_AUDIO_INFO_BITRATE,
	AUDIO_SVC_AUDIO_INFO_TRACK,
	AUDIO_SVC_AUDIO_INFO_DURATION,
	AUDIO_SVC_AUDIO_INFO_RATING,
	AUDIO_SVC_AUDIO_INFO_PLAYCOUNT,
	AUDIO_SVC_AUDIO_INFO_PLAYTIME,
	AUDIO_SVC_AUDIO_INFO_ADDTIME,
	AUDIO_SVC_AUDIO_INFO_RATEDTIME,
	AUDIO_SVC_AUDIO_INFO_ALBUM_RATING,
	AUDIO_SVC_AUDIO_INFO_DATE_MODIFIED,
	AUDIO_SVC_AUDIO_INFO_SIZE,
	AUDIO_SVC_AUDIO_INFO_CATEGORY,
	AUDIO_SVC_AUDIO_INFO_VALID,
	AUDIO_SVC_AUDIO_INFO_FOLDER_ID,
	AUDIO_SVC_AUDIO_INFO_STORAGE,
	AUDIO_SVC_AUDIO_INFO_FAVOURATE,
	AUDIO_SVC_AUDIO_INFO_CONTENT_TYPE,
} audio_svc_audio_info_e;

#define AUDIO_SVC_ORDER_BY_TITLE		"ORDER BY title COLLATE NOCASE"
#define AUDIO_SVC_ORDER_BY_ALBUM		"ORDER BY album COLLATE NOCASE"
#define AUDIO_SVC_ORDER_BY_GENRE		"ORDER BY genre COLLATE NOCASE"
#define AUDIO_SVC_ORDER_BY_AUTHOR	"ORDER BY author COLLATE NOCASE"
#define AUDIO_SVC_ORDER_BY_ARTIST		"ORDER BY artist COLLATE NOCASE"
#define AUDIO_SVC_COLLATE_NOCASE		"COLLATE NOCASE"

static const char *g_audio_svc_music_fields =
    "audio_uuid, path, thumbnail_path, title, album, artist, genre, author, year,\
copyright, description, format, bitrate,track_num,duration, rating, played_count, last_played_time, added_time, modified_date, size, category, valid, folder_uuid, storage_type";

static __thread GList *g_audio_svc_item_valid_query_list = NULL;
static __thread GList *g_audio_svc_move_item_query_list = NULL;
static __thread GList *g_audio_svc_insert_item_query_list = NULL;


static int __audio_svc_create_music_db_table(sqlite3 *handle);
static void __audio_svc_get_next_record(audio_svc_audio_item_s *item, sqlite3_stmt *stmt);
static int __audio_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e storage_type);
static int __audio_svc_count_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e storage_type);
static int __audio_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e storage_type, int count, mp_thumbnailpath_record_t * thumb_path);
static int __audio_svc_get_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e storage_type, int count, mp_thumbnailpath_record_t * thumb_path);


static int __audio_svc_create_music_db_table(sqlite3 *handle)
{
	int err = -1;

	char *sql = sqlite3_mprintf("create table if not exists %s (\
				audio_uuid		text primary key, \
				path				text unique, \
				thumbnail_path	text, \
				title				text, \
				album			text, \
				artist			text, \
				genre			text, \
				author			text, \
				year				integer default -1, \
				copyright		text, \
				description		text, \
				format			text, \
				bitrate			integer default -1, \
				track_num		integer default -1, \
				duration			integer default -1, \
				rating			integer default 0, \
				played_count		integer default 0, \
				last_played_time	integer default -1, \
				added_time		integer,\
				rated_time		integer,\
				album_rating		integer default 0,\
				modified_date	integer default 0, \
				size				integer default 0, \
				category 		INTEGER default 0, \
				valid		   		integer default 0, \
				folder_uuid			TEXT NOT NULL, \
				storage_type		integer, \
				favourite			integer default 0, \
				content_type		integer default %d);",
				AUDIO_SVC_DB_TABLE_AUDIO,
				AUDIO_SVC_CONTENT_TYPE);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to create db table (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_CREATE_TABLE;
	}

	return AUDIO_SVC_ERROR_NONE;
}

static void __audio_svc_get_next_record(audio_svc_audio_item_s *item,
					sqlite3_stmt *stmt)
{
	_strncpy_safe(item->audio_uuid,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_AUDIO_ID), sizeof(item->audio_uuid));
	_strncpy_safe(item->pathname,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_PATH), sizeof(item->pathname));
	_strncpy_safe(item->thumbname,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_THUMBNAIL_PATH), sizeof(item->thumbname));
	_strncpy_safe(item->audio.title,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_TITLE), sizeof(item->audio.title));
	_strncpy_safe(item->audio.album,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_ALBUM), sizeof(item->audio.album));
	_strncpy_safe(item->audio.artist,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_ARTIST), sizeof(item->audio.artist));
	_strncpy_safe(item->audio.genre,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_GENRE), sizeof(item->audio.genre));
	_strncpy_safe(item->audio.author,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_AUTHOR), sizeof(item->audio.author));
	_strncpy_safe(item->audio.year,
		      _year_2_str(sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_YEAR)), sizeof(item->audio.year));
	_strncpy_safe(item->audio.copyright,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_COPYRIGHT), sizeof(item->audio.copyright));
	_strncpy_safe(item->audio.description,
		      (const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_DESCRIPTION), sizeof(item->audio.description));
	_strncpy_safe(item->audio.format, 
			(const char *)sqlite3_column_text(stmt, AUDIO_SVC_AUDIO_INFO_FORMAT), sizeof(item->audio.format));
	
	item->audio.bitrate = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_BITRATE);
	item->audio.track = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_TRACK);
	item->audio.duration = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_DURATION);
	item->rating = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_RATING);
	item->played_count = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_PLAYCOUNT);
	item->time_played = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_PLAYTIME);
	item->time_added = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_ADDTIME);
	item->audio.album_rating = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_ALBUM_RATING);
	item->category = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_CATEGORY);
	item->storage_type = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_STORAGE);
	item->favourate = sqlite3_column_int(stmt, AUDIO_SVC_AUDIO_INFO_FAVOURATE);

}

static int
__audio_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e
						 storage_type)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql =
	    sqlite3_mprintf
	    ("select count(*) from %s where valid=0 and storage_type=%d and thumbnail_path is not null",
	     AUDIO_SVC_DB_TABLE_AUDIO, storage_type);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when __audio_svc_count_invalid_records_with_thumbnail. err = [%d]",
		     ret);
		return count;
	}

	count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return count;

}

static int __audio_svc_count_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e
						    storage_type)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql =
	    sqlite3_mprintf
	    ("select count(*) from %s where storage_type=%d and thumbnail_path is not null",
	     AUDIO_SVC_DB_TABLE_AUDIO, storage_type);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when __audio_svc_count_records_with_thumbnail. err = [%d]",
		     ret);
		return count;
	}

	count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return count;

}

static int
__audio_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e
					       storage_type, int count,
					       mp_thumbnailpath_record_t *
					       thumb_path)
{
	int err = -1;
	int idx = 0;
	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	char *sql =
	    sqlite3_mprintf
	    ("select thumbnail_path from %s where valid=0 and storage_type=%d and thumbnail_path is not null",
	     AUDIO_SVC_DB_TABLE_AUDIO, storage_type);

	audio_svc_debug("[SQL query] : %s", sql);

	err = sqlite3_prepare_v2(handle, sql, -1, &sql_stmt, NULL);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(thumb_path[idx].thumbnail_path,
			      (const char *)sqlite3_column_text(sql_stmt, 0),
			      sizeof(thumb_path[idx]));
		idx++;
		audio_svc_debug("thumb_path[%d]=[%s]", idx,
				thumb_path[idx].thumbnail_path);
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

static int __audio_svc_get_records_with_thumbnail(sqlite3 *handle, audio_svc_storage_type_e
						  storage_type, int count,
						  mp_thumbnailpath_record_t *
						  thumb_path)
{
	int err = -1;
	int idx = 0;
	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	char *sql =
	    sqlite3_mprintf
	    ("select thumbnail_path from %s where storage_type=%d and thumbnail_path is not null",
	     AUDIO_SVC_DB_TABLE_AUDIO, storage_type);

	audio_svc_debug("[SQL query] : %s", sql);

	err = sqlite3_prepare_v2(handle, sql, -1, &sql_stmt, NULL);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(thumb_path[idx].thumbnail_path,
			      (const char *)sqlite3_column_text(sql_stmt, 0),
			      sizeof(thumb_path[idx]));
		idx++;
		audio_svc_debug("thumb_path[%d]=[%s]", idx,
				thumb_path[idx].thumbnail_path);
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_create_music_table(sqlite3 *handle)
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = __audio_svc_create_music_db_table(handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_truncate_music_table(sqlite3 *handle, audio_svc_storage_type_e storage_type)
{
	int idx = 0;
	mp_thumbnailpath_record_t *thumbpath_record = NULL;
	int err = -1;
	int invalid_count = 0;
	int ret = AUDIO_SVC_ERROR_NONE;

	invalid_count = __audio_svc_count_records_with_thumbnail(handle, storage_type);
	audio_svc_debug("invalid count: %d\n", invalid_count);

	if (invalid_count > 0) {
		thumbpath_record =
		    (mp_thumbnailpath_record_t *)
		    malloc(sizeof(mp_thumbnailpath_record_t) * invalid_count);
		if (thumbpath_record == NULL) {
			audio_svc_debug("fail to memory allocation");
			return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
		}
		memset(thumbpath_record, 0,
		       sizeof(mp_thumbnailpath_record_t) * invalid_count);

		ret = __audio_svc_get_records_with_thumbnail(handle, storage_type, invalid_count, thumbpath_record);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error when get thumbnail record");
			SAFE_FREE(thumbpath_record);
			return ret;
		}
	} else {
		audio_svc_debug("There is no item with thumbnail");
	}

	char *sql =
	    sqlite3_mprintf("delete from %s where storage_type=%d",
			    AUDIO_SVC_DB_TABLE_AUDIO, storage_type);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to truncate table (%d)", err);
		SAFE_FREE(thumbpath_record);

		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	for (idx = 0; idx < invalid_count; idx++) {
		if (strlen(thumbpath_record[idx].thumbnail_path) > 0) {
			ret = _audio_svc_check_and_remove_thumbnail(handle, thumbpath_record[idx].thumbnail_path);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				audio_svc_error
				    ("error _audio_svc_check_and_remove_thumbnail");
				SAFE_FREE(thumbpath_record);
				return ret;
			}
		}
	}

	SAFE_FREE(thumbpath_record);
	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_create_folder_table(sqlite3 *handle)
{
	int err = -1;
	char *sql = sqlite3_mprintf("create table if not exists %s (\
			folder_uuid		text primary key, \
			path				text,\
			folder_name		text,\
			storage_type		integer,\
			modified_date	integer default 0);", AUDIO_SVC_DB_TABLE_AUDIO_FOLDER);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("error while create folder table");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_CREATE_TABLE;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_folder(sqlite3 *handle, audio_svc_storage_type_e storage_type, const char *folder_id)
{
	int err = -1;
	char *sql = NULL;
	if ((storage_type != AUDIO_SVC_STORAGE_PHONE)
	    && (storage_type != AUDIO_SVC_STORAGE_MMC)) {
		audio_svc_error("Invalid storage type");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	if ((storage_type == AUDIO_SVC_STORAGE_MMC) && (folder_id == NULL)) {	/* when mmc card removed. */
		sql = sqlite3_mprintf("delete from %s where storage_type=%d", 
				    AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, storage_type);
		err = _audio_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			audio_svc_debug("It failed to delete item (%d)", err);
			if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
				return err;
			}
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	} else {
		sql =
		    sqlite3_mprintf("delete from %s where folder_uuid='%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, folder_id);
		err = _audio_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			audio_svc_debug("It failed to delete item (%d)", err);
			if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
				return err;
			}
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_insert_item_with_data(sqlite3 *handle, audio_svc_audio_item_s *item, bool stack_query)
{
	int err = -1;
	int ret = AUDIO_SVC_ERROR_NONE;
	struct stat st;
	int modified_date = -1;
	int size = -1;
	char folder_id[AUDIO_SVC_UUID_SIZE+1] = {0,};
	int year = -1;
	char *audio_id = NULL;

#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	if (item == NULL) {
		audio_svc_error("Invalid handle");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_id = _media_info_generate_uuid();
	if(audio_id == NULL ) {
		audio_svc_error("Invalid UUID");
		return AUDIO_SVC_ERROR_INTERNAL;
	}
	
	/* set creation date */
	memset(&st, 0, sizeof(struct stat));
	if (stat(item->pathname, &st) == 0) {
		modified_date = st.st_mtime;
		size = st.st_size;
	}

	year = str_2_year(item->audio.year);
	if (year <= 0) {
		year = -1;
		audio_svc_debug("year = %d", year);
		_strncpy_safe(item->audio.year, AUDIO_SVC_TAG_UNKNOWN,
			      sizeof(item->audio.year));
	}

	ret = _audio_svc_get_and_append_folder_id_by_path(handle, item->pathname,  item->storage_type, folder_id);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	char *sql =
	    sqlite3_mprintf
	    ("insert into %s (%s) values ('%q', '%q', '%q', '%q', '%q','%q', '%q','%q',%d,'%q','%q','%q',%d,%d,%d,%d,%d,%d,%d,%d, %d, %d, %d, '%q', %d);",
	     AUDIO_SVC_DB_TABLE_AUDIO, g_audio_svc_music_fields,
	     audio_id,
	     item->pathname,
	     item->thumbname,
	     item->audio.title,
	     item->audio.album,
	     item->audio.artist,
	     item->audio.genre,
	     item->audio.author,
	     str_2_year(item->audio.year),
	     item->audio.copyright,
	     item->audio.description,
	     item->audio.format,
	     item->audio.bitrate,
	     item->audio.track,
	     item->audio.duration,
	     item->rating,
	     item->played_count,
	     item->time_played,
	     item->time_added,
	     modified_date,
	     size,
	     item->category,
	     1,
	     folder_id,
	     item->storage_type);

	audio_svc_debug("query : %s", sql);

	if(!stack_query) {
		err = _audio_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			audio_svc_error("failed to insert music record");

			if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
				return err;
			}
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	} else {
		_audio_svc_sql_query_add(&g_audio_svc_insert_item_query_list, &sql);
	}

	//item->audio_uuid = sqlite3_last_insert_rowid(handle);
	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_select_music_record_by_audio_id(sqlite3 *handle, const char *audio_id,
					       audio_svc_audio_item_s *item)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql =
	    sqlite3_mprintf
	    ("select * from %s where audio_uuid='%q' and valid=1 and category=%d",
	     AUDIO_SVC_DB_TABLE_AUDIO, audio_id, AUDIO_SVC_CATEGORY_MUSIC);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error("error when _audio_svc_select_music_record_by_audio_id. ret = [%d]", ret);
		return ret;
	}

	__audio_svc_get_next_record(item, sql_stmt);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_select_music_record_by_path(sqlite3 *handle, const char *path,
					   audio_svc_audio_item_s *item)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql =
	    sqlite3_mprintf("select * from %s where valid=1 and path='%q' ",
			    AUDIO_SVC_DB_TABLE_AUDIO, path);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_select_music_record_by_path. ret=[%d]",
		     ret);
		return ret;
	}

	__audio_svc_get_next_record(item, sql_stmt);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_music_record_by_audio_id(sqlite3 *handle, const char *audio_id)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf("delete from %s where valid=1 and audio_uuid='%q'",
			    AUDIO_SVC_DB_TABLE_AUDIO, audio_id);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to delete item (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_metadata_in_music_record(sqlite3 *handle, const char *audio_id,
					       audio_svc_audio_item_s *item)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set thumbnail_path='%q',title='%q',album='%q',artist='%q',arist_seq='%q' ,genre='%q',author='%q',\
		year=%d,copyright='%q',description='%q',format='%q',track_num=%d,duration=%d,bitrate=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO,
	     item->thumbname,
	     item->audio.title,
	     item->audio.album,
	     item->audio.artist,
	     item->audio.genre,
	     item->audio.author,

	     str_2_year(item->audio.year),

	     item->audio.copyright,
	     item->audio.description,
	     item->audio.format,

	     item->audio.track, item->audio.duration, item->audio.bitrate,
	     audio_id);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update metadata (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_path_in_music_record(sqlite3 *handle, const char *src_path,
					   const char *path, const char *title)
{
	char *sql = NULL;
	int err = -1;

	if (STRING_VALID(title)) {
		sql =
		    sqlite3_mprintf
		    ("update %s set path='%q', title='%q', where valid=1 and path='%q'",
		     AUDIO_SVC_DB_TABLE_AUDIO, path, title, src_path);
	} else {
		sql =
		    sqlite3_mprintf
		    ("update %s set path='%q', where valid=1 and path='%q'",
		     AUDIO_SVC_DB_TABLE_AUDIO, path, src_path);
	}

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update metadata (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_path_and_storage_in_music_record(sqlite3 *handle, const char *src_path, const char *path, audio_svc_storage_type_e storage_type)
{
	int err = -1;
	char *sql = sqlite3_mprintf("update %s set path='%q', storage_type=%d where valid=1 and path='%q'",
				 AUDIO_SVC_DB_TABLE_AUDIO, path, storage_type, src_path);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update metadata (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_folder_id_in_music_record(sqlite3 *handle, const char *path, const char *folder_id)
{
	int err = -1;
	char *sql = sqlite3_mprintf("update %s set folder_uuid='%q' where path='%q'",
			    AUDIO_SVC_DB_TABLE_AUDIO, folder_id, path);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update metadata (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_thumb_path_in_music_record(sqlite3 *handle, const char *file_path,
						 const char *path)
{
	int err = -1;

	char *sql =
	    sqlite3_mprintf
	    ("update %s set thumbnail_path='%q' where valid=1 and path='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, path, file_path);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update thumb path (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_rating_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{
	int err = -1;
	int rated_time = -1;
	rated_time = time(NULL);

	char *sql =
	    sqlite3_mprintf
	    ("update %s set rating=%d, rated_time=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, rated_time, audio_id);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_debug("To update rating is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_playtime_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{

	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set last_played_time=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update last_played_time is failed(%d)",
				err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_playcount_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{

	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set played_count=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update played count is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_addtime_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set added_time=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update added_time is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_track_num_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set track_num=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("update track num is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_album_rating_in_music_record(sqlite3 *handle, const char *audio_id,
						   int changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set album_rating=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("update album_rating is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_year_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set year=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("update year is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_title_in_music_record(sqlite3 *handle, const char *audio_id,
					    const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set title='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update title is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_artist_in_music_record(sqlite3 *handle, const char *audio_id,
					     const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set artist='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update artist is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_album_in_music_record(sqlite3 *handle, const char *audio_id,
					    const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set album='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update album is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_genre_in_music_record(sqlite3 *handle, const char *audio_id,
					    const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set genre='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update genre is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_author_in_music_record(sqlite3 *handle, const char *audio_id,
					     const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set author='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update author is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_description_in_music_record(sqlite3 *handle, const char *audio_id,
						  const char *changed_value)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set description='%q' where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update description is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_favourite_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value)
{
	int err = -1;
	char *sql = sqlite3_mprintf("update %s set favourite=%d where valid=1 and audio_uuid='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, changed_value, audio_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("update album_rating is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_count_music_group_records(sqlite3 *handle, audio_svc_group_type_e group_type,
					 const char *limit_string1,
					 const char *limit_string2,
					 const char *filter_string,
					 const char *filter_string2, int *count)
{
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	int err = -1;
	int text_bind = 1;

	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	if (filter_string) {
		if (strlen(filter_string) > 0) {
			snprintf(filter_query, sizeof(filter_query), "%%%s%%",
				 filter_string);
			filter_mode = TRUE;
		}
	}
	if (filter_string2) {
		if (strlen(filter_string2) > 0) {
			snprintf(filter_query2, sizeof(filter_query2), "%%%s%%",
				 filter_string2);
			filter_mode2 = TRUE;
		}
	}

	switch (group_type) {
	case AUDIO_SVC_GROUP_BY_ALBUM:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct album) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_ARTIST:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct artist) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct genre) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				g_strlcat(query, " and genre like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and genre like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_FOLDER:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct folder_uuid) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			/*FIX ME. if filter_mode exist. */
			if (filter_mode) {
				snprintf(query, sizeof(query),
					 "select count(distinct a.folder_uuid) from %s a inner join %s b on a.folder_uuid=b.folder_uuid \
					where a.valid=1 and a.category=%d and b.path like ?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, AUDIO_SVC_CATEGORY_MUSIC);
			}
			if (filter_mode2) {
				g_strlcat(query, " and b.path like ?",
					  sizeof(query));
			}
		}
		break;

	case AUDIO_SVC_GROUP_BY_YEAR:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct year) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				g_strlcat(query, " and year like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and year like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_COMPOSER:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct author) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				g_strlcat(query, " and author like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and author like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_ARTIST_ALBUM:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct album) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (limit_string1 && strlen(limit_string1) > 0) {
				g_strlcat(query, " and artist=?",
					  sizeof(query));
			} else {
				g_strlcat(query, " and artist is null",
					  sizeof(query));
			}
			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ARTIST:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct artist) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (limit_string1 && strlen(limit_string1) > 0) {
				g_strlcat(query, " and genre=?", sizeof(query));
			} else {
				g_strlcat(query, " and genre is null",
					  sizeof(query));
			}
			if (filter_mode) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ALBUM:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct album) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (limit_string1 && strlen(limit_string1) > 0) {
				g_strlcat(query, " and genre=?", sizeof(query));
			} else {
				g_strlcat(query, " and genre is null",
					  sizeof(query));
			}
			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM:
		{
			snprintf(query, sizeof(query),
				 "select count(distinct album) from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			if (limit_string1 && strlen(limit_string1) > 0) {
				if (limit_string2 && strlen(limit_string2) > 0) {
					g_strlcat(query,
						  " and genre=? and artist=?",
						  sizeof(query));
				} else {
					g_strlcat(query,
						  " and genre=? and artist is null",
						  sizeof(query));
				}
			} else {
				if (limit_string2 && strlen(limit_string2) > 0) {
					g_strlcat(query,
						  " and genre is null and artist=?",
						  sizeof(query));
				} else {
					g_strlcat(query,
						  " and genre is null and artist is null",
						  sizeof(query));
				}
			}

			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

		}
		break;

	default:
		{
			audio_svc_error("Wrong type [%d]", group_type);
			return AUDIO_SVC_ERROR_INVALID_PARAMETER;
		}
	}

	audio_svc_debug("[SQL query] : %s", query);

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (group_type != AUDIO_SVC_GROUP_BY_ALBUM
	    && group_type != AUDIO_SVC_GROUP_BY_ARTIST
	    && group_type != AUDIO_SVC_GROUP_BY_GENRE
	    && group_type != AUDIO_SVC_GROUP_BY_FOLDER) {
		if (limit_string1 && strlen(limit_string1) > 0) {
			if (SQLITE_OK !=
			    (err =
			     sqlite3_bind_text(sql_stmt, text_bind,
					       (char *)limit_string1,
					       strlen(limit_string1), NULL))) {
				audio_svc_error
				    ("limit_string(%s) binding is failed (%d)",
				     limit_string1, err);
				SQLITE3_FINALIZE(sql_stmt);
				return AUDIO_SVC_ERROR_DB_INTERNAL;
			}
			text_bind++;
		}

		if (limit_string2 && strlen(limit_string2) > 0
		    && group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM) {
			if (SQLITE_OK !=
			    (err =
			     sqlite3_bind_text(sql_stmt, text_bind,
					       (char *)limit_string2,
					       strlen(limit_string2), NULL))) {
				audio_svc_error
				    ("limit_string2(%s) binding is failed (%d)",
				     limit_string2, err);
				SQLITE3_FINALIZE(sql_stmt);
				return AUDIO_SVC_ERROR_DB_INTERNAL;
			}
			text_bind++;
		}
	}

	if (filter_mode) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}
	if (filter_mode2) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}

	err = sqlite3_step(sql_stmt);
	if (err != SQLITE_ROW) {
		audio_svc_error("end of row [%s]", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(sql_stmt);
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_get_music_group_records(sqlite3 *handle, audio_svc_group_type_e group_type,
				       const char *limit_string1,
				       const char *limit_string2,
				       const char *filter_string,
				       const char *filter_string2, int offset,
				       int rows, audio_svc_group_item_s *group)
{
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char tail_query[100] = { 0 };
	char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	int err = -1;
	int text_bind = 1;
	int idx = 0;

	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	if (filter_string) {
		if (strlen(filter_string) > 0) {
			snprintf(filter_query, sizeof(filter_query), "%%%s%%",
				 filter_string);
			filter_mode = TRUE;
		}
	}
	if (filter_string2) {
		if (strlen(filter_string2) > 0) {
			snprintf(filter_query2, sizeof(filter_query2), "%%%s%%",
				 filter_string2);
			filter_mode2 = TRUE;
		}
	}

	switch (group_type) {
	case AUDIO_SVC_GROUP_BY_ALBUM:
		{
			snprintf(query, sizeof(query),
				 "select album, min(audio_uuid), artist, thumbnail_path, album_rating from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by album %s limit %d,%d", AUDIO_SVC_ORDER_BY_ALBUM, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_ARTIST:
		{
			snprintf(query, sizeof(query),
				 "select artist, min(audio_uuid), album, thumbnail_path, album_rating from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and artist like ? ",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and artist like ? ",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by artist %s limit %d,%d", AUDIO_SVC_ORDER_BY_ARTIST, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_ARTIST_ALBUM:
		{
			if (limit_string1 && strlen(limit_string1) > 0) {
				snprintf(query, sizeof(query),
					 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
				from %s where valid=1 and category=%d and artist=?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
					from %s where valid=1 and category=%d and artist is null",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			}
			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by album %s limit %d,%d", AUDIO_SVC_ORDER_BY_ALBUM, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE:
		{
			snprintf(query, sizeof(query),
				 "select genre, min(audio_uuid), album, thumbnail_path, album_rating from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and genre like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and genre like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by genre %s limit %d,%d", AUDIO_SVC_ORDER_BY_GENRE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_FOLDER:
		{
			snprintf(query, sizeof(query),
				 "select b.folder_name, min(a.audio_uuid), (b.path), (a.thumbnail_path), (a.album_rating) \
								from %s as a inner join %s b on a.folder_uuid=b.folder_uuid where a.valid=1 and a.category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, AUDIO_SVC_CATEGORY_MUSIC);
			if (filter_mode) {
				snprintf(query, sizeof(query),
					 "select b.folder_name, min(a.audio_uuid), (b.path), (a.thumbnail_path), (a.album_rating) \
					from %s as a inner join %s b on a.folder_uuid=b.folder_uuid where a.valid=1 and a.category=%d and b.path like ?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, AUDIO_SVC_CATEGORY_MUSIC);
			}
			if (filter_mode2) {
				snprintf(query, sizeof(query),
					 "select b.folder_name, min(a.audio_uuid), (b.path), (a.thumbnail_path), (a.album_rating) \
					from %s as a inner join %s b on a.folder_uuid=b.folder_uuid where a.valid=1 and a.category=%d and b.path like ? and b.path like ?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, AUDIO_SVC_CATEGORY_MUSIC);
			}

			snprintf(tail_query, sizeof(tail_query),
				 " group by a.folder_uuid order by b.folder_name %s, b.path %s limit %d,%d",
				 AUDIO_SVC_COLLATE_NOCASE, AUDIO_SVC_COLLATE_NOCASE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_YEAR:
		{
			snprintf(query, sizeof(query),
				 "select year, min(audio_uuid), album, thumbnail_path, album_rating from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and year like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and year like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query),
				 " group by year order by year desc limit %d,%d",
				 offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_COMPOSER:
		{
			snprintf(query, sizeof(query),
				 "select author, min(audio_uuid), album, thumbnail_path, album_rating from %s where valid=1 and category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and author like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and author like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by author %s limit %d,%d", AUDIO_SVC_ORDER_BY_AUTHOR, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ARTIST:
		{
			if (limit_string1 && strlen(limit_string1) > 0) {
				snprintf(query, sizeof(query),
					 "select artist, min(audio_uuid), album, thumbnail_path, album_rating \
					from %s where valid=1 and category=%d and genre=?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select artist, min(audio_uuid), album, thumbnail_path, album_rating \
					from %s where valid=1 and category=%d and genre is null",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			}
			if (filter_mode) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and artist like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by artist %s limit %d,%d", AUDIO_SVC_ORDER_BY_ARTIST, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ALBUM:
		{
			if (limit_string1 && strlen(limit_string1) > 0) {
				snprintf(query, sizeof(query),
					 "select distinct album, min(audio_uuid), artist, thumbnail_path, album_rating \
					from %s where valid=1 and category=%d and genre=?",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select distinct album, min(audio_uuid), artist, thumbnail_path, album_rating \
					from %s where valid=1 and category=%d and genre is null",
					 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
			}
			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by album %s limit %d,%d", AUDIO_SVC_ORDER_BY_ALBUM, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM:
		{
			if (limit_string1 && strlen(limit_string1) > 0) {
				if (limit_string2 && strlen(limit_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
						from %s where valid=1 and category=%d and genre=? and artist=?",
						 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
						from %s where valid=1 and category=%d and genre=? and artist is null",
						 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
				}
			} else {
				if (limit_string2 && strlen(limit_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
						from %s where valid=1 and category=%d and genre is null and artist=?",
						 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select album, min(audio_uuid), artist, thumbnail_path, album_rating \
						from %s where valid=1 and category=%d and genre is null and artist is null",
						 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
				}
			}

			if (filter_mode) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and album like ?",
					  sizeof(query));
			}

			snprintf(tail_query, sizeof(tail_query), " group by album %s limit %d,%d", AUDIO_SVC_ORDER_BY_ALBUM, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	}

	audio_svc_debug("[SQL query] : %s", query);

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if ((limit_string1) && strlen(limit_string1)
	    && (group_type != AUDIO_SVC_GROUP_BY_ALBUM)
	    && (group_type != AUDIO_SVC_GROUP_BY_ARTIST)) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)limit_string1,
				       strlen(limit_string1), NULL))) {
			audio_svc_error
			    ("limit_string1(%s) binding is failed (%d)",
			     limit_string1, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}

	if ((limit_string2) && strlen(limit_string2)
	    && (group_type == AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM)) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)limit_string2,
				       strlen(limit_string2), NULL))) {
			audio_svc_error
			    ("limit_string2(%s) binding is failed (%d)",
			     limit_string2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}

	if (filter_mode) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}
	if (filter_mode2) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		if (group_type == AUDIO_SVC_GROUP_BY_YEAR) {
			int year = sqlite3_column_int(sql_stmt, 0);
			if (year == -1 || year == 0) {
				_strncpy_safe(group[idx].maininfo,
					      AUDIO_SVC_TAG_UNKNOWN,
					      sizeof(group[idx].maininfo));
			} else {
				_strncpy_safe(group[idx].maininfo,
					      (const char *)
					      sqlite3_column_text(sql_stmt, 0),
					      sizeof(group[idx].maininfo));
			}
		} else {
			_strncpy_safe(group[idx].maininfo,
				      (const char *)
				      sqlite3_column_text(sql_stmt, 0),
				      sizeof(group[idx].maininfo));
		}

		_strncpy_safe(group[idx].subinfo,
			      (const char *)sqlite3_column_text(sql_stmt, 2),
			      sizeof(group[idx].subinfo));

		_strncpy_safe(group[idx].thumbnail_path,
			      (const char *)sqlite3_column_text(sql_stmt, 3),
			      sizeof(group[idx].thumbnail_path));

		group[idx].album_rating = sqlite3_column_int(sql_stmt, 4);

		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_count_music_track_records(sqlite3 *handle, audio_svc_track_type_e track_type,
					 const char *type_string,
					 const char *type_string2,
					 const char *filter_string,
					 const char *filter_string2, int *count)
{
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	int err = -1;
	int text_bind = 1;

	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	if (filter_string) {
		if (strlen(filter_string) > 0) {
			snprintf(filter_query, sizeof(filter_query), "%%%s%%",
				 filter_string);
			filter_mode = TRUE;
		}
	}
	if (filter_string2) {
		if (strlen(filter_string2) > 0) {
			snprintf(filter_query2, sizeof(filter_query2), "%%%s%%",
				 filter_string2);
			filter_mode2 = TRUE;
		}
	}

	switch (track_type) {
	case AUDIO_SVC_TRACK_ALL:
		snprintf(query, sizeof(query),
			 "select count(*) from %s where valid=1 and category=%d",
			 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
		break;

	case AUDIO_SVC_TRACK_BY_ALBUM:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and album=?",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and album is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_ARTIST_ALBUM:
		if (type_string && strlen(type_string) > 0) {
			if (type_string2 && strlen(type_string2) > 0) {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and album=? and artist=?",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and album=? and artist is null",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
		} else {
			if (type_string2 && strlen(type_string2) > 0) {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and album is null and artist=?",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and album is null and artist is null",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
		}

		break;

	case AUDIO_SVC_TRACK_BY_ARTIST:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and artist=?",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and artist is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_ARTIST_GENRE:
		if (type_string && strlen(type_string) > 0) {
			if (type_string2 && strlen(type_string2) > 0) {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and genre=? and artist=?",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and genre=? and artist is null",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
		} else {
			if (type_string2 && strlen(type_string2) > 0) {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and genre is null and artist=?",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select count(*) from %s where valid=1 and category=%d and genre is null and artist is null",
					 AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}

		}

		break;

	case AUDIO_SVC_TRACK_BY_GENRE:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and genre=?",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and genre is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_FOLDER:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and folder_uuid=(select folder_uuid from %s where path = ?)",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC,
				 AUDIO_SVC_DB_TABLE_AUDIO_FOLDER);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and folder_uuid is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_YEAR:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and year=?",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and year is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_COMPOSER:
		if (type_string && strlen(type_string) > 0) {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and author=?",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		} else {
			snprintf(query, sizeof(query),
				 "select count(*) from %s where valid=1 and category=%d and author is null",
				 AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
		}
		break;

	case AUDIO_SVC_TRACK_BY_TOPRATING:
		snprintf(query, sizeof(query),
			 "select count(*) from %s where valid=1 and category=%d and rating > %d",
			 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC,
			 AUDIO_SVC_RATING_3);
		break;

	case AUDIO_SVC_TRACK_BY_PLAYED_TIME:
		snprintf(query, sizeof(query),
			 "select count(*) from %s where valid=1 and category=%d and last_played_time > 0",
			 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
		break;

	case AUDIO_SVC_TRACK_BY_ADDED_TIME:
		snprintf(query, sizeof(query),
			 "select count(*) from %s where valid=1 and category=%d and added_time > 0",
			 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
		break;

	case AUDIO_SVC_TRACK_BY_PLAYED_COUNT:
		snprintf(query, sizeof(query),
			 "select count(*) from %s where valid=1 and category=%d and played_count > 0",
			 AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC);
		break;

	case AUDIO_SVC_TRACK_BY_PLAYLIST:
		{
			snprintf(query, sizeof(query),
				 "select count(*) from %s a, %s b where a.playlist_id=%d and b.audio_uuid=a.audio_uuid and b.valid=1 and b.category=%d",
				 AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
				 AUDIO_SVC_DB_TABLE_AUDIO, (int)type_string,
				 AUDIO_SVC_CATEGORY_MUSIC);

			if (filter_mode) {
				g_strlcat(query, " and b.title like ?",
					  sizeof(query));
			}
			if (filter_mode2) {
				g_strlcat(query, " and b.title like ?",
					  sizeof(query));
			}
		}
		break;

	default:
		break;
	}

	if (track_type != AUDIO_SVC_TRACK_BY_PLAYLIST) {
		if (filter_mode) {
			g_strlcat(query, " and title like ?", sizeof(query));
		}
		if (filter_mode2) {
			g_strlcat(query, " and title like ?", sizeof(query));
		}
	}
	audio_svc_debug("[SQL query] : %s", query);

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (filter_mode) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}
	if (filter_mode2) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}

	if ((track_type >= AUDIO_SVC_TRACK_BY_ALBUM)
	    && (track_type <= AUDIO_SVC_TRACK_BY_COMPOSER)) {
		if (type_string && strlen(type_string) > 0) {
			if ((track_type == AUDIO_SVC_TRACK_BY_YEAR)
			    && (!strcmp(type_string, AUDIO_SVC_TAG_UNKNOWN))) {
				if (SQLITE_OK !=
				    (err =
				     sqlite3_bind_text(sql_stmt, text_bind,
						       "-1", strlen("-1"),
						       NULL))) {
					audio_svc_error
					    ("cont_string1(%s) binding is failed (%d)",
					     type_string, err);
					SQLITE3_FINALIZE(sql_stmt);
					return AUDIO_SVC_ERROR_DB_INTERNAL;
				}
				text_bind++;
			} else
			    if ((track_type == AUDIO_SVC_TRACK_BY_ARTIST_GENRE)
				|| (track_type ==
				    AUDIO_SVC_TRACK_BY_ARTIST_ALBUM)) {
				if (type_string2 && strlen(type_string2) > 0) {
					if (SQLITE_OK !=
					    (err =
					     sqlite3_bind_text(sql_stmt,
							       text_bind,
							       (char *)
							       type_string,
							       strlen
							       (type_string),
							       NULL))) {
						audio_svc_error
						    ("cont_string1(%s) binding is failed (%d)",
						     type_string, err);
						SQLITE3_FINALIZE(sql_stmt);
						return
						    AUDIO_SVC_ERROR_DB_INTERNAL;
					}
					text_bind++;
					if (SQLITE_OK !=
					    (err =
					     sqlite3_bind_text(sql_stmt,
							       text_bind,
							       (char *)
							       type_string2,
							       strlen
							       (type_string2),
							       NULL))) {
						audio_svc_error
						    ("cont_string2(%s) binding is failed (%d)",
						     type_string2, err);
						SQLITE3_FINALIZE(sql_stmt);
						return
						    AUDIO_SVC_ERROR_DB_INTERNAL;
					}
					text_bind++;
				} else {
					if (SQLITE_OK !=
					    (err =
					     sqlite3_bind_text(sql_stmt,
							       text_bind,
							       (char *)
							       type_string,
							       strlen
							       (type_string),
							       NULL))) {
						audio_svc_error
						    ("cont_string1(%s) binding is failed (%d)",
						     type_string, err);
						SQLITE3_FINALIZE(sql_stmt);
						return
						    AUDIO_SVC_ERROR_DB_INTERNAL;
					}
				}
			} else {
				if (SQLITE_OK !=
				    (err =
				     sqlite3_bind_text(sql_stmt, text_bind,
						       (char *)type_string,
						       strlen(type_string),
						       NULL))) {
					audio_svc_error
					    ("cont_string1(%s) binding is failed (%d)",
					     type_string, err);
					SQLITE3_FINALIZE(sql_stmt);
					return AUDIO_SVC_ERROR_DB_INTERNAL;
				}
			}
			text_bind++;
		}
	}

	audio_svc_debug("[SQL query] : %s", query);

	err = sqlite3_step(sql_stmt);
	if (err != SQLITE_ROW) {
		audio_svc_error("end of row [%s]", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(sql_stmt);
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_get_music_track_records(sqlite3 *handle, audio_svc_track_type_e track_type,
				       const char *type_string,
				       const char *type_string2,
				       const char *filter_string,
				       const char *filter_string2, int offset,
				       int rows, audio_svc_list_item_s *track)
{
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char *result_field =
	    "audio_uuid, path, thumbnail_path, title, artist, duration, rating";

	char tail_query[70] = { 0 };
	char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	int err = -1;
	int len = -1;
	int text_bind = 1;
	int idx = 0;

	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
#define filter_condition(filter_mode, filter_mode2, query)	\
	if ((filter_mode)) 	g_strlcat((query), " and title like ?", sizeof((query))); \
	if ((filter_mode2))	g_strlcat((query), " and title like ?", sizeof((query)));

	filter_mode = STRING_VALID(filter_string);
	filter_mode2 = STRING_VALID(filter_string2);

	if (filter_mode) {
		snprintf(filter_query, sizeof(filter_query), "%%%s%%",
			 filter_string);
	}
	if (filter_mode2) {
		snprintf(filter_query2, sizeof(filter_query2), "%%%s%%",
			 filter_string2);
	}

	switch (track_type) {
	case AUDIO_SVC_TRACK_ALL:
		{
			snprintf(query, sizeof(query),
				 "select %s from %s where valid=1 and category=%d",
				 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);
			filter_condition(filter_mode, filter_mode2, query);
			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_ALBUM:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and album=?",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and album is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);
			snprintf(tail_query, sizeof(tail_query),
				 " order by track_num limit %d,%d", offset,
				 rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_ARTIST_ALBUM:
		{
			if (type_string && strlen(type_string) > 0) {
				if (type_string2 && strlen(type_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and album=? and artist=?",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and album=? and artist is null",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				}
			} else {
				if (type_string2 && strlen(type_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and album is null and artist=?",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and album is null and artist is null",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				}
			}

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_ARTIST:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s, album from %s where valid=1 and category=%d and artist=?",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select %s, album from %s where valid=1 and category=%d and artist is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);
			snprintf(tail_query, sizeof(tail_query), " order by album %s, title %s limit %d,%d", AUDIO_SVC_COLLATE_NOCASE, AUDIO_SVC_COLLATE_NOCASE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_ARTIST_GENRE:
		{
			if (type_string && strlen(type_string) > 0) {
				if (type_string2 && strlen(type_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and genre=? and artist=?",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and genre=? and artist is null",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				}
			} else {
				if (type_string2 && strlen(type_string2) > 0) {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and genre is null and artist=?",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				} else {
					snprintf(query, sizeof(query),
						 "select %s from %s where valid=1 and category=%d and genre is null and artist is null",
						 result_field,
						 AUDIO_SVC_DB_TABLE_AUDIO,
						 AUDIO_SVC_CATEGORY_MUSIC);
				}
			}

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_GENRE:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and genre=?",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and genre is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_FOLDER:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and folder_uuid=(select folder_uuid from %s where path = ?)",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC,
					 AUDIO_SVC_DB_TABLE_AUDIO_FOLDER);
			} else {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and folder_uuid is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_YEAR:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and year=?",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and year is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_COMPOSER:
		{
			if (type_string && strlen(type_string) > 0) {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and author=?",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			} else {
				snprintf(query, sizeof(query),
					 "select %s from %s where valid=1 and category=%d and author is null",
					 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
					 AUDIO_SVC_CATEGORY_MUSIC);
			}
			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " %s limit %d,%d", AUDIO_SVC_ORDER_BY_TITLE, offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_TOPRATING:
		{
			snprintf(query, sizeof(query),
				 "select %s from %s where valid=1 and category=%d and rating >= %d",
				 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC, AUDIO_SVC_RATING_4);

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " order by rating desc, rated_time desc limit %d,%d",
				 offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_PLAYED_TIME:
		{
			snprintf(query, sizeof(query),
				 "select %s from %s where valid=1 and category=%d and last_played_time > 0",
				 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " order by last_played_time desc limit %d,%d",
				 offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_ADDED_TIME:
		{
			snprintf(query, sizeof(query),
				 "select %s from %s where valid=1 and category=%d and added_time > 0",
				 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " order by added_time desc limit %d,%d",
				 offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	case AUDIO_SVC_TRACK_BY_PLAYED_COUNT:
		{
			snprintf(query, sizeof(query),
				 "select %s from %s where valid=1 and category=%d and played_count > 0",
				 result_field, AUDIO_SVC_DB_TABLE_AUDIO,
				 AUDIO_SVC_CATEGORY_MUSIC);

			filter_condition(filter_mode, filter_mode2, query);

			snprintf(tail_query, sizeof(tail_query),
				 " order by played_count desc limit %d,%d",
				 offset, rows);
			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

		/* To permit duplicated track in a playlist, playlist item index should be returned. */
	case AUDIO_SVC_TRACK_BY_PLAYLIST:
		{
			char *result_field_for_playlist =
			    "b.audio_uuid, b.path, b.thumbnail_path, b.title, b.artist, b.duration, b.rating";
			len =
			    snprintf(query, sizeof(query),
				     "select %s from %s a, %s b where a.playlist_id=%d and b.audio_uuid=a.audio_uuid and b.valid=1 and b.category=%d ",
				     result_field_for_playlist,
				     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
				     AUDIO_SVC_DB_TABLE_AUDIO, (int)type_string,
				     AUDIO_SVC_CATEGORY_MUSIC);

			if (len < 1 || len >= sizeof(query)) {
				audio_svc_error
				    ("snprintf error occured or truncated");
				return AUDIO_SVC_ERROR_INTERNAL;
			} else {
				query[len] = '\0';
			}

			if (filter_mode) {
				snprintf(tail_query, sizeof(tail_query),
					 " and b.title like ?");
				g_strlcat(query, tail_query, sizeof(query));
			}
			if (filter_mode2) {
				snprintf(tail_query, sizeof(tail_query),
					 " and b.title like ?");
				g_strlcat(query, tail_query, sizeof(query));
			}
			snprintf(tail_query, sizeof(tail_query),
				 "  order by a.play_order limit %d,%d", offset,
				 rows);

			g_strlcat(query, tail_query, sizeof(query));
		}
		break;

	default:
		audio_svc_error("Invalid track type");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;

	}

	audio_svc_debug("[SQL query] : %s", query);

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if ((track_type >= AUDIO_SVC_TRACK_BY_ALBUM)
	    && (track_type <= AUDIO_SVC_TRACK_BY_COMPOSER)) {
		if (type_string && strlen(type_string) > 0) {
			if ((track_type == AUDIO_SVC_TRACK_BY_YEAR)
			    && (!strcmp(type_string, AUDIO_SVC_TAG_UNKNOWN))) {
				if (SQLITE_OK !=
				    (err =
				     sqlite3_bind_text(sql_stmt, text_bind,
						       "-1", strlen("-1"),
						       NULL))) {
					audio_svc_error
					    ("cont_string1(%s) binding is failed (%d)",
					     type_string, err);
					SQLITE3_FINALIZE(sql_stmt);
					return AUDIO_SVC_ERROR_DB_INTERNAL;
				}

			} else {
				if (SQLITE_OK !=
				    (err =
				     sqlite3_bind_text(sql_stmt, text_bind,
						       (const char *)
						       type_string, -1,
						       NULL))) {
					audio_svc_error
					    ("cont_string1(%s) binding is failed (%d)",
					     type_string, err);
					SQLITE3_FINALIZE(sql_stmt);
					return AUDIO_SVC_ERROR_DB_INTERNAL;
				}
			}
			text_bind++;
		}
	}

	if ((track_type == AUDIO_SVC_TRACK_BY_ARTIST_GENRE)
	    || (track_type == AUDIO_SVC_TRACK_BY_ARTIST_ALBUM)) {
		if (type_string2 && strlen(type_string2) > 0) {
			if (SQLITE_OK !=
			    (err =
			     sqlite3_bind_text(sql_stmt, text_bind,
					       (char *)type_string2,
					       strlen(type_string2), NULL))) {
				audio_svc_error
				    ("cont_string2(%s) binding is failed (%d)",
				     type_string2, err);
				SQLITE3_FINALIZE(sql_stmt);
				return AUDIO_SVC_ERROR_DB_INTERNAL;
			}
			text_bind++;
		}
	}

	if (filter_mode) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}
	if (filter_mode2) {
		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, text_bind,
				       (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
		text_bind++;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(track[idx].audio_uuid,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_LIST_ITEM_AUDIO_ID), sizeof(track[idx].audio_uuid));
		_strncpy_safe(track[idx].pathname,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_LIST_ITEM_PATHNAME), sizeof(track[idx].pathname));
		_strncpy_safe(track[idx].thumbnail_path,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH), sizeof(track[idx].thumbnail_path));
		_strncpy_safe(track[idx].title,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_LIST_ITEM_TITLE), sizeof(track[idx].title));
		_strncpy_safe(track[idx].artist,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_LIST_ITEM_ARTIST), sizeof(track[idx].artist));
		track[idx].duration = sqlite3_column_int(sql_stmt, AUDIO_SVC_LIST_ITEM_DURATION);
		track[idx].rating = sqlite3_column_int(sql_stmt, AUDIO_SVC_LIST_ITEM_RATING);
		audio_svc_debug ("Index : %d : audio_uuid = %s, title = %s, pathname = %s, duration = %d",
		     idx, track[idx].audio_uuid, track[idx].title, track[idx].pathname, track[idx].duration);

		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_delete_music_track_groups(sqlite3 *handle, audio_svc_group_type_e group_type,
					 const char *type_string)
{
	int err = -1;
	char *sql = NULL;

	switch (group_type) {
	case AUDIO_SVC_GROUP_BY_ALBUM:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where album='%q' and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, type_string,
		     AUDIO_SVC_CATEGORY_MUSIC);
		break;
	case AUDIO_SVC_GROUP_BY_ARTIST:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where artist='%q' and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, type_string,
		     AUDIO_SVC_CATEGORY_MUSIC);
		break;
	case AUDIO_SVC_GROUP_BY_GENRE:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where genre='%q' and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, type_string,
		     AUDIO_SVC_CATEGORY_MUSIC);
		break;
	case AUDIO_SVC_GROUP_BY_FOLDER:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where folder_uuid=(select folder_uuid from %s where path = '%q') and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER,
		     type_string, AUDIO_SVC_CATEGORY_MUSIC);
		break;
	case AUDIO_SVC_GROUP_BY_YEAR:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where year= %d and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, str_2_year(type_string),
		     AUDIO_SVC_CATEGORY_MUSIC);
		break;
	case AUDIO_SVC_GROUP_BY_COMPOSER:
		sql =
		    sqlite3_mprintf
		    ("delete from %s where author='%q' and valid=1 and category=%d",
		     AUDIO_SVC_DB_TABLE_AUDIO, type_string,
		     AUDIO_SVC_CATEGORY_MUSIC);
		break;
	default:
		audio_svc_error("Invalid track type");
		return AUDIO_SVC_ERROR_INVALID_PARAMETER;
	}

	audio_svc_debug("query (%s)", sql);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To delete group is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_search_audio_id_by_path(sqlite3 *handle, const char *path, char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	*audio_id = -1;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql =
	    sqlite3_mprintf
	    ("select audio_uuid from %s where valid=1 and category=%d and path='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_CATEGORY_MUSIC, path);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_search_audio_id_by_path. ret = [%d]",
		     ret);
		return ret;
	}

	_strncpy_safe(audio_id, (const char *)sqlite3_column_text(sql_stmt, 0), AUDIO_SVC_UUID_SIZE+1);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_update_valid_of_music_records(sqlite3 *handle, audio_svc_storage_type_e
					    storage_type, int valid)
{
	audio_svc_debug("storage_type: %d", storage_type);

	int err = -1;
	char *sql =
	    sqlite3_mprintf("update %s set valid = %d where storage_type = %d",
			    AUDIO_SVC_DB_TABLE_AUDIO, valid, storage_type);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To set all items as invalid is failed(%d)",
				err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_count_record_with_path(sqlite3 *handle, const char *path)
{

	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql =
	    sqlite3_mprintf("select count(*) from %s where path='%q'",
			    AUDIO_SVC_DB_TABLE_AUDIO, path);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_count_record_with_path. err = [%d]",
		     ret);
		return count;
	}

	count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return count;

}

int _audio_svc_delete_invalid_music_records(sqlite3 *handle, audio_svc_storage_type_e
					    storage_type)
{
	int idx = 0;
	mp_thumbnailpath_record_t *thumbpath_record = NULL;
	int err = -1;
	int invalid_count = 0;
	int ret = AUDIO_SVC_ERROR_NONE;

	invalid_count = __audio_svc_count_invalid_records_with_thumbnail(handle, storage_type);
	audio_svc_debug("invalid count: %d\n", invalid_count);

	if (invalid_count > 0) {
		thumbpath_record = (mp_thumbnailpath_record_t *)malloc(sizeof(mp_thumbnailpath_record_t) * invalid_count);
		if (thumbpath_record == NULL) {
			audio_svc_debug("fail to memory allocation");
			return AUDIO_SVC_ERROR_OUT_OF_MEMORY;
		}
		memset(thumbpath_record, 0, sizeof(mp_thumbnailpath_record_t) * invalid_count);

		ret = __audio_svc_get_invalid_records_with_thumbnail(handle, storage_type, invalid_count, thumbpath_record);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error("error when get thumbnail record");
			SAFE_FREE(thumbpath_record);
			return ret;
		}
	} else {
		audio_svc_debug("There is no item with thumbnail");
	}

	char *sql = sqlite3_mprintf("delete from %s where valid = 0 and storage_type=%d", AUDIO_SVC_DB_TABLE_AUDIO, storage_type);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To delete invalid items is failed(%d)", err);
		SAFE_FREE(thumbpath_record);

		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

#if 0
	for (idx = 0; idx < invalid_count; idx++) {
		if (strlen(thumbpath_record[idx].thumbnail_path) > 0) {
			ret =
			    _audio_svc_check_and_remove_thumbnail
			    (thumbpath_record[idx].thumbnail_path);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				audio_svc_error
				    ("error _audio_svc_check_and_remove_thumbnail");
				SAFE_FREE(thumbpath_record);
				return ret;
			}
		}
	}

	SAFE_FREE(thumbpath_record);
#endif
	for (idx = 0; idx < invalid_count; idx++) {
		if (strlen(thumbpath_record[idx].thumbnail_path) > 0) {
			if (_audio_svc_remove_file(thumbpath_record[idx].thumbnail_path) == FALSE) {
				audio_svc_error("fail to remove thumbnail file.");
				SAFE_FREE(thumbpath_record);
				return AUDIO_SVC_ERROR_INTERNAL;
			}
		}
	}
	SAFE_FREE(thumbpath_record);
	
	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_valid_in_music_record(sqlite3 *handle, const char *path, int valid)
{
	int err = -1;

	char *sql = sqlite3_mprintf("update %s set valid=%d where path= '%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO, valid, path);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("To update item as valid is failed(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_update_valid_in_music_record_query_add(sqlite3 *handle, const char *path, int valid)
{
	char *sql = sqlite3_mprintf("update %s set valid=%d where path= '%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO, valid, path);

	audio_svc_debug("SQL = [%s]", sql);
	
	_audio_svc_sql_query_add(&g_audio_svc_item_valid_query_list, &sql);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_move_item_query_add(sqlite3 *handle, const char *src_path, const char *path, audio_svc_storage_type_e storage_type, const char *folder_id)
{
	char *sql = NULL;
	sql = sqlite3_mprintf("update %s set path='%q', storage_type=%d where valid=1 and path='%q'",
					AUDIO_SVC_DB_TABLE_AUDIO, path, storage_type, src_path);

	audio_svc_debug("SQL = [%s]", sql);
	_audio_svc_sql_query_add(&g_audio_svc_move_item_query_list, &sql);

	sql = sqlite3_mprintf("update %s set folder_uuid='%q' where path='%q'",
					AUDIO_SVC_DB_TABLE_AUDIO, folder_id, path);

	audio_svc_debug("SQL = [%s]", sql);
	_audio_svc_sql_query_add(&g_audio_svc_move_item_query_list, &sql);

	return AUDIO_SVC_ERROR_NONE;
}

//call this API after beginning transaction. this API do sqlite_exec for the g_audio_svc_sqli_query_list.
int _audio_svc_list_query_do(sqlite3 *handle, audio_svc_query_type_e query_type)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	
	ret = _audio_svc_sql_begin_trans(handle);
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	if (query_type == AUDIO_SVC_QUERY_SET_ITEM_VALID)
		ret = _audio_svc_sql_query_list(handle, &g_audio_svc_item_valid_query_list);
	else if (query_type == AUDIO_SVC_QUERY_MOVE_ITEM)
		ret = _audio_svc_sql_query_list(handle, &g_audio_svc_move_item_query_list);
	else if (query_type == AUDIO_SVC_QUERY_INSERT_ITEM)
		ret = _audio_svc_sql_query_list(handle, &g_audio_svc_insert_item_query_list);
	else
		ret = AUDIO_SVC_ERROR_INVALID_PARAMETER;
	
	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error("_audio_svc_list_query_do failed. start rollback");
		_audio_svc_sql_rollback_trans(handle);
		return ret;
	}

	ret = _audio_svc_sql_end_trans(handle);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error("mb_svc_sqlite3_commit_trans failed.. Now start to rollback\n");
		_audio_svc_sql_rollback_trans(handle);
		return ret;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_get_path(sqlite3 *handle, const char *audio_id, char *path)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql =
	    sqlite3_mprintf
	    ("select path from %s where audio_uuid='%q' and valid=1 and category=%d",
	     AUDIO_SVC_DB_TABLE_AUDIO, audio_id, AUDIO_SVC_CATEGORY_MUSIC);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error("error when _audio_svc_get_path. ret = [%d]",
				ret);
		return ret;
	}

	_strncpy_safe(path, (const char *)sqlite3_column_text(sql_stmt, 0),
		      AUDIO_SVC_PATHNAME_SIZE);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;

}

int _audio_svc_get_and_append_folder_id_by_path(sqlite3 *handle, const char *path, audio_svc_storage_type_e storage_type, char *folder_id)
{
	char *path_name = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	
	path_name = g_path_get_dirname(path);

	ret = _audio_svc_get_folder_id_by_foldername(handle, path_name, folder_id);

	if(ret == AUDIO_SVC_ERROR_DB_NO_RECORD) {
		char *folder_name = NULL;
		int folder_modified_date = 0;
		char *folder_uuid = _media_info_generate_uuid();
		if(folder_uuid == NULL ) {
			audio_svc_error("Invalid UUID");
			SAFE_FREE(path_name);
			return AUDIO_SVC_ERROR_INTERNAL;
		}
		
		folder_name = g_path_get_basename(path_name);
		folder_modified_date = _audio_svc_get_file_dir_modified_date(path_name);

		ret = _audio_svc_append_audio_folder(handle, storage_type, folder_uuid, path_name, folder_name, folder_modified_date);
		SAFE_FREE(folder_name);
		_strncpy_safe(folder_id, folder_uuid, AUDIO_SVC_UUID_SIZE+1);
	}

	SAFE_FREE(path_name);

	return ret;
}
int _audio_svc_get_folder_id_by_foldername(sqlite3 *handle, const char *folder_name, char *folder_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT folder_uuid FROM %s WHERE path = '%q';", AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, folder_name);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		if(ret == AUDIO_SVC_ERROR_DB_NO_RECORD) {
			audio_svc_debug("there is no folder.");
		}	
		else {
			audio_svc_error("error when _audio_svc_get_folder_id_by_foldername. err = [%d]", ret);
		}
		return ret;
	}

	_strncpy_safe(folder_id, (const char *)sqlite3_column_text(sql_stmt, 0), AUDIO_SVC_UUID_SIZE+1);

	SQLITE3_FINALIZE(sql_stmt);

	return ret;
}

int _audio_svc_append_audio_folder(sqlite3 *handle, audio_svc_storage_type_e storage_type,
				    const char *folder_id, const char *path_name, const char *folder_name, int modified_date)
{
	int err = -1;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	char *sql = sqlite3_mprintf("insert into %s (folder_uuid, path, folder_name, storage_type, modified_date) values ('%q', '%q', '%q', '%d', '%d'); ",
					     AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, folder_id, path_name, folder_name, storage_type, modified_date);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("failed to insert albums");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

/* handle by one items. */
int _audio_svc_check_and_update_folder_table(sqlite3 *handle, const char *path_name)
{
	int err = -1;
	char folder_id[AUDIO_SVC_UUID_SIZE+1] = {0,};
	char *folder_path_name = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;

	folder_path_name = g_path_get_dirname(path_name);
	if (folder_path_name == NULL) {
		audio_svc_error("error when get path name");
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	ret = _audio_svc_get_folder_id_by_foldername(handle, folder_path_name, folder_id);
	SAFE_FREE(folder_path_name);

	if(ret == AUDIO_SVC_ERROR_DB_NO_RECORD) {
		audio_svc_error("error when _audio_svc_get_folder_id_by_foldername ");
		return AUDIO_SVC_ERROR_DB_NO_RECORD;
	}

	char *sql =
	    sqlite3_mprintf
	    ("delete from %s where folder_uuid='%q' and ((SELECT count(*) FROM %s WHERE folder_uuid='%q')=0);",
	     AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, folder_id,
	     AUDIO_SVC_DB_TABLE_AUDIO, folder_id);
	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("failed to delete audio_folder item");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

/* batch processing */
int _audio_svc_update_folder_table(sqlite3 *handle)
{
	int err = -1;
	char *sql = NULL;

	sql =
	    sqlite3_mprintf
	    ("delete from %s where folder_uuid IN (select folder_uuid FROM %s where folder_uuid NOT IN (select folder_uuid from %s))",
	     AUDIO_SVC_DB_TABLE_AUDIO_FOLDER, AUDIO_SVC_DB_TABLE_AUDIO_FOLDER,
	     AUDIO_SVC_DB_TABLE_AUDIO);

	err = _audio_svc_sql_query(handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("failed to delete audio_folder item");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_check_and_update_albums_table(sqlite3 *handle, const char *album)
{
	int err = -1;
	char *sql = NULL;
	
	/* batch processing */
	if(!STRING_VALID(album)) {
		sql = sqlite3_mprintf("delete from %s where album_id NOT IN (SELECT album_id FROM %s);",
			     AUDIO_SVC_DB_TABLE_ALBUMS, AUDIO_SVC_DB_TABLE_AUDIO);
		err = _audio_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			audio_svc_error("failed to update albums table");
			if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
				return err;
			}
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	} else {
		sql = sqlite3_mprintf("delete from %s where album='%q' and ((SELECT count(*) FROM %s WHERE album='%q')=0);",
		     AUDIO_SVC_DB_TABLE_ALBUMS, album, AUDIO_SVC_DB_TABLE_AUDIO, album);
		err = _audio_svc_sql_query(handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			audio_svc_error("failed to update albums table");
			if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
				return err;
			}
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *path, char *thumb_path)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql =
	    sqlite3_mprintf
	    ("select thumbnail_path from %s where valid=1 and path='%q' ",
	     AUDIO_SVC_DB_TABLE_AUDIO, path);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_get_thumbnail_path_by_path. ret=[%d]",
		     ret);
		return ret;
	}

	_strncpy_safe(thumb_path,
		      (const char *)sqlite3_column_text(sql_stmt, 0),
		      AUDIO_SVC_PATHNAME_SIZE);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

char *_audio_svc_get_thumbnail_path_by_album_id(sqlite3 *handle, int album_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE] = { 0, };

	char *sql =
	    sqlite3_mprintf("select _data from %s where album_id=%d",
			    AUDIO_SVC_DB_TABLE_ALBUM_ART, album_id);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when __audio_svc_get_genre_id. err = [%d]", ret);
		return NULL;
	}

	_strncpy_safe(thumbnail_path,
		      (const char *)sqlite3_column_text(sql_stmt, 0),
		      sizeof(thumbnail_path));

	SQLITE3_FINALIZE(sql_stmt);

	return g_strdup(thumbnail_path);
}

int _audio_svc_check_and_remove_thumbnail(sqlite3 *handle, const char *thumbnail_path)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql =
	    sqlite3_mprintf("select count(*) from %s where thumbnail_path='%q'",
			    AUDIO_SVC_DB_TABLE_AUDIO, thumbnail_path);

	ret = _audio_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_check_and_remove_thumbnail. err = [%d]",
		     ret);
		return ret;
	}

	count = sqlite3_column_int(sql_stmt, 0);
	SQLITE3_FINALIZE(sql_stmt);

	if (count < 1) {
		if (_audio_svc_remove_file(thumbnail_path) == FALSE) {
			audio_svc_error("fail to remove thumbnail file.");
			return AUDIO_SVC_ERROR_INTERNAL;
		}
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_list_search(sqlite3 *handle, audio_svc_audio_item_s *item,
							char *where_query,
							audio_svc_search_order_e order_field,
							int offset,
							int count
							)
{
	int len = 0;
	int err = -1;
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char condition_str[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char order_str[AUDIO_SVC_QUERY_SIZE] = { 0 };

	sqlite3_stmt *sql_stmt = NULL;
#if 0
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
#endif
	snprintf(query, sizeof(query), "SELECT * FROM audio_media WHERE %s", where_query);

	audio_svc_debug("");
	if (_audio_svc_get_order_field_str(order_field, order_str, 
			AUDIO_SVC_QUERY_SIZE) < 0) {
		audio_svc_error("_audio_svc_get_order_field_str failure");
		return AUDIO_SVC_ERROR_INTERNAL;
	}
	audio_svc_debug("");

	snprintf(condition_str, sizeof(condition_str), " ORDER BY %s ", order_str);
	len = g_strlcat(query, condition_str, sizeof(query));

	if (len >= sizeof(query)) {
		audio_svc_error("strlcat returns failure ( %d )", len);
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	memset(condition_str, 0x00, sizeof(condition_str));

	snprintf(condition_str, sizeof(condition_str), " LIMIT %d,%d ", offset, count);
	len = g_strlcat(query, condition_str, sizeof(query));

	if (len >= sizeof(query)) {
		audio_svc_error("strlcat returns failure ( %d )", len);
		return AUDIO_SVC_ERROR_INTERNAL;
	}

	audio_svc_debug("Query : %s", query);

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (err != SQLITE_OK) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		__audio_svc_get_next_record(item, sql_stmt);
		item++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

