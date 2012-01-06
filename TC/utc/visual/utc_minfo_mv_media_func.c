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
* @file 	utc_minfo_mv_media_func.c
* @brief 	This is a suit of unit test cases to test minfo_mv_media API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_mv_media_func.h"


/**
* @brief	This tests int minfo_mv_media() API with valid parameter
* 		move a record identified by media id to destination folder identified by folder id.
* @par ID	utc_minfo_mv_media_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_mv_media_func_01()
{
	int ret = -1;
	
	char *file_url = "/opt/media/Images and videos/Wallpapers/Home_default.png";
	char src_media_uuid[256] = {0,};
	char src_cluster_uuid[256] = {0,};
	char *dst_cluster_uuid = "8ddcdba9-9df4-72b4-4890-8d21d13854ad";
	Mitem* item = NULL;


	ret = minfo_get_item( file_url, &item );
	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to get a media content from media table. error code->%d", ret);
	}

	strncpy(src_media_uuid, item->uuid, sizeof(src_media_uuid));
	strncpy(src_cluster_uuid, item->cluster_uuid, sizeof(src_cluster_uuid));

	ret = minfo_mv_media(src_media_uuid, dst_cluster_uuid);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "failed to move a record identified by media id to destination folder identified by folder id. error code->%d", ret);
	}

	ret = minfo_mv_media(src_media_uuid, src_cluster_uuid);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "failed to move a record identified by media id to destination folder identified by folder id. error code->%d", ret);
	}
	
	dts_pass(API_NAME, "utc_minfo_mv_media_func_01 succeeded");
	return;
}


/**
* @brief 		This tests int minfo_mv_media() API with invalid parameter
* 			move a record identified by media id to destination folder identified by folder id.
* @par ID	utc_minfo_mv_media_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_mv_media_func_02()
{
	int ret = -1;
	
	char *src_media_uuid = NULL;
	char *dst_cluster_uuid = NULL;

	ret = minfo_mv_media(src_media_uuid, dst_cluster_uuid);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"move a record identified by media id to destination folder identified by folder id should be failed because of the src_media_id -1.");
}
