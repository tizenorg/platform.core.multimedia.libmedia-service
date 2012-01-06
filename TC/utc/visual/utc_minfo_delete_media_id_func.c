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
* @file 	utc_minfo_delete_media_id_func.c
* @brief 	This is a suit of unit test cases to test minfo_delete_media_id API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_delete_media_id_func.h"

int _get_id_by_url(const char* url)
{
	int id;
	int ret;
	Mitem* item = NULL;

	ret = minfo_get_item( url, &item );
	if( ret < 0 ) {
		UTC_MM_LOG("minfo_get_item failed : %d", ret );
		return -1;
	}

	return item->_id;
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
	
	UTC_MINFO_INIT()
	int media_id = _get_id_by_url("/opt/media/Images and videos/Wallpapers/Home_default_1.png");
	ret = minfo_delete_media_id(media_id);

	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "failed to delete a record identified by media id from 'media' table. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}
	
	UTC_MINFO_FINALIZE()
	tet_result(TET_PASS);

	return;
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
	
	int media_id = -1;
	UTC_MINFO_INIT()
	ret = minfo_delete_media_id(media_id);
		
	if (ret<0)
	{
		UTC_MM_LOG("abnormal condition test for null, error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_PASS);
	}
	else
	{
		UTC_MM_LOG("delete a record identified by media id from 'media' table should be failed because of the media_id -1.");
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
	}

	return ;
}
