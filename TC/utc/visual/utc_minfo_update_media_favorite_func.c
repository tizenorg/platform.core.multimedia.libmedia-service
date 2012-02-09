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
* @file 	utc_minfo_update_media_favorite_func.c
* @brief 	This is a suit of unit test cases to test minfo_update_media_favorite API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_update_media_favorite_func.h"


/**
* @brief	This tests int minfo_update_media_favorite() API with valid parameter
* 		update 'favorite' feild of a media content in media table.
* @par ID	utc_minfo_update_media_name_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_update_media_favorite_func_01()
{
	int ret = -1;
	const char *media_uuid = "aa33f347-988b-41f4-8a53-9df24ea86bc4";
	int favorite_level = 1;

    ret = minfo_update_media_favorite(handle, media_uuid, favorite_level);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to update 'favorite' feild of a media content in media table. error code->%d", ret);
	}
	
    ret = minfo_update_media_favorite(handle, media_uuid, 0);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to reupdate 'favorite' feild of a media content in media table. error code->%d", ret);
	}

	dts_pass(API_NAME, "utc_minfo_update_media_favorite_func_01 succeeded");
	
}


/**
* @brief 		This tests int minfo_update_media_favorite() API with invalid parameter
* 			update 'favorite' feild of a media content in media table.
* @par ID	utc_minfo_update_media_name_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_update_media_favorite_func_02()
{
	int ret = -1;
	const char *media_uuid = NULL;
	int favorite_level = 1;

    ret = minfo_update_media_favorite(handle, media_uuid, favorite_level);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "update 'favorite' feild of a media content in media table should be failed because of the new_name NULL.");
}
