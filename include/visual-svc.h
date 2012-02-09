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
	
#ifndef _VISUAL_SVC_H_
#define _VISUAL_SVC_H_

/** 
 * This file defines minfo apis for media service..
 *
 * @file       	visual-svc.h
 * @author      Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines apis for visual media service.
 */

#include "media-svc-types.h"
#include "visual-svc-types.h"
#include "visual-svc-error.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
	@mainpage VISUAL_SVC

	@par
	This document provides necessary visual media information for developers who are
	going to implement gallery application and ug-imageviewer or other
	3rd party applications.
 */

/**
 *  @ingroup MEDIA_SVC
	@defgroup VISUAL_SVC_API	Media Service
	@{

	@par
	Call Directly.
 */


/**
 * minfo_get_item_list
 * This function gets mitem list, which include all or portion of a cluster or folder specified by 
 * @p cluster_id. @p filter could specify some filter conditions, like, type of got items, sort by type,
 * start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * Menawhile data of each mitem instance mainly derive from media table record. However meta data
 * is composed of video_meta or image_meta record.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	the folder id in which media files are in. if the parameter is NULL, then query all folders.
 * @param	filter			[in]	the filter to specify some filter conditions, like, type of got items, sort by type, start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out]   user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	type of data memeber of list is pointer to the structure type 'Mitem'
 *			when free list, it need free every item first and then free list itself. 
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code
 
    #include <media-svc.h>
     
	int mitem_ite_cb(Mitem *item, void *user_data)
	{
		GList** list = (GList**)user_data;
		//append an item to linked list.
		*list = g_list_append(*list, item);
	}

	void test_minfo_get_item_list(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int img_cnt = 0;
		GList *p_list = NULL;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		minfo_item_filter item_filter = {MINFO_ITEM_VIDEO,MINFO_MEDIA_SORT_BY_DATE_ASC,3,10,true,true};
		//get a set of items
		ret = minfo_get_item_list(mb_svc_handle, cluster_id, item_filter, mitem_ite_cb, &p_list);

		if(ret< 0) { 
			printf("minfo_get_item_list error\n");
			return;
		}
	}
 * @endcode  

 */

int
minfo_get_item_list(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const minfo_item_filter filter, minfo_item_ite_cb func, void *user_data);

/**
 * minfo_get_all_item_list
 * This function gets mitem list, which include all or portion of a or many clusters or folders specified by 
 * @p cluster_type. @p filter could specify some filter conditions, like, type of got items, sort by type,
 * start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * Meanwhile data of each mitem instance mainly derive from media table record. However meta data
 * is composed of video_meta or image_meta record.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_type	[in]	the folder type which specify media files belong to.
 * @param	filter			[in]	the filter to specify some filter conditions, like, type of got items, sort by type, start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out]   user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	type of data memeber of list is pointer to the structure type 'Mitem'
 *			when free list, it need free every item first and then free list itself. 
 * @see		minfo_get_item_list.
 * @pre		None
 * @post	None
 * @par example
 * @code
 
    #include <media-svc.h>

	int mitem_ite_cb(Mitem *item, void *user_data)
	{
		GList** list = (GList**)user_data;
		//append an item to linked list.
		*list = g_list_append(*list, item);
	}

	void test_minfo_get_all_item_list(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int img_cnt = 0;
		GList *p_list = NULL;

		minfo_item_filter item_filter = {MINFO_ITEM_VIDEO,MINFO_MEDIA_SORT_BY_DATE_ASC,3,10,true,true};
		//get a set of items, which all reside on local storage, including MMC and phone.
		ret = minfo_get_all_item_list(mb_svc_handle, MINFO_CLUSTER_TYPE_LOCAL_ALL, item_filter, mitem_ite_cb, &p_list);

		if(ret< 0) { 
			printf("minfo_get_item_list error\n");
			return;
		}
	}
 * @endcode

 */

int
minfo_get_all_item_list(MediaSvcHandle *mb_svc_handle, const minfo_folder_type cluster_type, const minfo_item_filter filter, minfo_item_ite_cb func, void *user_data);

/**
 * minfo_get_item_list_search
 * This function gets mitem list, which is searched by string specified by user.
 * @p search_field is a field to want to search. @p search_str could specify string to search.
 * Menawhile data of each mitem instance mainly derive from media table record. However meta data
 * is composed of video_meta or image_meta record.
 *
 * @param	mb_svc_handle	[in] the handle of DB
 * @param	search_field	[in] A field to want search. Please refer the enum type minfo_search_field_t in 'minfo-types.h'.
 * @param	search_str		[in] A string to search.
 * @param	folder_type		[in] the folder type which specify media files belong to.
 * @param	filter			[in] the filter to specify some filter conditions, like, type of got items, sort by type, start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out] user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 
 * @return	This function returns 0 on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * @remarks	type of data memeber of list is pointer to the structure type 'Mitem'
 *			when free list, it need free every item first and then free list itself. 
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	int mitem_ite_cb(Mitem *item, void *user_data)
	{
		GList **list = (GList **)user_data;
		//append an item to linked list.
		*list = g_list_append(*list, item);
	}

	void test_minfo_get_item_list_search(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		GList *p_list = NULL;
		const char *search_str = "Hiphop";
		minfo_search_field_t search_field = MINFO_SEARCH_BY_NAME;
		minfo_folder_type folder_type = MINFO_CLUSTER_TYPE_ALL;

		minfo_item_filter item_filter = {MINFO_ITEM_VIDEO, MINFO_MEDIA_SORT_BY_NAME_ASC, 0, 9, false, MINFO_MEDIA_FAV_ALL};

		ret = minfo_get_item_list_search(mb_svc_handle, search_field, search_str, folder_type, item_filter, mitem_ite_cb, &p_list);

		if (ret< 0) {
			printf("minfo_get_item_list_search error\n");
			return;
		}
	}
 * @endcode  

 */

int
minfo_get_item_list_search(MediaSvcHandle *mb_svc_handle, minfo_search_field_t search_field, const char *search_str, minfo_folder_type folder_type, const minfo_item_filter filter, minfo_item_ite_cb func, void *user_data);

/**
 * minfo_get_all_item_cnt
 * This function gets count of all records in media table. This function returns the count of all items, which are unlocked excluding web media.
 *
 * @param	cnt         [out]   returned value, count of all records
 * @return	This function returns zero(MB_SVC_ERROR_BASE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * @remarks	None. 
 * @see 	None.
 * @pre		None
 * @post	None
 * @par example
 * @code


     #include <media-svc.h>
     
	void test_minfo_get_all_item_cnt(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int cnt = 0;

		ret = minfo_get_all_item_cnt(mb_svc_handle, &cnt);
		if(ret< 0) { 
			printf("minfo_get_all_item_cnt error\n");
			return;
		}
	}
 * @endcode
 */

DEPRECATED_API int
minfo_get_all_item_cnt(MediaSvcHandle *mb_svc_handle, int *cnt);

/**
 * minfo_get_all_item_conut
 * This function gets count of all records in the specific storage.
 * User can specify folder type like MINFO_CLUSTER_TYPE_ALL, MINFO_CLUSTER_TYPE_LOCAL_PHONE, etc.
 * Please refer 'visual-svc-types.h' to know what folder type exists.
 * This function returns the count of all items, which are unlocked.
 *
 * @param	mb_svc_handle	[in] the handle of DB
 * @param	folder_type	[in]	folder type
 * @param	file_type	[in]	file type
 * @param	fav_type	[in]	favortie type
 * @param	cnt		[out]	returned value, count of all records
 * @return	This function returns zero(MB_SVC_ERROR_BASE) on success, or negative value with error  code.
 *			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * @remarks	None.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

#include <media-svc.h>

void test_minfo_get_all_item_conut(MediaSvcHandle *mb_svc_handle)
{
	int ret = -1;
	int cnt = 0;
	minfo_folder_type folder_type = MINFO_CLUSTER_TYPE_LOCAL_PHONE;
	minfo_file_type file_type = MINFO_ITEM_ALL;
	minfo_media_favorite_type fav_type = MINFO_MEDIA_FAV_ALL;

	ret = minfo_get_all_item_conut(mb_svc_handle, folder_type, file_type, fav_type, &cnt);
	if(ret< 0) {
		printf("minfo_get_all_item_cnt error\n");
		return;
	}
}
* @endcode
*/

EXPORT_API int minfo_get_all_item_count(
						MediaSvcHandle *mb_svc_handle,
						minfo_folder_type folder_type,
						minfo_file_type file_type,
						minfo_media_favorite_type fav_type,
						int *cnt);

/**
 * minfo_get_item_cnt
 * This function gets count of matched records in media table with the specified @p filter, which 
 * specify some filter conditions, like, type of got items, sort by type, start and end positions 
 * of items, including meta data or not, whether just get the favorites, etc.
 * The detail structure type of @p filter, could refer to the defination 'minfo_item_filter'
 * in header file, minfo-types.h.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	the folder id in which media files are in. if the parameter is -1, then query all folders.
 * @param	filter			[in]	the filter to specify some filter conditions, like, type of got items, sort by type, start and end positions of items, including meta data or not, whether just get the favorites, etc.
 * @param	cnt				[out]   returned value, count of matched records
 
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	list contains a set of full path string of cover file. 
 * @see 		minfo_get_item_list.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_item_cnt(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		minfo_item_filter item_filter = {MINFO_ITEM_VIDEO,MINFO_MEDIA_SORT_BY_DATE_ASC,-1,10,true,true};

		//get count of a set of items.
		ret = minfo_get_item_cnt(mb_svc_handle, cluster_id, item_filter, &cnt);
		if(ret< 0) { 
			printf("test_minfo_get_item_cnt error\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_get_item_cnt(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const minfo_item_filter filter, int *cnt);


/**
 * minfo_get_cluster_cnt
 * This function gets count of matched records from folder table  with the specified @p filter, which 
 * specify some filter conditions, like, type of got clusters, sort by type, start and end positions 
 * of clusters, etc. The detail structure type of @p filter, could refer to the defination 'minfo_cluster_filter'
 * in header file, minfo-types.h.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	filter			[in]	filter to specify some filter conditions, like, type of got clusters, sort by type, start and end positions of clusters
 * @param	cnt				[out]  returned value, count of matched records
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see		minfo_get_cluster_list.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_cluster_cnt(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_NONE,-1,10};

		//get the count of items which is owned by a cluster.
		ret = minfo_get_cluster_cnt(mb_svc_handle, cluster_filter, &cnt);
		if(ret< 0) { 
			printf("test_minfo_get_cluster_cnt error\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_get_cluster_cnt(MediaSvcHandle *mb_svc_handle, const minfo_cluster_filter filter, int *cnt);


/**
 * minfo_get_cluster_list
 * This function gets Mcluster instances list. Data of each instance is composed of the matched records from folder table 
 * with the @p filter, which specify some filter conditions, like, type of got clusters, sort by type, start and end positions 
 * The detail structure type of @p filter, could refer to the defination 'minfo_cluster_filter'
 * in header file, minfo-types.h.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	filter			[in]	filter to specify some filter conditions, like, type of got clusters, sort by type, start and end positions of clusters
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out]   user's data structure to contain items of the type Mcluster. It is passed to the iterative callback.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	item type in list is pointer to structure mcluster
 *          when free list, it need free every item first and then free list itself.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

    #include <media-svc.h>
     
	int mcluster_ite_cb(Mcluster *cluster, void *user_data)
	{
		GList** list = (GList**)user_data;
		*list = g_list_append(*list, cluster);
	}

	void test_minfo_get_cluster_list(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int i;
		int img_cnt;
		GList *p_list = NULL;
		Mcluster* cluster = NULL;

        //get a linked list which include all of clusters based on a specified filter.
		minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_NONE,0,10};

		ret = minfo_get_cluster_list(mb_svc_handle, cluster_filter, mcluster_ite_cb, &p_list);

		if( ret < 0) {
			 return;
		}
	}
 * @endcode
 */

int
minfo_get_cluster_list(MediaSvcHandle *mb_svc_handle, const minfo_cluster_filter filter, minfo_cluster_ite_cb func, void *user_data);


/**
 * minfo_get_meta_info
 * This function gets matched 'Mmeta' instances. Data of the instance is composed of the matched media record from 
 * 'video_meta'/'image_meta' table, when finish using the Mmeta instance, should call API, minfo_mmeta_destroy to 
 * destroy this created instance.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	specified _id field in media table record
 * @param	meta 			[out]	pointer to pointer of matched Mmeta instance
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	after using meta, it must be freed.
 * @see		None
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_meta_info(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		Mmeta* mt = NULL;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		//get a matched meta data.
		ret = minfo_get_meta_info(mb_svc_handle, media_id, &mt);
		if( ret < 0) {
			printf("minfo_get_meta_info failed\n");
			return ret;
		}
		minfo_mmeta_destroy(mt);
	}

 * @endcode
 */

int
minfo_get_meta_info(MediaSvcHandle *mb_svc_handle, const char *media_id, Mmeta** meta);

/**
 * minfo_update_image_meta_info_int
 * This function will update the corresponding field's value in database 'image_meta' table, this will be decided by the
 * @p meta_field, whose type is 'minfo_image_meta_field_t', and indicate which field will be replaced by the value of int type
 * pointered to by @p updated_value.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	specified _id field in media table record
 * @param	meta_field		[in]	the enum value indicate which field of database table will be updated
 * @param	updated_value	[in]	value of int, which will replace the original value in database image meta table
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	after using meta, it must be freed.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_image_meta_info_int(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int width = 640;
		int height = 480;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		// update image meta's 'width and height' member 
		ret = minfo_update_image_meta_info_int(mb_svc_handle, media_id, MINFO_IMAGE_META_WIDTH, width, MINFO_IMAGE_META_HEIGHT, height);
		if( ret < 0) {
			printf("minfo_update_image_meta_info_int failed\n");
			return;
		}
	}

 * @endcode
 */

EXPORT_API int
minfo_update_image_meta_info_int(MediaSvcHandle *mb_svc_handle, const char *media_id,
				minfo_image_meta_field_t meta_field,
				const int updated_value,
				...);

/**
 * minfo_update_video_meta_info_int
 * This function will update the corresponding field's value in database 'video_meta' table, this will be decided by the
 * @p meta_field, whose type is 'minfo_video_meta_field_t', and indicate which field will be replaced by the value of int type
 * pointered to by @p updated_value.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	specified _id field in media table record
 * @param	meta_field		[in]	the enum value indicate which field of database table will be updated
 * @param	updated_value	[in]	value of int, which will replace the original value in database video meta table
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	after using meta, it must be freed.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_video_meta_info_int(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int _value = 876;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		//update video meta's 'last_played_time' member
		ret = minfo_update_video_meta_info_int(mb_svc_handle, media_id, MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED, _value);

		if( ret < 0) {
			printf("minfo_update_video_meta_info_int failed\n");
			return;
		}
	}

 * @endcode
 */

int
minfo_update_video_meta_info_int(MediaSvcHandle *mb_svc_handle, const char *media_id, minfo_video_meta_field_t meta_field, const int updated_value);


/**
* minfo_destroy_mtype_item
* This function free an type instantiated object, whose type may be any one recognized by media-svc, these type
* will include Mitem, Mimage, Mvideo, Mmeta, Mcluster, etc. In this function, it will check the concrete type for the 
* @p item, and then decide which free function will really be called.
*
* @return       This function returns 0 on success, and negativa value on failure.
* @param        item [in]        the input parameter, inlcuding the instanciated object.
* @exception    None.
* @remarks      This function is general one, it will be able to destroy any recognized instantiated object
* 				by media-svc, so when you create or get an instantiated object from media-svc, and then
*				call this function to free it.
* @see 	None
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_destroy_mtype_item(void)
	{
		int ret = -1;
		Mitem *mi = NULL;
		
		char* file_url = "/opt/media/Images/Wallpapers/Home_01.png";
		ret = minfo_get_item(file_url, &mi);

		//destroy an item whose type is 'Mitem'.
		ret = minfo_destroy_mtype_item(mi);

		if( ret < 0) {
			 return ret;
		}
		
	}
 * @endcode
*/

int
minfo_destroy_mtype_item(void* item);


/**
 * minfo_add_media_start
 * This function inserts new media file information into media table,video_meta table/image_meta table.
   Or updates file information in these tables if the file is updated
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	trans_count		[in]	count of trasaction user wants
 * @return	This function returns 0/positive on success, or negative value with error code. (0 : added, 1: updated, 2: skipped )
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	minfo_add_media_end, minfo_add_media_batch
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_add_media_batch(MediaSvcHandle *mb_svc_handle)
	{
		int err = -1, i;

		err = minfo_add_media_start(mb_svc_handle, 100);
		if( err < 0) {
			printf("minfo_add_media_start failed\n");
			return;
		}

		for (i = 0; i < 200; i++) {
			err = minfo_add_media_batch(mb_svc_handle, image_files[i], MINFO_ITEM_IMAGE);

			if( err < 0) {
				printf("minfo_add_media_start failed\n");
				return;
			}
		}

		err = minfo_add_media_end(mb_svc_handle);
		if( err < 0) {
			printf("minfo_add_media_end failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_add_media_start(MediaSvcHandle *mb_svc_handle, int trans_count);


/**
 * minfo_add_media_batch
 * This function inserts new media file information into media table,video_meta table/image_meta table.
   Or updates file information in these tables if the file is updated
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	the local file full path
 * @param	type			[in]	the file type, maybe it's video file or image file
 * @return	This function returns 0/positive on success, or negative value with error code. (0 : added, 1: updated, 2: skipped )
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	 minfo_add_media_start, minfo_add_media_end
 * @pre	None
 * @post	None
 * @par example
 * @code

    #include <media-svc.h>

	void test_minfo_add_media_batch(MediaSvcHandle *mb_svc_handle)
	{
		int err = -1, i;

		err = minfo_add_media_start(mb_svc_handle, 100);
		if( err < 0) {
			printf("minfo_add_media_start failed\n");
			return;
		}

		for (i = 0; i < 100; i++) {
			err = minfo_add_media_batch(mb_svc_handle, image_files[i], MINFO_ITEM_IMAGE);

			if( err < 0) {
				printf("minfo_add_media_start failed\n");
				return;
			}
		}

		err = minfo_add_media_end(mb_svc_handle);
		if( err < 0) {
			printf("minfo_add_media_end failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_add_media_batch(MediaSvcHandle *mb_svc_handle, const char* file_url, minfo_file_type content_type);

/**
 * minfo_add_media_end
 * This function inserts new media file information into media table,video_meta table/image_meta table.
   Or updates file information in these tables if the file is updated
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @return	This function returns 0/positive on success, or negative value with error code. (0 : added, 1: updated, 2: skipped )
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	minfo_add_media_start, minfo_add_media_batch
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_add_media_batch(MediaSvcHandle *mb_svc_handle)
	{
		int err = -1, i;

		err = minfo_add_media_start(100);
		if( err < 0) {
			printf("minfo_add_media_start failed\n");
			return;
		}

		for (i = 0; i < 200; i++) {
			err = minfo_add_media_batch(mb_svc_handle, image_files[i], MINFO_ITEM_IMAGE);

			if( err < 0) {
				printf("minfo_add_media_start failed\n");
				return;
			}
		}

		err = minfo_add_media_end();
		if( err < 0) {
			printf("minfo_add_media_end failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_add_media_end(MediaSvcHandle *mb_svc_handle);

/**
 * minfo_add_media
 * This function inserts new media file information into media table,video_meta table/image_meta table.
   Or updates file information in these tables if the file is updated
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	the local file full path
 * @param	type			[in]	the file type, maybe it's video file or image file
 * @return	This function returns 0/positive on success, or negative value with error code. (0 : added, 1: updated, 2: skipped )
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	 minfo_delete_media
 * @pre	None
 * @post None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_add_media(MediaSvcHandle *mb_svc_handle)
	{
		int err = -1;
		char *file_url = "/opt/media/Images/Wallpapers/Home_01.png";

		//add a new media content whose url is 'file_url'.
		err = minfo_add_media(mb_svc_handle, file_url, MINFO_ITEM_IMAGE);

		if( err < 0) {
			printf("minfo_add_media failed\n");
			return;
		}
	}
 * @endcode
 */


int
minfo_add_media(MediaSvcHandle *mb_svc_handle, const char* file_url, minfo_file_type content_type);



/**
 * minfo_delete_media
 * This function deletes matched media table record, video_meta/image_meta record. After that, if the folder which this file is in is empty, then delete the folder record.
 * When user actually delete a media file in file system, he/she should call this function to delete the corresponding record in 'media' table, meanwhile it may delete the 
 * folder record in 'folder' table if this folder will not include any media content.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	 matched local file full path
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	 None.
 * @see  minfo_add_media
 * @pre	None
 * @post None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_delete_media(void)
	{
		int ret = -1;
		char *file_url = "/opt/media/Images/Wallpapers/Home_01.png";

		//delete a media reord from 'media' table.
		ret= minfo_delete_media(file_url);

		if( ret < 0) {
			printf("minfo_delete_media failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_delete_media(MediaSvcHandle *mb_svc_handle, const char* file_url);



/**
 * minfo_move_media
 * This function moves the media file to another place. When user actually move a media file ( @p old_file_url )in file system to the destination
 * pathname ( @p new_file_url ), he/she need to call this function to move the record of 'media' table. Meanwhile user is responsible for identifying 
 * the file's type, like image, video, etc. 
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	old_file_url	[in]	old local file full path of the media file
 * @param	new_file_url	[in]	new local file full path of the media file
 * @param	type			[in]	 media file type,maybe vidoe or image
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see   minfo_mv_media, minfo_copy_media, minfo_update_media_name.
 * @pre	None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_move_media(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char* old_file_url = "/opt/media/Images/Wallpapers/Home_01.png";
		char* new_file_url = "/opt/media/Images/Wallpapers/Home_01_1.png";

		//move an item to a specified location.
		ret = minfo_move_media(mb_svc_handle, old_file_url, new_file_url, MINFO_ITEM_IMAGE);

		if( ret < 0) {
			printf("minfo_move_media failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_move_media(MediaSvcHandle *mb_svc_handle, const char* old_file_url, const char *new_file_url, minfo_file_type content_type);

/**
 * minfo_move_media_start
 * This function starts to move multiple media files
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	trans_count		[in]	count of trasaction user wants
 * @return	This function returns 0/positive on success, or negative value with error code.
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	minfo_move_media_end, minfo_move_media
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_move_media_batch(MediaSvcHandle *mb_svc_handle)
	{
		int err = -1, i;

		err = minfo_move_media_start(mb_svc_handle, 100);
		if( err < 0) {
			printf("minfo_move_media_start failed\n");
			return;
		}

		for (i = 0; i < 200; i++) {
			err = minfo_move_media(mb_svc_handle, src_image_file_path[i], dst_image_file_path[i], MINFO_ITEM_IMAGE);

			if( err < 0) {
				printf("minfo_move_media failed\n");
				return;
			}
		}

		err = minfo_move_media_end(mb_svc_handle);
		if( err < 0) {
			printf("minfo_move_media_end failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_move_media_start(MediaSvcHandle *mb_svc_handle, int trans_count);



/**
 * minfo_move_media_end
 * This function ends to move multiple media files
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @return	This function returns 0/positive on success, or negative value with error code.
 * @remarks	if invoke this function with same input parameter, it fails
 * @see 	minfo_add_move_start, minfo_move_media
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_move_media_batch(void)
	{
		int err = -1, i;

		err = minfo_move_media_start(mb_svc_handle, 100);
		if( err < 0) {
			printf("minfo_add_media_start failed\n");
			return;
		}

		for (i = 0; i < 200; i++) {
			err = minfo_move_media(mb_svc_handle, src_image_file_path[i], dst_image_file_path[i], MINFO_ITEM_IMAGE);

			if( err < 0) {
				printf("minfo_move_media failed\n");
				return;
			}
		}

		err = minfo_move_media_end(mb_svc_handle);
		if( err < 0) {
			printf("minfo_move_media_end failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_move_media_end(MediaSvcHandle *mb_svc_handle);


/**
 * minfo_copy_media
 * This function copies the media file to another place. User should pass the full pathnames for these parameters, @p old_file_url and @ new_file_url
 * respectively. The @p old_file_url indicate the original full pathname of this media content, and @ new_file_url indicate the destination full pathname of
 * the copied file.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	old_file_url	[in]	old local file full path of the media file
 * @param	new_file_url	[in]	new local file full path of the media file
 * @param	type			[in]	media file type, maybe vidoe or image
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	This function will not return the thumbnail pathname of new media file, user could get this new thumnail file using the function, minfo_get_thumb_path.
 * @see         minfo_cp_media, minfo_update_media_name, minfo_move_media
 * @pre	None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

    void test_minfo_copy_media(MediaSvcHandle *mb_svc_handle)
    {
        int ret =-1;

        char *file_url = "/opt/media/Images/Wallpapers/Home_01.png";
	    char *new_file_url = "/opt/media/Images/Wallpapers/Home_01_1.png";

	    //copy a media file to other location whos name is specified by 'new_file_url'.
        ret  = minfo_copy_media(mb_svc_handle, old_file_url, new_file_url, file_type);
        if( ret < 0) {
              printf("minfo_copy_media failed\n");
              return;
		}

    }
 * @endcode
 */

int
minfo_copy_media(MediaSvcHandle *mb_svc_handle, const char* old_file_url, const char *new_file_url, minfo_file_type content_type);


/**
 * minfo_update_media_name
 * This function rename a image or video file. Here this function will assume the folder name of 
 * file whose name is changed keep unchanged. That is, this file which is changed name still is located in the same folder.
 * This function actually call sqlite3 
 * UPDATE %s SET ... WHERE _id = _id;
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	the id of specified media file
 * @param	new_name		[in]	new name of the media file
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	 None.
 * @see    minfo_move_media, minfo_copy_media.
 * @pre	None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_media_name(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";	

		//update media name
		ret = minfo_update_media_name(mb_svc_handle, media_id, new_name);
		if( ret < 0) {
			printf("test_minfo_update_media_name failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_update_media_name(MediaSvcHandle *mb_svc_handle, const char *media_id, const char* new_name);

/**
 * minfo_update_media_thumb
 * This function updates a thumbpath of the media in DB.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	the id of specified media file
 * @param	thumb_path		[in]	new thumbnail path of the media file
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	 None.
 * @see None
 * @pre	None
 * @post None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_media_thumb(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		const char *thumb_path = "/opt/media/test.jpg";

		//update thumbnail path
		ret = minfo_update_media_thumb(mb_svc_handle, media_id, thumb_path);
		if( ret < 0) {
			printf("minfo_update_media_thumb failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_update_media_thumb(MediaSvcHandle *mb_svc_handle, const char *media_id, const char* thumb_path);

/**
 * minfo_update_media_favorite
 * This function updates favorite field of image or video file in 'media' table. This function actually call the Sqlite3 UPDATE,
 * In Gallery application or ug-imageviewer, user could want to set a media file as favorite or unfovarite, so he/she could call
 * this API to do it.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	Unique id of the media file
 * @param	favorite_level	[in]	new favorite_level of the media file, indicate whether this media content is favorite or unfavorite.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see     None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_media_favorite(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		//update an item's favorite record
		ret = minfo_update_media_favorite(mb_svc_handle, media_id, favorite_level);

		if( ret < 0) {
			printf("minfo_update_media_favorite failed\n");
			return;
		}
	}
 * @endcode
 */

int  
minfo_update_media_favorite(MediaSvcHandle *mb_svc_handle, const char *media_id, const int favorite_level);


/**
 * minfo_update_media_date
 * This function updates modified date of image or video file in 'media' table. This function actually call the Sqlite3 UPDATE.
 * In Gallery application or ug-imageviewer, user could want to set moedified date of a media, so he/she could call this API to do it.
 * @return  This function returns zero(MB_SVC_ERROR_BASE) on success, or negative value with error code.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	Unique id of the media file
 * @param	modified_date	[in]	date to modify, which is a type of time_t
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see     None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_media_date(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		time_t today;
		time(&today);

		//update an item's date record
		ret = minfo_update_media_date(mb_svc_handle, media_id, today);
		if( ret < 0) {
			printf("minfo_update_media_date failed\n");
			return;
		}
	}
 * @endcode
 */

int  
minfo_update_media_date(MediaSvcHandle *mb_svc_handle, const char *media_id, time_t modified_date);


/**
 * minfo_add_cluster
 * This function adds new local folder. This function could be called when user want to add a Album(local folder)
 * in Gallery application. This function actually call the sqlite INSERT statement to insert the record in folder
 * table. Meanwhile it will return new added local folder's ID to @p id.
 * Sqlie3 statement looks like this, INSERT INTO folder (...);
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_url		[in]	the local directory of added folder, it should be full pathname of local folder
 * @param	id				[out]	id of the added folder, this function will return a unique ID to calling application
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	if invoke this function with same input parameter, it fails   
 * @see     minfo_delete_cluster
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>
 
	void test_minfo_add_cluster(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char *cluster_url = "/opt/media/Images/Wallpapers";
		char cluster_id[256];

		//add a new cluster whose url is 'cluster_url'.
		ret = minfo_add_cluster(mb_svc_handle, cluster_url, cluster_id, sizeof(cluster_id));
		if( ret < 0) {
			printf("minfo_add_cluster failed\n");
			return;
		}
	}
 * @endcode    
 */

int
minfo_add_cluster(MediaSvcHandle *mb_svc_handle, const char* cluster_url, char *id, int max_length);


/**
 * minfo_check_cluster_exist
 * This function checks to exist the cluster in media database by its path.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	path			[in]	the local directory to check if it exists, it should be full pathname of local folder
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	if invoke this function with same input parameter, it fails
 * @see     minfo_check_item_exist
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_check_cluster_exist(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char *cluster_url = "/opt/media/Images/Wallpapers";

		//check if the cluster exists by path.
		ret = minfo_check_cluster_exist(mb_svc_handle, cluster_url);
		if( ret < 0) {
			printf("minfo_check_cluster_exist failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_check_cluster_exist(MediaSvcHandle *mb_svc_handle, const char *path);

/**
 * minfo_check_item_exist
 * This function checks to exist the media in media database by its path.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	path			[in]	the local media path to check if it exists, it should be full pathname.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	if invoke this function with same input parameter, it fails
 * @see     minfo_check_cluster_exist
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_check_item_exist(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char *media_url = "/opt/media/Images/Wallpapers/Wallpaper1.jpg";

		//check if the item exists by path.
		ret = minfo_check_item_exist(mb_svc_handle, media_url);
		if( ret < 0) {
			printf("minfo_check_item_exist failed\n");
			return;
		}
	}
 * @endcode
 */

int
minfo_check_item_exist(MediaSvcHandle *mb_svc_handle, const char *path);

/**
 * minfo_get_item_by_id
 * This function gets mitem information. When user could get the unique ID of a media content, he/she
 * could get the detail information with the type 'Mitem', which include the below feilds, like, item's unique id,
 * media content type, media content's thumbnail name, media content's pathname, etc. The detail defination 
 * of this structute, could refer to the header, minfo-item.h.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	the local file media id
 * @param	mitem			[out]	the returned data structure whose type is Mitem.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	   when invoking this function, *mitem must equals NULL, and
 *          the @p media_id should be valid ID, if it is invalid, like -1 or 0, this
 *			function will return @p mitem whose content is NULL. If normally, @p mitem must be freed with minfo_mitem_destroy.
 * @see     minfo_get_item.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_item_by_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		Mitem *mi = NULL;

		//get an item based on its media ID.
		ret = minfo_get_item_by_id(mb_svc_handle, media_id, &mi);
		if(ret < 0) {
			return;
		}

		minfo_destroy_mtype_item(mi);
	}
 * @endcode
 */

int
minfo_get_item_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, Mitem **mitem);


/**
 * minfo_get_item
 * This function gets mitem information. When user could get the full pathname of a media content, he/she
 * could get the detail information with the type 'Mitem', which include the below feilds, like, item's unique id,
 * media content type, media content's thumbnail name, media content's pathname, etc. The detail defination 
 * of this structute, could refer to the header, minfo-item.h.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	the local file full pathname
 * @param	mitem			[out]	the returned data structure whose type is Mitem.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	   when invoking this function, *mitem must equals NULL, and
 *                  after using mitem, it must be freed with minfo_mitem_destroy.
 * @see     minfo_get_item_by_id.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_item(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		Mitem *mi = NULL;
		char* file_url = "/opt/media/Images/Wallpapers/Home_01.png";

		//get an item based on its url.
		ret = minfo_get_item(mb_svc_handle, file_url, &mi);
		if(ret < 0) {
			return;
		}

		minfo_destroy_mtype_item(mi);
	}
 * @endcode
 */

int
minfo_get_item(MediaSvcHandle *mb_svc_handle, const char* file_url, Mitem **mitem);

/**
 * minfo_get_item_by_http_url
 * This function gets mitem information. When user could get the http url of a media content, which is downloaded from web, he/she
 * could get the detail information with the type 'Mitem', which include the below feilds, like, item's unique id,
 * media content type, media content's thumbnail name, etc. The detail defination 
 * of this structute, could refer to the header, minfo_item/minfo-item.h.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	http_url		[in]	the http url of a media, which is downloaded from web.
 * @param	mitem			[out]	the returned data structure whose type is Mitem.
 * @return	This function returns 0 on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * @remarks	   when invoking this function, *mitem must equals NULL, and
 *                  after using mitem, it must be freed with minfo_mitem_destroy.
 * @see     minfo_get_item_by_id, minfo_get_item
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_item_by_http_url(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		Mitem *mi = NULL;
		char* http_url = "http://picasa.com/myaccount/Home_01.png";

		//get an item based on its http url.
		ret = minfo_get_item_by_http_url(mb_svc_handle, http_url, &mi);
		if(ret < 0) {
			return;
		}

		minfo_destroy_mtype_item(mi);
	}
 * @endcode
 */

int
minfo_get_item_by_http_url(MediaSvcHandle *mb_svc_handle, const char* http_url, Mitem **mitem);


/**
* minfo_get_cluster
* This function gets mcluster information by folder full path or cluster id when user could not know exact url of cluster. When user could get full path of a folder, he/she
* could get the detail information with the type 'Mcluster' about this cluster/local folder, the type 'Mcluster'
* mainly include folder/cluster ID, display name, count of included media content, etc. The detail defination
* of this type could refer to the herder file, minfo-cluster.h.
*
* @return	This function returns 0 on success, or negative value with error code.
* @param	mb_svc_handle	[in]	the handle of DB
* @param	cluster_url		[in]	local folder full path, it indicate which folder user want to get it's detail information
* @param	cluster_id		[in]	the cluster ID which indentify a cluster
* @param	mcluster		[out]	mcluster to be returned, which is a 'Mcluster' type
* @exception None.
* @remarks  when user could not know exact url of a cluster, he/she could choose alternative way he/she pass cluster id to this function, so that
*           this function still could get the wanted cluster.
*           when invoking this function, *mcluster must equals NULL, and
*           after using mitem, it must be freed with minfo_mcluster_destroy.
* @see      minfo_mcluster_destroy
* @pre		None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_cluster(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		Mcluster *mc = NULL;
		char *cluster_url = "/opt/media/Images/Wallpapers";
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//get a cluster using cluster's url.
		ret = minfo_get_cluster(mb_svc_handle, cluster_url, cluster_id, &mc);
		if(ret < 0) {
			printf("minfo_get_cluster fail: %d \n", ret);
			return;
		}

		printf("minfo_get_cluster: %s \n", mc->display_name);
		minfo_mcluster_destroy(mc);
	}
* @endcode
*/

int 
minfo_get_cluster(MediaSvcHandle *mb_svc_handle, const char* cluster_url, const char *cluster_id, Mcluster **mcluster);


/**
 * minfo_get_cluster_cover
 * This function gets thumbnail path of cover files by cluster id. This function could get the cover of a cluster
 * or folder which may include first several items' thumbnails, maybe 5 or other number, user could specify it
 * using @p img_cnt.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	the folder id in which media files are in. if the parameter is -1, then query all folders.
 * @param	img_cnt			[in]	the count of cover thumbnails
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an thumbnail path has to be inserted to user's list.
 * @param	user_data	[out]   user's data structure to contain items of thumbnail path. It is passed to the iterative callback.
 *
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	type of item is pointer to char*,
 *			when free list, needn't free every string.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	int cover_ite_cb(char *thumb_path, void *user_data)
	{
		GList** list = (GList**)user_data;
		*list = g_list_append(*list, thumb_path);
	}

	void test_minfo_get_cluster_cover(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		GList *p_list = NULL;
		int img_cnt = 5;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//get the cover of a cluster.
		ret = minfo_get_cluster_cover(mb_svc_handle, cluster_id, img_cnt, cover_ite_cb, &p_list);
		if(ret< 0) { 
			printf("test_minfo_get_cluster_cover error\n");
			return;
		}
	}

  * @endcode
  */

int 
minfo_get_cluster_cover(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const int img_cnt, minfo_cover_ite_cb func, void *user_data);


/**
 * minfo_get_bookmark_list
 * This function gets the type 'Mbookmark' instances list. Data of each this type instance is 
 * composed of the matched record indentified by the media id from  'video_bookmark' table. 
 * The type 'Mbookmark' mainly include the these information, like, bookmark id, media id, 
 * marked time, corresponding thumbnail pathanme, etc.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	media_id field of video_bookmark table record
 * @param	func			[in]  Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out]   User's data structure to contain items of the type Mbookmark. It is passed to the iterative callback.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	   member data in list is pointer to structure Mbookmark,
 *                    when free list, it need free every item first and then free list itself.
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	int mbookmark_ite_cb(Mbookmark *bm, void *user_data)
	{
		GList** list = (GList**)user_data;
		*list = g_list_append(*list, bm);
	}

	void test_minfo_get_bookmark_list(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		GList *p_list = NULL;

        //get a linked list which will include all of bookmarks of a media file.
		ret = minfo_get_bookmark_list(mb_svc_handle, media_id, mbookmar_ite_cb, &p_list);
		if( ret < 0) {
			return;
		}
	}
  * @endcode 	 
 */

int
minfo_get_bookmark_list(MediaSvcHandle *mb_svc_handle, const char *media_id, minfo_bm_ite_cb func, void *user_data);


/**
 * minfo_get_geo_item_list
 * This function gets the type 'Mitem' instances list. Data of each instance is composed of the matched record identified 
 * by @p filter from 'media' table. Except that the got items pass the criterion of  @p filter, they should position where the longitude
 * is between @p min_longitude and @p max_longitude, and the latitude is between @p min_latitude and @p max_latitude.
*  This function gets 'Mitem' list matched with latitude, longitude, filter and cluster id.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	indicate the value, 'fold_id' field in 'media' table record
 * @param	filter			[in]	specified filter to get matched media record
 * @param	store_filter	[in]	specified storage filter to get matched media record
 * @param	min_longitude	[in]	 minimum value of 'longitude' field in 'vidoe_meta'/'image_meta' table
 * @param	max_longitude	[in]	 maximum value of longitude field in 'vidoe_meta'/'image_meta' table
 * @param	min_latitude	[in]	 minimum value of 'latitude' field in 'video_meta'/'image_meta' record
 * @param	max_latitude	[in]	 maximum value of 'latitude' field in 'video_meta'/'image_meta' record
 * @param	func			[in]	Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * @param	user_data		[out]	user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	     item type in list is pointer to structure Mitem,
 *                    when free list, it need free every item first  and then free list itself. 
 * @see		None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	int mitem_ite_cb(Mitem *item, void *user_data)
	{
		GList** list = (GList**)user_data;
		*list = g_list_append(*list, item);
	}

    void test_minfo_get_geo_item_list(MediaSvcHandle *mb_svc_handle)
    {
		int ret = -1; 
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		int min_longitude = 120.0;
		int max_longitude = 123.0;
		int min_latitude = 19.0;
		int max_latitude = 24.0;
		GList *p_list = NULL;

		//get a linked list which include a set of items based on their location.
		ret = minfo_get_geo_item_list(mb_svc_handle,
						cluster_id,
						store_filter,
						filter,
						min_longitude,
						max_longitude,
						min_latitude,
						max_latitude,
						mitem_ite_cb,
						&p_list);
		if( ret < 0) {
			printf("minfo_get_geo_item_list failed\n");
			return;
		}
    }
  * @endcode
 */

int
minfo_get_geo_item_list(MediaSvcHandle *mb_svc_handle,
						const char *cluster_id, 
						minfo_folder_type store_filter,
						minfo_item_filter filter, 
						double min_longitude, 
						double max_longitude, 
						double min_latitude, 
						double max_latitude,
						minfo_item_ite_cb func,
                        void *user_data);



/**
 * minfo_get_thumb_path
 * This function gets thumbnail path of specified image file. When user could get the full pathname of a image content.
 * He/She wants to get the thumbnail file corresponding to the image content identified by the @p file_url.
 * User is responsible for allocating the memory for @p thumb_path so that this function could fill up the thumbnail pathname to it.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	local file full path, identify a media record in 'media' table
 * @param	thumb_path		[out]	the returned thumbnail path of specified file, user is responsible for allocating memory for it first
 * @param	max_thumb_path	[in]	The max length of the returned thumbnail path
 *
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	one file full path always matches one thumbnail path,
            here, it returns thumbnail path , but maybe thumbnail file doesn't exist, return NULL.
 * @see		minfo_get_thumb_path_for_video.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_thumb_path(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char thumb_path[256] = {'\0'};
		char *file_url = "/opt/media/Images/Wallpapers/Home_01.png";

		//get thumbnail pathname of an item.
		ret = minfo_get_thumb_path(mb_svc_handle, file_url, thumb_path, sizeof(thumb_path));
		if( ret < 0) {
			printf("minfo_get_thumb_path failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_get_thumb_path(MediaSvcHandle *mb_svc_handle, const char* file_url, char* thumb_path, size_t max_thumb_path);

/**
 * minfo_get_thumb_path_for_video
 * This function gets thumbnail path of specified video file. When user could get the full pathname of a video content.
 * He/She wants to get the thumbnail file corresponding to the video content identified by the @p file_url.
 * User is responsible for allocating the memory for @p thumb_path so that this function could fill up the thumbnail pathname to it.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	file_url		[in]	local file full path, identify a media record in 'media' table
 * @param	thumb_path		[out]	the returned thumbnail path of specified file, user is responsible for allocating memory for it first
 * @param	max_thumb_path	[in] The max length of the returned thumbnail path
 *
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	one file full path always matches one thumbnail path,
            here, it returns thumbnail path , but maybe thumbnail file doesn't exist, return NULL.
 * @see		minfo_get_thumb_path.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_get_thumb_path_for_video(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char thumb_path[256] = {'\0'};
		char *file_url = "/opt/media/Images and videos/My video clip/Helicopter.mp4";

		//get thumbnail pathname of an item.
		ret = minfo_get_thumb_path_for_video(mb_svc_handle, file_url,thumb_path, sizeof(thumb_path));
		if( ret < 0) {
			printf("minfo_get_thumb_path_for_video failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_get_thumb_path_for_video(MediaSvcHandle *mb_svc_handle, const char* file_url, char* thumb_path, size_t max_thumb_path);

/**
 * minfo_delete_media_id
 * This function deletes matched record identified by the @p media_id from 'media' table , 'video_meta'/'image_meta' record. 
 * After that, if the folder which this deleted file is in becomes empty, then delete the folder record from 'folder' table, too. 
 * In order that user could successfully delete the corresponding record from 'media' table, he/she should be able to the correct _id 
 * of media content before it.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	represent the value, media '_id' in 'media' table record
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see     minfo_delete_media.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_delete_media_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

		//delete an item according to its ID.
		ret = minfo_delete_media_id(mb_svc_handle, media_id);
		if( ret < 0) {
			printf("test_minfo_delete_media_id failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_delete_media_id(MediaSvcHandle *mb_svc_handle, const char *media_id);

/**
 * 	minfo_delete_all_media_records:\n
 * 	This function delete all media records in a type of storage like phone or MMC.
 *	This function is always used for MMC card insert/inject operation, in file manager service library.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	storage_type	[in]	information for storage type
 * @return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None
 *	@pre	None
 *	@post	None
 *	@remark	None
 * 	@par example
 * 	@code

	#include <media-svc.h>

	void test_minfo_delete_all_media_records(MediaSvcHandle *mb_svc_handle)
	{
		int ret = MB_SVC_ERROR_NONE;

		//delete all media records in MMC storage in db.
		ret = minfo_delete_all_media_records(mb_svc_handle, MINFO_MMC);
		if (ret < 0) {
			printf( "failed to delete items. error code->%d", ret);
			return;
		}
	}

 * 	@endcode
 */

int
minfo_delete_all_media_records(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type);

/**
 * minfo_cp_media
 * This function copies specified media file to another folder, which is identified by the folder id, @p dst_cluster_id. Meanwhile the copied media file
 * is identified by it's media id. Compared to API, minfo_copy_media, the different is , this function copy a media content to specified folder,
 * according to the media content id and the destination folder's id, however the function, minfo_copy_media, copy a media content to specified folder
 * according to the media file's full pathname and folder's full name.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	src_media_id	[in]	id of the source media file, it's value is from '_id' field of the 'media' table
 * @param	dst_cluster_id	[in]	id of the destination folder, it's value is from '_id' field of the 'folder' table
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	This function will implement the same functionality as minfo_copy_media.
 * @see  minfo_copy_media
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_cp_media(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *src_media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		const char *dst_cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//copy the a media file whose ID is specified by, 'src_media_id', to a cluster.
		ret = minfo_cp_media(mb_svc_handle, src_media_id, dst_cluster_id);
		if( ret < 0) {
			printf("minfo_cp_media failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_cp_media(MediaSvcHandle *mb_svc_handle, const char *src_media_id, const char *dst_cluster_id);



/**
 * minfo_mv_media
 * This function moves specified media file to another folder, which is identified by the folder id, @p dst_cluster_id. Meanwhile the moved media file
 * is identified by it's media id. Compared to API, minfo_move_media, the difference is that this function moves a media content to specified folder,
 * according to the media content id and the destination folder's id, however the function, minfo_move_media, move a media content to specified folder
 * according to the media file's full pathname and folder's full name.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	src_media_id	[in]	id of the source media file, it's value is from '_id' field of the 'media' table
 * @param	dst_cluster_id	[in]	id of the destination folder, it's value is from '_id' field of the 'folder'
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see   minfo_move_media.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_mv_media(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *src_media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		const char *dst_cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

        //move an item to specified cluster.
		ret = minfo_mv_media(mb_svc_handle, src_media_id, dst_cluster_id);
		if( ret < 0) {
			printf("minfo_mv_media failed\n");
			return;
		}
	}
* @endcode
 */


int
minfo_mv_media(MediaSvcHandle *mb_svc_handle, const char *src_media_id, const char *dst_cluster_id);



/**
 * minfo_delete_cluster
 * This function deletes specified cluster, which is identified by the @p cluster_id. When user launch Gallery and in edit 'Albums' mode, if he/she
 * want to delete a cluster/folder, so call this function to do it. When delete a cluster/folder, the media-svc will not only delete the record in 'folder' table,
 * but delete all of records in 'media' table which are located in this folder, meanwhile delete the corresponding records in 'video_bookmark' table, etc.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	cluster id, to indicate the deleted folder/cluster
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks delete all releated contents in cluster together with cluster
 *          like all media files, image/video meta,bookmark information.
 * @see   minfo_add_cluster
 * @pre		None
 * @post	None
 * @par example
 * @code


	#include <media-svc.h>

	void test_minfo_delete_cluster(MediaSvcHandle *mb_svc_handle)
	{
	int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//delete a cluster record, meanwhile this function will delete all of items owned by this cluster.
		ret = minfo_delete_cluster(mb_svc_handle, cluster_id);
		if( ret < 0) {
			printf("minfo_delete_cluster failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_delete_cluster(MediaSvcHandle *mb_svc_handle, const char *cluster_id);



/**
 * minfo_update_cluster_name
 * This function updates the specified cluster name using @p new_name, which just indicate the new folder name. User could 
 * call this function, when he/she wants to change some folder/cluster name. This really update the corresponding record in 
 * 'folder' table.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	cluster id, this value is from the '_id' field of 'folder' table
 * @param	new_name		[in]	new cluster name
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	None.
 * @see   None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>

	void test_minfo_update_cluster_name(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		char *new_name = "newfolder";

		//update a cluster's name
		ret = minfo_update_cluster_name(mb_svc_handle, cluster_id,new_name);
		if( ret < 0) {
			printf("minfo_update_cluster_name failed\n");
			return;
		}
	}
* @endcode
 */

int
minfo_update_cluster_name(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const char* new_name);

/**
 * minfo_update_cluster_date
 * This function updates the specified cluster modified date using @p modified_date, which just indicate the new modified date. User could 
 * call this function, when he/she wants to change some clsuter's modified date. This really update the corresponding record in the DB
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	cluster_id		[in]	cluster id, this value is the identifier of the cluster
 * @param	modified_date	[in]	date to modify, which is a type of time_t
 * @return	This function returns zero(MB_SVC_ERROR_BASE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * @remarks	None.
 * @see   None.
 * @pre		None
 * @post	None
 * @par example
 * @code

	#include <media-svc.h>
	#include <time.h>

	void test_minfo_update_cluster_date(void)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		time_t today;
		time(&today);

		//update a cluster's name
		ret = minfo_update_cluster_date(cluster_id, today);

		if( ret < 0) {
			printf("minfo_update_cluster_date failed\n");
			return;
		}
     }
* @endcode
 */

int
minfo_update_cluster_date(MediaSvcHandle *mb_svc_handle, const char *cluster_id,  time_t modified_date);



/**
 * minfo_add_bookmark
 * This function inserts new bookmark record into 'video_bookmark' table. The inserted data should include marked time of video content, @p position 
 * and @p thumb_path, current extracted thumbnail file in marked time, etc. User should use @p media_id to identify the 
 * video content, so that add bookmark to it.
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	media_id		[in]	media file id, uniquely identify the media content
 * @param	position		[in]	marked time of the media file
 * @param	thumb_path		[in]	the extracted thumbnail path for this marked time
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks                         if add same input parameters twice, it fails
 * @see    minfo_delete_bookmark
 * @pre		None
 * @post	None
 * @par example
 * @code


	#include <media-svc.h>

	void test_minfo_add_bookmark(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int position = 2346;
		const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
		char *thumb_path = "tmp1";

		//add a bookmark which include position, thumbnail, etc. to an item.
		ret = minfo_add_bookmark(mb_svc_handle, media_id,position,thumb_path);
		if( ret < 0) {
			printf("minfo_add_bookmark failed\n");
			return;
		}
     }
* @endcode
 */


int
minfo_add_bookmark(MediaSvcHandle *mb_svc_handle, const char *media_id, const int position, const char* thumb_path);


/**
 * minfo_delete_bookmark
 * This function deletes specified bookmark record from 'video_bookmark' table, the deleted bookmark should be identified by @p bookmark_id.
 * This function actually call the sqlite3 statement, 
 * "DELETE FROM video_bookmark WHERE _id = bookmark_id; "
 * In gallery or ug-imageviewer, user could get a linked list bookmark for some media file, so he/she could delete one of them using @p bookmark_id.
 *
 * @param	mb_svc_handle	[in]	the handle of DB
 * @param	bookmark_id		[in]	_id field in video_bookmark table.
 * @return	This function returns 0 on success, or negative value with error code.
 * @remarks	user should give a correct bookmark ID to successfully delete it.
 * @see  minfo_add_bookmark
 * @pre		None
 * @post	None
 * @par example
 * @code


	#include <media-svc.h>

	void test_minfo_delete_bookmark(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int bookmark_id = 1;

		//delete a bookmark record in 'video_bookmark' table.
		ret = minfo_delete_bookmark(mb_svc_handle, bookmark_id);

		if( ret < 0) {
			printf("minfo_delete_bookmark failed\n");
			return;
		}
     }
* @endcode
 */

int
minfo_delete_bookmark(MediaSvcHandle *mb_svc_handle, const int bookmark_id);




/**
* minfo_get_cluster_id_by_url
* This function gets some folder's full path. This will be called when user want to know what one folder's unique
* ID is.
*
* @return					This function returns 0 on success, and -1 on failure.
* @param	mb_svc_handle	[in]	the handle of DB
* @param	url				[in]	folder  path
* @param	cluster_id		[out]	folder ID
* @exception	None.
* @remarks		None.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_cluster_id_by_url(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		char cluster_id[256] = {0,};
		char *url = "/opt/media/Images/Wallpapers";

		//get cluster's ID using cluster's url.
		ret = minfo_get_cluster_id_by_url(url, cluster_id, sizeof(mb_svc_handle, cluster_id));
		if( ret < 0) {
			printf("test_minfo_get_cluster_id_by_url failed\n");
			return;
		}
     }
* @endcode
*/
int 
minfo_get_cluster_id_by_url(MediaSvcHandle *mb_svc_handle, const char* url, char* cluster_id, int max_length);


/**
* minfo_get_cluster_name_by_id
* This function gets folder's name. This will be called when user want to know what one folder's name
*
* @return	This function returns 0 on success, and -1 on failure.
* @param[in]	mb_svc_handle	the handle of DB
* @param[in]	cluster_id		folder ID
* @param[out]	cluster_name	folder name
* @param[in]	max_length		The max length of the returned folder name.
* @exception	None.
* @remarks	None.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_cluster_name_by_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		char *cluster_name[1024];

		//get cluster's name using cluster's id.
		ret = minfo_get_cluster_name_by_id(mb_svc_handle, cluster_id, cluster_name, sizeof(cluster_name));

		if( ret < 0) {
			printf("test_minfo_get_cluster_name_by_id failed\n");
			return;
		} else {
			printf("cluster name is %s\n", cluster_name);
			return;
		}
	}
* @endcode
*/
int 
minfo_get_cluster_name_by_id(MediaSvcHandle *mb_svc_handle, const char *cluster_id, char *cluster_name, int max_length );

/**
* minfo_get_cluster_fullpath_by_id
* This function gets folder's full path. This will be called when user want to know what one folder's full path.
* User should specify the maximum length of the @p folder_path, so as to avoid over flow of the string.
*
* @return	This function returns 0 on success, and -1 on failure.
* @param[in]	mb_svc_handle	the handle of DB
* @param[in]	cluster_id		folder ID
* @param[out]	folder_path		folder path name
* @param[in]	max_length		specify the maximum length of @p folder_path.
* @exception	None.
* @remarks	None.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_cluster_fullpath_by_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		char *folder_path[1024];

		//get cluster's path name using cluster's id.
		ret = minfo_get_cluster_fullpath_by_id(mb_svc_handle, cluster_id, folder_path, sizeof(folder_path));

		if( ret < 0) {
			printf("test_minfo_get_cluster_fullpath_by_id failed\n");
			return;
		} else {
			printf("path name is %s\n", cluster_name);
			return;
		}
     }
* @endcode
*/

int
minfo_get_cluster_fullpath_by_id(MediaSvcHandle *mb_svc_handle, const char *cluster_id, char *folder_path, int max_length);



/**
* minfo_set_cluster_lock_status
* @fn     int  minfo_set_cluster_lock_status( int cluster_id, int lock_status );
* This function set status for lock to DB. This will be called when user want to set to lock an album.
*
* @return	This function returns 0 on success, and -1 on failure.
* @param[in]	mb_svc_handle	the handle of DB
* @param[in]	cluster_id		folder ID
* @param[in]	lock_status		status for lock to be saved ( 0 : unlock, 1 : lock )
* @exception	None.
* @remarks	None.
* @see	minfo_get_cluster_lock_status.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_set_cluster_lock_status(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		int status = 1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//set s cluster lock status.
		ret = minfo_set_cluster_lock_status(mb_svc_handle, cluster_id, status);

		if( ret < 0) {
			printf("test_minfo_set_cluster_lock_status failed\n");
			return;
		}
	}
* @endcode
*/

int
minfo_set_cluster_lock_status(MediaSvcHandle *mb_svc_handle, const char *cluster_id, int lock_status);

/**
* minfo_get_cluster_lock_status
* @fn     int  minfo_get_cluster_lock_status( int cluster_id, int *lock_status );
* This function gets status for lock from DB.  This will be called when user want to get lock status for an album.
*
* @return	This function returns 0 on success, and -1 on failure.
* @param[in]	mb_svc_handle	the handle of DB
* @param[in]	cluster_id		folder ID
* @param[out]	lock_status		status for cuurent lock status
* @exception	None.
* @remarks	None.
* @see	minfo_set_cluster_lock_status.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_cluster_lock_status(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
		int status = -1;

		//get a cluster's status.
		ret = minfo_get_cluster_lock_status(mb_svc_handle, cluster_id, &status);

		if( ret < 0) {
			printf("test_minfo_get_cluster_lock_status failed\n");
			return;
		} else {
			print("Current status : %d\n", status);
			return;
		}
	}
* @endcode
*/

int
minfo_get_cluster_lock_status(MediaSvcHandle *mb_svc_handle, const char *cluster_id, int *lock_status );

/**
* @fn     int  minfo_get_media_path( minfo_store_type storage_type, char* media_path, size_t max_media_path);
* This function gets the path of media.  This will be called when user want to get path of direcotry containing media in device.
*
* @return	This function returns 0 on success, and -1 on failure.
* @param[in]	storage_type	store type, which means type of device containg media.
* @param[out]	media_path		path of media
* @param[in]	max_media_path	The max length of the returned media_path.
* @exception	None.
* @remarks	None.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_media_path(void)
	{
		int ret = -1;
		char media_path[256] = {'\0'};

		//get media's fullpath.
		ret = minfo_get_media_path(MINFO_PHONE, media_path, sizeof(media_path));

		if( ret < 0) {
			printf("minfo_get_media_path failed\n");
			return;
		} else {
			print("The returned path : %s\n", media_path);
			return;
		}
	}
* @endcode
*/

int
minfo_get_media_path(minfo_store_type storage_type, char* media_path, size_t max_media_path );


/**
 * 	minfo_set_db_valid
 * 	This function set whether all the media contents in a type of storage are valid, like phone or MMC.
 * 	Actually media service will filter all the media contents query from database by the media validation.
 *	This function is always used for MMC card insert/inject operation, in file manager service library.
 * 	When inject a MMC card, the media records for MMC are not deleted really, but are set to be invalid.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	storage_type	information for storage type
 * 	@param[in]	valid			whether the track item is valid.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_delete_invalid_media_records.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void set_db_valid(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	bool valid = TRUE;

	//set the validation of medias in MMC storage in db.
	ret = minfo_set_db_valid(mb_svc_handle, MINFO_MMC, valid);
	if (ret < 0) {
		printf( "failed to set db invalid. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int minfo_set_db_valid(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type, int valid);

/**
 * 	minfo_set_item_valid_start
 * 	This function set whether the media content in a type of storage is valid, like phone or MMC.
 * 	Actually media service will filter all the media contents query from database by the media validation.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	trans_count		count of trasaction user wants
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_set_item_valid, minfo_set_item_valid_end
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void set_item_valid_batch(MediaSvcHandle *mb_svc_handle)
{
	int i;
	int ret = MB_SVC_ERROR_NONE;
	bool valid = TRUE;

	ret = minfo_set_item_valid_start(mb_svc_handle, 100);
	if (ret < 0) {
		printf( "minfo_set_item_valid_start failed. error code->%d", ret);
		return;
	}

	for (i = 0; i < 200; i++) {
		//set the validation of a media in MMC storage in db.
		ret = minfo_set_item_valid(mb_svc_handle, MINFO_MMC, image_files[i], valid);
		if (ret < 0) {
			printf( "failed to set item valid. error code->%d", ret);
			return;
		}
	}

	ret = minfo_set_item_valid_end(mb_svc_handle);
	if (ret < 0) {
		printf( "minfo_set_item_valid_end failed. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int minfo_set_item_valid_start(MediaSvcHandle *mb_svc_handle, int trans_count);


/**
 * 	minfo_set_item_valid_end
 * 	This function set whether the media content in a type of storage is valid, like phone or MMC.
 * 	Actually media service will filter all the media contents query from database by the media validation.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_set_item_valid_start, minfo_set_item_valid
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void set_item_valid_batch(MediaSvcHandle *mb_svc_handle)
{
	int i;
	int ret = MB_SVC_ERROR_NONE;
	bool valid = TRUE;

	ret = minfo_set_item_valid_start(mb_svc_handle, 100);
	if (ret < 0) {
		printf( "minfo_set_item_valid_start failed. error code->%d", ret);
		return;
	}

	for (i = 0; i < 200; i++) {
		//set the validation of a media in MMC storage in db.
		ret = minfo_set_item_valid(mb_svc_handle, MINFO_MMC, image_files[i], valid);
		if (ret < 0) {
			printf( "failed to set item valid. error code->%d", ret);
			return;
		}
	}

	ret = minfo_set_item_valid_end(mb_svc_handle);
	if (ret < 0) {
		printf( "minfo_set_item_valid_end failed. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int minfo_set_item_valid_end(MediaSvcHandle *mb_svc_handle);


/**
 * 	minfo_set_item_valid
 * 	This function set whether the media content in a type of storage is valid, like phone or MMC.
 * 	Actually media service will filter all the media contents query from database by the media validation.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	storage_type	information for storage type
 * 	@param[in]	full_path		The path of the media
 * 	@param[in]	valid			whether the track item is valid.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_set_item_valid_start, minfo_set_item_valid_end
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void set_item_valid_batch(MediaSvcHandle *mb_svc_handle)
{
	int i;
	int ret = MB_SVC_ERROR_NONE;
	bool valid = TRUE;

	ret = minfo_set_item_valid_start(mb_svc_handle, 100);
	if (ret < 0) {
		printf( "minfo_set_item_valid_start failed. error code->%d", ret);
		return;
	}

	for (i = 0; i < 200; i++) {
		//set the validation of a media in MMC storage in db.
		ret = minfo_set_item_valid(mb_svc_handle, MINFO_MMC, image_files[i], valid);
		if (ret < 0) {
			printf( "failed to set item valid. error code->%d", ret);
			return;
		}
	}

	ret = minfo_set_item_valid_end(mb_svc_handle);
	if (ret < 0) {
		printf( "minfo_set_item_valid_end failed. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */


int minfo_set_item_valid(MediaSvcHandle *mb_svc_handle,
				const minfo_store_type storage_type,
				const char *full_path,
				int valid);


/**
 * 	minfo_delete_invalid_media_records
 * 	This function delete all of invalid media records in a type of storage are valid, like phone or MMC.
 * 	Actually media service will filter all the media contents query from database by the media validation.
 *	This function is always used for MMC card insert/inject operation, in file manager service library.
 * 	When inject a MMC card, the media records for MMC are not deleted really, but are set to be invalid.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]   storage_type	information for storage type
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_set_db_valid.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void test_minfo_delete_invalid_media_records(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;

	//delete the invalid media records in MMC storage in db.
	ret = minfo_delete_invalid_media_records(mb_svc_handle, MINFO_MMC);
	if (ret < 0) {
		printf( "failed to delete invalid items. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_delete_invalid_media_records(MediaSvcHandle *mb_svc_handle, const minfo_store_type storage_type);

/**
 * 	minfo_delete_tag
 * 	This function could delete a tag or some member of the tag in 'media_tag' table in database. When user pass @p media_id not equal
 *    to -1, the tag will be deleted, otherwise some member of the tag will be deleted. Whatever cases, user should pass the correct tag name with
 *    @p tag_name to successfully delete.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	media_id	identify a media item with this ID
 * 	@param[in]	tag_name	name of deleted tag
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void delete_a_tag(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;

	//delete all tag records in 'media_tag' in db, whose tag name is 'test_tag'.
	ret = minfo_delete_tag(mb_svc_handle, -1, "test tag");
	if (ret < 0) {
		printf( "failed to delete a tag record. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_delete_tag(MediaSvcHandle *mb_svc_handle, const char *media_id, const char* tag_name);


/**
 * 	minfo_rename_tag:
 * 	This function could rename a tag_name  to another tag_name in 'media_tag' table in database. User need to pass @p src_tagname which indicate original 
 *    tag name, @p dst_tag_name which is new tag name replacing @p src_tagname. This function will check whether the new tag name, @p dst_tag_name, has
 *    existed in 'media_tag' table. If yes, this function will item by item replace old tag name, if no, this function will directly update old tag name to new tag name.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	src_tagname		identify original tag name
 * 	@param[in]	dst_tag_name	new tag name.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void test_minfo_rename_tag(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;

	//rename all tag records with new tag name 'test_tag2'.
	ret = minfo_rename_tag(mb_svc_handle, "test tag1", "test tag2");
	if (ret < 0) {
		printf( "failed to rename tag records. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_rename_tag(MediaSvcHandle *mb_svc_handle, const char* src_tagname, const char* dst_tag_name);

/**
 * 	minfo_rename_tag_by_id:
 * 	This function could rename a tag_name for some tag record to another tag_name in 'media_tag' table in database. User need to pass @p src_tagname which indicate original 
 *    tag name, @p media_id which combine with the @p src_tagname to indentify one tag record, @p dst_tag_name which is new tag name replacing @p src_tagname. 
 *    This function will check whether the new tag name with @p media_id has existed in 'media_tag' table. If yes, this function will delete old tag record, if no, this function will directly 
 *    update old tag record  to new tag record.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	media_id		identify original tag record with @p src_tagname
 * 	@param[in]	src_tagname		identify original tag record with @p media_id
 * 	@param[in]	dst_tag_name	new tag name.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void test_minfo_rename_tag_by_id(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";

	//rename some tag record with new tag name 'test_tag2'.
	ret = minfo_rename_tag_by_id(mb_svc_handle, media_id, "test tag1", "test tag2");
	if (ret < 0) {
		printf( "failed to rename tag records. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_rename_tag_by_id(MediaSvcHandle *mb_svc_handle, const char *media_id, const char* src_tagname, const char* dst_tag_name);




/**
 * 	minfo_add_tag:
 * 	This function could add a new tag into 'media_tag' table in database. When user create a new tag and will
 *    not add any media item to it, he/she should set @p media_id as 0. When user create a new tag and want to add 
 *    some media items to it, he/she should do a loop to insert them into 'media_tag' table in database, meanwhile
 *    should fill up @p media_id and @p tag_name with appropriate values.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	media_id		identify a media item with this ID
 * 	@param[in]	tag_name		name of new added tag
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void add_a_tag(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;

	//add a tag record in 'media_tag' in db, and not add any media item to it.
	ret = minfo_add_tag(mb_svc_handle, NULL, "test tag");
	if (ret < 0)
	{
		printf( "failed to add a tag record. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int
minfo_add_tag(MediaSvcHandle *mb_svc_handle, const char *media_id, const char* tag_name);

/**
 * 	minfo_get_media_list_by_tagname:
 * 	This function could get a media items' list who are included to the same tag according to tag name .
 * 	User could dictate whether he/she hope to get meta data of media item with @p with_meta. Yes if TRUE,
 *    no if FALSE.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	tag_name	tag name
 * 	@param[in]	with_meta	indicate whether want to get meta data of media item
 * 	@param[in]	func		Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * 	@param[out]	user_data	user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

int mitem_ite_cb(Mitem *item, void *user_data)
{
	GList** list = (GList**)user_data;
	*list = g_list_append(*list, item);
}


void get_media_list_tag_name(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	GList *p_list = NULL;

	//get a media items' list who are included to the same tag with 'test tag'.
	ret = minfo_get_media_list_by_tagname(mb_svc_handle, "test tag", FALSE, mitem_ite_cb, &p_list);
	if (ret < 0) {
		printf( "failed to get a media items' list. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_get_media_list_by_tagname(MediaSvcHandle *mb_svc_handle, const char* tag_name, bool with_meta, minfo_item_ite_cb func, void* user_data );

/**
 * 	minfo_get_media_list_by_tagname_with_filter:
 * 	This function could get a media items' list who are included to the same tag according to tag name and filter.
 * 	User could dictate whether he/she hope to get meta data of media item with @p with_meta. Yes if TRUE,
 *  no if FALSE.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	tag_name		tag name
 *  @param[in]	filter			the filter to specify some tag filter conditions, like, type of got items, sort by type, start and end positions of items, including meta data or not, etc.
 * 	@param[in]	func			Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * 	@param[out]	user_data		user's data structure to contain items of the type Mitem. It is passed to the iterative callback.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

int mitem_ite_cb(Mitem *item, void *user_data)
{
	GList** list = (GList**)user_data;
	*list = g_list_append(*list, item);
}


void get_media_list_by_tagname_with_filter(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	GList *p_list = NULL;
	minfo_tag_filter filter;

	filter.start_pos = 0;
	filter.end_pos = 3;
	filter.file_type = MINFO_ITEM_ALL;
	filter.with_meta = FALSE;

	//get a media items' list who are included to the same tag with 'test tag'.
	ret = minfo_get_media_list_by_tagname_with_filter(mb_svc_handle, "test tag", filter, mitem_ite_cb, &p_list);
	if (ret < 0) {
		printf( "failed to get a media items' list. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_get_media_list_by_tagname_with_filter(MediaSvcHandle *mb_svc_handle, const char* tag_name, minfo_tag_filter filter, minfo_item_ite_cb func, void* user_data );

/**
 * 	minfo_get_media_count_by_tagname:
 * 	This function could get count of media items, which are included to the same tag according to tag name .
 * 	User could dictate whether he/she hope to get count of media items.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	tag_name		tag name
 * 	@param[out]	count			count of media items
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void test_minfo_get_media_count_by_tagname(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	int count = 0;

	//get count of media items, which are included to the same tag with 'test tag'.
	ret = minfo_get_media_count_by_tagname(mb_svc_handle, "test tag", &count);
	if (ret < 0) {
		printf( "failed to get a media items' list. error code->%d", ret);
	} else {
		printf( "Count is %d\n", count );	
	}

	return;
}

 * 	@endcode
 */

int
minfo_get_media_count_by_tagname(MediaSvcHandle *mb_svc_handle, const char* tag_name, int* count );

/**
 * 	minfo_get_tag_list_by_media_id:
 * 	This function could get a tags' list whose memeber is Mtag type. User should pass @p media_id to indicate which
 *    media item will be searched. Also he/she should define a callback function to be called by this function.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	media_id		identify a media item with ID
 * 	@param[in]	func			Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
 * 	@param[out]	user_data		user's data structure to contain items of the type Mtag. It is passed to the iterative callback.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_set_db_valid.
 *	@pre	None
 *	@post	None
 *	@remark	None
 * 	@par example
 * 	@code

#include <media-svc.h>

int mtag_ite_cb(Mtag *i_tag, void *user_data)
{
	GList** list = (GList**)user_data;
	*list = g_list_append(*list, item);
}


void get_tag_list_media_id(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	const char *media_id = "b6a4f4ac-26ea-458c-a228-9aef7f70349d";
	GList *p_list = NULL;

	//get a tags' list which include the same media item and it's media_id is b6a4f4ac-26ea-458c-a228-9aef7f70349d.
	ret = minfo_get_tag_list_by_media_id(mb_svc_handle, media_id, mtag_ite_cb, &p_list);
	if (ret < 0) {
		printf( "failed to get a tags' list. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int
minfo_get_tag_list_by_media_id(MediaSvcHandle *mb_svc_handle, const char *media_id, minfo_tag_ite_cb func, void* user_data);

/**
 * 	minfo_add_web_cluster
 * 	This function could add a web album through specifying it's @p name, @p account_id. After adding a web
 *    album, this function will return @p id.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	sns_type		sns type, like, facebook, flickr, etc.
 * 	@param[in]	name		new added web album's name.
 * 	@param[in]	account_id	account ID.
 * 	@param[out]	id			return album's id.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_add_web_cluster_album_id.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>


void add_web_cluster(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	char cluster_id[256] = {0,};
	
	//add a web album.
	ret = minfo_add_web_cluster(mb_svc_handle, 1, "web_album",  "1", cluster_id, sizeof(cluster_id));
	if (ret < 0) {
		printf( "failed to add a web album. error code->%d", ret);
	}

	return;
}

 * 	@endcode
 */

int
minfo_add_web_cluster(MediaSvcHandle *mb_svc_handle, int sns_type, const char* name,const char *account_id, char* id, int max_length);


/**
 * 	minfo_add_web_cluster_album_id
 * 	This function could add a web album through specifying it's @p name, @p account_id, @p album_id. After adding a web
 *    album, this function will return @p id.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	sns_type	sns type, like, facebook, flickr, etc.
 * 	@param[in]	name		new added web album's name.
 * 	@param[in]	account_id	account ID.
 * 	@param[in]	album_id	web album id
 * 	@param[out]	id			return album's id.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	minfo_add_web_cluster.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>


void add_web_cluster_album_id(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	char cluster_id[256] = {0,};

	//add a web album.
	ret = minfo_add_web_cluster_album_id(mb_svc_handle, 1, "web_album",  "1", "1", cluster_id, sizeof(cluster_id));
	if (ret < 0) {
		printf( "failed to add a web album. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int
minfo_add_web_cluster_album_id(MediaSvcHandle *mb_svc_handle, int sns_type, const char* name, const char *account_id, const char *album_id, char *id, int max_length);

/**
 * 	minfo_delete_web_cluster
 * 	This function could delete a web album through specifying @p cluster_id. After deleteing a web
 *    album, the application will not be able to get this web album displaying.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	cluster_id		cluster ID identifying a web album.
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void delete_web_cluster(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
	
	//delete a web album.
	ret = minfo_delete_web_cluster(mb_svc_handle, cluster_id);
	if (ret < 0) {
		printf( "failed to delete a web album. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int
minfo_delete_web_cluster(MediaSvcHandle *mb_svc_handle, const char *cluster_id);

/**
* minfo_get_web_cluster_by_web_account_id
* This function gets list of mcluster by web account id. User could get the detail information with
* the type 'Mcluster' about this cluster, the type 'Mcluster' mainly include folder/cluster ID, display name, 
* count of included media content, etc. The detail defination of this type could refer to the herder file, minfo_item/minfo-cluster.h.
*
* @return This function returns 0 on success, or negative value with error code.
*         Please refer 'media-svc-error.h' to know the exact meaning of the error.
* @param	mb_svc_handle	[in]	the handle of DB
* @param	web_account_id	[in]	the web account ID which indentify a cluster
* @param	func			[in]	Iterative callback implemented by a user. This callback is called when an item has to be inserted to user's list.
* @param	user_data		[out]	user's data structure to contain items of the type Mcluster. It is passed to the iterative callback.
* @exception None.
* @remarks User could pass web account id to this function, so that
*          this function still could get the wanted list of clusters
*          when invoking this function.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code


	#include <media-svc.h>
	int mcluster_ite_cb(Mcluster *cluster, void *user_data)
	{
		GList** list = (GList**)user_data;
		*list = g_list_append(*list, cluster);
	}

	void test_minfo_get_web_cluster_by_web_account_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		const char* account_id = "user";
		GList *p_list = NULL;
		
		//get a web cluster using account id.
		ret = minfo_get_web_cluster_by_web_account_id(mb_svc_handle, account_id, mcluster_ite_cb, &p_list);
		if(ret < 0) {
			printf("minfo_get_web_cluster_by_web_account_id fail: %d \n", ret);
			return;
		}
	 }
* @endcode
*/

int
minfo_get_web_cluster_by_web_account_id(MediaSvcHandle *mb_svc_handle, const char* web_account_id, minfo_cluster_ite_cb func, void *user_data);

/**
* minfo_get_web_cluster_web_album_id
* This function gets mcluster information by web cluster id. User could get the detail information with
* the type 'Mcluster' about this cluster, the type 'Mcluster' mainly include folder/cluster ID, display name,
* count of included media content, etc. The detail defination of this type could refer to the herder file, minfo-cluster.h.
*
* @return	This function returns 0 on success, or negative value with error code.
* @param	mb_svc_handle	[in]	the handle of DB
* @param	cluster_id		[in]	the cluster ID which indentify a cluster
* @param	mcluster		[out]	mcluster to be returned, which is a 'Mcluster' type
* @exception None.
* @remarks User could pass cluster id to this function, so that
*          this function still could get the wanted cluster.
*          when invoking this function, *mcluster must equals NULL, and
*          after using mitem, it must be freed with minfo_destroy_mtype_item.
* @see	None.
* @pre	None
* @post	None
* @par example
* @code

	#include <media-svc.h>

	void test_minfo_get_web_cluster_web_album_id(MediaSvcHandle *mb_svc_handle)
	{
		int ret = -1;
		Mcluster *mc = NULL;
		const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";

		//get a cluster using cluster's id.
		ret = minfo_get_web_cluster_web_album_id(mb_svc_handle, cluster_id, &mc);
		if(ret < 0) {
			printf("minfo_get_web_cluster_web_album_id fail: %d \n", ret);
			return;
		}

		minfo_destroy_mtype_item(mc);
	 }
* @endcode
*/

int
minfo_get_web_cluster_web_album_id(MediaSvcHandle *mb_svc_handle, const char *web_album_id, Mcluster **mcluster);

/**
 * 	minfo_add_web_media
 * 	This function could add a web media to web album specified by @p cluster_id, in addition, user need to pass @p http_url, @p file_name
 *    @p thumb_path. If failed to add it to web album, this function will return an error code.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	cluster_id		specify cluster id to indentify a web album.
 * 	@param[in]	http_url		web media's url.
 * 	@param[in]	file_name		file name.
 * 	@param[in]	thumb_path		thumbnail full path of this web media
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remarks	None
 * 	@par example
 * 	@code

#include <media-svc.h>

void test_minfo_add_web_media(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
	
	//add a web media to a web album.
	ret = minfo_add_web_media(mb_svc_handle, cluster_id, "http://user/specifying/address",  "web_media", "thumbnail path");
	if (ret < 0) {
		printf( "failed to add to a web album. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

DEPRECATED_API int
minfo_add_web_media(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const char* http_url, const char* file_name, const char* thumb_path);

/**
 * 	minfo_add_web_media_with_type
 * 	This function could add a web media to web album specified by @p cluster_id, in addition, user need to pass @p http_url, @p file_name, @p content_type, 
 *    @p thumb_path. If failed to add it to web album, this function will return an error code.
 *
 * 	@param[in]	mb_svc_handle	the handle of DB
 * 	@param[in]	cluster_id		specify cluster id to indentify a web album.
 * 	@param[in]	http_url		web media's url.
 * 	@param[in]	file_name		file name.
 * 	@param[in]	content_type	type of the media.
 * 	@param[in]	thumb_path		thumbnail full path of this web media
 * 	@return	This function returns zero(MB_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'media-svc-error.h' to know the exact meaning of the error.
 * 	@see	None.
 *	@pre	None
 *	@post	None
 *	@remark	None
 * 	@par example
 * 	@code

#include <media-svc.h>


void test_minfo_add_web_media_with_type(MediaSvcHandle *mb_svc_handle)
{
	int ret = MB_SVC_ERROR_NONE;
	const char *cluster_id = "51298053-feb7-4261-a1c8-26b05a6e0ae0";
	
	//add a web media to a web album.
	ret = minfo_add_web_media_with_type(mb_svc_handle, cluster_id, "http://user/specifying/address",  "web_media", MINFO_ITEM_IMAGE, "thumbnail name");
	if (ret < 0) {
		printf( "failed to add to a web album. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int
minfo_add_web_media_with_type(MediaSvcHandle *mb_svc_handle, const char *cluster_id, const char* http_url, const char* file_name, minfo_file_type content_type, const char* thumb_path);

/**
	@}
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */





#endif /*_VISUAL_SVC_H_*/


