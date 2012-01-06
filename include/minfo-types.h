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
 * This file defines structure for minfo part.
 *
 * @file       	minfo-types.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines structure for minfo part.
 */

 
 /**
  * @ingroup MINFO_SVC_API
  * @defgroup MINFO_TYPES minfo types
  * @{
  */

#ifndef _MINFO_TYPES_H_
#define _MINFO_TYPES_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef EXPORT_API
#  define EXPORT_API __attribute__ ((visibility("default")))
#endif

#ifndef DEPRECATED_API
#  define DEPRECATED_API __attribute__ ((deprecated))
#endif


//(un)favoriate item
#define MB_SVC_DEFAULT 0
#define MB_SVC_FAVORITE 1    //favorite item

#define MINFO_DEFAULT_GPS 1000.00


/**
 *@enum minfo_file_type
 * Enumerations of  minfo file type
 */

typedef enum {
	MINFO_ITEM_NONE = 0x00000000,			/**< none */
	MINFO_ITEM_IMAGE = 0x00000001,			/**< image files */
	MINFO_ITEM_VIDEO = 0x00000002,			/**< video files */
	MINFO_ITEM_ALL   = 0x00000008,				/**< all the supported media types */
}minfo_file_type;

/**
 * @enum minfo_folder_type
 * Enumerations of  folder(cluster/album) type
 */
typedef enum {
	MINFO_CLUSTER_TYPE_ALL,				/**< All type of media */
	MINFO_CLUSTER_TYPE_LOCAL_ALL,		/**< lcoal both phone and mmc */
	MINFO_CLUSTER_TYPE_LOCAL_PHONE,		/**< lcoal phone only */
	MINFO_CLUSTER_TYPE_LOCAL_MMC,		/**< lcoal mmc only */
	MINFO_CLUSTER_TYPE_WEB,				/**< web album */
	MINFO_CLUSTER_TYPE_STREAMING,		/**< streaming album */
	MINFO_CLUSTER_TYPE_MAX,				/**< Max value*/
} minfo_folder_type;

/**
* @enum minfo_media_favorite_type
* Enumerations of favorite of getting media item list.
*/
typedef enum{	
	MINFO_MEDIA_FAV_ALL,			/**< Includes all favorite and unfavorite media */
	MINFO_MEDIA_FAV_ONLY,			/**< Includes only favorite media */
	MINFO_MEDIA_UNFAV_ONLY,			/**< Includes only unfavorite media */
}minfo_media_favorite_type;

/**
* @enum minfo_media_sort_type
* Enumerations of sort of getting media item list.
*/
typedef enum{	
	MINFO_MEDIA_SORT_BY_NONE,			/**< No Sort */
	MINFO_MEDIA_SORT_BY_NAME_DESC, 		/**< Sort by display name descending */
	MINFO_MEDIA_SORT_BY_NAME_ASC, 		/**< Sort by display name ascending */
	MINFO_MEDIA_SORT_BY_DATE_DESC, 		/**< Sort by modified_date descending */
	MINFO_MEDIA_SORT_BY_DATE_ASC, 		/**< Sort by modified_date ascending */
}minfo_media_sort_type;

/**
* @enum minfo_folder_sort_type
* Enumerations of sort of getting folder item list.
*/
typedef enum{	
	MINFO_CLUSTER_SORT_BY_NONE,				/**< No Sort */
	MINFO_CLUSTER_SORT_BY_NAME_DESC, 		/**< Sort by display name descending */
	MINFO_CLUSTER_SORT_BY_NAME_ASC, 		/**< Sort by display name ascending */
	MINFO_CLUSTER_SORT_BY_DATE_DESC, 		/**< Sort by modified_date descending */
	MINFO_CLUSTER_SORT_BY_DATE_ASC, 		/**< Sort by modified_date ascending */
}minfo_folder_sort_type;

/**
* @enum minfo_store_type
* Enumerations of store type.
*/

typedef enum
{
	MINFO_PHONE,			/**< Stored only in phone */
	MINFO_MMC,				/**< Stored only in MMC */	
	MINFO_WEB,				/**< Stored only in web album */
	MINFO_WEB_STREAMING,	/**< Stored only in web streaming album */
	MINFO_SYSTEM,			/**< Stored in ALL*/
} minfo_store_type;

/**
 * @enum minfo_image_meta_field_t
 * Enumerations for image_meta field name.
 */
typedef enum {
	MINFO_IMAGE_META_LONGITUDE,         /**< image meta longitude(double) field */
	MINFO_IMAGE_META_LATITUDE,          /**< image meta latitude(double) field */
	MINFO_IMAGE_META_DESCRIPTION,		/**< image meta description(string) field */
	MINFO_IMAGE_META_WIDTH,             /**< image meta width(int) field */
	MINFO_IMAGE_META_HEIGHT,       		/**< image meta height(int) field */
	MINFO_IMAGE_META_ORIENTATION,       /**< image meta orientation(int) field */
	MINFO_VIDEO_META_DATE_TAKEN,        /**< image meta datetaken(int) field */
} minfo_image_meta_field_t;

/**
 * @enum minfo_video_meta_field_t
 * Enumerations for video_meta field name.
 */
typedef enum {	
	MINFO_VIDEO_META_ID,             /**< media medta ID field */
	MINFO_VIDEO_META_MEDIA_ID,       /**< media medta ID field */
	MINFO_VIDEO_META_ALBUM,          /**< medta album field */
	MINFO_VIDEO_META_ARTIST,         /**< medta artist field */
	MINFO_VIDEO_META_TITLE,          /**< medta title field */
	MINFO_VIDEO_META_DESCRIPTION,      /**< medta description field */
	MINFO_VIDEO_META_YOUTUBE_CATEGORY, /**< medta youtube cat field */
	MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED,  /**< medta bookmark field */
	MINFO_VIDEO_META_DURATION,              /**< medta duration field */
	MINFO_VIDEO_META_LONGISTUDE,            /**< medta longistude field */
	MINFO_VIDEO_META_LATITUDE,              /**< medta latitude field */
} minfo_video_meta_field_t;

/**
 * @enum minfo_search_field_t
 * Enumerations for field to search
 */
typedef enum {
	MINFO_SEARCH_BY_NAME = 0x00000001,       /**< media display name field */
	MINFO_SEARCH_BY_PATH = 0x00000002,       /**< media path field */
	MINFO_SEARCH_BY_HTTP_URL = 0x00000004,   /**< media http url field */
	MINFO_SEARCH_MAX = 0x00000008,   		/**< maximum */
} minfo_search_field_t;

/**
* @struct minfo_item_filter
* This structure defines filter of minfo item.
* it assumes that there are (n) records matching filter, the valid index range is from 0 to n-1,  
* so there are some limitation on start_pos and end_pos.
* start_pos, if equals -1, it gets all records, end_pos is meaningless,
* start_pos it can't set to be bigger than (n-1), its valid range is 0 to (n-2).
* end_pos, if equals -1, it gets continuous record from start_pos until last item,
* end_pos, if bigger than n-1, it's meaningless and automatically regarded as (n-1).
*
*/

typedef struct {
   minfo_file_type file_type; 			/**< Image, Video */
   minfo_media_sort_type sort_type; 	/**< sort type */
   int start_pos;						/**< first item index, start from 0*/ 
   int end_pos;							/**< last item index */
   bool with_meta;						/**< include image_meta or video_meta */
   int favorite;						/**< favourite */
}minfo_item_filter;

/**
* @struct minfo_cluster_filter
* This structure defines filter of minfo cluster.
* it assumes that there are (n) records matching filter, the valid index range is from 0 to n-1, 
* so there are some limitation on start_pos and end_pos
* start_pos, if equals -1, it gets all records, end_pos is meaningless,
* start_pos it can't set to be bigger than (n-1), its valid range is 0 to (n-2).
* end_pos, if equals -1, it gets  continuous record from start_pos until last item,
* end_pos, if bigger than n-1, it's meaningless and automatically regarded as (n-1).
*
*/

typedef struct {
   minfo_folder_type cluster_type;	/**< Local/Web/Streaming */ 
   minfo_folder_sort_type sort_type;	/**< sort type */ 
   int start_pos;						/**< first item index, start from 0 */ 
   int end_pos;							/**< last item index*/ 
}minfo_cluster_filter;

/**
* @struct minfo_tag_filter
* This structure defines filter of minfo tag.
* it assumes that there are (n) records matching filter, the valid index range is from 0 to n-1,  
* so there are some limitation on start_pos and end_pos.
* start_pos, if equals -1, it gets all records, end_pos is meaningless,
* start_pos it can't set to be bigger than (n-1), its valid range is 0 to (n-2).
* end_pos, if equals -1, it gets continuous record from start_pos until last item,
* end_pos, if bigger than n-1, it's meaningless and automatically regarded as (n-1).
*
*/

typedef struct {
   minfo_file_type file_type; 			/**< Image, Video */
   int start_pos;						/**< first item index, start from 0*/ 
   int end_pos;							/**< last item index */
   bool with_meta;						/**< include image_meta or video_meta */
}minfo_tag_filter;

/**
* @struct _Mcluster
* This structure defines _Mcluster, same with Mcluster
*/

typedef struct _Mcluster
{
	int gtype;						/**< self-defination type */
	
	/*< public >*/
	//unsigned int _id;				/**< cluster id */
	char *uuid;						/**< UUID */
	char *thumb_url;				/**< thumbnail full path */
	time_t mtime;					/**< modified time */
	int type;						/**< type */
	char *display_name;				/**< cluster name */
	int count;						/**< content count */
	int sns_type;					/**< web account type */
	char *account_id;				/**< web account */
	int lock_status;				/**< status for album lock */
	char *web_album_id;              /**< web album id */
	void* _reserved;				/**< reserved  */
}Mcluster;

/**
* @struct _Mvideo
* This structure defines _Mvideo, same with Mvideo
*/

typedef struct _Mvideo
{
	int gtype;

	char *album_name;					/**< album name */
	char *artist_name;					/**< artist name */
	char *title;						/**< title */
	unsigned int last_played_pos;		/**< last played position */
	unsigned int duration;				/**< duration */
	char *web_category;					/**< web category */
	GList* bookmarks;				/**< bookmark info */
	void* _reserved;					/**< reserved */
}Mvideo;

/**
* @struct _Mimage
* This structure defines _Mimage, same with Mimage
*/

typedef struct _Mimage
{
	int gtype;
	
	int 	orientation;			/**< orientation */
	void* _reserved;				/**< reserved */
	
}Mimage;


/**
* @struct _Mmeta
* This structure defines _Mmeta, same with Mmeta
*/

typedef struct _Mmeta
{
	int gtype;

	//int item_id;					/**< media id */
	char *media_uuid;				/**< media UUID */
	int type;						/**< type */
	char *description;				/**< description */
	double	longitude;				/**< longitude */
	double	latitude;				/**< latitude */

	int width;						/**< width */
	int height;						/**< height */
	int datetaken;					/**< datetaken */

	union
	{
		Mimage* image_info;			/**< image info */
		Mvideo* video_info;			/**< video info */
	};
	void* _reserved;				/**< reserve */
}Mmeta;

/**
* @struct _Mitem
* This structure defines _Mitem, same with Mitem
*/

typedef struct _Mitem
{
	int gtype;
	
	//unsigned int _id;					/**< item id */
	char *uuid;							/**< UUID */
	int type;							/**< file type */
	char *thumb_url;					/**< thumbnail full path */	
	char *file_url;						/**< file full path */
	time_t mtime;						/**< modified time */
	char *ext;							/**< ext */	
	//unsigned int cluster_id;			/**< cluster id */
	char *cluster_uuid;					/**< cluster UUID */
	char *display_name;					/**< item name */
	int rate;							/**< favorite level */
	Mmeta* meta_info;					/**< image or video info */
	void *_reserved;					/**< reserved */
}Mitem;


/**
* @struct _Mbookmark
* This structure defines _Mbookmark, same with Mbookmark
*/
typedef struct _Mbookmark
{
	int gtype;					/**< self-defination type */
	
	unsigned int _id;       	/**< bookmark id */
	//unsigned int media_id;  	/**< media id */
	char *media_uuid;			/**< media UUID */
	unsigned int position;  	/**< marked time */
	char *thumb_url;        	/**< thumnail full path */
}Mbookmark;

/**
* @struct _Mtag
* This structure defines _Mtag, same with Mtag
*/
typedef struct _Mtag
{
	int gtype;					/**< self-defination type */
	
	unsigned int _id;       	/**< tag id */
	//unsigned int media_id;  	/**< media id */
	char *media_uuid;			/**< media UUID */
	char *tag_name;        	/**< tag name*/
	int  count;             /**< count of media content included into a tag*/
	void *_reserved;					/**< reserved */
}Mtag;


typedef int (*minfo_cluster_ite_cb)( Mcluster *cluster, void *user_data );
typedef int (*minfo_item_ite_cb)( Mitem *item, void *user_data );
typedef int (*minfo_bm_ite_cb)( Mbookmark *bookmark, void *user_data );
typedef int (*minfo_cover_ite_cb)( const char *thumb_path, void *user_data );
typedef int (*minfo_tag_ite_cb)( Mtag *_tag_, void *user_data );


#ifdef __cplusplus
}
#endif /* __cplusplus */


/**
* @}
*/

#endif /*_MINFO_TYPES__H_*/


