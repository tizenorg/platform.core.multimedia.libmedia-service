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
* @file 	utc_minfo_delete_bookmark_func.c
* @brief 	This is a suit of unit test cases to test minfo_delete_bookmark API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_delete_bookmark_func.h"


/**
* @brief	This tests int minfo_delete_bookmark() API with valid parameter
* 		delete a bookmark to a media file.
* @par ID	utc_minfo_delete_bookmark_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_delete_bookmark_func_01()
{
	int ret = -1;
	 
	int bookmark_id = 1;
	UTC_MINFO_INIT()
	ret = minfo_delete_bookmark(bookmark_id);
	
	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "unable to delete a bookmark to a media file. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}
	
	UTC_MINFO_FINALIZE()
	tet_result(TET_PASS);
	
	return;
}


/**
* @brief 		This tests int minfo_delete_bookmark() API with invalid parameter
* 			delete a bookmark to a media file.
* @par ID	utc_minfo_delete_bookmark_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_delete_bookmark_func_02()
{	
	int ret = -1;
	 
	int bookmark_id = -1;
	UTC_MINFO_INIT()
	ret = minfo_delete_bookmark(bookmark_id);
	

	if (ret<0)
	{
		UTC_MM_LOG("abnormal condition test for null, error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_PASS);
	}
	else
	{
		UTC_MM_LOG("delete a bookmark to a media file should be failed because of the bookmark_id parameter -1.");
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
	}

	return ;
}
