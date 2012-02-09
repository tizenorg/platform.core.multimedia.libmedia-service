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
* @file 	utc_minfo_get_cluster_id_by_url_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_cluster_id_by_url API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_cluster_id_by_url_func.h"


/**
* @brief	This tests int minfo_get_cluster_id_by_url() API with valid parameter
* 		get folder id using it's full name.
* @par ID	utc_minfo_get_cluster_id_by_url_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_cluster_id_by_url_func_01()
{
	int ret = -1;
	 
	char cluster_uuid[256] = {0,};
	char *folder_full_path = "/opt/media/Images/Wallpapers";
	ret = minfo_get_cluster_id_by_url(handle, folder_full_path, cluster_uuid, sizeof(cluster_uuid));
	
	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to get folder id using it's full name. error code->%d", ret);

}


/**
* @brief 		This tests int minfo_get_cluster_id_by_url() API with invalid parameter
* 			get folder id using it's full name.
* @par ID	utc_minfo_get_cluster_id_by_url_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_cluster_id_by_url_func_02()
{	
	int ret = -1;
	 
	char *cluster_uuid = NULL;
	char *folder_full_path = NULL; /*= "/opt/media/Images/Wallpapers";*/

	ret = minfo_get_cluster_id_by_url(handle, folder_full_path, cluster_uuid, sizeof(cluster_uuid));

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"get folder id using it's full name should be failed because of the folder_full_path parameter NULL.");
}
