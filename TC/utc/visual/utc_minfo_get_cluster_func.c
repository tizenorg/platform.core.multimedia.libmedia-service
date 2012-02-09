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
* @file 	utc_minfo_get_cluster_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_cluster API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_cluster_func.h"


/**
* @brief	This tests int minfo_get_cluster() API with valid parameter
* 		get a cluster/folder record from folder table with Mcluster type.
* @par ID	utc_minfo_get_cluster_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_cluster_func_01()
{
	int ret = -1;

    Mcluster *mc = NULL;
    char *cluster_url = "/opt/media/Images/Wallpapers";

    ret = minfo_get_cluster(handle, cluster_url, 0, &mc);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to get a cluster/folder record from folder table with Mcluster type. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_cluster() API with invalid parameter
* 			get a cluster/folder record from folder table with Mcluster type.
* @par ID	utc_minfo_get_cluster_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_cluster_func_02()
{
	int ret = -1;

    Mcluster *mc = NULL;
    char *cluster_url = NULL; /*= "/opt/media/Images/Wallpapers";*/

    ret = minfo_get_cluster(NULL, cluster_url, 0, &mc);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "get a cluster/folder record from folder table with Mcluster type should be failed because of mc NULL.");
}
