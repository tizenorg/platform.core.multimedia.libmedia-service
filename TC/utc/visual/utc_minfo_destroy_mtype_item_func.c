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
* @file 	utc_minfo_destroy_mtype_item_func.c
* @brief 	This is a suit of unit test cases to test minfo_destroy_mtype_item_func API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_destroy_mtype_item_func.h"

static int _cluster_ite_fn( Mcluster* cluster, void* user_data)
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, cluster );
	return 0;
}

/**
* @brief	This tests int minfo_destroy_mtype_item_func() API with valid parameter
* 		free glist of media clusters.
* @par ID	utc_minfo_destroy_mtype_item_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_destroy_mtype_item_func_01()
{
	int ret = -1;
	int i = 0;
	GList *p_list = NULL;
	void* p = NULL;

    minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_NONE,0,5};
	ret = minfo_get_cluster_list(handle, cluster_filter, _cluster_ite_fn, &p_list);

	if( ret < MB_SVC_ERROR_NONE ) {
		dts_fail(API_NAME, "unable to get list of clusters.  error code->%d", ret);
		return;
	}

	for( i = 0; i < g_list_length(p_list); i++ ) 
	{
		p = g_list_nth_data(p_list, i);
		minfo_destroy_mtype_item(p);	

		if (ret < MB_SVC_ERROR_NONE)
		{
			dts_fail(API_NAME, "unable to destroy item of mtype.  error code->%d", ret);
			return;
		}
	}

	dts_pass(API_NAME, "utc_minfo_destroy_mtype_item_func_01 succeeded");
	
	return;
}


/**
* @brief 		This tests int minfo_destroy_mtype_item() API with invalid parameter
* 			free glist of media clusters.
* @par ID	utc_minfo_destroy_mtype_item_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_destroy_mtype_item_func_02()
{	
	int ret = -1;
	void* p = NULL;

	ret = minfo_destroy_mtype_item(p);	

	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"destroying item of mtype should be failed because of the p_list NULL.");
}
