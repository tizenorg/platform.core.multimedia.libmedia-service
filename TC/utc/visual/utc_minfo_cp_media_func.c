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
* @file 	utc_minfo_cp_media_func.c
* @brief 	This is a suit of unit test cases to test minfo_cp_media API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_cp_media_func.h"

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
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
	
	char *file_url = "/opt/media/Images and videos/Wallpapers/Home_default.png";
	int src_media_id = 1;
	int src_cluster_id = 2;
	int dst_cluster_id = 2;
	Mitem* item = NULL;
	UTC_MINFO_INIT()

	ret = minfo_get_item( file_url, &item );
	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "unable to get a media content from media table. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}

	src_media_id = item->_id;
	src_cluster_id = item->cluster_id;
	dst_cluster_id = item->cluster_id + 1;

	minfo_destroy_mtype_item(item);	

	ret = minfo_cp_media(src_media_id, dst_cluster_id);

	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "failed to copy a record identified by media id to destination folder identified by folder id. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}
	
	GList *p_list = NULL;
	minfo_item_filter item_filter = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,false,false};
	Mitem* dest_item = NULL;

	ret = minfo_get_item_list(dst_cluster_id, item_filter, _ite_fn, &p_list);
	
	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "unable to get media records. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}

	int len = g_list_length( p_list );
	dest_item = (Mitem*)g_list_nth_data(p_list, len-1);
	ret = minfo_delete_media_id( dest_item->_id );

	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "unable to delete media records. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}

	minfo_destroy_mtype_item(dest_item);	

	UTC_MINFO_FINALIZE()
	tet_result(TET_PASS);

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
	
	int src_media_id = -1;
	int dst_cluster_id = 2;
	UTC_MINFO_INIT()
	ret = minfo_cp_media(src_media_id, dst_cluster_id);
		
	if (ret<0)
	{
		UTC_MM_LOG("abnormal condition test for null, error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_PASS);
	}
	else
	{
		UTC_MM_LOG("copy a record identified by media id to destination folder identified by folder id should be failed because of the src_media_id -1.");
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
	}

	return ;
}
