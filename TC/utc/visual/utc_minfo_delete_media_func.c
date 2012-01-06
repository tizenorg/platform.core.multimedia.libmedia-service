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
* @file 	utc_minfo_delete_media_func.c
* @brief 	This is a suit of unit test cases to test minfo_delete_media API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_delete_media_func.h"


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
	char *file_url = "/opt/media/Images and videos/Wallpapers/Home_01.png";

	UTC_MINFO_INIT()
	err = minfo_delete_media(file_url);

	if (err < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "unable to Add a media content to media table. error code->%d", err);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}
	
	UTC_MINFO_FINALIZE()
	tet_result(TET_PASS);
	
	return;
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
	char *file_url = NULL; /*= "/opt/media/Images/Wallpapers/Home_01.png";*/

	UTC_MINFO_INIT()
	err = minfo_delete_media(file_url);
		
	if (err<0)
	{
		UTC_MM_LOG("abnormal condition test for null, error code->%d", err);
		UTC_MINFO_FINALIZE()
		tet_result(TET_PASS);
	}
	else
	{
		UTC_MM_LOG("Delete a media content from media table should be failed because of the file_url NULL.");
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
	}

	return ;
}
