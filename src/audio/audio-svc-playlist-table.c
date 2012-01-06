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
 * This file defines structure and functions related to database for managing playlists.
 *
 * @file       	audio-svc-playlist-table.c
 * @version 	0.1
 */

#include <glib.h>
#include "media-info-util.h"
#include "audio-svc-error.h"
#include "audio-svc-playlist-table.h"
#include "audio-svc-debug.h"
#include "audio-svc-types-priv.h"
#include "audio-svc-utils.h"
#include "audio-svc-db-utils.h"

#define AUDIO_SVC_ORDER_BY_PLAYLIST_NAME		"ORDER BY name COLLATE NOCASE"

static int __audio_svc_create_playlist_db_table(void);
static int __audio_svc_create_playlist_item_db_table(void);
static int __audio_svc_delete_playlist_item_records_by_playlist_id(int
								   playlist_id);

static int __audio_svc_create_playlist_db_table(void)
{
	int err = -1;

	/* hjkim, 110125, make name to unique */
	char *sql = sqlite3_mprintf("create table if not exists %s (\
			_id				INTEGER PRIMARY KEY, \
			name			TEXT NOT NULL UNIQUE\
			);", AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to create playlist table (%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

/* when memory card is removed, the items in the memory card is displayed in grey out when selecting playlist item view.
  If same memory card is inserted again, they are handled as same as other phone items in the playlist view. */
static int __audio_svc_create_playlist_item_db_table(void)
{
	int err = -1;
	char *sql = NULL;

	sql = sqlite3_mprintf("create table if not exists %s (\
			_id			INTEGER primary key autoincrement, \
			playlist_id	INTEGER NOT NULL,\
			audio_id		TEXT NOT NULL,\
			play_order	INTEGER NOT NULL\
			);", AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP);

	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to create playlist item table (%d)",
				err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}
	/* hjkim, 111020, add trigger to remove item from audio_playlists_map when item remove */
	sql =
	    sqlite3_mprintf
	    ("CREATE TRIGGER IF NOT EXISTS audio_playlists_map_cleanup_1 DELETE ON %s BEGIN DELETE FROM %s WHERE audio_id = old.audio_id;END;",
	     AUDIO_SVC_DB_TABLE_AUDIO, AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("error while create TRIGGER albumart_cleanup1");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

/* when deleting a playlist, all items included in the playlist should be removed together */
static int __audio_svc_delete_playlist_item_records_by_playlist_id(int
								   playlist_id)
{
	int err = -1;
	char *sql = sqlite3_mprintf("delete from %s where playlist_id = %d",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
				    playlist_id);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to delete items by playlist index");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_create_playlist_table(void)
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = __audio_svc_create_playlist_db_table();
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	ret = __audio_svc_create_playlist_item_db_table();
	audio_svc_retv_if(ret != AUDIO_SVC_ERROR_NONE, ret);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_truncate_playlist_table(void)
{
	int err = -1;
	char *sql = sqlite3_mprintf("delete from %s",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_debug("It failed to truncate playlist table", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	sql =
	    sqlite3_mprintf("delete from %s",
			    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to truncate playlist item table",
				err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_insert_playlist_record(const char *playlist_name,
				      int *playlist_id)
{
	int err = -1;
	int ret = AUDIO_SVC_ERROR_NONE;
	int plist_idx = 0;
	char *sql = NULL;
	sqlite3_stmt *sql_stmt = NULL;

	/* get the max play_order */
	sql =
	    sqlite3_mprintf("select max(_id) from %s",
			    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_insert_playlist_record. ret = [%d]",
		     ret);
		return ret;
	}

	plist_idx = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	++plist_idx;

	sql =
	    sqlite3_mprintf("insert into %s (_id, name) values (%d, '%q')",
			    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS, plist_idx,
			    playlist_name);

	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to insert playlist(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	*playlist_id = plist_idx;
	audio_svc_debug("new playlist id is [%d]", *playlist_id);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_playlist_record(int playlist_id)
{
	int err = -1;
	char *sql = sqlite3_mprintf("delete from %s where _id=%d",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS,
				    playlist_id);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to delete playlist(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return
	    __audio_svc_delete_playlist_item_records_by_playlist_id
	    (playlist_id);

}

int _audio_svc_update_playlist_record_by_name(int playlist_id,
					      const char *new_playlist_name)
{
	int err = -1;
	char *sql = sqlite3_mprintf("update %s set name='%q' where _id=%d",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS,
				    new_playlist_name, playlist_id);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update playlist name(%d)", err);
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_count_playlist_records(const char *filter_string,
				      const char *filter_string2, int *count)
{
	int err = -1;
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;

	sqlite3_stmt *sql_stmt = NULL;
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	const char *filter_1 = " where name like ?";
	const char *filter_2 = " and name like ?";
	if (filter_string) {
		if (strlen(filter_string) > 0) {
			filter_mode = TRUE;
		}
	}
	if (filter_string2) {
		if (strlen(filter_string2) > 0) {
			filter_mode2 = TRUE;
		}
	}

	snprintf(query, sizeof(query), "select count(*) from %s",
		 AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS);

	if (filter_mode) {
		g_strlcat(query, filter_1, sizeof(query));
	}
	if (filter_mode2) {
		if (filter_mode) {
			g_strlcat(query, filter_2, sizeof(query));
		} else {
			g_strlcat(query, filter_1, sizeof(query));
		}
	};

	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (SQLITE_OK != err) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (filter_mode) {
		char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
		snprintf(filter_query, sizeof(filter_query), "%s%%",
			 filter_string);

		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, 1, (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}

	if (filter_mode2) {
		char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
		snprintf(filter_query2, sizeof(filter_query2), "%s%%",
			 filter_string2);

		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, 1, (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
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

int _audio_svc_get_playlist_records(int offset, int rows,
				    const char *filter_string,
				    const char *filter_string2,
				    audio_svc_playlist_s *playlists)
{
	int err = -1;
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char tail_query[70] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	const char *filter_1 = " where name like ?";
	const char *filter_2 = " and name like ?";
	int idx = 0;
	int idx_1 = 0;
	int ret = AUDIO_SVC_ERROR_NONE;

	sqlite3_stmt *sql_stmt = NULL;
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (filter_string) {
		if (strlen(filter_string) > 0) {
			filter_mode = TRUE;
		}
	}
	if (filter_string2) {
		if (strlen(filter_string2) > 0) {
			filter_mode2 = TRUE;
		}
	}

	snprintf(query, sizeof(query), "select _id, name from %s",
		 AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS);

	if (filter_mode) {
		g_strlcat(query, filter_1, sizeof(query));
	}
	if (filter_mode2) {
		if (filter_mode) {
			g_strlcat(query, filter_2, sizeof(query));
		} else {
			g_strlcat(query, filter_1, sizeof(query));
		}
	}
	snprintf(tail_query, sizeof(tail_query), " %s limit %d,%d", AUDIO_SVC_ORDER_BY_PLAYLIST_NAME, offset, rows);
	g_strlcat(query, tail_query, sizeof(query));

	audio_svc_debug("[SQL query] : %s", query);
	err = sqlite3_prepare_v2(handle, query, -1, &sql_stmt, NULL);
	if (SQLITE_OK != err) {
		audio_svc_error("prepare error [%s]", sqlite3_errmsg(handle));
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	if (filter_mode) {
		char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = "";
		snprintf(filter_query, sizeof(filter_query), "%s%%",
			 filter_string);

		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, 1, (char *)filter_query,
				       strlen(filter_query), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}
	if (filter_mode2) {
		char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = "";
		snprintf(filter_query2, sizeof(filter_query2), "%s%%",
			 filter_string2);

		if (SQLITE_OK !=
		    (err =
		     sqlite3_bind_text(sql_stmt, 2, (char *)filter_query2,
				       strlen(filter_query2), NULL))) {
			audio_svc_error
			    ("filter_query(%s) binding is failed (%d)",
			     filter_query2, err);
			SQLITE3_FINALIZE(sql_stmt);
			return AUDIO_SVC_ERROR_DB_INTERNAL;
		}
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		playlists[idx].playlist_id = sqlite3_column_int(sql_stmt, 0);
		_strncpy_safe(playlists[idx].name,
			      (const char *)sqlite3_column_text(sql_stmt, 1),
			      sizeof(playlists[idx].name));
		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	/* hjkim, 110303, add query to get thumbnail_path of playlists */
	for (idx_1 = 0; idx_1 < idx; idx_1++) {
		char *sql =
		    sqlite3_mprintf
		    ("select thumbnail_path from %s where audio_id= \
			(select audio_id from %s where playlist_id=%d and \
			play_order=(select min(a.play_order) from %s a, %s b where a.playlist_id=%d and b.audio_id=a.audio_id and b.valid=1 and b.category=%d))",
		     AUDIO_SVC_DB_TABLE_AUDIO,
		     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
		     playlists[idx_1].playlist_id,
		     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
		     AUDIO_SVC_DB_TABLE_AUDIO, playlists[idx_1].playlist_id,
		     AUDIO_SVC_CATEGORY_MUSIC);

		ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

		if (ret == AUDIO_SVC_ERROR_DB_NO_RECORD) {
			audio_svc_debug("No item in playlist");
			continue;
		}

		if (ret != AUDIO_SVC_ERROR_NONE) {
			audio_svc_error
			    ("error when _audio_svc_get_playlist_records. ret = [%d]",
			     ret);
			return ret;
		}
		_strncpy_safe(playlists[idx_1].thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), sizeof(playlists[idx_1].thumbnail_path));
		
		SQLITE3_FINALIZE(sql_stmt);
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_count_playlist_item_records(int playlist_id,
					   const char *filter_string,
					   const char *filter_string2,
					   int *count)
{
	int err = -1;
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };
	char filter_query[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	char filter_query2[AUDIO_SVC_METADATA_LEN_MAX + 5] = { 0 };
	bool filter_mode = FALSE;
	bool filter_mode2 = FALSE;
	int text_bind = 1;

	sqlite3_stmt *sql_stmt = NULL;
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

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

	snprintf(query, sizeof(query),
		 "select count(*) from %s a, %s b where a.playlist_id=%d and b.audio_id=a.audio_id and b.valid=1 and b.category=%d",
		 AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
		 AUDIO_SVC_DB_TABLE_AUDIO, playlist_id,
		 AUDIO_SVC_CATEGORY_MUSIC);

	if (filter_mode) {
		g_strlcat(query, " and b.title like ?", sizeof(query));
	}
	if (filter_mode2) {
		g_strlcat(query, " and b.title like ?", sizeof(query));
	}

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

int _audio_svc_get_playlist_item_records(int playlist_id,
					 const char *filter_string,
					 const char *filter_string2, int offset,
					 int rows,
					 audio_svc_playlist_item_s *track)
{
	char query[AUDIO_SVC_QUERY_SIZE] = { 0 };

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
	sqlite3 *handle = _media_info_get_proper_handle();
	if (handle == NULL) {
		audio_svc_debug("handle is NULL");
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

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

	char *result_field_for_playlist =
	    "a._id, b.audio_id, b.path, b.thumbnail_path, b.title, b.artist, b.duration, b.rating, a.play_order";
	len =
	    snprintf(query, sizeof(query),
		     "select %s from %s a, %s b where a.playlist_id=%d and b.audio_id=a.audio_id and b.valid=1 and b.category=%d ",
		     result_field_for_playlist,
		     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
		     AUDIO_SVC_DB_TABLE_AUDIO, playlist_id,
		     AUDIO_SVC_CATEGORY_MUSIC);

	if (len < 1 || len >= sizeof(query)) {
		audio_svc_error("snprintf error occured or truncated");
		return AUDIO_SVC_ERROR_INTERNAL;
	} else {
		query[len] = '\0';
	}

	if (filter_mode) {
		snprintf(tail_query, sizeof(tail_query), " and b.title like ?");
		g_strlcat(query, tail_query, sizeof(query));
	}
	if (filter_mode2) {
		snprintf(tail_query, sizeof(tail_query), " and b.title like ?");
		g_strlcat(query, tail_query, sizeof(query));
	}
	snprintf(tail_query, sizeof(tail_query),
		 "  order by a.play_order limit %d,%d", offset, rows);

	g_strlcat(query, tail_query, sizeof(query));

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

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		track[idx].u_id =
		    sqlite3_column_int(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_UID);
		_strncpy_safe(track[idx].audio_id,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID), sizeof(track[idx].audio_id));
		_strncpy_safe(track[idx].pathname,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_PATHNAME), sizeof(track[idx].pathname));
		_strncpy_safe(track[idx].thumbnail_path,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_THUMBNAIL_PATH), sizeof(track[idx].thumbnail_path));
		_strncpy_safe(track[idx].title,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_TITLE), sizeof(track[idx].title));
		_strncpy_safe(track[idx].artist,
			      (const char *)sqlite3_column_text(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_ARTIST), sizeof(track[idx].artist));
		track[idx].duration = sqlite3_column_int(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_DURATION);
		track[idx].rating = sqlite3_column_int(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_RATING);
		track[idx].play_order = sqlite3_column_int(sql_stmt, AUDIO_SVC_PLAYLIST_ITEM_PLAY_ORDER);
		
		audio_svc_debug("u_id = %d, audio_id = %s, title = %s, pathname = %s, duration = %d, play_order = %d",
		     track[idx].u_id, track[idx].audio_id, track[idx].title, track[idx].pathname, track[idx].duration, track[idx].play_order);

		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_count_playlist_records_by_name(const char *playlist_name,
					      int *count)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("select count(*) from %s where name='%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS,
				    playlist_name);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_count_playlist_records_by_name. ret = [%d]",
		     ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_get_playlist_id_by_name(const char *playlist_name,
				       int *playlist_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("select _id from %s where name='%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS,
				    playlist_name);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_get_playlist_id_by_name. ret = [%d]",
		     ret);
		return ret;
	}

	*playlist_id = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_get_audio_id_by_uid(int uid, char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("select audio_id from %s where _id=%d",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, uid);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error("error when _audio_svc_get_audio_id_by_uid. ret = [%d]", ret);
		return ret;
	}

	_strncpy_safe(audio_id, (const char *)sqlite3_column_text(sql_stmt, 0), AUDIO_SVC_UUID_SIZE+1);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_get_playlist_name_by_playlist_id(int playlist_id,
						char *playlist_name)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("select name from %s where _id =%d",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS,
				    playlist_id);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_get_playlist_name_by_playlist_id. ret = [%d]",
		     ret);
		return ret;
	}

	_strncpy_safe(playlist_name,
		      (const char *)sqlite3_column_text(sql_stmt, 0),
		      AUDIO_SVC_PLAYLIST_NAME_SIZE);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_insert_playlist_item_record(int playlist_id, const char *audio_id)
{
	int err = -1;
	int ret = AUDIO_SVC_ERROR_NONE;
	char *sql = NULL;
	int play_order = 0;
	sqlite3_stmt *sql_stmt = NULL;

	/* get the max play_order */
	sql =
	    sqlite3_mprintf
	    ("select max(play_order) from %s where playlist_id = %d",
	     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, playlist_id);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_insert_playlist_item_record. ret = [%d]",
		     ret);
		return ret;
	}

	play_order = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	++play_order;

	/* insert the new record */
	sql = sqlite3_mprintf("insert into %s ( playlist_id, audio_id, play_order) values (%d, '%q', %d)",
	     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, playlist_id, audio_id, play_order);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error(" It failed to insert item to playlist");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_check_duplication_records_in_playlist(int playlist_id,
						     const char *audio_id, int *count)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql =
	    sqlite3_mprintf
	    ("select count(*) from %s where playlist_id=%d and audio_id='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, playlist_id, audio_id);

	ret = _audio_svc_sql_prepare_to_step(sql, &sql_stmt);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		audio_svc_error
		    ("error when _audio_svc_check_duplication_records_in_playlist. ret = [%d]",
		     ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_playlist_item_record_from_playlist_by_audio_id(int playlist_id, const char *audio_id)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("delete from %s where playlist_id=%d and audio_id='%q'",
	     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, playlist_id, audio_id);

	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error
		    ("It failed to remove item from playlist by audio_id");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_playlist_item_record_from_playlist_by_uid(int playlist_id,
								int uid)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf("delete from %s where playlist_id=%d and _id=%d",
			    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, playlist_id,
			    uid);

	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error
		    ("It failed to remove item from playlist by uid");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_delete_playlist_item_records_by_audio_id(const char *audio_id)
{
	int err = -1;
	char *sql = sqlite3_mprintf("delete from %s where audio_id='%q'",
				    AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP,
				    audio_id);

	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to delete items by audio_id");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}

int _audio_svc_update_item_play_order(int playlist_id, int uid,
				      int new_play_order)
{
	int err = -1;
	char *sql =
	    sqlite3_mprintf
	    ("update %s set play_order=%d where _id=%d and playlist_id = %d",
	     AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP, new_play_order, uid,
	     playlist_id);
	err = _audio_svc_sql_query(sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		audio_svc_error("It failed to update play order");
		if (err == AUDIO_SVC_ERROR_DB_CONNECT) {
			return err;
		}
		return AUDIO_SVC_ERROR_DB_INTERNAL;
	}

	return AUDIO_SVC_ERROR_NONE;
}
