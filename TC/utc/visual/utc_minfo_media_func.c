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
* @file 	utc_minfo_free_glist_func.c
* @brief 	This is a suit of unit test cases to test minfo_add_media, minfo_copy_media, minfo_cp_media, minfo_delete_media, minfo_delete_media_id API functions
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_media_func.h"

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

int _get_id_by_url(const char* url, char *id)
{
	int ret;
	Mitem* item = NULL;

	ret = minfo_get_item(handle, url, &item );
	if( ret < 0 ) {
		dts_message(API_NAME, "minfo_get_item fail");
		return -1;
	}

	strncpy(id, item->uuid, 256);

	return 0;
}


/**
* @brief	This tests int minfo_add_media() API with valid parameter
* 		Add a media content to media table.
* @par ID	utc_minfo_add_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_add_media_func_01()
{
	int err = -1;
	char *file_url = "/opt/media/Images/Wallpapers/Home_01.jpg";
	int type = 1;

	err = minfo_add_media(handle, file_url, type);

	dts_check_ge(API_NAME, err, MB_SVC_ERROR_NONE, "unable to Add a media content to media table. error code->%d", err);
}


/**
* @brief 		This tests int minfo_add_media() API with invalid parameter
* 			Add a media content to media table.
* @par ID	utc_minfo_add_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_add_media_func_02()
{	
	int err = -1;
	char *file_url = NULL; /*= "/opt/media/Images/Wallpapers/Home_01.jpg";*/
	int type = 1;

	err = minfo_add_media(handle, file_url, type);	
		
	dts_check_lt(API_NAME, err, MB_SVC_ERROR_NONE,"Add a media content to media table should be failed because of the file_url NULL.");
}

/**
* @brief	This tests int minfo_copy_media() API with valid parameter
* 		Copy a media content from media table.
* @par ID	utc_minfo_copy_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_copy_media_func_01()
{
	int err = -1;
	char *old_file_url = "/opt/media/Images/Wallpapers/Home_default.jpg";
	char *new_file_url = "/opt/media/Images/Wallpapers/Home_default_1.jpg";
	int type = 1;

	err = minfo_copy_media(handle, old_file_url, new_file_url, type);

	dts_check_ge(API_NAME, err, MB_SVC_ERROR_NONE, "unable to Copy a media content from media table.. error code->%d", err);

}


/**
* @brief 		This tests int minfo_copy_media() API with invalid parameter
* 			Copy a media content from media table.
* @par ID	utc_minfo_copy_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_copy_media_func_02()
{	
	int err = -1;
	char *old_file_url = NULL; /*= "/opt/media/Images/Wallpapers/Home_default.jpg";*/
	char *new_file_url = "/opt/media/Images/Wallpapers/Home_default_1.jpg";
	int type = 1;

	err = minfo_copy_media(handle, old_file_url, new_file_url, type);
		
	dts_check_lt(API_NAME, err, MB_SVC_ERROR_NONE, "Copy a media content from media table should be failed because of the file_url NULL.");
}


/**
* @brief	This tests int minfo_cp_media() API with valid parameter
* 		copy a record identified by media id to destination folder identified by folder id.
* @par ID	utc_minfo_cp_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_cp_media_func_01()
{
	int ret = -1;
	
	char *file_url = "/opt/media/Images/Wallpapers/Home_default.jpg";
	char src_media_uuid[256] = {0,};
	char src_cluster_uuid[256] = {0,};
	char *dst_cluster_uuid = "8ac1df34-efa8-4143-a47e-5b6f4bac8c96";

	Mitem* item = NULL;

	ret = minfo_get_item(handle, file_url, &item );
	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail( API_NAME, "unable to get a media content from media table. error code->%d", ret);
		return;
	}

	strncpy(src_media_uuid, item->uuid, sizeof(src_media_uuid));
	strncpy(src_cluster_uuid, item->cluster_uuid, sizeof(src_cluster_uuid));

	minfo_destroy_mtype_item(item);	

	ret = minfo_cp_media(handle, src_media_uuid, dst_cluster_uuid);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "failed to copy a record identified by media id to destination folder identified by folder id. error code->%d", ret);
		return;
	}
	
	GList *p_list = NULL;
	minfo_item_filter item_filter = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,false,false};
	Mitem* dest_item = NULL;

	ret = minfo_get_item_list(handle, dst_cluster_uuid, item_filter, _ite_fn, &p_list);
	
	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to get media records. error code->%d", ret);
	}

	int len = g_list_length( p_list );
	dest_item = (Mitem*)g_list_nth_data(p_list, len-1);
	ret = minfo_delete_media_id(handle, dest_item->uuid );

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to delete media records. error code->%d", ret);
	}

	minfo_destroy_mtype_item(dest_item);	

	dts_pass(API_NAME, "minfo_cp_media succeeded");

	return;
}

/**
* @brief 		This tests int minfo_cp_media() API with invalid parameter
* 			copy a record identified by media id to destination folder identified by folder id.
* @par ID	utc_minfo_cp_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_cp_media_func_02()
{
	int ret = -1;
	
	char *src_media_uuid = NULL;
	char *dst_cluster_uuid = NULL;

	ret = minfo_cp_media(handle, src_media_uuid, dst_cluster_uuid);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"copy a record identified by media id to destination folder identified by folder id should be failed because of the src_media_id -1.");
}

/**
* @brief	This tests int minfo_delete_media() API with valid parameter
* 		Delete a media content from media table.
* @par ID	utc_minfo_delete_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_delete_media_func_01()
{
	int err = -1;
	char *file_url = "/opt/media/Images/Wallpapers/Home_01.jpg";

	err = minfo_delete_media(handle, file_url);

	dts_check_ge(API_NAME, err, MB_SVC_ERROR_NONE, "unable to Add a media content to media table. error code->%d", err);
}


/**
* @brief 		This tests int minfo_delete_media() API with invalid parameter
* 			Delete a media content from media table.
* @par ID	utc_minfo_delete_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_delete_media_func_02()
{	
	int err = -1;
	char *file_url = NULL; /*= "/opt/media/Images/Wallpapers/Home_01.jpg";*/

	err = minfo_delete_media(handle, file_url);
	dts_check_lt(API_NAME, err, MB_SVC_ERROR_NONE,"Delete a media content from media table should be failed because of the file_url NULL.");
}


/**
* @brief	This tests int minfo_delete_media_id() API with valid parameter
* 		delete a record identified by media id from 'media' table.
* @par ID	utc_minfo_delete_media_id_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_delete_media_id_func_01()
{
	int ret = -1;
	char media_uuid[256] = {0,};
	
	_get_id_by_url("/opt/media/Images/Wallpapers/Home_default_1.jpg", media_uuid);
	ret = minfo_delete_media_id(handle, media_uuid);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to delete a record identified by media id from 'media' table. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_delete_media_id() API with invalid parameter
* 			delete a record identified by media id from 'media' table.
* @par ID	utc_minfo_delete_media_id_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_delete_media_id_func_02()
{
	int ret = -1;
	
	char *media_uuid = NULL;
	ret = minfo_delete_media_id(handle, media_uuid);

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "delete a record identified by media id from 'media' table should be failed because of the media_id -1.");
}
