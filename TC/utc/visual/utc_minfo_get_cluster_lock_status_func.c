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
* @file 	utc_minfo_get_cluster_lock_status.c
* @brief 	This is a suit of unit test cases to test minfo_get_cluster_lock_status API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_cluster_lock_status_func.h"


/**
* @brief	This tests int minfo_get_cluster_lock_status() API with valid parameter
* 			Gets status for lock from DB
* @par ID	utc_minfo_get_cluster_lock_status_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_cluster_lock_status_func_01()
{
	int ret = -1;
	int status = 0;
	const char *cluster_uuid = "8ddcdba9-9df4-72b4-4890-8d21d13854ad";

    ret = minfo_get_cluster_lock_status(cluster_uuid, &status);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to varify password to lock/unlock.  error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_cluster_lock_status() API with invalid parameter
* 			Gets status for lock from DB
* @par ID	utc_minfo_get_cluster_lock_status_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_cluster_lock_status_func_02()
{
	int ret = -1;
	const char *cluster_uuid = NULL;
	int status = 0;

    ret = minfo_get_cluster_lock_status(cluster_uuid, &status);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"Get status for lock should be failed because the passed status is NULL.");
}

