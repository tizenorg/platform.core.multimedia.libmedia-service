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
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <media-util-err.h>
#include "media-svc-media.h"
#include "media-svc-media-folder.h"
#include "media-svc-debug.h"
#include "media-svc-util.h"
#include "media-svc-db-utils.h"
#include "media-svc-noti.h"

typedef struct {
	char thumbnail_path[MEDIA_SVC_PATHNAME_SIZE];
} media_svc_thumbnailpath_s;

static __thread GList *g_media_svc_item_validity_query_list = NULL;
static __thread GList *g_media_svc_insert_item_query_list = NULL;
__thread GList *g_media_svc_move_item_query_list = NULL;
static __thread GList *g_media_svc_update_item_query_list = NULL;

static int __media_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, int *count);
static int __media_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, int count, media_svc_thumbnailpath_s *thumb_path);
static int __media_svc_count_invalid_folder_records_with_thumbnail(sqlite3 *handle, const char *storage_id, const char *folder_path, const char *folder_uuid, bool is_recursive, int *count);
static int __media_svc_get_invalid_folder_records_with_thumbnail(sqlite3 *handle, const char *storage_id, const char *folder_path, const char *folder_uuid, bool is_recursive, int count, media_svc_thumbnailpath_s *thumb_path);

static int __media_svc_count_invalid_records_with_thumbnail(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE validity=0 AND storage_type=%d AND thumbnail_path IS NOT NULL",
					storage_id, storage_type);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when __media_svc_count_invalid_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;

}

static int __media_svc_get_invalid_records_with_thumbnail(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type,
							int count, media_svc_thumbnailpath_s * thumb_path)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	int idx = 0;

	char *sql = sqlite3_mprintf("SELECT thumbnail_path from (select thumbnail_path, validity from '%s' WHERE storage_type=%d AND thumbnail_path IS NOT NULL GROUP BY thumbnail_path HAVING count() = 1) WHERE validity=0",
					MEDIA_SVC_DB_TABLE_MEDIA, storage_type);

	media_svc_debug("[SQL query] : %s", sql);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when __media_svc_get_invalid_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(thumb_path[idx].thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), sizeof(thumb_path[idx]));
		/*media_svc_debug("thumb_path[%d]=[%s]", idx, thumb_path[idx].thumbnail_path); */
		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

static int __media_svc_count_invalid_folder_records_with_thumbnail(sqlite3 *handle, const char *storage_id, const char *folder_path, const char *folder_uuid, bool is_recursive, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = NULL;

	if (is_recursive)
		sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE validity=0 AND path LIKE '%q/%%' AND thumbnail_path IS NOT NULL", storage_id, folder_path);
	else
		sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE validity=0 AND folder_uuid = '%q' AND thumbnail_path IS NOT NULL", storage_id, folder_uuid);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when __media_svc_count_invalid_folder_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;

}

static int __media_svc_get_invalid_folder_records_with_thumbnail(sqlite3 *handle, const char *storage_id, const char *folder_path, const char *folder_uuid, bool is_recursive,
							int count, media_svc_thumbnailpath_s * thumb_path)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	int idx = 0;
	char *sql = NULL;

	if (is_recursive)
		sql = sqlite3_mprintf("SELECT thumbnail_path from (select thumbnail_path, validity from '%s' WHERE path LIKE '%q/%%' AND thumbnail_path IS NOT NULL GROUP BY thumbnail_path HAVING count() = 1) WHERE validity=0", storage_id, folder_path);
	else
		sql = sqlite3_mprintf("SELECT thumbnail_path from (select thumbnail_path, validity from '%s' WHERE folder_uuid = '%q' AND thumbnail_path IS NOT NULL GROUP BY thumbnail_path HAVING count() = 1) WHERE validity=0", storage_id, folder_uuid);

	media_svc_debug("[SQL query] : %s", sql);

	ret = _media_svc_sql_prepare_to_step_simple(handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when __media_svc_get_invalid_folder_records_with_thumbnail. err = [%d]", ret);
		return ret;
	}

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		_strncpy_safe(thumb_path[idx].thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), sizeof(thumb_path[idx]));
		idx++;
	}

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_count_record_with_path(sqlite3 *handle, const char *storage_id, const char *path, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE path='%q'", storage_id, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

char *_media_svc_get_thumb_default_path(uid_t uid)
{
	char *result_passwd = NULL;
	struct group *grpinfo = NULL;
	if (uid == getuid()) {
		grpinfo = getgrnam("users");
		if (grpinfo == NULL) {
			media_svc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
		result_passwd = g_strdup(MEDIA_SVC_THUMB_DEFAULT_PATH);
	} else {
		char passwd_str[MEDIA_SVC_PATHNAME_SIZE] = {0, };
		struct passwd *userinfo = getpwuid(uid);
		if (userinfo == NULL) {
			media_svc_error("getpwuid(%d) returns NULL !", uid);
			return NULL;
		}
		grpinfo = getgrnam("users");
		if (grpinfo == NULL) {
			media_svc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
		/* Compare git_t type and not group name */
		if (grpinfo->gr_gid != userinfo->pw_gid) {
			media_svc_error("UID [%d] does not belong to 'users' group!", uid);
			return NULL;
		}
		sprintf(passwd_str, "%s/share/media/.thumb/thumb_default.png", userinfo->pw_dir);
		result_passwd = g_strdup(passwd_str);
	}

	return result_passwd;
}

int _media_svc_insert_item_with_data(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *burst_id = NULL;
	int ini_val = _media_svc_get_ini_value();

	const char *db_fields = "media_uuid, path, file_name, media_type, mime_type, size, added_time, modified_time, folder_uuid, \
					thumbnail_path, title, album_id, album, artist, album_artist, genre, composer, year, recorded_date, copyright, track_num, description, \
					category, keyword, location_tag, content_name, age_rating, author, provider, last_played_time, played_count, favourite, \
					bitrate, bitpersample, samplerate, channel, duration, longitude, latitude, altitude, exposure_time, fnumber, iso, model, width, height, datetaken, orientation, \
					rating, is_drm, storage_type, burst_id, timeline, weather, sync_status, \
					file_name_pinyin, title_pinyin, album_pinyin, artist_pinyin, album_artist_pinyin, genre_pinyin, composer_pinyin, copyright_pinyin, description_pinyin, storage_uuid";

	/* This sql is due to sqlite3_mprintf's wrong operation when using floating point in the text format */
	/* This code will be removed when sqlite3_mprintf works clearly */
	char *test_sql = sqlite3_mprintf("%f, %f, %f", content_info->media_meta.longitude, content_info->media_meta.latitude, content_info->media_meta.altitude);
	sqlite3_free(test_sql);

	if (is_burst) {
		int burst_id_int = 0;
		ret = _media_svc_get_burst_id(handle, storage_id, &burst_id_int);
		if (ret != MS_MEDIA_ERR_NONE)
			burst_id = NULL;

		if (burst_id_int > 0) {
			media_svc_debug("Burst id : %d", burst_id_int);
			burst_id = sqlite3_mprintf("%d", burst_id_int);
		}

		/* Get thumbnail for burst shot */
		if (ini_val == 1) {
			char thumb_path[MEDIA_SVC_PATHNAME_SIZE + 1] = {0, };
			int width = 0;
			int height = 0;

			ret = _media_svc_request_thumbnail_with_origin_size(content_info->path, thumb_path, sizeof(thumb_path), &width, &height, uid);
			if (ret == MS_MEDIA_ERR_NONE) {
				ret = __media_svc_malloc_and_strncpy(&(content_info->thumbnail_path), thumb_path);
				if (ret != MS_MEDIA_ERR_NONE)
					content_info->thumbnail_path = NULL;
			}

			if (content_info->media_meta.width <= 0)
				content_info->media_meta.width = width;

			if (content_info->media_meta.height <= 0)
				content_info->media_meta.height = height;
		}
	}

	/*Update Pinyin If Support Pinyin*/
	if (_media_svc_check_pinyin_support()) {
		if (STRING_VALID(content_info->file_name))
			_media_svc_get_pinyin_str(content_info->file_name, &content_info->file_name_pinyin);
		if (STRING_VALID(content_info->media_meta.title))
			_media_svc_get_pinyin_str(content_info->media_meta.title, &content_info->media_meta.title_pinyin);
		if (STRING_VALID(content_info->media_meta.album))
			_media_svc_get_pinyin_str(content_info->media_meta.album, &content_info->media_meta.album_pinyin);
		if (STRING_VALID(content_info->media_meta.artist))
			_media_svc_get_pinyin_str(content_info->media_meta.artist, &content_info->media_meta.artist_pinyin);
		if (STRING_VALID(content_info->media_meta.album_artist))
			_media_svc_get_pinyin_str(content_info->media_meta.album_artist, &content_info->media_meta.album_artist_pinyin);
		if (STRING_VALID(content_info->media_meta.genre))
			_media_svc_get_pinyin_str(content_info->media_meta.genre, &content_info->media_meta.genre_pinyin);
		if (STRING_VALID(content_info->media_meta.composer))
			_media_svc_get_pinyin_str(content_info->media_meta.composer, &content_info->media_meta.composer_pinyin);
		if (STRING_VALID(content_info->media_meta.copyright))
			_media_svc_get_pinyin_str(content_info->media_meta.copyright, &content_info->media_meta.copyright_pinyin);
		if (STRING_VALID(content_info->media_meta.description))
			_media_svc_get_pinyin_str(content_info->media_meta.description, &content_info->media_meta.description_pinyin);
	}

	char *sql = sqlite3_mprintf("INSERT INTO '%s' (%s) VALUES (%Q, %Q, %Q, %d, %Q, %lld, %d, %d, %Q, \
													%Q, %Q, %d, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, \
													%Q, %Q, %Q, %Q, %Q, %Q, %Q, %d, %d, %d, \
													%d, %d, %d, %d, %d, %.6f, %.6f, %.6f, %Q, %.6f, %d, %Q, %d, %d, %Q, %d, \
													%d, %d, %d, %Q, %d, %Q, %d, \
													%Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q, %Q);",
								content_info->storage_uuid, db_fields,
								content_info->media_uuid,
								content_info->path,
								content_info->file_name,
								content_info->media_type,
								content_info->mime_type,
								content_info->size,
								content_info->added_time,
								content_info->modified_time,
								content_info->folder_uuid,		/**/
								content_info->thumbnail_path,
								content_info->media_meta.title,
								content_info->album_id,
								content_info->media_meta.album,
								content_info->media_meta.artist,
								content_info->media_meta.album_artist,
								content_info->media_meta.genre,
								content_info->media_meta.composer,
								content_info->media_meta.year,
								content_info->media_meta.recorded_date,
								content_info->media_meta.copyright,
								content_info->media_meta.track_num,
								content_info->media_meta.description,	/**/
								content_info->media_meta.category,
								content_info->media_meta.keyword,
								content_info->media_meta.location_tag,
								content_info->media_meta.content_name,
								content_info->media_meta.age_rating,
								content_info->media_meta.author,
								content_info->media_meta.provider,
								content_info->last_played_time,
								content_info->played_count,
								content_info->favourate,	/**/
								content_info->media_meta.bitrate,
								content_info->media_meta.bitpersample,
								content_info->media_meta.samplerate,
								content_info->media_meta.channel,
								content_info->media_meta.duration,
								content_info->media_meta.longitude,
								content_info->media_meta.latitude,
								content_info->media_meta.altitude,
								content_info->media_meta.exposure_time,
								content_info->media_meta.fnumber,
								content_info->media_meta.iso,
								content_info->media_meta.model,
								content_info->media_meta.width,
								content_info->media_meta.height,
								content_info->media_meta.datetaken,
								content_info->media_meta.orientation,
								content_info->media_meta.rating,
								content_info->is_drm,
								content_info->storage_type,
								burst_id,
								content_info->timeline,
								content_info->media_meta.weather,
								content_info->sync_status,
								content_info->file_name_pinyin,
								content_info->media_meta.title_pinyin,
								content_info->media_meta.album_pinyin,
								content_info->media_meta.artist_pinyin,
								content_info->media_meta.album_artist_pinyin,
								content_info->media_meta.genre_pinyin,
								content_info->media_meta.composer_pinyin,
								content_info->media_meta.copyright_pinyin,
								content_info->media_meta.description_pinyin,
								content_info->storage_uuid
				);

	if (burst_id) {
		sqlite3_free(burst_id);
		burst_id = NULL;
	}

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("failed to insert item");
			return ret;
		}
	} else {
		media_svc_debug("query : %s", sql);
		_media_svc_sql_query_add(&g_media_svc_insert_item_query_list, &sql);
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_update_meta_with_data(sqlite3 *handle, media_svc_content_info_s *content_info)
{
	int ret = MS_MEDIA_ERR_NONE;

	/* This sql is due to sqlite3_mprintf's wrong operation when using floating point in the text format */
	/* This code will be removed when sqlite3_mprintf works clearly */
	char *test_sql = sqlite3_mprintf("%f, %f, %f", content_info->media_meta.longitude, content_info->media_meta.latitude, content_info->media_meta.altitude);
	sqlite3_free(test_sql);

	/*Update Pinyin If Support Pinyin*/
	if (_media_svc_check_pinyin_support()) {
		if (STRING_VALID(content_info->file_name))
			_media_svc_get_pinyin_str(content_info->file_name, &content_info->file_name_pinyin);
		if (STRING_VALID(content_info->media_meta.title))
			_media_svc_get_pinyin_str(content_info->media_meta.title, &content_info->media_meta.title_pinyin);
		if (STRING_VALID(content_info->media_meta.album))
			_media_svc_get_pinyin_str(content_info->media_meta.album, &content_info->media_meta.album_pinyin);
		if (STRING_VALID(content_info->media_meta.artist))
			_media_svc_get_pinyin_str(content_info->media_meta.artist, &content_info->media_meta.artist_pinyin);
		if (STRING_VALID(content_info->media_meta.album_artist))
			_media_svc_get_pinyin_str(content_info->media_meta.album_artist, &content_info->media_meta.album_artist_pinyin);
		if (STRING_VALID(content_info->media_meta.genre))
			_media_svc_get_pinyin_str(content_info->media_meta.genre, &content_info->media_meta.genre_pinyin);
		if (STRING_VALID(content_info->media_meta.composer))
			_media_svc_get_pinyin_str(content_info->media_meta.composer, &content_info->media_meta.composer_pinyin);
		if (STRING_VALID(content_info->media_meta.copyright))
			_media_svc_get_pinyin_str(content_info->media_meta.copyright, &content_info->media_meta.copyright_pinyin);
		if (STRING_VALID(content_info->media_meta.description))
			_media_svc_get_pinyin_str(content_info->media_meta.description, &content_info->media_meta.description_pinyin);
	}

	char *sql = sqlite3_mprintf("UPDATE %s SET title=%Q, album=%Q, artist=%Q, album_artist=%Q, genre=%Q, composer=%Q, copyright=%Q, description=%Q, \
		file_name_pinyin=%Q, title_pinyin=%Q, album_pinyin=%Q, artist_pinyin=%Q, album_artist_pinyin=%Q, genre_pinyin=%Q, composer_pinyin=%Q, copyright_pinyin=%Q, description_pinyin=%Q \
		WHERE path=%Q",
								MEDIA_SVC_DB_TABLE_MEDIA,
								content_info->media_meta.title,
								content_info->media_meta.album,
								content_info->media_meta.artist,
								content_info->media_meta.album_artist,
								content_info->media_meta.genre,
								content_info->media_meta.composer,
								content_info->media_meta.copyright,
								content_info->media_meta.description,
								content_info->file_name_pinyin,
								content_info->media_meta.title_pinyin,
								content_info->media_meta.album_pinyin,
								content_info->media_meta.artist_pinyin,
								content_info->media_meta.album_artist_pinyin,
								content_info->media_meta.genre_pinyin,
								content_info->media_meta.composer_pinyin,
								content_info->media_meta.copyright_pinyin,
								content_info->media_meta.description_pinyin,
								content_info->path
				);

	if (sql != NULL) {
		media_svc_debug("query : %s", sql);
		_media_svc_sql_query_add(&g_media_svc_update_item_query_list, &sql);
	} else {
		media_svc_error("sqlite3_mprintf failed");
		ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
	}

	return ret;
}

int _media_svc_update_item_with_data(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	/* This sql is due to sqlite3_mprintf's wrong operation when using floating point in the text format */
	/* This code will be removed when sqlite3_mprintf works clearly */
	char *test_sql = sqlite3_mprintf("%f, %f, %f", content_info->media_meta.longitude, content_info->media_meta.latitude, content_info->media_meta.altitude);
	sqlite3_free(test_sql);

	/*Update Pinyin If Support Pinyin*/
	if (_media_svc_check_pinyin_support()) {
		if (STRING_VALID(content_info->file_name))
			_media_svc_get_pinyin_str(content_info->file_name, &content_info->file_name_pinyin);
		if (STRING_VALID(content_info->media_meta.title))
			_media_svc_get_pinyin_str(content_info->media_meta.title, &content_info->media_meta.title_pinyin);
		if (STRING_VALID(content_info->media_meta.album))
			_media_svc_get_pinyin_str(content_info->media_meta.album, &content_info->media_meta.album_pinyin);
		if (STRING_VALID(content_info->media_meta.artist))
			_media_svc_get_pinyin_str(content_info->media_meta.artist, &content_info->media_meta.artist_pinyin);
		if (STRING_VALID(content_info->media_meta.album_artist))
			_media_svc_get_pinyin_str(content_info->media_meta.album_artist, &content_info->media_meta.album_artist_pinyin);
		if (STRING_VALID(content_info->media_meta.genre))
			_media_svc_get_pinyin_str(content_info->media_meta.genre, &content_info->media_meta.genre_pinyin);
		if (STRING_VALID(content_info->media_meta.composer))
			_media_svc_get_pinyin_str(content_info->media_meta.composer, &content_info->media_meta.composer_pinyin);
		if (STRING_VALID(content_info->media_meta.copyright))
			_media_svc_get_pinyin_str(content_info->media_meta.copyright, &content_info->media_meta.copyright_pinyin);
		if (STRING_VALID(content_info->media_meta.description))
			_media_svc_get_pinyin_str(content_info->media_meta.description, &content_info->media_meta.description_pinyin);
	}

	char *sql = sqlite3_mprintf("UPDATE '%s' SET \
		size=%lld, modified_time=%d, thumbnail_path=%Q, title=%Q, album_id=%d, album=%Q, artist=%Q, album_artist=%Q, genre=%Q, \
		composer=%Q, year=%Q, recorded_date=%Q, copyright=%Q, track_num=%Q, description=%Q, \
		bitrate=%d, bitpersample=%d, samplerate=%d, channel=%d, duration=%d, longitude=%f, latitude=%f, altitude=%f, exposure_time=%Q, fnumber=%f, iso=%d, model=%Q, width=%d, height=%d, datetaken=%Q, \
													orientation=%d WHERE path=%Q",
								storage_id,
								content_info->size,
								content_info->modified_time,
								content_info->thumbnail_path,
								content_info->media_meta.title,
								content_info->album_id,
								content_info->media_meta.album,
								content_info->media_meta.artist,
								content_info->media_meta.album_artist,
								content_info->media_meta.genre,
								content_info->media_meta.composer,
								content_info->media_meta.year,
								content_info->media_meta.recorded_date,
								content_info->media_meta.copyright,
								content_info->media_meta.track_num,
								content_info->media_meta.description,
								content_info->media_meta.bitrate,
								content_info->media_meta.bitpersample,
								content_info->media_meta.samplerate,
								content_info->media_meta.channel,
								content_info->media_meta.duration,
								content_info->media_meta.longitude,
								content_info->media_meta.latitude,
								content_info->media_meta.altitude,
								content_info->media_meta.exposure_time,
								content_info->media_meta.fnumber,
								content_info->media_meta.iso,
								content_info->media_meta.model,
								content_info->media_meta.width,
								content_info->media_meta.height,
								content_info->media_meta.datetaken,
								content_info->media_meta.orientation,
								content_info->path
				);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}
int _media_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *storage_id, const char *path, char *thumbnail_path)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT thumbnail_path FROM '%s' WHERE path='%q'", storage_id, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		if (ret == MS_MEDIA_ERR_DB_NO_RECORD)
			media_svc_debug("there is no thumbnail.");
		else
			media_svc_error("error when _media_svc_get_thumbnail_path_by_path. err = [%d]", ret);

		return ret;
	}

	_strncpy_safe(thumbnail_path, (const char *)sqlite3_column_text(sql_stmt, 0), MEDIA_SVC_PATHNAME_SIZE);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_media_type_by_path(sqlite3 *handle, const char *storage_id, const char *path, int *media_type)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT media_type FROM '%s' WHERE path='%q'", storage_id, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_get_media_type_by_path. err = [%d]", ret);
		return ret;
	}

	*media_type = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_delete_item_by_path(sqlite3 *handle, const char *storage_id, const char *path, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = sqlite3_mprintf("DELETE FROM '%s' WHERE path='%q'", storage_id, path);

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("failed to delete item");
			return ret;
		}
	} else {
		media_svc_debug("query : %s", sql);
		_media_svc_sql_query_add(&g_media_svc_insert_item_query_list, &sql);
	}

	return ret;
}

int _media_svc_truncate_table(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	char *sql = sqlite3_mprintf("DELETE FROM '%s' where storage_type=%d", storage_id, storage_type);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_delete_invalid_items(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, uid_t uid)
{
	int idx = 0;
	media_svc_thumbnailpath_s *thumbpath_record = NULL;
	int invalid_count = 0;
	int ret = MS_MEDIA_ERR_NONE;

	ret = __media_svc_count_invalid_records_with_thumbnail(handle, storage_id, storage_type, &invalid_count);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	media_svc_debug("invalid count: %d", invalid_count);

	if (invalid_count > 0) {
		thumbpath_record = (media_svc_thumbnailpath_s *)calloc(invalid_count, sizeof(media_svc_thumbnailpath_s));
		if (thumbpath_record == NULL) {
			media_svc_error("fail to memory allocation");
			return MS_MEDIA_ERR_OUT_OF_MEMORY;
		}

		ret = __media_svc_get_invalid_records_with_thumbnail(handle, storage_id, storage_type, invalid_count, thumbpath_record);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("error when get thumbnail record");
			SAFE_FREE(thumbpath_record);
			return ret;
		}
	} else {
		media_svc_debug("There is no item with thumbnail");
	}

	char *sql = sqlite3_mprintf("DELETE FROM '%s' WHERE validity = 0", storage_id);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);
	if (ret != MS_MEDIA_ERR_NONE) {
		SAFE_FREE(thumbpath_record);
		return ret;
	}

	/*Delete thumbnails*/
	char *default_thumbnail_path = _media_svc_get_thumb_default_path(uid);
	for (idx = 0; idx < invalid_count; idx++) {
		if ((strlen(thumbpath_record[idx].thumbnail_path) > 0) && (STRING_VALID(default_thumbnail_path)) && (strncmp(thumbpath_record[idx].thumbnail_path, default_thumbnail_path, strlen(default_thumbnail_path)) != 0)) {
			ret = _media_svc_remove_file(thumbpath_record[idx].thumbnail_path);
			if (ret != MS_MEDIA_ERR_NONE)
				media_svc_error("fail to remove thumbnail file.");
		}
	}

	SAFE_FREE(thumbpath_record);
	SAFE_FREE(default_thumbnail_path);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_delete_invalid_folder_items(sqlite3 *handle, const char *storage_id, const char *folder_path, bool is_recursive, uid_t uid)
{
	int idx = 0;
	media_svc_thumbnailpath_s *thumbpath_record = NULL;
	int invalid_count = 0;
	int ret = MS_MEDIA_ERR_NONE;
	char folder_uuid[MEDIA_SVC_UUID_SIZE+1] = {0,};
	char *sql = NULL;

	/*get folder uuid from DB*/
	ret = _media_svc_get_folder_id_by_foldername(handle, storage_id, folder_path, folder_uuid, uid);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	ret = __media_svc_count_invalid_folder_records_with_thumbnail(handle, storage_id, folder_path, folder_uuid, is_recursive, &invalid_count);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	media_svc_debug("invalid count: %d", invalid_count);

	if (invalid_count > 0) {
		thumbpath_record = (media_svc_thumbnailpath_s *)calloc(invalid_count, sizeof(media_svc_thumbnailpath_s));
		if (thumbpath_record == NULL) {
			media_svc_error("fail to memory allocation");
			return MS_MEDIA_ERR_OUT_OF_MEMORY;
		}

		ret = __media_svc_get_invalid_folder_records_with_thumbnail(handle, storage_id, folder_path, folder_uuid, is_recursive, invalid_count, thumbpath_record);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("error when get thumbnail record");
			SAFE_FREE(thumbpath_record);
			return ret;
		}
	} else {
		media_svc_debug("There is no item with thumbnail");
	}

	if (is_recursive)
		sql = sqlite3_mprintf("DELETE FROM '%s' WHERE validity = 0 AND path LIKE '%q/%%'", storage_id, folder_path);
	else
		sql = sqlite3_mprintf("DELETE FROM '%s' WHERE validity = 0 AND folder_uuid='%q'", storage_id, folder_uuid);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);
	if (ret != MS_MEDIA_ERR_NONE) {
		SAFE_FREE(thumbpath_record);
		return ret;
	}

	/*Delete thumbnails*/
	if (thumbpath_record != NULL)	 {
		char *default_thumbnail_path = _media_svc_get_thumb_default_path(uid);
		if (STRING_VALID(default_thumbnail_path)) {
			for (idx = 0; idx < invalid_count; idx++) {
				if ((strlen(thumbpath_record[idx].thumbnail_path) > 0) && (strncmp(thumbpath_record[idx].thumbnail_path, default_thumbnail_path, strlen(default_thumbnail_path)) != 0)) {
					ret = _media_svc_remove_file(thumbpath_record[idx].thumbnail_path);
					if (ret != MS_MEDIA_ERR_NONE)
						media_svc_error("fail to remove thumbnail file.");
				}
			}
		}

		SAFE_FREE(thumbpath_record);
		SAFE_FREE(default_thumbnail_path);
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_update_item_validity(sqlite3 *handle, const char *storage_id, const char *path, int validity, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	char *sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE path= '%q'", storage_id, validity, path);

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
	} else {
		_media_svc_sql_query_add(&g_media_svc_item_validity_query_list, &sql);
	}

	return ret;
}

int _media_svc_update_thumbnail_path(sqlite3 *handle, const char *storage_id, const char *path, const char *thumb_path, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	char *sql = sqlite3_mprintf("UPDATE '%s' SET thumbnail_path=%Q WHERE path= %Q", storage_id, thumb_path, path);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_storage_item_validity(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, int validity, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	char *sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE storage_type=%d", storage_id, validity, storage_type);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_folder_item_validity(sqlite3 *handle, const char *storage_id, const char *folder_path, int validity, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;
	char folder_uuid[MEDIA_SVC_UUID_SIZE + 1] = {0, };
	sqlite3_stmt *sql_stmt = NULL;

	/*Get folder ID*/
	sql = sqlite3_mprintf("SELECT folder_uuid FROM '%s' WHERE path='%q' AND storage_uuid='%q'", MEDIA_SVC_DB_TABLE_FOLDER, folder_path, storage_id);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		if (ret == MS_MEDIA_ERR_DB_NO_RECORD)
			media_svc_debug("folder not exist");
		else
			media_svc_error("error when get folder_id. err = [%d]", ret);

		return ret;
	}

	_strncpy_safe(folder_uuid, (const char *)sqlite3_column_text(sql_stmt, 0), MEDIA_SVC_UUID_SIZE + 1);
	SQLITE3_FINALIZE(sql_stmt);

	/*Update folder item validity*/
	sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE folder_uuid='%q'", storage_id, validity, folder_uuid);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_recursive_folder_item_validity(sqlite3 *handle, const char *storage_id, const char *folder_path, int validity, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	/*Update folder item validity*/
	char *sql = sqlite3_mprintf("UPDATE '%s' SET validity=%d WHERE (storage_type = 0 OR storage_type = 1) AND path LIKE '%q/%%'", storage_id, validity, folder_path);

	ret = _media_svc_sql_query(handle, sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_item_by_path(sqlite3 *handle, const char *storage_id, const char *src_path, media_svc_storage_type_e dest_storage, const char *dest_path,
				const char *file_name, int modified_time, const char *folder_uuid, const char *thumb_path, bool stack_query, uid_t uid)
{
	/* update path, filename, modified_time, folder_uuid, thumbnail_path, */
	/* played_count, last_played_time, last_played_position, favourite, storaget_type*/

	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	if (thumb_path != NULL) {
		sql = sqlite3_mprintf("UPDATE '%s' SET \
					path=%Q, file_name=%Q, modified_time=%d, folder_uuid=%Q, thumbnail_path=%Q, storage_type=%d, \
					played_count=0, last_played_time=0, last_played_position=0 \
					WHERE path=%Q",
					storage_id, dest_path, file_name, modified_time, folder_uuid, thumb_path, dest_storage, src_path);
	} else {
		sql = sqlite3_mprintf("UPDATE '%s' SET \
					path=%Q, file_name=%Q, modified_time=%d, folder_uuid=%Q, storage_type=%d, \
					played_count=0, last_played_time=0, last_played_position=0 \
					WHERE path=%Q",
					storage_id, dest_path, file_name, modified_time, folder_uuid, dest_storage, src_path);
	}

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
	} else {
		_media_svc_sql_query_add(&g_media_svc_move_item_query_list, &sql);
	}

	return ret;
}

int _media_svc_list_query_do(sqlite3 *handle, media_svc_query_type_e query_type, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	ret = _media_svc_sql_begin_trans(handle, uid);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	if (query_type == MEDIA_SVC_QUERY_SET_ITEM_VALIDITY)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_item_validity_query_list, uid);
	else if (query_type == MEDIA_SVC_QUERY_MOVE_ITEM)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_move_item_query_list, uid);
	else if (query_type == MEDIA_SVC_QUERY_INSERT_ITEM)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_insert_item_query_list, uid);
	else if (query_type == MEDIA_SVC_QUERY_UPDATE_ITEM)
		ret = _media_svc_sql_query_list(handle, &g_media_svc_update_item_query_list, uid);
	else if (query_type == MEDIA_SVC_QUERY_INSERT_FOLDER)
		ret = _media_svc_sql_query_list(handle, _media_svc_get_folder_list_ptr(), uid);
	else
		ret = MS_MEDIA_ERR_INVALID_PARAMETER;

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_list_query_do failed. start rollback");
		_media_svc_sql_rollback_trans(handle, uid);
		return ret;
	}

	ret = _media_svc_sql_end_trans(handle, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("mb_svc_sqlite3_commit_trans failed.. Now start to rollback");
		_media_svc_sql_rollback_trans(handle, uid);
		return ret;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_burst_id(sqlite3 *handle, const char *storage_id, int *id)
{
	int ret = MS_MEDIA_ERR_NONE;
	int cur_id = -1;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT max(CAST(burst_id AS INTEGER)) FROM '%s'", storage_id);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_get_burst_id. err = [%d]", ret);
		return ret;
	}

	cur_id = sqlite3_column_int(sql_stmt, 0);
	*id = ++cur_id;
	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_noti_info(sqlite3 *handle, const char *storage_id, const char *path, int update_item, media_svc_noti_item **item)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = NULL;
	int is_root_dir = FALSE;

	if (item == NULL) {
		media_svc_error("invalid parameter");
		return MS_MEDIA_ERR_INVALID_PARAMETER;
	}

	if (update_item == MS_MEDIA_ITEM_FILE)
		sql = sqlite3_mprintf("SELECT media_uuid, media_type, mime_type FROM '%s' WHERE path=%Q", storage_id, path);
	else if (update_item == MS_MEDIA_ITEM_DIRECTORY)
		sql = sqlite3_mprintf("SELECT folder_uuid FROM '%s' WHERE path=%Q AND storage_uuid='%s'", MEDIA_SVC_DB_TABLE_FOLDER, path, storage_id);
	else {
		media_svc_error("_media_svc_get_noti_info failed : update item");
		return MS_MEDIA_ERR_INVALID_PARAMETER;
	}

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		if (ret == MS_MEDIA_ERR_DB_NO_RECORD && update_item == MS_MEDIA_ITEM_DIRECTORY) {
			media_svc_debug("This is root directory of media");
			sql_stmt = NULL;
			is_root_dir = TRUE;
		} else {
			media_svc_error("error when _media_svc_get_noti_info. err = [%d]", ret);
			return ret;
		}
	}

	*item = calloc(1, sizeof(media_svc_noti_item));
	if (*item == NULL) {
		media_svc_error("_media_svc_get_noti_info failed : calloc");
		return MS_MEDIA_ERR_OUT_OF_MEMORY;
	}

	if (update_item == MS_MEDIA_ITEM_FILE) {
		(*item)->media_uuid = g_strdup((const char *)sqlite3_column_text(sql_stmt, 0));
		(*item)->media_type = sqlite3_column_int(sql_stmt, 1);
		(*item)->mime_type = g_strdup((const char *)sqlite3_column_text(sql_stmt, 2));
	} else if (update_item == MS_MEDIA_ITEM_DIRECTORY) {
		if (is_root_dir)
			(*item)->media_uuid = NULL;
		else
			(*item)->media_uuid = g_strdup((const char *)sqlite3_column_text(sql_stmt, 0));
	}

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_count_invalid_folder_items(sqlite3 *handle, const char *storage_id, const char *folder_path, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;

	char *sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE validity=0 AND path LIKE '%q/%%'", storage_id, folder_path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_count_invalid_folder_items. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_thumbnail_count(sqlite3 *handle, const char *storage_id, const char *thumb_path, int *count)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT count(*) FROM '%s' WHERE thumbnail_path=%Q", storage_id, thumb_path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_get_thumbnail_count. err = [%d]", ret);
		return ret;
	}

	*count = sqlite3_column_int(sql_stmt, 0);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_get_fileinfo_by_path(sqlite3 *handle, const char *storage_id, const char *path, time_t *modified_time, unsigned long long *size)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("SELECT modified_time, size FROM '%s' WHERE path='%q'", storage_id, path);

	ret = _media_svc_sql_prepare_to_step(handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when _media_svc_get_fileinfo_by_path. err = [%d]", ret);
		return ret;
	}

	*modified_time = (int)sqlite3_column_int(sql_stmt, 0);
	*size = (unsigned long long)sqlite3_column_int64(sql_stmt, 1);

	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_insert_item_pass1(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *burst_id = NULL;

	media_svc_debug("START pass1");

	char * db_fields = (char *)"media_uuid, folder_uuid, path, file_name, media_type, mime_type, size, added_time, modified_time, is_drm, storage_type, timeline, burst_id, storage_uuid";
#if 0
	if (is_burst) {
		int burst_id_int = 0;
		ret = _media_svc_get_burst_id(handle, storage_id, &burst_id_int);
		if (ret != MS_MEDIA_ERR_NONE) {
			burst_id = NULL;
		}

		if (burst_id_int > 0) {
			media_svc_debug("Burst id : %d", burst_id_int);
			burst_id = sqlite3_mprintf("%d", burst_id_int);
		}

		/* Get thumbnail for burst shot */
		char thumb_path[MEDIA_SVC_PATHNAME_SIZE + 1] = {0, };
		int width = 0;
		int height = 0;

		ret = _media_svc_request_thumbnail_with_origin_size(content_info->path, thumb_path, sizeof(thumb_path), &width, &height);
		if (ret == MS_MEDIA_ERR_NONE) {
			ret = __media_svc_malloc_and_strncpy(&(content_info->thumbnail_path), thumb_path);
			if (ret != MS_MEDIA_ERR_NONE) {
				content_info->thumbnail_path = NULL;
			}
		}

		if (content_info->media_meta.width <= 0)
			content_info->media_meta.width = width;

		if (content_info->media_meta.height <= 0)
			content_info->media_meta.height = height;
	}

	/* Update Pinyin If Support Pinyin */
	if (_media_svc_check_pinyin_support()) {
		if (STRING_VALID(content_info->file_name))
			_media_svc_get_pinyin_str(content_info->file_name, &content_info->file_name_pinyin);
	}
#endif

	char *sql = sqlite3_mprintf("INSERT INTO '%s' (%s) VALUES (%Q, %Q, %Q, %Q, %d, %Q, %lld, \
								%d, %d, %d, %d, %d, %Q, %Q);",
		storage_id, db_fields,
		content_info->media_uuid,
		content_info->folder_uuid,
		content_info->path,
		content_info->file_name,
		content_info->media_type,
		content_info->mime_type,
		content_info->size,
		content_info->added_time,
		content_info->modified_time,
		content_info->is_drm,
		content_info->storage_type,
		content_info->timeline,
		burst_id,
		storage_id
		);
#if 0
	if (burst_id) {
		sqlite3_free(burst_id);
		burst_id = NULL;
	}
#endif
	media_svc_debug("MAKE PASS 1 QUERY END");

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
		if (ret != MS_MEDIA_ERR_NONE) {
			media_svc_error("failed to insert item");
			return ret;
		}
	} else {
		media_svc_debug("query : %s", sql);
		_media_svc_sql_query_add(&g_media_svc_insert_item_query_list, &sql);
	}
	media_svc_debug("END");
	return MS_MEDIA_ERR_NONE;
}

int _media_svc_insert_item_pass2(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_debug_fenter();

	/*Update Pinyin If Support Pinyin*/
	if (_media_svc_check_pinyin_support()) {
		if (STRING_VALID(content_info->file_name))
			_media_svc_get_pinyin_str(content_info->file_name, &content_info->file_name_pinyin);
		if (STRING_VALID(content_info->media_meta.title))
			_media_svc_get_pinyin_str(content_info->media_meta.title, &content_info->media_meta.title_pinyin);
		if (STRING_VALID(content_info->media_meta.album))
			_media_svc_get_pinyin_str(content_info->media_meta.album, &content_info->media_meta.album_pinyin);
		if (STRING_VALID(content_info->media_meta.artist))
			_media_svc_get_pinyin_str(content_info->media_meta.artist, &content_info->media_meta.artist_pinyin);
		if (STRING_VALID(content_info->media_meta.album_artist))
			_media_svc_get_pinyin_str(content_info->media_meta.album_artist, &content_info->media_meta.album_artist_pinyin);
		if (STRING_VALID(content_info->media_meta.genre))
			_media_svc_get_pinyin_str(content_info->media_meta.genre, &content_info->media_meta.genre_pinyin);
		if (STRING_VALID(content_info->media_meta.composer))
			_media_svc_get_pinyin_str(content_info->media_meta.composer, &content_info->media_meta.composer_pinyin);
		if (STRING_VALID(content_info->media_meta.copyright))
			_media_svc_get_pinyin_str(content_info->media_meta.copyright, &content_info->media_meta.copyright_pinyin);
		if (STRING_VALID(content_info->media_meta.description))
			_media_svc_get_pinyin_str(content_info->media_meta.description, &content_info->media_meta.description_pinyin);
	}

	/*modified month does not exist in Tizen 2.4*/
	char *sql = sqlite3_mprintf("UPDATE '%s' SET \
		thumbnail_path=%Q, title=%Q, album_id=%d, album=%Q, artist=%Q, album_artist=%Q, genre=%Q, composer=%Q, year=%Q, \
		recorded_date=%Q, copyright=%Q, track_num=%Q, description=%Q, bitrate=%d, bitpersample=%d, samplerate=%d, channel=%d, \
		duration=%d, longitude=%.6f, latitude=%.6f, altitude=%.6f, width=%d, height=%d, datetaken=%Q, orientation=%d, exposure_time=%Q,\
		fnumber=%.6f, iso=%d, model=%Q,	rating=%d, weather=%Q, file_name_pinyin=%Q, title_pinyin=%Q, album_pinyin=%Q, \
		artist_pinyin=%Q, album_artist_pinyin=%Q, genre_pinyin=%Q, composer_pinyin=%Q, copyright_pinyin=%Q, description_pinyin=%Q WHERE path=%Q",
		storage_id,
		//content_info->folder_uuid,
		content_info->thumbnail_path,		/**/
		content_info->media_meta.title,
		content_info->album_id,
		content_info->media_meta.album,
		content_info->media_meta.artist,
		content_info->media_meta.album_artist,
		content_info->media_meta.genre,
		content_info->media_meta.composer,
		content_info->media_meta.year,
		content_info->media_meta.recorded_date,
		content_info->media_meta.copyright,
		content_info->media_meta.track_num,
		content_info->media_meta.description,	/**/
		content_info->media_meta.bitrate,
		content_info->media_meta.bitpersample,
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
		content_info->media_meta.exposure_time,
		content_info->media_meta.fnumber,
		content_info->media_meta.iso,
		content_info->media_meta.model,
		content_info->media_meta.rating,
		content_info->media_meta.weather,
		content_info->file_name_pinyin,
		content_info->media_meta.title_pinyin,
		content_info->media_meta.album_pinyin,
		content_info->media_meta.artist_pinyin,
		content_info->media_meta.album_artist_pinyin,
		content_info->media_meta.genre_pinyin,
		content_info->media_meta.composer_pinyin,
		content_info->media_meta.copyright_pinyin,
		content_info->media_meta.description_pinyin,
		content_info->path
		);

	if (!stack_query) {
		ret = _media_svc_sql_query(handle, sql, uid);
		sqlite3_free(sql);
		if (ret != MS_MEDIA_ERR_NONE) {

			media_svc_error("failed to insert item");
			return ret;
		}
	} else {
		/*media_svc_debug("query : %s", sql);*/
		_media_svc_sql_query_add(&g_media_svc_update_item_query_list, &sql);
	}

	media_svc_debug_fleave();

	return MS_MEDIA_ERR_NONE;
}
