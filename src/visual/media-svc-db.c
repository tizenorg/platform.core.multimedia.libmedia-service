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
#include <util-func.h>
#include <sys/stat.h>
#include "media-svc-env.h"
#include "media-svc-util.h"
#include "media-svc-db.h"
#include "visual-svc-debug.h"
#include "media-svc-db-util.h"
#include "visual-svc-util.h"
#include "media-svc-api.h"
#include "media-svc-structures.h"
#include "visual-svc-types.h"
#include "visual-svc-error.h"

/**
 * Enumerations for table name.
 */
typedef enum {
	MB_SVC_TABLE_NONE = -1,
	MB_SVC_TABLE_BOOKMARK,
	MB_SVC_TABLE_FOLDER,
	MB_SVC_TABLE_WEB_STREAMING,
	MB_SVC_TABLE_MEDIA,
	MB_SVC_TABLE_VIDEO_META,
	MB_SVC_TABLE_IMAGE_META,
	MB_SVC_TABLE_TAG_MAP,
	MB_SVC_TABLE_TAG,
	MB_SVC_TABLE_NUM,
} mb_svc_tbl_name_e;


/**
 * Enumerations for bookmark field name.
 */
typedef enum {
	MB_SVC_BOOKMARK_NONE		= -1,
	MB_SVC_BOOKMARK_ID,
	MB_SVC_BOOKMARK_MEDIA_UUID,
	MB_SVC_BOOKMARK_MARKED_TIME,
	MB_SVC_BOOKMARK_THUMBNAIL_PATH,
	MB_SVC_BOOKMARK_NUM,
} mb_svc_bookmark_field_e;

/**
 * Enumerations for FOLDER field name.
 */
typedef enum {
	MB_SVC_FOLDER_NONE		= -1,
	MB_SVC_FOLDER_UUID,
	MB_SVC_FOLDER_PATH,
	MB_SVC_FOLDER_DISPLAY_NAME,
	MB_SVC_FOLDER_MODIFIED_DATE,
	MB_SVC_FOLDER_WEB_ACCOUNT_ID,
	MB_SVC_FOLDER_STORAGE_TYPE,
	MB_SVC_FOLDER_SNS_TYPE,
	MB_SVC_FOLDER_ALBUM_LOCK_STATUS,
	MB_SVC_FOLDER_WEB_ALBUM_ID,
	MB_SVC_FOLDER_VALID,
	MB_SVC_FOLDER_NUM,
} mb_svc_folder_field_e;
/**
 * Enumerations for web_streaming field name.
 */
typedef enum {
	MB_SVC_WEB_STREAMING_NONE		= -1,
	MB_SVC_WEB_STREAMING_ID,
	MB_SVC_WEB_STREAMING_FOLDER_UUID,
	MB_SVC_WEB_STREAMING_TITLE,
	MB_SVC_WEB_STREAMING_DURATION,
	MB_SVC_WEB_STREAMING_URL,
	MB_SVC_WEB_STREAMING_THNMB_PATH,
	MB_SVC_WEB_STREAMING_NUM,
} mb_svc_web_streaming_field_e;


/**
 * Enumerations for MEDIA field name.
 */
typedef enum {
	MB_SVC_MEDIA_NONE		= -1,
	MB_SVC_MEDIA_UUID,
	MB_SVC_MEDIA_PATH,
	MB_SVC_MEDIA_FOLDER_UUID,
	MB_SVC_MEDIA_DISPLAY_NAME,
	MB_SVC_MEDIA_CONTENT_TYPE,
	MB_SVC_MEDIA_RATING,
	MB_SVC_MEDIA_MODIFIED_DATE,
	MB_SVC_MEDIA_THUMBNAIL_PATH,
	MB_SVC_MEDIA_HTTP_URL,
	MB_SVC_MEDIA_SIZE,
	MB_SVC_MEDIA_VALID,
	MB_SVC_MEDIA_NUM,
} mb_svc_media_field_e;

/**
 * Enumerations for videl_meta field name.
 */
typedef enum {
	MB_SVC_VIDEO_META_NONE		= -1,
	MB_SVC_VIDEO_META_ID,
	MB_SVC_VIDEO_META_MEDIA_UUID,
	MB_SVC_VIDEO_META_ALBUM,
	MB_SVC_VIDEO_META_ARTIST,
	MB_SVC_VIDEO_META_TITLE,
	MB_SVC_VIDEO_META_GENRE,
	MB_SVC_VIDEO_META_DESCRIPTION,
	MB_SVC_VIDEO_META_YOUTUBE_CATEGORY,
	MB_SVC_VIDEO_META_BOOKMARK_LAST_PLAYED,
	MB_SVC_VIDEO_META_DURATION,
	MB_SVC_VIDEO_META_LONGISTUDE,
	MB_SVC_VIDEO_META_LATITUDE,
	MB_SVC_VIDEO_META_WIDTH,
	MB_SVC_VIDEO_META_HEIGHT,
	MB_SVC_VIDEO_META_DATETAKEN,
	MB_SVC_VIDEO_META_NUM,
} mb_svc_video_meta_field_e;


/**
 * Enumerations for IMAGE_META field name.
 */
typedef enum {
	MB_SVC_IMAGE_META_NONE		= -1,
	MB_SVC_IMAGE_META_ID,
	MB_SVC_IMAGE_META_MEDIA_UUID,
	MB_SVC_IMAGE_META_LONGISTUDE,
	MB_SVC_IMAGE_META_LATITUDE,
	MB_SVC_IMAGE_META_DESCRIPTION,
	MB_SVC_IMAGE_META_WIDTH,
	MB_SVC_IMAGE_META_HEIGHT,
	MB_SVC_IMAGE_META_ORIENTATION,
	MB_SVC_IMAGE_META_DATETAKEN,
	MB_SVC_IMAGE_META_NUM,
} mb_svc_image_meta_field_e;

/**
 * Enumerations for VISUAL_TAG_MAP field name.
 */
typedef enum {
	MB_SVC_TAG_MAP_NONE		= -1,
	MB_SVC_TAG_MAP_ID,
	MB_SVC_TAG_MAP_MEDIA_UUID,
	MB_SVC_TAG_MAP_TAG_ID,
	MB_SVC_TAG_MAP_NUM,
} mb_svc_tag_map_field_e;

/**
 * Enumerations for VISUAL_TAG field name.
 */
typedef enum {
	MB_SVC_TAG_NONE		= -1,
	MB_SVC_TAG_ID,
	MB_SVC_TAG_NAME,
	MB_SVC_TAG_NUM,
} mb_svc_tag_field_e;

/**
 *	table fields information
 */
typedef struct {
	 char *field_name;
	 char *field_type;
}mb_svc_tbl_field_s;

/**
 *	table information
 */

#define MB_SVC_FIELD_CNT_MAX 16
#define MB_SVC_TABLE_CNT_MAX 8

typedef struct {
	char *table_name;
	mb_svc_tbl_field_s mb_svc_field[MB_SVC_FIELD_CNT_MAX];
	char *primary_key;
} mb_svc_tbl_s;


mb_svc_tbl_s mb_svc_tbl[MB_SVC_TABLE_CNT_MAX] = {
	{"video_bookmark", {
			    {"_id", "INTEGER"} ,	/* PK */
			    {"visual_uuid", "VARCHAR(256)"} ,
			    {"marked_time", "INT"}
			    ,
			    {"thumbnail_path", "VARCHAR(256)"}
			    ,
			    {0}
			    }
	 ,
	 ", primary key (_id)"}
	,

	{"visual_folder", {
			   {"folder_uuid", "VARCHAR(256)"} ,	/* PK */
			   {"path", "VARCHAR(256)"} ,	/* full path */
			   {"folder_name", "VARCHAR(256)"}
			   ,
			   {"modified_date", "INT"}
			   ,
			   {"web_account_id", "VARCHAR(256)"}
			   ,
			   {"storage_type", "INT"}
			   ,
			   {"sns_type", "INT"}
			   ,
			   {"lock_status", "INT"}
			   ,
			   {"web_album_id", "VARCHAR(256)"}
			   ,
			   {"valid", "INT"}
			   ,
			   {0}
			   }
	 ,
	 ", primary key (_id)"}
	,

	{"web_streaming", {
			   {"_id", "INTEGER"} /* PK */
			   ,
			   {"folder_uuid", "VARCHAR(256)"}	/* FK, folder table */
			   ,
			   {"title", "VARCHAR(256)"}
			   ,
			   {"duration", "INT"}
			   ,
			   {"url", "VARCHAR(256)"}
			   ,
			   {"thumb_path", "VARCHAR(256)"}
			   ,
			   {0}
			   }
	 ,
	 ", primary key (_id)"}
	,

	{"visual_media", {
			  {"visual_uuid", "VARCHAR(256)"}
			  ,
			  {"path", "VARCHAR(256)"}
			  ,
			  {"folder_uuid", "VARCHAR(256)"}
			  ,
			  {"display_name", "VARCHAR(256)"}
			  ,
			  {"content_type", "INT"}
			  ,
			  {"rating", "INT"}
			  ,
			  {"modified_date", "INT"}
			  ,
			  {"thumbnail_path", "VARCHAR(256)"}
			  ,
			  {"http_url", "VARCHAR(256)"}
			  ,
			  {"size", "INT"}
			  ,
			  {"valid", "INT"}
			  ,
			  {0}
			  }
	 ,
	 ", primary key (_id)"}
	,

	{"video_meta", {
			{"_id", "INTEGER"}
			,
			{"visual_uuid", "VARCHAR(256)"}
			,
			{"album", "VARCHAR(256)"}
			,
			{"artist", "VARCHAR(256)"}
			,
			{"title", "VARCHAR(256)"}
			,
			{"genre", "VARCHAR(256)"}
			,
			{"description", "VARCHAR(256)"}
			,
			{"youtube_category", "VARCHAR(256)"}
			,
			{"last_played_time", "INT"}
			,
			{"duration", "INT"}
			,
			{"longitude", "DOUBLE"}
			,
			{"latitude", "DOUBLE"}
			,
			{"width", "INT"}
			,
			{"height", "INT"}
			,
			{"datetaken", "INT"}
			,
			{0}
			}
	 ,
	 ", primary key (_id)"}
	,

	{"image_meta", {
			{"_id", "INTEGER"}
			,
			{"visual_uuid", "VARCHAR(256)"}
			,
			{"longitude", "DOUBLE"}
			,
			{"latitude", "DOUBLE"}
			,
			{"description", "VARCHAR(256)"}
			,
			{"width", "INT"}
			,
			{"height", "INT"}
			,
			{"orientation", "INT"}
			,
			{"datetaken", "INT"}
			,
			{0}
			}
	 ,
	 ", primary key (_id)"}
	,

	{"visual_tag_map", {
			    {"_id", "INTEGER"} /* PK */
			    ,
				{"visual_uuid", "VARCHAR(256)"}
			    ,
			    {"tag_id", "INT"}
			    ,
			    {0}
			    }
	 ,
	 ", primary key (_id)"}
	,

	{"visual_tag", {
			{"_id", "INTEGER"} /* FK, media table */
			,
			{"tag_name", "VARCHAR(256)"}
			,
			{0}
			}
	 ,
	 ", primary key (_id)"}
};

int field_num_max[MB_SVC_TABLE_CNT_MAX] = { MB_SVC_BOOKMARK_NUM,
	MB_SVC_FOLDER_NUM,
	MB_SVC_WEB_STREAMING_NUM,
	MB_SVC_MEDIA_NUM,
	MB_SVC_VIDEO_META_NUM,
	MB_SVC_IMAGE_META_NUM,
	MB_SVC_TAG_MAP_NUM,
	MB_SVC_TAG_NUM
};

static __thread GList *g_sql_list = NULL;
static __thread char g_last_updated_folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

#if 0
static int __mb_svc_create_tbl(bool external);
static int __mb_svc_drop_tbl(void);
#endif
static int __mb_svc_delete_record(MediaSvcHandle *mb_svc_handle, int id, mb_svc_tbl_name_e tbl_name);
static int __mb_svc_delete_record_by_uuid(MediaSvcHandle *mb_svc_handle, const char *id, mb_svc_tbl_name_e tbl_name);
static int __mb_svc_db_get_next_id(MediaSvcHandle *mb_svc_handle, int table_id);

#if 0
static int __mb_svc_create_tbl(bool external)
{
	int err = -1;
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	char *primary_key = NULL;
	int i, j;
	mb_svc_tbl_field_s *mb_svc_field = NULL;
	int field_count = 0;
	GString *query_string =
	    g_string_sized_new(MB_SVC_DEFAULT_QUERY_SIZE + 1);
	mb_svc_debug("__mb_svc_create_tbl--enter\n");

	for (i = 0; i < MB_SVC_TABLE_CNT_MAX; ++i) {
		field_count = field_num_max[i];
		memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

		snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
			 mb_svc_tbl[i].table_name);

		mb_svc_field = mb_svc_tbl[i].mb_svc_field;
		primary_key = mb_svc_tbl[i].primary_key;

		err = _mb_svc_table_exist(table_name);
		if (err > 0) {
			continue;
		}
		g_string_printf(query_string, "CREATE TABLE %s(", table_name);

		for (j = 0; j < field_count; ++j) {
			if (j != 0) {
				g_string_append(query_string, ", ");
			}
			g_string_append_printf(query_string, "%s %s",
					       mb_svc_field[j].field_name,
					       mb_svc_field[j].field_type);
		}

		g_string_append(query_string, primary_key);
		g_string_append(query_string, " );");

		err = mb_svc_query_sql_gstring(query_string);
		if (err < 0) {
			mb_svc_debug("failed to create table\n");
			mb_svc_debug("query string is %s\n", query_string->str);

			g_string_free(query_string, TRUE);
			return MB_SVC_ERROR_DB_INTERNAL;
		}
	}

	g_string_free(query_string, TRUE);

	mb_svc_debug("__mb_svc_create_tbl--leave\n");
	return 0;
}

static int __mb_svc_drop_tbl(void)
{
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int i, err;
	char *table_name;
	mb_svc_debug("__mb_svc_drop_tbl--enter\n");

	for (i = 0; i < MB_SVC_TABLE_CNT_MAX; ++i) {
		table_name = mb_svc_tbl[i].table_name;
		if (table_name == NULL) {
			mb_svc_debug("table_name is null\n");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
		err = _mb_svc_table_exist(table_name);
		if (err > 0) {
			snprintf(query_string, sizeof(query_string),
				 MB_SVC_TABLE_DROP_QUERY_STRING, table_name);
			err = mb_svc_query_sql(query_string);
			if (err < 0) {
				mb_svc_debug("drop failed\n");
				mb_svc_debug("query string is %s\n",
					     query_string);
				return MB_SVC_ERROR_DB_INTERNAL;
			}
		}
	}
	mb_svc_debug("__mb_svc_drop_tbl--leave\n");
	return 0;
}
#endif

int mb_svc_set_folder_as_valid_sql_add(const char *folder_id, int valid)
{
	mb_svc_debug("Folder ID:%s, valid:%d", folder_id, valid);
	char *sql = NULL;

	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[field_seq].table_name);

	sql = sqlite3_mprintf(MB_SVC_UPDATE_FOLDER_VALID_BY_UUID,
										table_name,
										valid,
										folder_id);

	mb_svc_sql_list_add(&g_sql_list, &sql);

	return 0;
}

int mb_svc_set_item_as_valid_sql_add(MediaSvcHandle *mb_svc_handle, const char *full_path, int valid)
{
	mb_svc_debug("full path: %s, valid:%d", full_path, valid);

	mb_svc_media_record_s media_record = {"",};
	int ret = -1;

	if (full_path == NULL) {
		mb_svc_debug("full_path == NULL \n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	ret = mb_svc_get_media_record_by_full_path(mb_svc_handle, full_path, &media_record);
	if (ret < 0) {
		mb_svc_debug(" mb_svc_get_media_record_by_full_path fails (%d)", ret);
		return ret;
	}
	mb_svc_debug("Media ID : %s", media_record.media_uuid);

	/* Set the record as valid/invalid in 'media' table */
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[field_seq].table_name);

	char *sql =
	    sqlite3_mprintf(MB_SVC_UPDATE_MEDIA_VALID_BY_UUID, table_name, valid,
			    media_record.media_uuid);

	mb_svc_sql_list_add(&g_sql_list, &sql);

	if ((valid == 1) && (strcmp(g_last_updated_folder_uuid, media_record.folder_uuid) != 0)) {
		strncpy(g_last_updated_folder_uuid, media_record.folder_uuid, MB_SVC_UUID_LEN_MAX + 1);

		ret =
		    mb_svc_set_folder_as_valid_sql_add(media_record.folder_uuid, valid);
		if (ret < 0) {
			mb_svc_debug
			    ("mb_svc_update_folder_valid_sql_add fail:%d\n", ret);
			return ret;
		}
	}

	return 0;
}

int mb_svc_set_item_as_valid(MediaSvcHandle *mb_svc_handle)
{
	mb_svc_debug("");
	int i = 0;
	int length = g_list_length(g_sql_list);

	for (i = 0; i < length; i++) {
		char *sql = (char*)g_list_nth_data(g_sql_list, i);
		mb_svc_query_sql(mb_svc_handle, sql);
	}

	mb_svc_sql_list_release(&g_sql_list);

	return 0;
}

#if 0
static int mb_svc_busy_handler(void *pData, int count)
{
	usleep(50000);
	printf("mb_svc_busy_handler called : %d\n", count);
	mb_svc_debug("mb_svc_busy_handler called : %d\n", count);

	return 100 - count;
}

/* connect to database-server */
int mb_svc_connect_db(sqlite3 **handle)
{
	mb_svc_debug("mb_svc_connect_db\n");
	int ret = 0;

	ret =
	    db_util_open(MEDIA_INFO_DATABASE_NAME, handle,
			 DB_UTIL_REGISTER_HOOK_METHOD);
	if (SQLITE_OK != ret) {
		mb_svc_debug("can not connect to db-server\n");
		if (*handle) mb_svc_debug("[sqlite] %s\n", sqlite3_errmsg(*handle));

		*handle = NULL;

		return MB_SVC_ERROR_DB_CONNECT;
	}

	/* Register Busy handler */
	ret = sqlite3_busy_handler(*handle, mb_svc_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		mb_svc_debug("Fail to register busy handler\n");	
		if (*handle) mb_svc_debug("[sqlite] %s\n", sqlite3_errmsg(*handle));

		db_util_close(*handle);
		*handle = NULL;

		return MB_SVC_ERROR_DB_CONNECT;
	}

	mb_svc_debug("connected to db-server\n");

	return 0;
}

/* disconnect from database-server */
int mb_svc_disconnect_db(sqlite3 *handle)
{
	/* disconnect from database-server */
	int ret = 0;

	ret = db_util_close(handle);

	if (SQLITE_OK != ret) {
		mb_svc_debug("can not disconnect database\n");
		mb_svc_debug("[sqlite] %s\n", sqlite3_errmsg(handle));

		return MB_SVC_ERROR_DB_DISCONNECT;
	}
	handle = NULL;

	mb_svc_debug("Disconnected successfully\n");
	return 0;
}
#endif
mb_svc_tbl_s *mb_svc_search_matched_svc_tbl(mb_svc_tbl_name_e tbl_name)
{
	int i;

	if (tbl_name <= MB_SVC_TABLE_NONE || tbl_name >= MB_SVC_TABLE_NUM) {
		mb_svc_debug("table name is invalid\n");
		return NULL;
	}

	char *table_name = mb_svc_tbl[tbl_name].table_name;

	if (table_name == NULL) {
		mb_svc_debug("table_name pointer is null\n");
		return NULL;
	}

	for (i = 0; i < MB_SVC_TABLE_CNT_MAX; ++i) { /* search the matched table */
		if (!strncmp
		    (table_name, mb_svc_tbl[i].table_name,
		     MB_SVC_ARRAY_LEN_MAX)) {
			return &mb_svc_tbl[i];
		}
	}
	mb_svc_debug("error, no matched mb svc table found. %s", table_name);
	return NULL;
}

int _mb_svc_table_exist(MediaSvcHandle *mb_svc_handle, char *table_name)
{
	char szQuery[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int ret = -1;;
	char szTableName[64] = { 0 };
	char *pszTempName = NULL;
	sqlite3_stmt *pstStmt_pb = NULL;

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	if (table_name == NULL) {
		mb_svc_debug("table_name pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	snprintf(szQuery, sizeof(szQuery), MB_SVC_TABLE_EXIST_QUERY_STRING,
		 table_name);

	ret =
	    sqlite3_prepare_v2(handle, szQuery, strlen(szQuery), &pstStmt_pb,
			       NULL);
	if (SQLITE_OK != ret) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", szQuery);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	ret = sqlite3_step(pstStmt_pb);
	if (SQLITE_ROW != ret) {
		mb_svc_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", szQuery);
		sqlite3_finalize(pstStmt_pb);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	pszTempName = (char *)sqlite3_column_text(pstStmt_pb, 0);
	if (NULL != pszTempName) {
		strncpy(szTableName, pszTempName, strlen(pszTempName));
	}
	sqlite3_finalize(pstStmt_pb);

	mb_svc_debug("if [%s]=[%s], table exist!!!\n", table_name, szTableName);
	ret = !(strcmp(table_name, szTableName));

	return ret;
}

int _mb_svc_truncate_tbl(MediaSvcHandle *mb_svc_handle)
{
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	int i, err;
	char *table_name;
	mb_svc_debug("_mb_svc_truncate_tbl--enter\n");

	for (i = 0; i < MB_SVC_TABLE_CNT_MAX; ++i) {
		table_name = mb_svc_tbl[i].table_name;
		if (table_name == NULL) {
			mb_svc_debug("table_name is null\n");
			return MB_SVC_ERROR_INVALID_PARAMETER;
		}
		err = _mb_svc_table_exist(mb_svc_handle, table_name);
		if (err > 0) {
			snprintf(query_string, sizeof(query_string),
				 MB_SVC_TABLE_DELETE_QUERY_STRING, table_name);
			err = mb_svc_query_sql(mb_svc_handle, query_string);
			if (err < 0) {
				mb_svc_debug("truncate table failed\n");
				mb_svc_debug("query string is %s\n",
					     query_string);
				return MB_SVC_ERROR_DB_INTERNAL;
			}
		}
	}
	mb_svc_debug("_mb_svc_truncate_tbl--leave\n");
	return 0;
}

static int __mb_svc_db_get_next_id(MediaSvcHandle *mb_svc_handle, int table_id)
{
	int ret = 0;
	sqlite3_stmt *stmt = NULL;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(query_string, sizeof(query_string), "select max(_id) from %s;",
		 (char *)mb_svc_tbl[table_id].table_name);

	mb_svc_debug("Query : %s", query_string);
	ret =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);

	if (ret != SQLITE_OK) {
		mb_svc_debug("Make Sequence error for prepare %s",
			     sqlite3_errmsg(handle));
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	ret = sqlite3_step(stmt);
	if (ret != SQLITE_ROW) {
		mb_svc_debug("There's no record %s", sqlite3_errmsg(handle));

		sqlite3_finalize(stmt);
		stmt = NULL;
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	ret = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);
	stmt = NULL;

	mb_svc_debug("Next ID : %d", ret + 1);

	return ret + 1;
}

static int __mb_svc_delete_record(MediaSvcHandle *mb_svc_handle, int id, mb_svc_tbl_name_e tbl_name)
{
	int err = -1;
	mb_svc_tbl_s *matched_tbl = NULL;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	if (id <= 0) {
		mb_svc_debug("id is wrong");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tbl_name <= MB_SVC_TABLE_NONE || tbl_name >= MB_SVC_TABLE_NUM) {
		mb_svc_debug("table name is invalid\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	matched_tbl = mb_svc_search_matched_svc_tbl(tbl_name);
	if (matched_tbl != NULL) {
		snprintf(table_name, sizeof(table_name), "%s",
			 mb_svc_tbl[tbl_name].table_name);
		snprintf(query_string, sizeof(query_string),
			 MB_SVC_RECORD_DELETE_QUERY_STRING, table_name, id);

		mb_svc_debug("Query: %s", query_string);

		err = mb_svc_query_sql(mb_svc_handle, query_string);
		if (err < 0) {
			mb_svc_debug("failed to delete record\n");
			mb_svc_debug("query string is %s\n", query_string);
			return MB_SVC_ERROR_DB_INTERNAL;
		} else {
			mb_svc_debug("Succeed to delete record\n");
			mb_svc_debug("query string is %s\n", query_string);
		}
	} else {
		mb_svc_debug("failed to delete record\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

static int __mb_svc_delete_record_by_uuid(MediaSvcHandle *mb_svc_handle, const char *id, mb_svc_tbl_name_e tbl_name)
{
	int err = -1;
	mb_svc_tbl_s *matched_tbl = NULL;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	if (id == NULL) {
		mb_svc_debug("id is NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (tbl_name <= MB_SVC_TABLE_NONE || tbl_name >= MB_SVC_TABLE_NUM) {
		mb_svc_debug("table name is invalid\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	matched_tbl = mb_svc_search_matched_svc_tbl(tbl_name);
	if (matched_tbl != NULL) {
		snprintf(table_name, sizeof(table_name), "%s",
			 mb_svc_tbl[tbl_name].table_name);

		if (tbl_name == MB_SVC_TABLE_FOLDER) {
			snprintf(query_string, sizeof(query_string),
				MB_SVC_RECORD_FOLDER_DELETE_BY_UUID, table_name, id);
		} else {
			snprintf(query_string, sizeof(query_string),
				MB_SVC_RECORD_MEDIA_DELETE_BY_UUID, table_name, id);
		}
		mb_svc_debug("Query: %s", query_string);

		err = mb_svc_query_sql(mb_svc_handle, query_string);
		if (err < 0) {
			mb_svc_debug("failed to delete record\n");
			mb_svc_debug("query string is %s\n", query_string);
			return MB_SVC_ERROR_DB_INTERNAL;
		} else {
			mb_svc_debug("Succeed to delete record\n");
			mb_svc_debug("query string is %s\n", query_string);
		}
	} else {
		mb_svc_debug("failed to delete record\n");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int mb_svc_delete_record_bookmark_by_id(MediaSvcHandle *mb_svc_handle, int id)
{
	return __mb_svc_delete_record(mb_svc_handle, id, MB_SVC_TABLE_BOOKMARK);
}

int mb_svc_delete_record_folder_by_id(MediaSvcHandle *mb_svc_handle, const char *id)
{
	return __mb_svc_delete_record_by_uuid(mb_svc_handle, id, MB_SVC_TABLE_FOLDER);
}

int mb_svc_delete_record_web_streaming_by_id(MediaSvcHandle *mb_svc_handle, int id)
{
	return __mb_svc_delete_record(mb_svc_handle, id, MB_SVC_TABLE_WEB_STREAMING);
}

int mb_svc_delete_record_media_by_id(MediaSvcHandle *mb_svc_handle, const char *id)
{
	return __mb_svc_delete_record_by_uuid(mb_svc_handle, id, MB_SVC_TABLE_MEDIA);
}

int mb_svc_delete_record_video_meta_by_id(MediaSvcHandle *mb_svc_handle, int id)
{
	return __mb_svc_delete_record(mb_svc_handle, id, MB_SVC_TABLE_VIDEO_META);
}

int mb_svc_delete_record_image_meta_by_id(MediaSvcHandle *mb_svc_handle, int id)
{
	return __mb_svc_delete_record(mb_svc_handle, id, MB_SVC_TABLE_IMAGE_META);
}

int mb_svc_insert_record_bookmark(MediaSvcHandle *mb_svc_handle, mb_svc_bookmark_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_BOOKMARK;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
/*
	record->_id = __mb_svc_db_get_next_id(MB_SVC_TABLE_BOOKMARK);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
*/
	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_BOOKMARK_TABLE,
			    mb_svc_tbl[field_seq].table_name,
//			    mb_svc_field[MB_SVC_BOOKMARK_ID].field_name,
			    mb_svc_field[MB_SVC_BOOKMARK_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_BOOKMARK_MARKED_TIME].field_name,
			    mb_svc_field[MB_SVC_BOOKMARK_THUMBNAIL_PATH].
			    field_name, /* record->_id, */record->media_uuid,
			    record->marked_time, record->thumbnail_path);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting bookmark table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_insert_record_folder(MediaSvcHandle *mb_svc_handle, mb_svc_folder_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	strncpy(record->uuid, _media_info_generate_uuid(), MB_SVC_UUID_LEN_MAX + 1);

	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_FOLDER_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_FOLDER_UUID].field_name,
			    mb_svc_field[MB_SVC_FOLDER_PATH].field_name,
			    mb_svc_field[MB_SVC_FOLDER_DISPLAY_NAME].field_name,
			    mb_svc_field[MB_SVC_FOLDER_MODIFIED_DATE].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ACCOUNT_ID].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_STORAGE_TYPE].field_name,
			    mb_svc_field[MB_SVC_FOLDER_SNS_TYPE].field_name,
			    mb_svc_field[MB_SVC_FOLDER_ALBUM_LOCK_STATUS].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ALBUM_ID].field_name,
			    mb_svc_field[MB_SVC_FOLDER_VALID].field_name,
			    record->uuid, record->uri, record->display_name,
			    record->modified_date, record->web_account_id,
			    record->storage_type, record->sns_type,
			    record->lock_status, record->web_album_id, 1);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting folder table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_insert_record_folder_sql(mb_svc_folder_record_s *record, char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;

	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	strncpy(record->uuid, _media_info_generate_uuid(), MB_SVC_UUID_LEN_MAX + 1);

	*sql =
	    sqlite3_mprintf(MB_SVC_INSERT_FOLDER_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_FOLDER_UUID].field_name,
			    mb_svc_field[MB_SVC_FOLDER_PATH].field_name,
			    mb_svc_field[MB_SVC_FOLDER_DISPLAY_NAME].field_name,
			    mb_svc_field[MB_SVC_FOLDER_MODIFIED_DATE].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ACCOUNT_ID].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_STORAGE_TYPE].field_name,
			    mb_svc_field[MB_SVC_FOLDER_SNS_TYPE].field_name,
			    mb_svc_field[MB_SVC_FOLDER_ALBUM_LOCK_STATUS].
			    field_name,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ALBUM_ID].field_name,
			    mb_svc_field[MB_SVC_FOLDER_VALID].field_name,
			    record->uuid, record->uri, record->display_name,
			    record->modified_date, record->web_account_id,
			    record->storage_type, record->sns_type,
			    record->lock_status, record->web_album_id, 1);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int mb_svc_insert_record_web_streaming(MediaSvcHandle *mb_svc_handle, mb_svc_web_streaming_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_WEB_STREAMING;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	record->_id = __mb_svc_db_get_next_id(mb_svc_handle, MB_SVC_TABLE_WEB_STREAMING);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_WEB_STREAMING_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_ID].field_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_FOLDER_UUID].
			    field_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_TITLE].field_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_DURATION].
			    field_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_URL].field_name,
			    mb_svc_field[MB_SVC_WEB_STREAMING_THNMB_PATH].
			    field_name, record->_id, record->folder_uuid,
			    record->title, record->duration, record->url,
			    record->thumb_path);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting web streaming table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

/**
 *	insert record into table--media 
 */
int mb_svc_insert_record_media(MediaSvcHandle *mb_svc_handle, mb_svc_media_record_s *record,
			       minfo_store_type storage_type)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	strncpy(record->media_uuid, _media_info_generate_uuid(), MB_SVC_UUID_LEN_MAX + 1);

	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_MEDIA_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_MEDIA_PATH].field_name,
			    mb_svc_field[MB_SVC_MEDIA_FOLDER_UUID].field_name,
			    mb_svc_field[MB_SVC_MEDIA_DISPLAY_NAME].field_name,
			    mb_svc_field[MB_SVC_MEDIA_CONTENT_TYPE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_RATING].field_name,
			    mb_svc_field[MB_SVC_MEDIA_MODIFIED_DATE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_THUMBNAIL_PATH].field_name,
			    mb_svc_field[MB_SVC_MEDIA_HTTP_URL].field_name,
			    mb_svc_field[MB_SVC_MEDIA_SIZE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_VALID].field_name,
			    record->media_uuid, record->path,
			    record->folder_uuid, record->display_name, record->content_type,
			    record->rate, record->modified_date,
			    record->thumbnail_path, record->http_url, record->size, 1);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting media table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_insert_record_media_sql(mb_svc_media_record_s *record,
					minfo_store_type storage_type,
					char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;

	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	strncpy(record->media_uuid, _media_info_generate_uuid(), MB_SVC_UUID_LEN_MAX + 1);

	*sql =
	    sqlite3_mprintf(MB_SVC_INSERT_MEDIA_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_MEDIA_PATH].field_name,
			    mb_svc_field[MB_SVC_MEDIA_FOLDER_UUID].field_name,
			    mb_svc_field[MB_SVC_MEDIA_DISPLAY_NAME].field_name,
			    mb_svc_field[MB_SVC_MEDIA_CONTENT_TYPE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_RATING].field_name,
			    mb_svc_field[MB_SVC_MEDIA_MODIFIED_DATE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_THUMBNAIL_PATH].field_name,
			    mb_svc_field[MB_SVC_MEDIA_HTTP_URL].field_name,
			    mb_svc_field[MB_SVC_MEDIA_SIZE].field_name,
			    mb_svc_field[MB_SVC_MEDIA_VALID].field_name,
			    record->media_uuid, record->path,
				record->folder_uuid,
			    record->display_name, record->content_type,
			    record->rate, record->modified_date,
			    record->thumbnail_path, record->http_url, record->size, 1);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int
mb_svc_insert_record_video_meta(MediaSvcHandle *mb_svc_handle, mb_svc_video_meta_record_s *record,
				minfo_store_type storage_type)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_VIDEO_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
/*
	record->_id = __mb_svc_db_get_next_id(MB_SVC_TABLE_VIDEO_META);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
*/
	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_VIDEO_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
//			    mb_svc_field[MB_SVC_VIDEO_META_ID].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_ALBUM].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_ARTIST].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_TITLE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_GENRE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DESCRIPTION].
			    field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_YOUTUBE_CATEGORY].
			    field_name,
			    mb_svc_field
			    [MB_SVC_VIDEO_META_BOOKMARK_LAST_PLAYED].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DURATION].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_LONGISTUDE].
			    field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_LATITUDE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_WIDTH].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_HEIGHT].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DATETAKEN].
			    field_name, /* record->_id, */record->media_uuid,
			    record->album, record->artist, record->title, record->genre,
			    record->description, record->youtube_category,
			    record->last_played_time, record->duration,
			    record->longitude, record->latitude, record->width,
			    record->height, record->datetaken);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting video meta table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_insert_record_video_meta_sql(mb_svc_video_meta_record_s *record,
				minfo_store_type storage_type,
				char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_VIDEO_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
/*
	record->_id = __mb_svc_db_get_next_id(MB_SVC_TABLE_VIDEO_META);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
*/
	*sql =
	    sqlite3_mprintf(MB_SVC_INSERT_VIDEO_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
//			    mb_svc_field[MB_SVC_VIDEO_META_ID].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_ALBUM].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_ARTIST].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_TITLE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_GENRE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DESCRIPTION].
			    field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_YOUTUBE_CATEGORY].
			    field_name,
			    mb_svc_field
			    [MB_SVC_VIDEO_META_BOOKMARK_LAST_PLAYED].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DURATION].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_LONGISTUDE].
			    field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_LATITUDE].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_WIDTH].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_HEIGHT].field_name,
			    mb_svc_field[MB_SVC_VIDEO_META_DATETAKEN].
			    field_name, /* record->_id, */record->media_uuid,
			    record->album, record->artist, record->title, record->genre,
			    record->description, record->youtube_category,
			    record->last_played_time, record->duration,
			    record->longitude, record->latitude, record->width,
			    record->height, record->datetaken);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int
mb_svc_insert_record_image_meta(MediaSvcHandle *mb_svc_handle,
				mb_svc_image_meta_record_s *record,
				minfo_store_type storage_type)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_IMAGE_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
/*
	record->_id = __mb_svc_db_get_next_id(MB_SVC_TABLE_IMAGE_META);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
*/
	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_IMAGE_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
//			    mb_svc_field[MB_SVC_IMAGE_META_ID].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_LONGISTUDE].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_LATITUDE].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_DESCRIPTION].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_WIDTH].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_HEIGHT].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_ORIENTATION].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_DATETAKEN].
			    field_name, /* record->_id, */record->media_uuid,
			    record->longitude, record->latitude,
			    record->description, record->width, record->height,
			    record->orientation, record->datetaken);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting image meta table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_insert_record_image_meta_sql(mb_svc_image_meta_record_s *record,
				minfo_store_type storage_type,
				char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_IMAGE_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
/*
	record->_id = __mb_svc_db_get_next_id(MB_SVC_TABLE_IMAGE_META);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}
*/
	*sql =
	    sqlite3_mprintf(MB_SVC_INSERT_IMAGE_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
//			    mb_svc_field[MB_SVC_IMAGE_META_ID].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_LONGISTUDE].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_LATITUDE].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_DESCRIPTION].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_WIDTH].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_HEIGHT].field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_ORIENTATION].
			    field_name,
			    mb_svc_field[MB_SVC_IMAGE_META_DATETAKEN].
			    field_name, /* record->_id, */record->media_uuid,
			    record->longitude, record->latitude,
			    record->description, record->width, record->height,
			    record->orientation, record->datetaken);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int mb_svc_update_record_folder(MediaSvcHandle *mb_svc_handle, mb_svc_folder_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_FOLDER_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_FOLDER_PATH].field_name,
			    record->uri,
			    mb_svc_field[MB_SVC_FOLDER_DISPLAY_NAME].field_name,
			    record->display_name,
			    mb_svc_field[MB_SVC_FOLDER_MODIFIED_DATE].
			    field_name, record->modified_date,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ACCOUNT_ID].
			    field_name, record->web_account_id,
			    mb_svc_field[MB_SVC_FOLDER_STORAGE_TYPE].field_name,
			    record->storage_type,
			    mb_svc_field[MB_SVC_FOLDER_SNS_TYPE].field_name,
			    record->sns_type,
			    mb_svc_field[MB_SVC_FOLDER_ALBUM_LOCK_STATUS].
			    field_name, record->lock_status, record->uuid);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating folder table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_update_record_folder_sql(mb_svc_folder_record_s *record, char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	*sql =
	    sqlite3_mprintf(MB_SVC_UPDATE_FOLDER_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_FOLDER_PATH].field_name,
			    record->uri,
			    mb_svc_field[MB_SVC_FOLDER_DISPLAY_NAME].field_name,
			    record->display_name,
			    mb_svc_field[MB_SVC_FOLDER_MODIFIED_DATE].
			    field_name, record->modified_date,
			    mb_svc_field[MB_SVC_FOLDER_WEB_ACCOUNT_ID].
			    field_name, record->web_account_id,
			    mb_svc_field[MB_SVC_FOLDER_STORAGE_TYPE].field_name,
			    record->storage_type,
			    mb_svc_field[MB_SVC_FOLDER_SNS_TYPE].field_name,
			    record->sns_type,
			    mb_svc_field[MB_SVC_FOLDER_ALBUM_LOCK_STATUS].
			    field_name, record->lock_status, record->uuid);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int mb_svc_update_record_folder_path(MediaSvcHandle *mb_svc_handle, char *old_path, char *new_path)
{
	mb_svc_debug("");

	if (old_path == NULL || new_path == NULL) {
		mb_svc_debug("old_path == NULL || new_path ");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_FOLDER_PATH_TABLE,
			    mb_svc_tbl[field_seq].table_name, old_path,
			    new_path);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating folder table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_update_folder_modified_date(MediaSvcHandle *mb_svc_handle, char *path, int date)
{
	mb_svc_debug("");

	if (path == NULL) {
		mb_svc_debug("old_path == NULL || new_path == NULL ");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;
	char path_like[MB_SVC_FILE_PATH_LEN_MAX + 1];

	snprintf(path_like, sizeof(path_like), "%s%%", path);

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_FOLDER_MODIFIED_DATE_TABLE,
			    mb_svc_tbl[field_seq].table_name, date, path_like);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating folder table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_update_record_media(MediaSvcHandle *mb_svc_handle, mb_svc_media_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_MEDIA_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_MEDIA_FOLDER_UUID].field_name,
			    record->folder_uuid,
			    mb_svc_field[MB_SVC_MEDIA_PATH].field_name,
			    record->path,
			    mb_svc_field[MB_SVC_MEDIA_DISPLAY_NAME].field_name,
			    record->display_name,
			    mb_svc_field[MB_SVC_MEDIA_CONTENT_TYPE].field_name,
			    record->content_type,
			    mb_svc_field[MB_SVC_MEDIA_RATING].field_name,
			    record->rate,
			    mb_svc_field[MB_SVC_MEDIA_MODIFIED_DATE].field_name,
			    record->modified_date,
			    mb_svc_field[MB_SVC_MEDIA_THUMBNAIL_PATH].
			    field_name, record->thumbnail_path,
			    mb_svc_field[MB_SVC_MEDIA_HTTP_URL].field_name,
			    record->http_url,
			    mb_svc_field[MB_SVC_MEDIA_SIZE].field_name,
			    record->size,
				record->media_uuid);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating media table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_update_record_media_sql(mb_svc_media_record_s *record, char **sql)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	*sql =
	    sqlite3_mprintf(MB_SVC_UPDATE_MEDIA_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_MEDIA_FOLDER_UUID].field_name,
			    record->folder_uuid,
			    mb_svc_field[MB_SVC_MEDIA_PATH].field_name,
			    record->path,
			    mb_svc_field[MB_SVC_MEDIA_DISPLAY_NAME].field_name,
			    record->display_name,
			    mb_svc_field[MB_SVC_MEDIA_CONTENT_TYPE].field_name,
			    record->content_type,
			    mb_svc_field[MB_SVC_MEDIA_RATING].field_name,
			    record->rate,
			    mb_svc_field[MB_SVC_MEDIA_MODIFIED_DATE].field_name,
			    record->modified_date,
			    mb_svc_field[MB_SVC_MEDIA_THUMBNAIL_PATH].
			    field_name, record->thumbnail_path,
			    mb_svc_field[MB_SVC_MEDIA_HTTP_URL].field_name,
			    record->http_url,
			    mb_svc_field[MB_SVC_MEDIA_SIZE].field_name,
			    record->size,
				record->media_uuid);

	mb_svc_debug("Query : %s", *sql);

	return err;
}

int mb_svc_update_record_video_meta(MediaSvcHandle *mb_svc_handle, mb_svc_video_meta_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_VIDEO_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_VIDEO_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_VIDEO_META_MEDIA_UUID].field_name,
			    record->media_uuid,
			    mb_svc_field[MB_SVC_VIDEO_META_ALBUM].field_name,
			    record->album,
			    mb_svc_field[MB_SVC_VIDEO_META_ARTIST].field_name,
			    record->artist,
			    mb_svc_field[MB_SVC_VIDEO_META_TITLE].field_name,
			    record->title,
			    mb_svc_field[MB_SVC_VIDEO_META_GENRE].field_name,
			    record->genre,
			    mb_svc_field[MB_SVC_VIDEO_META_DESCRIPTION].
			    field_name, record->description,
			    mb_svc_field[MB_SVC_VIDEO_META_YOUTUBE_CATEGORY].
			    field_name, record->youtube_category,
			    mb_svc_field
			    [MB_SVC_VIDEO_META_BOOKMARK_LAST_PLAYED].field_name,
			    record->last_played_time,
			    mb_svc_field[MB_SVC_VIDEO_META_DURATION].field_name,
			    record->duration,
			    mb_svc_field[MB_SVC_VIDEO_META_LONGISTUDE].
			    field_name, record->longitude,
			    mb_svc_field[MB_SVC_VIDEO_META_LATITUDE].field_name,
			    record->latitude,
			    mb_svc_field[MB_SVC_VIDEO_META_WIDTH].field_name,
			    record->width,
			    mb_svc_field[MB_SVC_VIDEO_META_HEIGHT].field_name,
			    record->height,
			    mb_svc_field[MB_SVC_VIDEO_META_DATETAKEN].
			    field_name, record->datetaken, record->_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating video meta table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_update_record_image_meta(MediaSvcHandle *mb_svc_handle, mb_svc_image_meta_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_IMAGE_META;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	query_string =
	    sqlite3_mprintf(MB_SVC_UPDATE_IMAGE_META_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_IMAGE_META_MEDIA_UUID].field_name,
			    record->media_uuid,
			    mb_svc_field[MB_SVC_IMAGE_META_LONGISTUDE].
			    field_name, record->longitude,
			    mb_svc_field[MB_SVC_IMAGE_META_LATITUDE].field_name,
			    record->latitude,
			    mb_svc_field[MB_SVC_IMAGE_META_DESCRIPTION].
			    field_name, record->description,
			    mb_svc_field[MB_SVC_IMAGE_META_WIDTH].field_name,
			    record->width,
			    mb_svc_field[MB_SVC_IMAGE_META_HEIGHT].field_name,
			    record->height,
			    mb_svc_field[MB_SVC_IMAGE_META_ORIENTATION].
			    field_name, record->orientation,
			    mb_svc_field[MB_SVC_IMAGE_META_DATETAKEN].
			    field_name, record->datetaken, record->_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Updating image meta table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int
mb_svc_load_record_bookmark(sqlite3_stmt *stmt,
			    mb_svc_bookmark_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	record->_id = sqlite3_column_int(stmt, MB_SVC_BOOKMARK_ID);
	strncpy(record->media_uuid,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_BOOKMARK_MEDIA_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	record->marked_time =
	    sqlite3_column_int(stmt, MB_SVC_BOOKMARK_MARKED_TIME);
	strncpy(record->thumbnail_path,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_BOOKMARK_THUMBNAIL_PATH),
		MB_SVC_FILE_PATH_LEN_MAX + 1);

	return 0;
}

int
mb_svc_load_record_folder(sqlite3_stmt *stmt, mb_svc_folder_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	strncpy(record->uuid,
		(const char *)sqlite3_column_text(stmt, MB_SVC_FOLDER_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	strncpy(record->uri,
		(const char *)sqlite3_column_text(stmt, MB_SVC_FOLDER_PATH),
		MB_SVC_DIR_PATH_LEN_MAX + 1);
	strncpy(record->display_name,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_FOLDER_DISPLAY_NAME),
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	record->modified_date =
	    sqlite3_column_int(stmt, MB_SVC_FOLDER_MODIFIED_DATE);
	strncpy(record->web_account_id,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_FOLDER_WEB_ACCOUNT_ID),
		MB_SVC_ARRAY_LEN_MAX + 1);
	record->storage_type =
	    sqlite3_column_int(stmt, MB_SVC_FOLDER_STORAGE_TYPE);
	record->sns_type = sqlite3_column_int(stmt, MB_SVC_FOLDER_SNS_TYPE);
	record->lock_status =
	    sqlite3_column_int(stmt, MB_SVC_FOLDER_ALBUM_LOCK_STATUS);
	strncpy(record->web_album_id,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_FOLDER_WEB_ALBUM_ID),
		MB_SVC_ARRAY_LEN_MAX + 1);

	return 0;
}

int
mb_svc_load_record_folder_name(sqlite3_stmt *stmt, char *folder_name,
			       int max_length)
{
	if (folder_name == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	strncpy(folder_name, (const char *)sqlite3_column_text(stmt, 0),
		max_length);

	return 0;
}

int
mb_svc_load_record_web_streaming(sqlite3_stmt *stmt,
				 mb_svc_web_streaming_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	record->_id = sqlite3_column_int(stmt, MB_SVC_WEB_STREAMING_ID);
	strncpy(record->folder_uuid,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_WEB_STREAMING_FOLDER_UUID),
		sizeof(record->folder_uuid));
	strncpy(record->title,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_WEB_STREAMING_TITLE),
		sizeof(record->title));
	record->title[sizeof(record->title) - 1] = '\0';
	record->duration =
	    sqlite3_column_int(stmt, MB_SVC_WEB_STREAMING_DURATION);
	strncpy(record->url,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_WEB_STREAMING_URL),
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(record->thumb_path,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_WEB_STREAMING_THNMB_PATH),
		MB_SVC_FILE_PATH_LEN_MAX + 1);

	return 0;
}

int
mb_svc_load_record_media(sqlite3_stmt *stmt, mb_svc_media_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	strncpy(record->media_uuid,
		(const char *)sqlite3_column_text(stmt, MB_SVC_MEDIA_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	strncpy(record->folder_uuid,
		(const char *)sqlite3_column_text(stmt, MB_SVC_MEDIA_FOLDER_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	strncpy(record->path,
		(const char *)sqlite3_column_text(stmt, MB_SVC_MEDIA_PATH),
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(record->display_name,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_MEDIA_DISPLAY_NAME),
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	record->content_type =
	    sqlite3_column_int(stmt, MB_SVC_MEDIA_CONTENT_TYPE);
	record->rate = sqlite3_column_int(stmt, MB_SVC_MEDIA_RATING);
	record->modified_date =
	    sqlite3_column_int(stmt, MB_SVC_MEDIA_MODIFIED_DATE);
	strncpy(record->thumbnail_path,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_MEDIA_THUMBNAIL_PATH),
		MB_SVC_FILE_PATH_LEN_MAX + 1);
	strncpy(record->http_url,
		(const char *)sqlite3_column_text(stmt, MB_SVC_MEDIA_HTTP_URL),
		MB_SVC_DIR_PATH_LEN_MAX + 1);

	record->size = sqlite3_column_int(stmt, MB_SVC_MEDIA_SIZE);

	return 0;
}

int
mb_svc_load_record_video_meta(sqlite3_stmt *stmt,
			      mb_svc_video_meta_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	record->_id = sqlite3_column_int(stmt, MB_SVC_VIDEO_META_ID);
	strncpy(record->media_uuid,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_MEDIA_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	strncpy(record->album,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_ALBUM),
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(record->artist,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_ARTIST),
		MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(record->title,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_TITLE),
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(record->genre,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_GENRE),
		MB_SVC_FILE_NAME_LEN_MAX + 1);
	strncpy(record->description,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_DESCRIPTION),
		MB_SVC_ARRAY_LEN_MAX + 1);
	strncpy(record->youtube_category,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_VIDEO_META_YOUTUBE_CATEGORY),
		MB_SVC_ARRAY_LEN_MAX + 1);
	record->last_played_time =
	    sqlite3_column_int(stmt, MB_SVC_VIDEO_META_BOOKMARK_LAST_PLAYED);
	record->duration = sqlite3_column_int(stmt, MB_SVC_VIDEO_META_DURATION);
	record->longitude =
	    sqlite3_column_double(stmt, MB_SVC_VIDEO_META_LONGISTUDE);
	record->latitude =
	    sqlite3_column_double(stmt, MB_SVC_VIDEO_META_LATITUDE);
	record->width = sqlite3_column_int(stmt, MB_SVC_VIDEO_META_LATITUDE);
	record->height = sqlite3_column_int(stmt, MB_SVC_VIDEO_META_LATITUDE);
	record->datetaken =
	    sqlite3_column_int(stmt, MB_SVC_VIDEO_META_LATITUDE);

	return 0;
}

int
mb_svc_load_record_image_meta(sqlite3_stmt *stmt,
			      mb_svc_image_meta_record_s *record)
{
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	record->_id = sqlite3_column_int(stmt, MB_SVC_IMAGE_META_ID);
	strncpy(record->media_uuid,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_IMAGE_META_MEDIA_UUID),
		MB_SVC_UUID_LEN_MAX + 1);
	record->longitude =
	    sqlite3_column_double(stmt, MB_SVC_IMAGE_META_LONGISTUDE);
	record->latitude =
	    sqlite3_column_double(stmt, MB_SVC_IMAGE_META_LATITUDE);
	strncpy(record->description,
		(const char *)sqlite3_column_text(stmt,
						  MB_SVC_IMAGE_META_DESCRIPTION),
		MB_SVC_ARRAY_LEN_MAX + 1);
	record->width = sqlite3_column_int(stmt, MB_SVC_IMAGE_META_WIDTH);
	record->height = sqlite3_column_int(stmt, MB_SVC_IMAGE_META_HEIGHT);
	record->orientation =
	    sqlite3_column_int(stmt, MB_SVC_IMAGE_META_ORIENTATION);
	record->datetaken =
	    sqlite3_column_int(stmt, MB_SVC_IMAGE_META_DATETAKEN);

	return 0;
}

int mb_svc_delete_record_media(MediaSvcHandle *mb_svc_handle, const char *folder_id, char *display_name)
{
	int err = -1;
	char *query_string = NULL;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[MB_SVC_TABLE_MEDIA].table_name);

	query_string = sqlite3_mprintf(MB_SVC_DELETE_MEDIA_BY_FOLDER_ID_AND_DISPLAY_NAME,
									table_name, folder_id, display_name);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to delete record\n");
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int mb_svc_delete_record_folder_sql(const char *folder_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_RECORD_FOLDER_DELETE_BY_UUID,
							mb_svc_tbl[MB_SVC_TABLE_FOLDER].table_name,
							folder_id);

	return 0;
}

int mb_svc_delete_record_media_sql(const char *media_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_RECORD_MEDIA_DELETE_BY_UUID,
							mb_svc_tbl[MB_SVC_TABLE_MEDIA].table_name,
							media_id);

	return 0;
}

int mb_svc_delete_record_image_meta_sql(const char *media_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID,
							mb_svc_tbl[MB_SVC_TABLE_IMAGE_META].table_name,
							media_id);

	return 0;
}


int mb_svc_delete_record_video_meta_sql(const char *media_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID,
							mb_svc_tbl[MB_SVC_TABLE_VIDEO_META].table_name,
							media_id);

	return 0;
}

int
mb_svc_delete_tagmap_by_media_id_sql(const char *media_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID,
							mb_svc_tbl[MB_SVC_TABLE_TAG_MAP].table_name,
							media_id);

	return 0;
}

int mb_svc_insert_record_tag(MediaSvcHandle *mb_svc_handle, mb_svc_tag_record_s *record)
{
	mb_svc_debug("");

	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_TAG;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	record->_id = __mb_svc_db_get_next_id(mb_svc_handle, MB_SVC_TABLE_TAG);

	if (record->_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_TAG_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_TAG_ID].field_name,
			    mb_svc_field[MB_SVC_TAG_NAME].field_name,
			    record->_id, record->tag_name);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting tag table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_insert_record_tag_map(MediaSvcHandle *mb_svc_handle, const char *media_uuid, int tag_id)
{
	mb_svc_debug("");

	int err = -1;
	mb_svc_tbl_field_s *mb_svc_field;
	char *query_string = NULL;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_TAG_MAP;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	int _id = __mb_svc_db_get_next_id(mb_svc_handle, MB_SVC_TABLE_TAG_MAP);

	if (_id < 0) {
		mb_svc_debug("__mb_svc_db_get_next_id failed");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	query_string =
	    sqlite3_mprintf(MB_SVC_INSERT_TAG_MAP_TABLE,
			    mb_svc_tbl[field_seq].table_name,
			    mb_svc_field[MB_SVC_TAG_MAP_ID].field_name,
			    mb_svc_field[MB_SVC_TAG_MAP_MEDIA_UUID].field_name,
			    mb_svc_field[MB_SVC_TAG_MAP_TAG_ID].field_name, _id,
			    media_uuid, tag_id);

	mb_svc_debug("Query : %s", query_string);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mb_svc_debug("Inserting tag map table failed\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return err;
}

int mb_svc_load_record_tag(sqlite3_stmt *stmt, mb_svc_tag_record_s *record)
{
	mb_svc_debug("mb_svc_load_record_tag--enter\n");
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}
	record->_id = sqlite3_column_int(stmt, MB_SVC_TAG_ID);
	strncpy(record->tag_name,
		(const char *)sqlite3_column_text(stmt, MB_SVC_TAG_NAME),
		MB_SVC_ARRAY_LEN_MAX + 1);

	mb_svc_debug("mb_svc_load_record_tag--leave\n");
	return 0;
}

int mb_svc_load_record_tagmap(sqlite3_stmt *stmt, mb_svc_tag_record_s *record)
{
	mb_svc_debug("mb_svc_load_record_tag--enter\n");
	if (record == NULL) {
		mb_svc_debug("record pointer is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	record->_id = sqlite3_column_int(stmt, MB_SVC_TAG_MAP_ID);	
	strncpy(record->media_uuid,
		(const char *)sqlite3_column_text(stmt, MB_SVC_TAG_MAP_MEDIA_UUID),
		MB_SVC_UUID_LEN_MAX + 1);

	strncpy(record->tag_name, "", MB_SVC_ARRAY_LEN_MAX + 1);

	mb_svc_debug("mb_svc_load_record_tag--leave\n");
	return 0;
}

int
mb_svc_delete_bookmark_meta_by_media_id(MediaSvcHandle *mb_svc_handle, const char *media_id, minfo_file_type content_type)
{
	int err = -1;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };
	char *tbl_name = NULL;

	if (content_type == MINFO_ITEM_IMAGE) {
		tbl_name = MB_SVC_TBL_NAME_IMAGE_META;
	} else if (content_type == MINFO_ITEM_VIDEO) {
		tbl_name = MB_SVC_TBL_NAME_VIDEO_META;
	} else {
		return MB_SVC_ERROR_INTERNAL;
	}

	/* delete video or image record */
	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s", tbl_name);

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID, table_name,
		 media_id);
	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to delete record\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	/* if video record delete, then its related bookmark should be deleted too. */
	if (content_type == MINFO_ITEM_VIDEO) {
		tbl_name = MB_SVC_TBL_NAME_BOOKMARK;
		snprintf(query_string, sizeof(query_string),
			 MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID, tbl_name,
			 media_id);
		err = mb_svc_query_sql(mb_svc_handle, query_string);
		if (err < 0) {
			mb_svc_debug("failed to delete record\n");
			mb_svc_debug("query string is %s\n", query_string);
			return MB_SVC_ERROR_DB_INTERNAL;
		}
	}

	/* still delete a tag record. */
	tbl_name = MB_SVC_TBL_NAME_TAG_MAP;
	snprintf(query_string, sizeof(query_string),
		 MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID, tbl_name,
		 media_id);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to delete record\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int
mb_svc_delete_bookmark_meta_by_media_id_sql(const char *media_id, char **sql)
{
	*sql = sqlite3_mprintf(MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID,
							mb_svc_tbl[MB_SVC_TABLE_BOOKMARK].table_name,
							media_id);

	return 0;
}

int mb_svc_update_thumb_path_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, const char *thumb_path)
{
	int err = -1;
	char *query_string = NULL;

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[MB_SVC_TABLE_MEDIA].table_name);

	query_string = sqlite3_mprintf(MB_SVC_UPDATE_MEDIA_THUMB_PATH, 
										table_name, thumb_path, media_id);

	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to update record favorite\n");
		mb_svc_debug("query string is %s\n", query_string);
		sqlite3_free(query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_free(query_string);

	return 0;
}

int mb_svc_update_favorite_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, int favorite)
{
	int err = -1;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[MB_SVC_TABLE_MEDIA].table_name);

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_UPDATE_MEDIA_FAVORITE_BY_ID, table_name, favorite,
		 media_id);
	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to update record favorite\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int mb_svc_update_date_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, int date)
{
	mb_svc_debug("");
	int err = -1;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, sizeof(table_name), "%s",
		 mb_svc_tbl[MB_SVC_TABLE_MEDIA].table_name);

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_UPDATE_MEDIA_DATE_BY_ID, table_name, date, media_id);
	err = mb_svc_query_sql(mb_svc_handle, query_string);
	if (err < 0) {
		mb_svc_debug("failed to update record date\n");
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	return 0;
}

int mb_svc_update_thumb_path_sql(const char *media_id, const char *thumb_path, char **sql)
{
	mb_svc_debug("");

	if (thumb_path == NULL) {
		mb_svc_debug("thumb_path is null\n");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	mb_svc_debug("thumb:%s, id:%d", thumb_path, media_id);

	int err = 0;
	mb_svc_tbl_field_s *mb_svc_field;
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_MEDIA;
	mb_svc_field = mb_svc_tbl[field_seq].mb_svc_field;

	*sql =
	    sqlite3_mprintf(MB_SVC_UPDATE_MEDIA_THUMB_PATH, 
						mb_svc_tbl[field_seq].table_name,
						thumb_path,
						media_id);
	
	mb_svc_debug("Query : %s", *sql);

	return err;
}

int
mb_svc_update_album_lock_status(MediaSvcHandle *mb_svc_handle, const char *folder_id, int lock,
				minfo_store_type storage_type)
{
	int err = 0;
	sqlite3_stmt *stmt = NULL;
	char query_string[MB_SVC_DEFAULT_QUERY_SIZE + 1] = { 0 };

	if (lock != 0 && lock != 1) {
		mb_svc_debug("lock status is invalid");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	mb_svc_debug
	    ("mb_svc_update_album_lock_status( folder id:%d, storage_type:%d, lock:%d",
	     folder_id, storage_type, lock);

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 MB_SVC_TBL_NAME_FOLDER);

	snprintf(query_string, sizeof(query_string),
		 MB_SVC_UPDATE_FOLDER_ALBUM_STATUS, table_name);
	mb_svc_debug("Query : %s", query_string);

	err =
	    sqlite3_prepare_v2(handle, query_string, strlen(query_string),
			       &stmt, NULL);

	if (err != SQLITE_OK) {
		mb_svc_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", query_string);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_bind_int(stmt, 1, lock);
	sqlite3_bind_text(stmt, 2, folder_id, strlen(folder_id), NULL);

	err = sqlite3_step(stmt);

	if (err != SQLITE_OK && err != SQLITE_DONE) {
		mb_svc_debug("sqlite3_step fails : %s, db error info is : %s",
			     query_string, sqlite3_errmsg(handle));

		err = MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_finalize(stmt);

	return err;
}

int
mb_svc_set_media_records_as_valid(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type,
				  int valid)
{
	mb_svc_debug("storage_type: %d", storage_type);

	/* 1. first set reords as valid/invalid in 'folder' table */
	mb_svc_tbl_name_e field_seq = MB_SVC_TABLE_FOLDER;
	sqlite3_stmt *stmt = NULL;
	int f_id = 0;
	char folder_uuid[MB_SVC_UUID_LEN_MAX + 1] = {0,};

	char table_name[MB_SVC_TABLE_NAME_MAX_LEN] = { 0, };
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);

	sqlite3 *handle = (sqlite3 *)mb_svc_handle;
	if (handle == NULL) {
		mb_svc_debug("handle is NULL");
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[field_seq].table_name);

	int err;
	char *sql =
	    sqlite3_mprintf("update %s set valid = %d where storage_type = %d;",
			    table_name, valid, storage_type);
	err = mb_svc_query_sql(mb_svc_handle, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		mb_svc_debug
		    ("To set all items as valid is failed in folder table(%d)",
		     err);
		return MB_SVC_ERROR_DB_INTERNAL;
	}
	/* 2. get all of related folder id. */
	sql =
	    sqlite3_mprintf("select folder_uuid from %s where storage_type = %d;",
			    table_name, storage_type);
	err = sqlite3_prepare_v2(handle, sql, strlen(sql), &stmt, NULL);

	mb_svc_debug("SQL : %s", sql);

	if (SQLITE_OK != err) {
		mb_svc_debug("prepare error [%s]", sqlite3_errmsg(handle));
		mb_svc_debug("query string is %s\n", sql);
		sqlite3_free(sql);
		return MB_SVC_ERROR_DB_INTERNAL;
	}

	sqlite3_free(sql);

	field_seq = MB_SVC_TABLE_MEDIA;
	memset(table_name, 0x00, MB_SVC_TABLE_NAME_MAX_LEN);
	snprintf(table_name, MB_SVC_TABLE_NAME_MAX_LEN, "%s",
		 mb_svc_tbl[field_seq].table_name);

	for (;;) {
		err = sqlite3_step(stmt);
		if (err != SQLITE_ROW) {
			break;
		}

		//f_id = sqlite3_column_int(stmt, 0);
		strncpy(folder_uuid, (const char *)sqlite3_column_text(stmt, 0), MB_SVC_UUID_LEN_MAX + 1);

		/* 3. then set every reords as valid/invalid in 'media' table */
		sql =
		    sqlite3_mprintf
		    ("update %s set valid = %d where folder_uuid = '%s';",
		     table_name, valid, folder_uuid);
		err = mb_svc_query_sql(mb_svc_handle, sql);
		sqlite3_free(sql);
		if (err != SQLITE_OK) {
			mb_svc_debug
			    ("To set all items as valid is failed in folder[%d]",
			     f_id);
			sqlite3_finalize(stmt);
			return MB_SVC_ERROR_DB_INTERNAL;
		}
	}

	sqlite3_finalize(stmt);

	return 0;
}
