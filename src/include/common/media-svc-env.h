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

#define MEDIA_SVC_DB_TABLE_MEDIA					"media"				/**<  media table*/
#define MEDIA_SVC_DB_TABLE_FOLDER					"folder"				/**<  media_folder table*/
#define MEDIA_SVC_DB_TABLE_PLAYLIST				"playlist"				/**<  playlist table*/
#define MEDIA_SVC_DB_TABLE_PLAYLIST_MAP			"playlist_map"			/**<  playlist_map table*/
#define MEDIA_SVC_DB_TABLE_ALBUM					"album"				/**<  album table*/
#define MEDIA_SVC_DB_TABLE_TAG					"tag"				/**<  tag table*/
#define MEDIA_SVC_DB_TABLE_TAG_MAP				"tag_map"			/**<  tag_map table*/
#define MEDIA_SVC_DB_TABLE_BOOKMARK				"bookmark"			/**<  bookmark table*/
#define MEDIA_SVC_DB_TABLE_CUSTOM				"custom"				/**<  custom table*/


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

#define MEDIA_SVC_PATH_PHONE				MEDIA_ROOT_PATH_INTERNAL
#define MEDIA_SVC_PATH_MMC					MEDIA_ROOT_PATH_SDCARD

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
} media_svc_query_type_e;

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_ENV_H_*/
