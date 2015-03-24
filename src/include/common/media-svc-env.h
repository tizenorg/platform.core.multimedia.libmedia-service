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

#if 0
/**
 * Media meta data information
 */
typedef struct {
	char		title[MEDIA_SVC_METADATA_LEN_MAX];				/**< track title*/
	char		album[MEDIA_SVC_METADATA_LEN_MAX];			/**< album name*/
	char		artist[MEDIA_SVC_METADATA_LEN_MAX];			/**< artist name*/
	char		genre[MEDIA_SVC_METADATA_LEN_MAX];			/**< genre of track*/
	char		author[MEDIA_SVC_METADATA_LEN_MAX];			/**< author name*/
	char		year[MEDIA_SVC_METADATA_LEN_MAX];				/**< author name*/
	char		recorded_date[MEDIA_SVC_METADATA_LEN_MAX];		/**< recorded date*/
	char		copyright[MEDIA_SVC_METADATA_LEN_MAX];			/**< copyright*/
	char		track_num[MEDIA_SVC_METADATA_LEN_MAX];		/**< track number*/
	char		description[MEDIA_SVC_METADATA_DESCRIPTION_MAX];	/**< description*/
	int		bitrate;											/**< bitrate*/
	int		samplerate;										/**< samplerate*/
	int		channel;											/**< channel*/
	int		duration;										/**< duration*/
	float		longitude;										/**< longitude*/
	float		latitude;											/**< latitude*/
	float		altitude;											/**< altitude*/
	int 		width;											/**< width*/
	int 		height;											/**< height*/
	char		datetaken[MEDIA_SVC_METADATA_LEN_MAX];		/**< datetaken*/
	char		timetaken[MEDIA_SVC_METADATA_LEN_MAX];		/**< timetaken*/
	int 		orientation;										/**< orientation*/
	int		rating;											/**< user defined rating */
} media_svc_content_meta_s;


/**
 * Media data information
 */
typedef struct {
	char		media_uuid[MEDIA_SVC_UUID_SIZE+1];			/**< Unique ID of item */
	char		path[MEDIA_SVC_PATHNAME_SIZE];				/**< Full path and file name of media file */
	char		file_name[MEDIA_SVC_PATHNAME_SIZE];			/**< Full path and file name of media file */
	int		media_type;									/**< Type of media file : internal/external */
	char		mime_type[MEDIA_SVC_PATHNAME_SIZE];			/**< Full path and file name of media file */
	int 		size;
	int		added_time;									/**< added time */
	int		modified_time;								/**< modified time */
	char		folder_uuid[MEDIA_SVC_UUID_SIZE+1];			/**< Unique ID of folder */
	int		album_id;									/**< Unique ID of album */
	char		thumbnail_path[MEDIA_SVC_PATHNAME_SIZE];		/**< Thumbnail image file path */
	int		played_count;								/**< played count */
	int		last_played_time;									/**< last played time */
	int		last_played_position;
	int		favourate;									/**< favourate. o or 1 */
	int 		hiding;										/**< hiding. o or 1 */
	int 		is_drm;										/**< is_drm. o or 1 */
	int		storage_type;									/**< Storage of media file : internal/external */
	media_svc_content_meta_s		media_meta;					/**< meta data structure for audio files */
} media_svc_content_info_s;
#else
/**
 * Media meta data information
 */
typedef struct {
	char	*	title;				/**< track title*/
	char	*	album;				/**< album name*/
	char	*	artist;				/**< artist name*/
	char	*	album_artist;		/**< artist name*/
	char	*	genre;				/**< genre of track*/
	char	*	composer;			/**< composer name*/
	char	*	year;				/**< year*/
	char	*	recorded_date;		/**< recorded date*/
	char	*	copyright;			/**< copyright*/
	char	*	track_num;			/**< track number*/
	char	*	description;			/**< description*/
	int		bitrate;				/**< bitrate*/
	int		samplerate;			/**< samplerate*/
	int		channel;				/**< channel*/
	int		duration;			/**< duration*/
	float		longitude;			/**< longitude*/
	float		latitude;				/**< latitude*/
	float		altitude;				/**< altitude*/
	int 		width;				/**< width*/
	int 		height;				/**< height*/
	char	*	datetaken;			/**< datetaken*/
	int		orientation;			/**< orientation*/
	int		rating;				/**< user defined rating */
	char	*	weather;				/**< weather of image */
	int		bitpersample;				/**< bitrate*/

	char	*	file_name_pinyin;				/**< pinyin for file_name*/
	char	*	title_pinyin;					/**< pinyin for title*/
	char	*	album_pinyin;				/**< pinyin for album*/
	char	*	artist_pinyin;					/**< pinyin for artist*/
	char	*	album_artist_pinyin;			/**< pinyin for album_artist*/
	char	*	genre_pinyin;					/**< pinyin for genre*/
	char	*	composer_pinyin;				/**< pinyin for composer*/
	char	*	copyright_pinyin;				/**< pinyin for copyright*/
	char	*	description_pinyin;			/**< pinyin for description*/
} media_svc_content_meta_s;


/**
 * Media data information
 */
typedef struct {
	char	*	media_uuid;					/**< Unique ID of item */
	char	*	path;						/**< Full path of media file */
	char	*	file_name;					/**< File name of media file. Display name */
	char	*	file_name_pinyin;				/**< File name pinyin of media file. Display name */
	int		media_type;					/**< Type of media file : internal/external */
	char	*	mime_type;					/**< Full path and file name of media file */
	unsigned long long	size;							/**< size */
	time_t	added_time;					/**< added time, time_t */
	time_t	modified_time;				/**< modified time, time_t */
	time_t	timeline;					/**< timeline of media, time_t */
	char	*	folder_uuid;					/**< Unique ID of folder */
	int		album_id;					/**< Unique ID of album */
	char	*	thumbnail_path;				/**< Thumbnail image file path */
	int		played_count;				/**< played count */
	int		last_played_time;				/**< last played time */
	int		last_played_position;			/**< last played position */
	int		favourate;					/**< favourate. o or 1 */
	int		is_drm;						/**< is_drm. o or 1 */
	int		sync_status;						/**< sync_status  */
	int		storage_type;					/**< Storage of media file : internal/external */
	media_svc_content_meta_s	media_meta;	/**< meta data structure for audio files */
} media_svc_content_info_s;
#endif

typedef enum{
	MEDIA_SVC_QUERY_INSERT_ITEM,
	MEDIA_SVC_QUERY_SET_ITEM_VALIDITY,
	MEDIA_SVC_QUERY_MOVE_ITEM,
} media_svc_query_type_e;

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_ENV_H_*/
