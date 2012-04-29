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



#ifndef _AUDIO_SVC_TYPES_H_
#define _AUDIO_SVC_TYPES_H_

/**
	@addtogroup AUDIO_SVC
	@{
		 * @file			audio-svc-types.h
		 * @brief		This file defines various types and macros of audio service.

 */

/**
        @addtogroup AUDIO_SVC_COMMON
        @{
*/


#define AUDIO_SVC_METADATA_LEN_MAX			193						/**<  Length of metadata*/
#define AUDIO_SVC_FILENAME_SIZE				1024 					/**<  Length of File name.*/
#define AUDIO_SVC_PATHNAME_SIZE				4096					/**<  Length of Path name. */
#define AUDIO_SVC_PLAYLIST_NAME_SIZE			101						/**<  Length of palylist name*/
#define AUDIO_SVC_FAVORITE_LIST_ID				0						/**< The index for favorite list*/
#define AUDIO_SVC_UUID_SIZE		    				36 						/**< Length of UUID*/

/**
 * Handle type
 */
typedef int AudioHandleType;		/**< Handle type */


/**
 * Type definition for storage_type
 */
typedef enum{
	AUDIO_SVC_STORAGE_PHONE,			/**< Phone storage*/
	AUDIO_SVC_STORAGE_MMC,			/**< MMC storage*/
}audio_svc_storage_type_e;

/**
 * Type definition for category
 */
typedef enum{
	AUDIO_SVC_CATEGORY_MUSIC,		/**< Music Category*/
	AUDIO_SVC_CATEGORY_SOUND		/**< Sound Category*/
}audio_svc_category_type_e;

/**
 * Type definition for rating
 */
typedef enum{
	AUDIO_SVC_RATING_NONE,			/**< No rating or Rating 0*/
	AUDIO_SVC_RATING_1,				/**< Rating 1*/
	AUDIO_SVC_RATING_2,				/**< Rating 2*/
	AUDIO_SVC_RATING_3,				/**< Rating 3*/
	AUDIO_SVC_RATING_4,				/**< Rating 4*/
	AUDIO_SVC_RATING_5,				/**< Rating 5*/
}audio_svc_rating_type_e;

/**
	@}
 */

/**
        @addtogroup AUDIO_SVC_GROUP_API
        @{
*/

/**
 * Type definition for group
 */
typedef enum{
	AUDIO_SVC_GROUP_BY_ALBUM,					/**< Group by album*/
	AUDIO_SVC_GROUP_BY_ARTIST,					/**< Group by artist*/
	AUDIO_SVC_GROUP_BY_ARTIST_ALBUM,			/**< Group by album which has special artist condition*/
	AUDIO_SVC_GROUP_BY_GENRE,					/**< Group by genre*/
	AUDIO_SVC_GROUP_BY_GENRE_ARTIST,				/**< Group by artist which has special genre condition*/
	AUDIO_SVC_GROUP_BY_GENRE_ALBUM,				/**< Group by album which has special genre condition*/
	AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM,		/**< Group by album which has special genre and artist condirion*/
	AUDIO_SVC_GROUP_BY_FOLDER,					/**< Group by folder*/
	AUDIO_SVC_GROUP_BY_YEAR,						/**< Group by year*/
	AUDIO_SVC_GROUP_BY_COMPOSER				/**< Group by author*/
}audio_svc_group_type_e;


/**
 * Type definition for tracks
 */
typedef enum{
	AUDIO_SVC_TRACK_ALL,							/**< All tracks*/
	AUDIO_SVC_TRACK_BY_ALBUM,					/**< Album tracks*/
	AUDIO_SVC_TRACK_BY_ARTIST_ALBUM,				/** < Albums which has special artist condition */
	AUDIO_SVC_TRACK_BY_ARTIST,					/**< Artist tracks*/
	AUDIO_SVC_TRACK_BY_ARTIST_GENRE,				/**< Genre tracks which has special artist condition*/
	AUDIO_SVC_TRACK_BY_GENRE,						/**< Genre tracks*/
	AUDIO_SVC_TRACK_BY_FOLDER,					/**< Genre tracks*/
	AUDIO_SVC_TRACK_BY_YEAR,						/**< Year tracks*/
	AUDIO_SVC_TRACK_BY_COMPOSER,				/**< Author tracks*/
	AUDIO_SVC_TRACK_BY_TOPRATING,				/**< Toprating tracks*/
	AUDIO_SVC_TRACK_BY_PLAYED_TIME,				/**< Recently played tracks*/
	AUDIO_SVC_TRACK_BY_ADDED_TIME,				/**< Recently added tracks*/
	AUDIO_SVC_TRACK_BY_PLAYED_COUNT,				/**< Most played tracks*/
	AUDIO_SVC_TRACK_BY_PLAYLIST,					/**< User playlist tracks*/
}audio_svc_track_type_e;

/**
 * Type definition for group data
 */
typedef enum{
	AUDIO_SVC_GROUP_ITEM_MAIN_INFO,				/**< The main group info for the list*/
	AUDIO_SVC_GROUP_ITEM_SUB_INFO,				/**< The sub group info for the list*/
	AUDIO_SVC_GROUP_ITEM_THUMBNAIL_PATH,		/**< Thumbnail path of first item in the group */
	AUDIO_SVC_GROUP_ITEM_RATING,					/**< Album rating*/
}audio_svc_group_item_type_e;

/**
 * Type definition for list data (track meta data)
 */
typedef enum{
	AUDIO_SVC_LIST_ITEM_AUDIO_ID,					/**< Unique media file index*/
	AUDIO_SVC_LIST_ITEM_PATHNAME,				/**< Full path and file name of media file*/
	AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH,			/**< Thumbnail path of first item in the group */
	AUDIO_SVC_LIST_ITEM_TITLE,						/**< Title of media file */
	AUDIO_SVC_LIST_ITEM_ARTIST,					/**< Artist of media file */
	AUDIO_SVC_LIST_ITEM_DURATION,					/**< Duration of media file*/
	AUDIO_SVC_LIST_ITEM_RATING,					/**< The rating used in mtp*/
	AUDIO_SVC_LIST_ITEM_ALBUM,					/**< Album of media file*/
}audio_svc_list_item_type_e;

/**
 * Type definition of track meta data for playlist
 */

typedef enum{
	AUDIO_SVC_PLAYLIST_ITEM_UID,						/**< Unique index of playlist item*/
	AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID,					/**< Unique media file index*/
	AUDIO_SVC_PLAYLIST_ITEM_PATHNAME,				/**< Full path and file name of media file*/
	AUDIO_SVC_PLAYLIST_ITEM_THUMBNAIL_PATH,			/**< Thumbnail path of first item in the group */
	AUDIO_SVC_PLAYLIST_ITEM_TITLE,						/**< Title of media file */
	AUDIO_SVC_PLAYLIST_ITEM_ARTIST,					/**< Artist of media file */
	AUDIO_SVC_PLAYLIST_ITEM_DURATION,				/**< Duration of media file*/
	AUDIO_SVC_PLAYLIST_ITEM_RATING,					/**< The rating used in mtp*/
	AUDIO_SVC_PLAYLIST_ITEM_PLAY_ORDER,				/**	< Play order of media file*/
}audio_svc_playlist_item_type_e;

/**
	@}
 */

/**
        @addtogroup AUDIO_SVC_ITEM_API
        @{
*/

/**
 * Type definition for track meta data
 */
typedef enum{
	AUDIO_SVC_TRACK_DATA_STORAGE,				/**< Storage of media file : internal/external*/
	AUDIO_SVC_TRACK_DATA_AUDIO_ID,				/**< Unique media file index*/
	AUDIO_SVC_TRACK_DATA_PATHNAME,				/**< Full path and file name of media file*/
	AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH,		/**< Thumbnail image file path*/
	AUDIO_SVC_TRACK_DATA_PLAYED_COUNT,			/**< Played count*/
	AUDIO_SVC_TRACK_DATA_PLAYED_TIME,			/**< Last played time*/
	AUDIO_SVC_TRACK_DATA_ADDED_TIME,			/**< Added time*/
	AUDIO_SVC_TRACK_DATA_RATING,					/**< User defined rating*/
	AUDIO_SVC_TRACK_DATA_CATEGORY,				/**< Category : Music/Sound*/
	AUDIO_SVC_TRACK_DATA_TITLE,					/**< Track title*/
	AUDIO_SVC_TRACK_DATA_ARTIST,					/**< Artist name*/
	AUDIO_SVC_TRACK_DATA_ALBUM,					/**< Album name*/
	AUDIO_SVC_TRACK_DATA_GENRE,					/**< Genre of track*/
	AUDIO_SVC_TRACK_DATA_AUTHOR,				/**< Author name*/
	AUDIO_SVC_TRACK_DATA_COPYRIGHT,				/**< Copyright of track*/
	AUDIO_SVC_TRACK_DATA_DESCRIPTION,			/**< Description of track*/
	AUDIO_SVC_TRACK_DATA_FORMAT,				/**< Format of track*/
	AUDIO_SVC_TRACK_DATA_DURATION,				/**< Duration of track*/
	AUDIO_SVC_TRACK_DATA_BITRATE,				/**< Bitrate of track*/
	AUDIO_SVC_TRACK_DATA_YEAR,					/**< Year of track*/
	AUDIO_SVC_TRACK_DATA_TRACK_NUM,			/**< Trac number*/
	AUDIO_SVC_TRACK_DATA_ALBUM_RATING,			/**< Rating of Album*/
	AUDIO_SVC_TRACK_DATA_FAVOURATE	,			/**< Favourate of media file*/
}audio_svc_track_data_type_e;
/**
	@}
 */

/**
        @addtogroup AUDIO_SVC_PLST_API
        @{
*/


/**
 * Type definition for track meta data
 */
typedef enum{
	AUDIO_SVC_PLAYLIST_ID,						/**< Playlist ID*/
	AUDIO_SVC_PLAYLIST_NAME,					/**< Playlist Name*/
	AUDIO_SVC_PLAYLIST_THUMBNAIL_PATH		/**< Thumbnail path of first item in the playlist */
}audio_svc_playlist_e;

/**
 * Type definition for search field
 */
typedef enum {
	AUDIO_SVC_SEARCH_TITLE,					/**< Track title*/
	AUDIO_SVC_SEARCH_ALBUM,					/**< Album name*/
	AUDIO_SVC_SEARCH_ARTIST,				/**< Artist name*/
	AUDIO_SVC_SEARCH_GENRE,					/**< Genre of track*/
	AUDIO_SVC_SEARCH_AUTHOR					/**< Author name*/
} audio_svc_serch_field_e;

/**
 * Type definition for order field
 */
typedef enum {
	AUDIO_SVC_ORDER_BY_TITLE_DESC,					/**< Title descending */
	AUDIO_SVC_ORDER_BY_TITLE_ASC,					/**< Title ascending */
	AUDIO_SVC_ORDER_BY_ALBUM_DESC,					/**< Album descending*/
	AUDIO_SVC_ORDER_BY_ALBUM_ASC,					/**< Album ascending*/
	AUDIO_SVC_ORDER_BY_ARTIST_DESC,					/**< Artist descending*/
	AUDIO_SVC_ORDER_BY_ARTIST_ASC,					/**< Artist ascending*/
	AUDIO_SVC_ORDER_BY_GENRE_DESC,					/**< Genre descending*/
	AUDIO_SVC_ORDER_BY_GENRE_ASC,					/**< Genre ascending*/
	AUDIO_SVC_ORDER_BY_AUTHOR_DESC,					/**< Author descending*/
	AUDIO_SVC_ORDER_BY_AUTHOR_ASC,					/**< Author ascending*/
	AUDIO_SVC_ORDER_BY_PLAY_COUNT_DESC,				/**< Play count descending*/
	AUDIO_SVC_ORDER_BY_PLAY_COUNT_ASC,				/**< Play count ascending*/
	AUDIO_SVC_ORDER_BY_ADDED_TIME_DESC,				/**< Added time descending*/
	AUDIO_SVC_ORDER_BY_ADDED_TIME_ASC,				/**< Added time ascending*/
} audio_svc_search_order_e;

/**
	@}
 */

/**
	@}
 */

#endif /*_AUDIO_SVC_TYPES_H_*/
