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
 * This file defines global data structrues and micros of media service.
 *
 * @file       	media-svc-structures.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines global data structrue and micros of internal media service.
 */
 
 /**
 * @ingroup MEDIA_SVC
 * @defgroup MEDIA_SVC_TYPE global variables
 * @{
 */

#ifndef _MEDIA_SVC_STRUCTURES_H_
#define _MEDIA_SVC_STRUCTURES_H_

#include <sqlite3.h>
#include "visual-svc-types.h"


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define MB_SVC_UUID_LEN_MAX			36
#define MB_SVC_FILE_NAME_LEN_MAX 		255 * 3									/**< File name max length */
#define MB_SVC_FILE_PATH_LEN_MAX 		4095 * 2								/**< File path max length  */
#define MB_SVC_DIR_NAME_LEN_MAX 		MB_SVC_FILE_NAME_LEN_MAX				/**< Directory name max length*/
#define MB_SVC_DIR_PATH_LEN_MAX 		MB_SVC_FILE_PATH_LEN_MAX				/**< Directory path max length */
#define MB_SVC_FILE_EXT_LEN_MAX      	6 										/**< file extention max length */
#define MB_SVC_ARRAY_LEN_MAX			255										/**< array max length */
#define MB_SVC_NO_RECORD_ANY_MORE	1

#define USEC_PER_SEC 					1000000    // 1s = 1000000us

/*
* @struct mb_svc_iterator_s
* This structure defines the mb_svc_iterator_s.
*/
typedef struct {
    sqlite3_stmt* stmt ;		/**< statement */
    int total_count;            /**< total count */
    int current_position;       /**< current count */
}mb_svc_iterator_s;


/* media svc item structure definitions */

/*
* @struct mb_svc_bookmark_record_s
* This structure defines the mb_svc_bookmark_record_s.
*/
typedef struct {
	int _id;     	                                 		 			/**< Bookmark id.  Prime Key */
	char media_uuid[MB_SVC_UUID_LEN_MAX+1];			/**< media UUID */
	int marked_time;	   									/**< Bookmark time*/
	char thumbnail_path[MB_SVC_FILE_PATH_LEN_MAX+1];   	/**< thumbnail path */
}mb_svc_bookmark_record_s;


/* DB record of folder, usecase: Album view */

/*
* @struct mb_svc_folder_record_s
* This structure defines the mb_svc_folder_record_s.
*/
typedef struct {
	char uuid[MB_SVC_UUID_LEN_MAX+1];					/**< UUID */
	char uri[MB_SVC_DIR_PATH_LEN_MAX+1];				/**< folder path */
	char display_name[MB_SVC_FILE_NAME_LEN_MAX+1] ;		/**< folder name */
	int modified_date;									/**< The modified time of folder */
	char web_account_id[MB_SVC_ARRAY_LEN_MAX+1];         	/**< web count ID in web streaming table */
	char web_album_id[MB_SVC_ARRAY_LEN_MAX+1];         	/**< web count ID in web streaming table */
	minfo_store_type storage_type;             				/**< file storage type, in phone, mmc, web, or web streaming*/
	int sns_type;											/**< web account type */
	int lock_status;         									/**< status for album lock */
}mb_svc_folder_record_s;


/* DB record of web_streaming, usecase: streaming album view */

/*
* @struct mb_svc_web_streaming_record_s
* This structure defines the mb_svc_web_streaming_record_s.
*/
typedef struct {
	int _id;											/**< web streaming id.  Prime Key */
	char folder_uuid[MB_SVC_UUID_LEN_MAX+1];		/**< folder UUID */
	char title[MB_SVC_FILE_NAME_LEN_MAX+1];			/**< title of web streaming*/
	int duration;										/**< duration of web streaming */
	char url[MB_SVC_FILE_PATH_LEN_MAX+1];			/**< url  */
	char thumb_path[MB_SVC_FILE_PATH_LEN_MAX+1];	/**< thumbnail path */
}mb_svc_web_streaming_record_s;


/* DB record of media */

/*
* @struct mb_svc_media_record_s
* This structure defines the mb_svc_media_record_s.
*/
typedef struct {
	char media_uuid[MB_SVC_UUID_LEN_MAX+1];				/**< UUID */
	char path[MB_SVC_FILE_PATH_LEN_MAX+1];    			/**< path */
	char folder_uuid[MB_SVC_UUID_LEN_MAX+1];			/**< folder UUID */
	char display_name[MB_SVC_FILE_NAME_LEN_MAX+1];		/**< media name */
	minfo_file_type content_type;						/**< media type,   1:image, 2:video*/
	bool rate;											/**< favoriate option */
	int modified_date;                              	/**< modified time */
	char thumbnail_path[MB_SVC_FILE_PATH_LEN_MAX+1];	/**< thumbail path */
	char http_url[MB_SVC_DIR_PATH_LEN_MAX +1];			/**< http url */
	int size;											/**< file size */
}mb_svc_media_record_s;


/*	DB record of video meta */

/*
* @struct mb_svc_video_meta_record_s
* This structure defines the mb_svc_video_meta_record_s.
*/
typedef struct {
	int _id;										/**< video meta id.  Prime Key */
	char media_uuid[MB_SVC_UUID_LEN_MAX+1];			/**< media UUID */
	char album[MB_SVC_FILE_NAME_LEN_MAX+1];			/**< album */
	char artist[MB_SVC_ARRAY_LEN_MAX+1];			/**< artist */
	char title[MB_SVC_FILE_NAME_LEN_MAX+1];			/**< title */
	char genre[MB_SVC_FILE_NAME_LEN_MAX+1];			/**< genre */
	char description[MB_SVC_ARRAY_LEN_MAX+1];		/**< description */
	char youtube_category[MB_SVC_ARRAY_LEN_MAX+1];	/**< youtube_category */
	int last_played_time;							/**< last palyed time*/
	int duration;									/**< duration */
	double longitude;								/**< longitude */
	double latitude;								/**< latitude */
	int width;										/**< width */
	int height;										/**< height */
	int datetaken;									/**< datetaken */
}mb_svc_video_meta_record_s;



/*	DB record of video_bookmark, usecase: bookmark in the video player. */

/*
* @struct mb_svc_image_meta_record_s
* This structure defines the mb_svc_image_meta_record_s.
*/
typedef struct {
	int _id;										/**< video meta id.  Prime Key */
	char media_uuid[MB_SVC_UUID_LEN_MAX+1];	/**< media UUID */
	double longitude;								/**< longitude */
	double latitude;                            /**< latitude */
	char description[MB_SVC_ARRAY_LEN_MAX+1];   /**< description */ 
	int width;												/**< width */
	int height;												/**< height */
	int orientation;											/**< orientation */
	int datetaken;											/**< datetaken */
}mb_svc_image_meta_record_s;


/*
* @struct mb_svc_tag_record_s
* This structure defines the mb_svc_tag_record_s.
*/
typedef struct {
	int _id;                                    /**< video meta id.  Prime Key */
	char media_uuid[MB_SVC_UUID_LEN_MAX+1];		/**< media UUID */
	char tag_name[MB_SVC_ARRAY_LEN_MAX+1];		/**< description */ 
}mb_svc_tag_record_s;


#ifdef __cplusplus
}
#endif /*__cplusplus*/


/**
* @}
*/


#endif /*_MEDIA_SVC_STRUCTURES_H_*/
