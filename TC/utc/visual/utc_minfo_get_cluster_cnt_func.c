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
* @file 	utc_minfo_get_cluster_cnt_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_cluster_cnt API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_cluster_cnt_func.h"


/**
* @brief	This tests int minfo_get_cluster_cnt() API with valid parameter
* 		Get count of media clusters.
* @par ID	utc_minfo_get_cluster_cnt_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_cluster_cnt_func_01()
{
	int ret = 0;
	int cnt = 0;

	minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_NONE,-1,10};
	ret = minfo_get_cluster_cnt(cluster_filter, &cnt);
	
	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to get count of media clusters. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_cluster_cnt() API with invalid parameter
* 			Get count of media clusters.
* @par ID	utc_minfo_get_cluster_cnt_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_cluster_cnt_func_02()
{	
	int ret = 0;
	int *cnt = NULL;

	minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_NONE,-1,10};
	ret = minfo_get_cluster_cnt(cluster_filter, cnt);
	
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"getting count of media clusters should be failed because of the item_filter parameter.");
}
