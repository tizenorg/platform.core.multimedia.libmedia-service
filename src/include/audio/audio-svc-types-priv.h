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

#include "audio-svc-types.h"

#ifndef _AUDIO_SVC_TYPES_PRIV_H_
#define _AUDIO_SVC_TYPES_PRIV_H_

#define AUDIO_SVC_TAG_UNKNOWN				"Unknown"
#define AUDIO_SVC_MEDIA_PATH					"/opt/data/file-manager-service"			/**<  Media path*/
#define AUDIO_SVC_THUMB_PATH_PREFIX			AUDIO_SVC_MEDIA_PATH"/.thumb"			/**< Thumbnail path prefix*/
#define AUDIO_SVC_THUMB_PHONE_PATH 			AUDIO_SVC_THUMB_PATH_PREFIX"/phone"	/**<  Phone thumbnail path*/
#define AUDIO_SVC_THUMB_MMC_PATH 			AUDIO_SVC_THUMB_PATH_PREFIX"/mmc"		/**<  MMC thumbnail path*/
#define AUDIO_SVC_QUERY_SIZE					4096									/**<  Length of db query */

/**
 * DB table information
 */
#define AUDIO_SVC_DB_TABLE_AUDIO						"audio_media"			/**<  audio_media table. (old mp_music_phone table)*/
#define AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS			"audio_playlists"			/**<  audio_playlists table*/
#define AUDIO_SVC_DB_TABLE_AUDIO_PLAYLISTS_MAP		"audio_playlists_map"		/**<  audio_playlists_map table*/
#define AUDIO_SVC_DB_TABLE_ALBUMS					"albums"				/**<  albums table*/
#define AUDIO_SVC_DB_TABLE_ALBUM_ART					"album_art"				/**<  album_art table*/
#define AUDIO_SVC_DB_TABLE_AUDIO_FOLDER				"audio_folder"			/**<  audio_folder table*/

#define AUDIO_SVC_CONTENT_TYPE						3						/**<  image-1, video-2, audio-3 */

/**
 * Audio meta data information
 */
typedef struct {
	char		title[AUDIO_SVC_METADATA_LEN_MAX];			/**< track title*/
	char		artist[AUDIO_SVC_METADATA_LEN_MAX];		/**< artist name*/
	char		album[AUDIO_SVC_METADATA_LEN_MAX];		/**< album name*/
	char		genre[AUDIO_SVC_METADATA_LEN_MAX];		/**< genre of track*/
	char		author[AUDIO_SVC_METADATA_LEN_MAX];		/**< author name*/
	char		copyright[AUDIO_SVC_METADATA_LEN_MAX];		/**< copyright of track*/
	char		description[AUDIO_SVC_METADATA_LEN_MAX];	/**< description of track*/
	char		format[50];									/**< format of track*/
	int		duration;									/**< duration of track*/
	int		bitrate;										/**< bitrate of track*/
	char		year[AUDIO_SVC_METADATA_LEN_MAX];			/**< year of track*/
	int		track;										/**< track number*/
	int		album_rating;								/**< rating of album*/
	char		parental_rating[20];							/**< parental rating*/
} audio_svc_audio_meta_s;


/**
 * Audio data information
 */
typedef struct {
	int		storage_type;									/**< Storage of media file : internal/external */
	char		audio_uuid[AUDIO_SVC_UUID_SIZE+1];		/**< Unique ID of item */
	char		pathname[AUDIO_SVC_PATHNAME_SIZE];			/**< Full path and file name of media file */
	char		thumbname[AUDIO_SVC_PATHNAME_SIZE];		/**< Thumbnail image file path */
	int		played_count;								/**< played count */
	int		time_played;									/**< last played time */
	int		time_added;									/**< added time */
	int		rating;										/**< user defined rating */
	int		category;									/**< category. sound or music*/
	int		favourate;									/**< favourate. o or 1 */
	audio_svc_audio_meta_s		audio;					/**< meta data structure for audio files */
} audio_svc_audio_item_s;


#define SAFE_FREE(src)      { if(src) {free(src); src = NULL;}}

/**
 * Group item search result record
 */
typedef struct{
	char maininfo[AUDIO_SVC_METADATA_LEN_MAX];			/**< main info of group */
	char subinfo[AUDIO_SVC_METADATA_LEN_MAX];			/**< sub info of group */
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE];			/**< Thumbnail image file path */
	int album_rating;										/**< album rating*/
}audio_svc_group_item_s;

/**
 * List item search result record
 */
typedef struct{
	char	 audio_uuid[AUDIO_SVC_UUID_SIZE+1];			/**< Unique ID of item */
	char pathname[AUDIO_SVC_PATHNAME_SIZE];			/**< Full path and file name of media file */
	char title[AUDIO_SVC_METADATA_LEN_MAX];				/**< title of track */
	char artist[AUDIO_SVC_METADATA_LEN_MAX];			/**< artist of track */
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE];			/**< Thumbnail image file path */
	int duration;											/**< track duration*/
	int rating;											/**< track rating*/
}audio_svc_list_item_s;

/**
 * Playlist record
 */
typedef struct{
	int playlist_id;										/**< Unique ID of playlist*/
	char name[AUDIO_SVC_METADATA_LEN_MAX];			/**< playlist name*/
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE];			/**< Thumbnail image file path */
}audio_svc_playlist_s;

typedef struct{
	int u_id;												/**< Unique ID of playlist item*/	
	char	 audio_uuid[AUDIO_SVC_UUID_SIZE+1];			/**< Unique Audio ID */
	char pathname[AUDIO_SVC_PATHNAME_SIZE];			/**< Full path and file name of media file */
	char title[AUDIO_SVC_METADATA_LEN_MAX];				/**< title of track */
	char artist[AUDIO_SVC_METADATA_LEN_MAX];			/**< artist of track */
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE];			/**< Thumbnail image file path */
	int duration;											/**< track duration*/
	int rating;											/**< track rating*/
	int play_order;										/**< Play order*/
}audio_svc_playlist_item_s;

typedef enum{
	AUDIO_SVC_QUERY_INSERT_ITEM,
	AUDIO_SVC_QUERY_SET_ITEM_VALID,
	AUDIO_SVC_QUERY_MOVE_ITEM,
}audio_svc_query_type_e;

/**
	@}
 */

#endif /*_AUDIO_SVC_TYPES_PRIV_H_*/
