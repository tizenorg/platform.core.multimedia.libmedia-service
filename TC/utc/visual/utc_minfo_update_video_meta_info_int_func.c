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
* @file 	utc_minfo_update_video_meta_info_int_func.c
* @brief 	This is a suit of unit test cases to test minfo_update_video_meta_info_int API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_update_video_meta_info_int_func.h"


/**
* @brief	This tests int minfo_update_video_meta_info_int() API with valid parameter
* 		Rename a media content from media table.
* @par ID	utc_minfo_update_video_meta_info_int_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_update_video_meta_info_int_func_01()
{
	int ret = -1;
	char *file_url = "/opt/media/Images and videos/My video clips/Helicopter.mp4";
	char origin_name[256];
	Mitem* item = NULL;
	memset( origin_name, 0x00, 256 );


	ret = minfo_get_item( file_url, &item );
	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to get a media content from media table. error code->%d", ret);
	}

	if( item ) {
		strncpy( origin_name, item->display_name, sizeof( origin_name ) );
    	ret = minfo_update_video_meta_info_int(item->uuid, MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED, 3000);

		if (ret < MB_SVC_ERROR_NONE)
		{
			dts_fail(API_NAME, "unable to update a media content from media table. error code->%d", ret);
		}

    	ret = minfo_update_video_meta_info_int(item->uuid, MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED, 0);

		if (ret < MB_SVC_ERROR_NONE)
		{
			dts_fail(API_NAME, "unable to reupdate a media content from media table. error code->%d", ret);
		}
	}
		
	dts_pass(API_NAME, "utc_minfo_update_video_meta_info_int_func_01 succeeded");
}


/**
* @brief 		This tests int minfo_update_video_meta_info_int() API with invalid parameter
* 			Rename a media content from media table.
* @par ID	utc_minfo_update_video_meta_info_int_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_update_video_meta_info_int_func_02()
{
	int ret = -1;
	const char *media_uuid = NULL;
	minfo_video_meta_field_t meta_field = MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED;

    ret = minfo_update_video_meta_info_int(media_uuid, meta_field, 3000);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"update a media content from media table should be failed because of the new_name NULL.");
}
