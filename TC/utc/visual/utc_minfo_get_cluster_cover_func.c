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
* @file 	utc_minfo_get_cluster_cover_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_cluster_cover API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_cluster_cover_func.h"

static int _cover_ite_fn(const char* thumb_path, void* user_data)
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, (char *)thumb_path );

	return 0;
}

/**
* @brief	This tests int minfo_get_cluster_cover() API with valid parameter
* 		get cover of a cluster/folder.
* @par ID	utc_minfo_get_cluster_cover_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_cluster_cover_func_01()
{
	int ret = -1;
	
	GList *p_list = NULL;
	int img_cnt = 5;
	const char *cluster_uuid = "8ac1df34-efa8-4143-a47e-5b6f4bac8c96";
	
	ret = minfo_get_cluster_cover(handle, cluster_uuid, img_cnt, _cover_ite_fn, &p_list);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to get cover of a cluster/folder. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_cluster_cover() API with invalid parameter
* 			get cover of a cluster/folder.
* @par ID	utc_minfo_get_cluster_cover_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_cluster_cover_func_02()
{
	int ret = -1;
	
	GList *p_list = NULL;
	int img_cnt = 5;
	const char *cluster_uuid = NULL;
	
	ret = minfo_get_cluster_cover(handle, cluster_uuid, img_cnt, NULL, &p_list);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"get cover of a cluster/folder should be failed because of the p_list non-NULL.");
}
