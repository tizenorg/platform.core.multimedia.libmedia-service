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



#ifndef _MEDIA_SVC_ENV_H_
#define _MEDIA_SVC_ENV_H_

#include <time.h>
#include <media-util.h>
#include <tzplatform_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * DB information
 */

#define MEDIA_SVC_DB_NAME 						MEDIA_DB_NAME		/**<  media db name*/
#define LATEST_VERSION_NUMBER					2

/**
 * DB table information
 */

/**
 * Table Name
 */
#define MEDIA_SVC_DB_TABLE_MEDIA				"media"				/**<  media table*/
#define MEDIA_SVC_DB_TABLE_FOLDER				"folder"			/**<  media_folder table*/
#define MEDIA_SVC_DB_TABLE_PLAYLIST				"playlist"			/**<  playlist table*/
#define MEDIA_SVC_DB_TABLE_PLAYLIST_MAP			"playlist_map"		/**<  playlist_map table*/
#define MEDIA_SVC_DB_TABLE_ALBUM				"album"				/**<  album table*/
#define MEDIA_SVC_DB_TABLE_TAG					"tag"				/**<  tag table*/
#define MEDIA_SVC_DB_TABLE_TAG_MAP				"tag_map"			/**<  tag_map table*/
#define MEDIA_SVC_DB_TABLE_BOOKMARK				"bookmark"			/**<  bookmark table*/
#define MEDIA_SVC_DB_TABLE_CUSTOM				"custom"				/**<  custom table*/
#define MEDIA_SVC_DB_TABLE_STORAGE				"storage"			/**<  storage table*/

/**
 * View Name
 */
#define MEDIA_SVC_DB_VIEW_MEDIA					"media_view"		/**<  media_view*/
#define MEDIA_SVC_DB_VIEW_PLAYLIST				"playlist_view"		/**<  playlist_view*/
#define MEDIA_SVC_DB_VIEW_TAG					"tag_view"		/**<  tag_view*/

/**
 * Trigger Name
 */
#define MEDIA_SVC_DB_TRIGGER_FOLDER				"folder_cleanup"
#define MEDIA_SVC_DB_TRIGGER_PLAYLIST_MAP			"playlist_map_cleanup"		/**<  media to map*/
#define MEDIA_SVC_DB_TRIGGER_PLAYLIST_MAP1			"playlist_map_cleanup_1"	/**<  playlist to map*/
#define MEDIA_SVC_DB_TRIGGER_ALBUM				"album_cleanup"
#define MEDIA_SVC_DB_TRIGGER_TAG_MAP			"tag_map_cleanup"		/**<  media to map*/
#define MEDIA_SVC_DB_TRIGGER_TAG_MAP1			"tag_map_cleanup_1"		/**<  tag to map*/
#define MEDIA_SVC_DB_TRIGGER_BOOKMARK			"bookmark_cleanup"
#define MEDIA_SVC_DB_TRIGGER_STORAGE			"storage_folder_cleanup"
#define MEDIA_SVC_DB_TRIGGER_CUSTOM			"custom_cleanup"

/**
 * Trigger Name
 */
#define MEDIA_SVC_DB_COLUMN_THUMBNAIL			"thumbnail_path"
#define MEDIA_SVC_DB_COLUMN_MAP_ID				"_id"


/**
 * option
 */
#define MEDIA_SVC_DB_TYPE_TEXT					"TEXT"
#define MEDIA_SVC_DB_TYPE_INT					"INTEGER"
#define MEDIA_SVC_DB_TYPE_DOUBLE				"DOUBLE"

/**
 * Query form
 */
#define MEDIA_SVC_DB_QUERY_TABLE_WITH_UNIQUE	"CREATE TABLE IF NOT EXISTS '%s' (%s, unique(%s));"
#define MEDIA_SVC_DB_QUERY_TABLE				"CREATE TABLE IF NOT EXISTS '%s' (%s);"
#define MEDIA_SVC_DB_QUERY_INDEX				"CREATE INDEX IF NOT EXISTS %s on '%s' (%s);"
#define MEDIA_SVC_DB_QUERY_TRIGGER				"CREATE TRIGGER IF NOT EXISTS %s DELETE ON %s BEGIN DELETE FROM %s WHERE %s=old.%s;END;"
#define MEDIA_SVC_DB_QUERY_TRIGGER_WITH_COUNT	"CREATE TRIGGER IF NOT EXISTS %s DELETE ON %s BEGIN DELETE FROM %s WHERE (SELECT count(*) FROM %s WHERE %s=old.%s)=1 AND %s=old.%s;END;"
#define MEDIA_SVC_DB_QUERY_VIEW_MEDIA			"CREATE VIEW IF NOT EXISTS %s AS SELECT * from %s;"
#define MEDIA_SVC_DB_QUERY_VIEW_PLAYLIST		"CREATE VIEW IF NOT EXISTS %s AS SELECT %s FROM playlist \
												LEFT OUTER JOIN playlist_map ON playlist.playlist_id = playlist_map.playlist_id \
												LEFT OUTER JOIN media ON (playlist_map.media_uuid = media.media_uuid AND media.validity=1) \
												LEFT OUTER JOIN (SELECT count(playlist_id) as media_count, playlist_id FROM playlist_map group by playlist_id) as cnt_tbl ON (cnt_tbl.playlist_id=playlist_map.playlist_id AND media.validity=1);"
#define MEDIA_SVC_DB_QUERY_VIEW_TAG				"CREATE VIEW IF NOT EXISTS %s AS SELECT %s FROM tag \
												LEFT OUTER JOIN tag_map ON tag.tag_id=tag_map.tag_id \
												LEFT OUTER JOIN media ON (tag_map.media_uuid = media.media_uuid AND media.validity=1) \
												LEFT OUTER JOIN (SELECT count(tag_id) as media_count, tag_id FROM tag_map group by tag_id) as cnt_tbl ON (cnt_tbl.tag_id=tag_map.tag_id AND media.validity=1);"
#define MEDIA_SVC_DB_QUERY_ALTER_TABLE			"ALTER TABLE %s ADD COLUMN %s;"
#define MEDIA_SVC_DB_QUERY_DROP_VIEW			"DROP VIEW IF EXISTS %s;"



#define MEDIA_SVC_METADATA_LEN_MAX			128						/**<  Length of metadata*/
#define MEDIA_SVC_METADATA_DESCRIPTION_MAX	512						/**<  Length of description*/
#define MEDIA_SVC_PATHNAME_SIZE				4096					/**<  Length of Path name. */
#define MEDIA_SVC_UUID_SIZE		    				36 						/**< Length of UUID*/

#define MEDIA_SVC_TAG_UNKNOWN				"Unknown"
#define MEDIA_SVC_MEDIA_PATH				tzplatform_mkpath(TZ_USER_DATA, "file-manager-service")			/**<  Media path*/
#define MEDIA_SVC_THUMB_PATH_PREFIX			tzplatform_mkpath(TZ_USER_DATA, "file-manager-service/.thumb")			/**< Thumbnail path prefix*/
#define MEDIA_SVC_THUMB_INTERNAL_PATH 		tzplatform_mkpath(TZ_USER_DATA, "file-manager-service/.thumb/phone")	/**<  Phone thumbnail path*/
#define MEDIA_SVC_THUMB_EXTERNAL_PATH 		tzplatform_mkpath(TZ_USER_DATA, "file-manager-service/.thumb/mmc")		/**<  MMC thumbnail path*/
#define MEDIA_SVC_THUMB_DEFAULT_PATH		tzplatform_mkpath(TZ_USER_DATA, "file-manager-service/.thumb/thumb_default.png") /** default thumbnail */

#define MEDIA_SVC_DEFAULT_GPS_VALUE			-200			/**<  Default GPS Value*/
#define THUMB_EXT 	"jpg"

enum Exif_Orientation {
    NOT_AVAILABLE=0,
    NORMAL  =1,
    HFLIP   =2,
    ROT_180 =3,
    VFLIP   =4,
    TRANSPOSE   =5,
    ROT_90  =6,
    TRANSVERSE  =7,
    ROT_270 =8
};

typedef enum{
	MEDIA_SVC_QUERY_INSERT_ITEM,
	MEDIA_SVC_QUERY_SET_ITEM_VALIDITY,
	MEDIA_SVC_QUERY_MOVE_ITEM,
	MEDIA_SVC_QUERY_UPDATE_ITEM
} media_svc_query_type_e;

typedef enum{
	MEDIA_SVC_DB_LIST_MEDIA 		= 0,
	MEDIA_SVC_DB_LIST_FOLDER 		= 1,
	MEDIA_SVC_DB_LIST_PLAYLIST_MAP = 2,
	MEDIA_SVC_DB_LIST_PLAYLIST 	= 3,
	MEDIA_SVC_DB_LIST_ALBUM		= 4,
	MEDIA_SVC_DB_LIST_TAG_MAP 		= 5,
	MEDIA_SVC_DB_LIST_TAG 			= 6,
	MEDIA_SVC_DB_LIST_BOOKMARK 	= 7,
	MEDIA_SVC_DB_LIST_STORAGE 		= 8,
	MEDIA_SVC_DB_LIST_CUSTOM 		= 9,
	MEDIA_SVC_DB_LIST_MAX			= 10,
} media_svc_table_slist_e;

typedef struct table_inform {
	char *triggerName;
	char *viewName;
	char *eventTable;
	char *actionTable;
}table_info;

typedef struct column_inform {
	char *name;
	char *type;
	bool hasOption;
	char *option;
	int version;
	bool isIndex;
	char *indexName;
	bool isUnique;
	bool isTrigger;
	bool isView;
}column_info;


#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_ENV_H_*/
