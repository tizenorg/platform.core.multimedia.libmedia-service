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
* @file 	utc_minfo_bookmark_func.c
* @brief 	This is a suit of unit test cases to test minfo_add_bookmark and minfo_delete_bookmark API functions
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_bookmark_func.h"

/**
* @brief	This tests int minfo_add_bookmark() API with valid parameter
* 		add a bookmark to a media file.
* @par ID	utc_minfo_add_bookmark_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_add_bookmark_func_01()
{
	int ret = -1;
	 
	int position = 2346;
	const char *media_id = "77b876ed-5db9-4114-82c0-d08e814b5051";
	char *thumb_path = "tmp1";

	ret = minfo_add_bookmark(handle, media_id, position, thumb_path);
	
	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to add a bookmark to a media file. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_add_bookmark() API with invalid parameter
* 			add a bookmark to a media file.
* @par ID	utc_minfo_add_bookmark_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_add_bookmark_func_02()
{	
	int ret = -1;
	 
	int position = 2346;
	const char *media_id = "77b876ed-5db9-4114-82c0-d08e814b5051";
	char *thumb_path = NULL;

	ret = minfo_add_bookmark(handle, media_id,position,thumb_path);

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "add a bookmark to a media file should be failed because of the media_id parameter -1.");
}

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
	ret = minfo_delete_bookmark(handle, bookmark_id);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to delete a bookmark to a media file. error code->%d", ret);
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
	ret = minfo_delete_bookmark(handle, bookmark_id);
	

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "delete a bookmark to a media file should be failed because of the bookmark_id parameter -1.");
}
