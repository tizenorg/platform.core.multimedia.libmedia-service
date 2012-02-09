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
* @file 	utc_minfo_set_cluster_lock_status.c
* @brief 	This is a suit of unit test cases to test minfo_set_cluster_lock_status API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_set_cluster_lock_status_func.h"


/**
* @brief	This tests int minfo_set_cluster_lock_status() API with valid parameter
* 			Set status for lock to DB
* @par ID	utc_minfo_set_cluster_lock_status_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_set_cluster_lock_status_func_01()
{
	int ret = -1;
	const char *cluster_uuid = "8ac1df34-efa8-4143-a47e-5b6f4bac8c96";
	int status = 1;

    ret = minfo_set_cluster_lock_status(handle, cluster_uuid, status);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to set status to lock the album.  error code->%d", ret);
	}

    ret = minfo_set_cluster_lock_status(handle, cluster_uuid, 0);

	if (ret < MB_SVC_ERROR_NONE)
	{
		dts_fail(API_NAME, "unable to set status to unlock the album.  error code->%d", ret);
	}
	
	dts_pass(API_NAME, "utc_minfo_set_cluster_lock_status_func_01 succeeded");
	
	return;
}


/**
* @brief 		This tests int minfo_set_cluster_lock_status() API with invalid parameter
* 			Set status for lock to DB
* @par ID	utc_minfo_set_cluster_lock_status_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_set_cluster_lock_status_func_02()
{
	int ret = -1;
	const char *cluster_uuid = NULL;
	int status = 2;

    ret = minfo_set_cluster_lock_status(handle, cluster_uuid, status);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "Setting lock status to the album should be failed because of the status is invalid.");
}

