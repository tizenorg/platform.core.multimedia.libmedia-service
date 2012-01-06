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
* @file 	utc_minfo_add_cluster_func.c
* @brief 	This is a suit of unit test cases to test minfo_add_cluster API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_add_cluster_func.h"


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
	char *cluster_url = "/opt/media/Images and videos/ForTC";
	int id = 0;
	
	UTC_MINFO_INIT()
	ret = minfo_add_cluster(cluster_url, &id);

	if (ret < MB_SVC_ERROR_NONE)
	{
		UTC_MM_LOG( "failed to add a cluser record content in folder table. error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
		return;
	}
	
	UTC_MINFO_FINALIZE()
	tet_result(TET_PASS);
	
	return;
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
	int id = 0;
	
	UTC_MINFO_INIT()
	ret = minfo_add_cluster(cluster_url, &id);

		
	if (ret<0)
	{
		UTC_MM_LOG("abnormal condition test for null, error code->%d", ret);
		UTC_MINFO_FINALIZE()
		tet_result(TET_PASS);
	}
	else
	{
		UTC_MM_LOG("add a cluser record content in folder table should be failed because of the cluster_url NULL.");
		UTC_MINFO_FINALIZE()
		tet_result(TET_FAIL);
	}


	return ;
}
