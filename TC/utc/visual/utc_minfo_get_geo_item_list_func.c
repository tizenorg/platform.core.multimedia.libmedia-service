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
* @file 	utc_minfo_get_geo_item_list_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_geo_item_list API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_geo_item_list_func.h"

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

/**
* @brief	This tests int minfo_get_geo_item_list() API with valid parameter
* 		get all of Mitem which pass the filter and meanwhile position where the longitude and latitude are specified.
* @par ID	utc_minfo_get_geo_item_list_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_geo_item_list_func_01()
{
	int ret = -1;

	const char *cluster_uuid = "8ddcdba9-9df4-72b4-4890-8d21d13854ad";
	int min_longitude = 120.0;
	int max_longitude = 123.0;
	int min_latitude = 19.0;
	int max_latitude = 24.0;
	GList *p_list = NULL;
	minfo_item_filter filter = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,true,MINFO_MEDIA_FAV_ALL};
	//minfo_store_type store_type = MINFO_CLUSTER_TYPE_LOCAL_ALL;
	minfo_store_type store_type = MINFO_CLUSTER_TYPE_ALL;

	ret = minfo_get_geo_item_list(cluster_uuid, 
						store_type,
			            filter, 
			            min_longitude, 
			            max_longitude, 
			            min_latitude, 
			            max_latitude,
						_ite_fn,
			            &p_list);

	if (ret == MB_SVC_ERROR_DB_NO_RECORD) {
		dts_pass(API_NAME, "No record. This is normal operation");
	}

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to get all of Mitem which pass the filter and meanwhile position where the longitude and latitude are specified. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_geo_item_list() API with invalid parameter
* 			get all of Mitem which pass the filter and meanwhile position where the longitude and latitude are specified.
* @par ID	utc_minfo_get_geo_item_list_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_geo_item_list_func_02()
{
	int ret = -1; 

	const char *cluster_uuid = NULL;
	int min_longitude = 220.0;
	int max_longitude = 223.0;
	int min_latitude = 119.0;
	int max_latitude = 24.0;
	GList *p_list = NULL;
	minfo_item_filter filter = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,true,MINFO_MEDIA_FAV_ALL};
	minfo_store_type store_type = MINFO_CLUSTER_TYPE_ALL;

	ret = minfo_get_geo_item_list(cluster_uuid, 
						store_type,
			            filter, 
			            min_longitude, 
			            max_longitude, 
			            min_latitude, 
			            max_latitude,
						_ite_fn,
			            &p_list);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"get all of Mitem which pass the filter and meanwhile position where the longitude and latitude are specified should be failed because of the p_list non-NULL.");
}
