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
* @file 	utc_minfo_cluster_func.c
* @brief 	This is a suit of unit test cases to test minfo_add_cluster and minfo_delete_cluster API functions
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_cluster_func.h"

int _get_id_by_url(const char *url, char *id)
{
	char cluster_id[256] = {0,};

	if( minfo_get_cluster_id_by_url(handle, url, cluster_id, sizeof(cluster_id)) < 0 )
	{
		return -1;
	}

	strncpy(id, cluster_id, 256);

	return 0;
}


/**
* @brief	This tests int minfo_add_cluster() API with valid parameter
* 		add a cluser record content in folder table.
* @par ID	utc_minfo_add_cluster_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_add_cluster_func_01()
{
	int ret = -1;
	char *cluster_url = "/opt/media/Images/ForTC";
	char cluster_id[256] = {0,};
	
	ret = minfo_add_cluster(handle, cluster_url, cluster_id, sizeof(cluster_id));
	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to add a cluser record content in folder table. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_add_cluster() API with invalid parameter
* 			add a cluser record content in folder table.
* @par ID	utc_minfo_add_cluster_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_add_cluster_func_02()
{
	int ret = -1;
	char *cluster_url = NULL; /*= "/opt/media/Images/Wallpapers_1";*/
	char cluster_id[256] = {0,};
	
	ret = minfo_add_cluster(handle, cluster_url, cluster_id, sizeof(cluster_id));

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "add a cluser record content in folder table should be failed because of the cluster_url NULL.");
}


/**
* @brief	This tests int minfo_delete_cluster() API with valid parameter
* 		delete a cluster/folder identified by folder id.
* @par ID	utc_minfo_delete_cluster_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_delete_cluster_func_01()
{
	int ret = -1;

	char cluster_id[256] = {0,};
	 _get_id_by_url("/opt/media/Images/ForTC", cluster_id);
	ret = minfo_delete_cluster(handle, cluster_id);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to delete a cluster/folder identified by folder id. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_delete_cluster() API with invalid parameter
* 			delete a cluster/folder identified by folder id.
* @par ID	utc_minfo_delete_cluster_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_delete_cluster_func_02()
{
	int ret = -1;
	 
	const char *cluster_id = NULL;
	ret = minfo_delete_cluster(handle, cluster_id);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "delete a cluster/folder identified by folder id should be failed because of the cluster_id -1.");
}
