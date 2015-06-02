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
#include <sys/stat.h>
#include <db-util.h>
#include <media-util.h>
#include <errno.h>
#include "media-svc-env.h"
#include "media-svc-debug.h"
#include "media-svc-util.h"
#include "media-svc-db-utils.h"
#include "media-util-err.h"
#include "media-util-db.h"
#include "media-svc-media.h"

static int __media_svc_create_update_media_table(sqlite3 *db_handle);

#define MEDIA_DB_SCHEMA	"CREATE TABLE IF NOT EXISTS %s (\
				media_uuid			TEXT PRIMARY KEY, \
				path				TEXT NOT NULL UNIQUE, \
				file_name			TEXT NOT NULL, \
				media_type			INTEGER,\
				mime_type			TEXT, \
				size				INTEGER DEFAULT 0, \
				added_time			INTEGER DEFAULT 0,\
				modified_time			INTEGER DEFAULT 0, \
				folder_uuid			TEXT NOT NULL, \
				thumbnail_path			TEXT, \
				title				TEXT, \
				album_id			INTEGER DEFAULT 0, \
				album				TEXT, \
				artist				TEXT, \
				album_artist    		TEXT, \
				genre				TEXT, \
				composer			TEXT, \
				year				TEXT, \
				recorded_date			TEXT, \
				copyright			TEXT, \
				track_num			TEXT, \
				description			TEXT, \
				bitrate				INTEGER DEFAULT -1, \
				bitpersample		INTEGER DEFAULT 0, \
				samplerate			INTEGER DEFAULT -1, \
				channel				INTEGER DEFAULT -1, \
				duration			INTEGER DEFAULT -1, \
				longitude			DOUBLE DEFAULT 0, \
				latitude			DOUBLE DEFAULT 0, \
				altitude			DOUBLE DEFAULT 0, \
				width				INTEGER DEFAULT -1, \
				height				INTEGER DEFAULT -1, \
				datetaken			TEXT, \
				orientation			INTEGER DEFAULT -1, \
				burst_id			TEXT, \
				played_count			INTEGER DEFAULT 0, \
				last_played_time		INTEGER DEFAULT 0, \
				last_played_position		INTEGER DEFAULT 0, \
				rating				INTEGER DEFAULT 0, \
				favourite			INTEGER DEFAULT 0, \
				author				TEXT, \
				provider			TEXT, \
				content_name			TEXT, \
				category			TEXT, \
				location_tag			TEXT, \
				age_rating			TEXT, \
				keyword				TEXT, \
				is_drm				INTEGER DEFAULT 0, \
				storage_type			INTEGER, \
				timeline 			INTEGER DEFAULT 0, \
				weather				TEXT, \
				sync_status		INTEGER DEFAULT 0, \
				file_name_pinyin	TEXT, \
				title_pinyin    TEXT, \
				album_pinyin    TEXT, \
				artist_pinyin    TEXT, \
				album_artist_pinyin      TEXT, \
				genre_pinyin    TEXT, \
				composer_pinyin   TEXT, \
				copyright_pinyin   TEXT, \
				description_pinyin   TEXT, \
				author_pinyin    TEXT, \
				provider_pinyin   TEXT, \
				content_name_pinyin  TEXT, \
				category_pinyin   TEXT, \
				location_tag_pinyin  TEXT, \
				age_rating_pinyin   TEXT, \
				keyword_pinyin   TEXT, \
				validity			INTEGER DEFAULT 1, \
				unique(path, file_name) \
				);"

static int __media_svc_busy_handler(void *pData, int count);

static int __media_svc_busy_handler(void *pData, int count)
{
	usleep(50000);

	media_svc_debug("media_svc_busy_handler called : %d", count);

	return 100 - count;
}

int _media_svc_connect_db_with_handle(sqlite3 **db_handle)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_debug_func();

	/*Connect DB*/
	ret = db_util_open(MEDIA_SVC_DB_NAME, db_handle, DB_UTIL_REGISTER_HOOK_METHOD);

	if (SQLITE_OK != ret) {

		media_svc_error("error when db open");
		*db_handle = NULL;
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/*Register busy handler*/
	if (*db_handle) {
		ret = sqlite3_busy_handler(*db_handle, __media_svc_busy_handler, NULL);

		if (SQLITE_OK != ret) {

			if (*db_handle) {
				media_svc_error("[error when register busy handler] %s\n", sqlite3_errmsg(*db_handle));
			}

			db_util_close(*db_handle);
			*db_handle = NULL;

			return MS_MEDIA_ERR_DB_INTERNAL;
		}
	} else {
		*db_handle = NULL;
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_disconnect_db_with_handle(sqlite3 *db_handle)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_debug_func();

	ret = db_util_close(db_handle);

	if (SQLITE_OK != ret) {
		media_svc_error("Error when db close : %s", sqlite3_errmsg(db_handle));
		db_handle = NULL;
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_media_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	media_svc_debug_func();

	sql = sqlite3_mprintf(MEDIA_DB_SCHEMA, MEDIA_SVC_DB_TABLE_MEDIA);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Index*/
	sql = sqlite3_mprintf("	CREATE INDEX IF NOT EXISTS media_media_type_idx on %s (media_type); \
						CREATE INDEX IF NOT EXISTS media_title_idx on %s (title); \
						CREATE INDEX IF NOT EXISTS media_modified_time_idx on %s (modified_time); \
						CREATE INDEX IF NOT EXISTS media_provider_idx on %s (provider); \
						CREATE INDEX IF NOT EXISTS folder_uuid_idx on %s (folder_uuid); \
						CREATE INDEX IF NOT EXISTS media_album_idx on %s (album); \
						CREATE INDEX IF NOT EXISTS media_artist_idx on %s (artist); \
						CREATE INDEX IF NOT EXISTS media_author_idx on %s (author); \
						CREATE INDEX IF NOT EXISTS media_category_idx on %s (category); \
						CREATE INDEX IF NOT EXISTS media_composer_idx on %s (composer); \
						CREATE INDEX IF NOT EXISTS media_content_name_idx on %s (content_name); \
						CREATE INDEX IF NOT EXISTS media_file_name_idx on %s (file_name); \
						CREATE INDEX IF NOT EXISTS media_genre_idx on %s (genre); \
						CREATE INDEX IF NOT EXISTS media_location_tag_idx on %s (location_tag); \
						CREATE INDEX IF NOT EXISTS media_media_uuid_idx on %s (media_uuid); \
						CREATE INDEX IF NOT EXISTS media_timeline_idx on %s (timeline); \
						CREATE INDEX IF NOT EXISTS media_path_idx on %s (path); \
						",
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA,
						MEDIA_SVC_DB_TABLE_MEDIA);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_folder_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	media_svc_debug_func();

	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				folder_uuid 		TEXT PRIMARY KEY, \
				path				TEXT NOT NULL UNIQUE, \
				name 			TEXT NOT NULL, \
				modified_time		INTEGER DEFAULT 0, \
				name_pinyin 		TEXT, \
				storage_type		INTEGER, \
				unique(path, name, storage_type) \
				);",
				MEDIA_SVC_DB_TABLE_FOLDER);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove folder which have no content from folder when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS folder_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s \
				WHERE (SELECT count(*) FROM %s WHERE folder_uuid=old.folder_uuid)=1 AND folder_uuid=old.folder_uuid;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_TABLE_MEDIA);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Index*/
	sql = sqlite3_mprintf("	CREATE INDEX IF NOT EXISTS folder_folder_uuid_idx on %s (folder_uuid); \
						",
						MEDIA_SVC_DB_TABLE_FOLDER);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_playlist_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char * sql = NULL;

	media_svc_debug_func();

	/*Create playlist table*/
	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				playlist_id		INTEGER PRIMARY KEY AUTOINCREMENT, \
				name			TEXT NOT NULL UNIQUE,\
				name_pinyin 		TEXT, \
				thumbnail_path 	TEXT\
				);",
				MEDIA_SVC_DB_TABLE_PLAYLIST);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/*Create playlist_map table*/
	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				_id				INTEGER PRIMARY KEY AUTOINCREMENT, \
				playlist_id		INTEGER NOT NULL,\
				media_uuid		TEXT NOT NULL,\
				play_order		INTEGER NOT NULL\
				);",
				MEDIA_SVC_DB_TABLE_PLAYLIST_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create playlist_view*/
	sql = sqlite3_mprintf(" \
		CREATE VIEW IF NOT EXISTS playlist_view AS \
		SELECT p.playlist_id, p.name, p.thumbnail_path AS p_thumbnail_path, media_count, pm._id as pm_id, pm.play_order, m.media_uuid, path, file_name, media_type, mime_type, size, added_time, modified_time, m.thumbnail_path, description, rating, favourite, author, provider, content_name, category, location_tag, age_rating, keyword, is_drm, storage_type, longitude, latitude, altitude, width, height, datetaken, orientation, title, album, artist, album_artist, genre, composer, year, recorded_date, copyright, track_num, bitrate, duration, played_count, last_played_time, last_played_position, samplerate, channel, weather, burst_id, timeline, sync_status, bitpersample FROM playlist AS p \
		INNER JOIN playlist_map AS pm \
		INNER JOIN media AS m \
		INNER JOIN (SELECT count(playlist_id) as media_count, playlist_id FROM playlist_map group by playlist_id) as cnt_tbl \
				ON (p.playlist_id=pm.playlist_id AND pm.media_uuid = m.media_uuid AND cnt_tbl.playlist_id=pm.playlist_id AND m.validity=1) \
		UNION \
			SELECT playlist_id, name, thumbnail_path, 0, 0, -1, NULL, NULL, -1, -1, -1, -1, -1, NULL, NULL, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1, NULL, -1, NULL, NULL, -1, 0, 0 FROM playlist \
				WHERE playlist_id NOT IN (select playlist_id from playlist_map) \
		UNION \
			SELECT playlist_id, name, thumbnail_path, 0, 0, -1, NULL, NULL, -1, -1, -1, -1, -1, NULL, NULL, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, -1, 0, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1, NULL, -1, NULL, NULL, -1, 0, 0 FROM playlist \
				WHERE playlist_id IN (select pm.playlist_id from playlist_map AS pm INNER JOIN media AS m ON (pm.media_uuid= m.media_uuid) AND m.validity=0); \
		");

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/* Create Trigger to remove media from playlist_map when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS playlist_map_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s WHERE media_uuid=old.media_uuid;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove media from playlist_map when playlist removed from playlist table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS playlist_map_cleanup_1 \
				DELETE ON %s BEGIN DELETE FROM %s WHERE playlist_id=old.playlist_id;END;",
				MEDIA_SVC_DB_TABLE_PLAYLIST, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_album_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char * sql = NULL;

	media_svc_debug_func();

	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				album_id			INTEGER PRIMARY KEY AUTOINCREMENT, \
				name			TEXT NOT NULL,\
				artist			TEXT, \
				album_art		TEXT, \
				unique(name, artist) \
				);",
				MEDIA_SVC_DB_TABLE_ALBUM);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove album when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS album_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s \
				WHERE (SELECT count(*) FROM %s WHERE album_id=old.album_id)=1 AND album_id=old.album_id;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_ALBUM, MEDIA_SVC_DB_TABLE_MEDIA);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_tag_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char * sql = NULL;

	media_svc_debug_func();

	/*Create tag table*/
	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				tag_id		INTEGER PRIMARY KEY AUTOINCREMENT, \
				name		TEXT NOT NULL UNIQUE, \
				name_pinyin 		TEXT \
				);",
				MEDIA_SVC_DB_TABLE_TAG);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/*Create tag_map table*/
	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				_id				INTEGER PRIMARY KEY AUTOINCREMENT, \
				tag_id			INTEGER NOT NULL,\
				media_uuid		TEXT NOT NULL,\
				unique(tag_id, media_uuid) \
				);",
				MEDIA_SVC_DB_TABLE_TAG_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/*Create tag_view*/
	sql = sqlite3_mprintf("\
				CREATE VIEW IF NOT EXISTS tag_view AS \
				SELECT \
					t.tag_id, t.name, media_count, tm._id as tm_id, m.media_uuid, path, file_name, media_type, mime_type, size, added_time, modified_time, thumbnail_path, description, rating, favourite, author, provider, content_name, category, location_tag, age_rating, keyword, is_drm, storage_type, longitude, latitude, altitude, width, height, datetaken, orientation, title, album, artist, album_artist, genre, composer, year, recorded_date, copyright, track_num, bitrate, duration, played_count, last_played_time, last_played_position, samplerate, channel, weather, timeline, sync_status, bitpersample FROM tag AS t \
				INNER JOIN tag_map AS tm \
				INNER JOIN media AS m \
				INNER JOIN (SELECT count(tag_id) as media_count, tag_id FROM tag_map group by tag_id) as cnt_tbl \
						 ON (t.tag_id=tm.tag_id AND tm.media_uuid = m.media_uuid AND cnt_tbl.tag_id=tm.tag_id AND m.validity=1) \
				UNION \
				SELECT \
					tag_id, name, 0, 0,  NULL, NULL, -1, -1, -1, -1, -1, NULL, NULL, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, -1, -1,  0, -1, -1, -1, -1, -1, -1, NULL, NULL, NULL, NULL, NULL, NULL,NULL, NULL, NULL, NULL, -1, -1, -1, -1, -1, -1, NULL, -1, NULL, -1, 0, 0 FROM tag \
				WHERE tag_id \
				NOT IN (select tag_id from tag_map); \
				");

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/* Create Trigger to remove media from tag_map when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS tag_map_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s WHERE media_uuid=old.media_uuid;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_TAG_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove media from tag_map when tag removed from tag table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS tag_map_cleanup_1 \
				DELETE ON %s BEGIN DELETE FROM %s WHERE tag_id=old.tag_id;END;",
				MEDIA_SVC_DB_TABLE_TAG, MEDIA_SVC_DB_TABLE_TAG_MAP);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}
	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_bookmark_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char * sql = NULL;

	media_svc_debug_func();

	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				bookmark_id		INTEGER PRIMARY KEY AUTOINCREMENT, \
				media_uuid		TEXT NOT NULL,\
				marked_time		INTEGER DEFAULT 0, \
				thumbnail_path	TEXT, \
				unique(media_uuid, marked_time) \
				);",
				MEDIA_SVC_DB_TABLE_BOOKMARK);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove media from tag_map when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS bookmark_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s WHERE media_uuid=old.media_uuid;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_BOOKMARK);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_create_custom_table(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char * sql = NULL;

	media_svc_debug_func();

	sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS %s (\
				_id				INTEGER PRIMARY KEY AUTOINCREMENT, \
				media_uuid		TEXT, \
				media_type		INTEGER,\
				author			TEXT, \
				provider			TEXT, \
				content_name	TEXT, \
				category			TEXT, \
				location_tag		TEXT, \
				age_rating		TEXT \
				);",
				MEDIA_SVC_DB_TABLE_CUSTOM);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Trigger to remove media from tag_map when media remove from media_table*/
	sql = sqlite3_mprintf("CREATE TRIGGER IF NOT EXISTS custom_cleanup \
				DELETE ON %s BEGIN DELETE FROM %s WHERE media_uuid=old.media_uuid;END;",
				MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_CUSTOM);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create trigger (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	/* Create Index*/
	sql = sqlite3_mprintf("CREATE INDEX IF NOT EXISTS custom_provider_idx on %s (provider); \
						",
						MEDIA_SVC_DB_TABLE_CUSTOM);

	media_svc_retv_if(sql == NULL, MS_MEDIA_ERR_OUT_OF_MEMORY);

	ret = _media_svc_sql_query(db_handle, sql, uid);
	sqlite3_free(sql);
	if (ret != SQLITE_OK) {
		media_svc_error("It failed to create db table (%d)", ret);
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_request_update_db(const char *sql_str, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	ret = media_db_request_update_db(sql_str, uid);

	return ret;
}

int _media_svc_sql_query(sqlite3 *db_handle, const char *sql_str, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_debug("[SQL query] : %s", sql_str);

	//DB will be updated by Media Server.
	ret = _media_svc_request_update_db(sql_str, uid);

	return ret;
}

int _media_svc_sql_prepare_to_step(sqlite3 *handle, const char *sql_str, sqlite3_stmt** stmt)
{
	int err = -1;

	media_svc_debug("[SQL query] : %s", sql_str);

	err = sqlite3_prepare_v2(handle, sql_str, -1, stmt, NULL);
	sqlite3_free((char *)sql_str);

	if (err != SQLITE_OK) {
		media_svc_error ("prepare error %d[%s]", err, sqlite3_errmsg(handle));
		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	err = sqlite3_step(*stmt);
	if (err != SQLITE_ROW) {
		media_svc_error("Item not found. end of row [%s]", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(*stmt);
		return MS_MEDIA_ERR_DB_NO_RECORD;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_sql_begin_trans(sqlite3 *handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_error("========_media_svc_sql_begin_trans");

	ret = media_db_request_update_db_batch_start("BEGIN IMMEDIATE;", uid);

	return ret;
}

int _media_svc_sql_end_trans(sqlite3 *handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_error("========_media_svc_sql_end_trans");

	ret = media_db_request_update_db_batch_end("COMMIT;", uid);

	return ret;
}

int _media_svc_sql_rollback_trans(sqlite3 *handle, uid_t uid)
{
	media_svc_error("========_media_svc_sql_rollback_trans");

	return _media_svc_request_update_db("ROLLBACK;", uid);

}

int _media_svc_sql_query_list(sqlite3 *handle, GList **query_list, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	int idx = 0;
	int length = g_list_length(*query_list);
	char *sql = NULL;

	media_svc_debug("query list length : [%d]", length);

	for (idx = 0; idx < length; idx++) {
		sql = (char*)g_list_nth_data(*query_list, idx);
		if(sql != NULL) {
			//ret = _media_svc_sql_query(handle, sql);
			ret = media_db_request_update_db_batch(sql, uid);
			if (ret != MS_MEDIA_ERR_NONE) {
				media_svc_error("media_db_request_update_db_batch failed : %d", ret);
			}
			sqlite3_free(sql);
			sql = NULL;
		}
	}

	_media_svc_sql_query_release(query_list);

	return MS_MEDIA_ERR_NONE;

}

void _media_svc_sql_query_add(GList **query_list, char **query)
{
	*query_list = g_list_append( *query_list, *query);
}

void _media_svc_sql_query_release(GList **query_list)
{
	if (*query_list) {
		media_svc_debug("_svc_sql_query_release");
		g_list_free(*query_list);
		*query_list = NULL;
	}
}
