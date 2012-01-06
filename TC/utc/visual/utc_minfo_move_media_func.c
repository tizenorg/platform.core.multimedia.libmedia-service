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
* @file 	utc_minfo_move_media_func.c
* @brief 	This is a suit of unit test cases to test minfo_move_media API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_move_media_func.h"


/**
* @brief	This tests int minfo_move_media() API with valid parameter
* 		Move a media content from media table.
* @par ID	utc_minfo_move_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_move_media_func_01()
{
	int err = -1;
	char *old_file_url = "/opt/media/Images and videos/Wallpapers/Home_default.png";
	char *new_file_url = "/opt/media/Images and videos/Wallpapers/Home_default_2.png";
	int type = 1;

	err = minfo_move_media(old_file_url, new_file_url, type);

	if (err < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to Move a media content from media table. error code->%d", err);
	}

	err = minfo_move_media(new_file_url, old_file_url, type);
	if (err < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to Move a media content from media table. error code->%d", err);
	}
	
	dts_pass(API_NAME, "utc_minfo_move_media_func_01 succeeded");
	
	return;
}


/**
* @brief 		This tests int minfo_move_media() API with invalid parameter
* 			Move a media content from media table.
* @par ID	utc_minfo_move_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_move_media_func_02()
{	
	int err = -1;
	char *old_file_url = NULL; /*= "/opt/media/Images/Wallpapers/Home_01.png";*/
	char *new_file_url = "/opt/media/Images/Wallpapers/Home_01_1.png";
	int type = 1;

	err = minfo_move_media(old_file_url, new_file_url, type);
	dts_check_lt(API_NAME, err, MB_SVC_ERROR_NONE,"Move a media content from media table should be failed because of the file_url NULL.");
}
