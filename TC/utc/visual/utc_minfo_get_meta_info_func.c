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
* @file 	utc_minfo_get_cluster_list_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_meta_info API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_meta_info_func.h"


/**
* @brief	This tests int minfo_get_meta_info() API with valid parameter
* 		Get meta record of media Mitem.
* @par ID	utc_minfo_get_cluster_list_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_meta_info_func_01()
{
	int ret = -1;
	Mmeta* mt = NULL;
	const char *media_uuid = "aa33f347-988b-41f4-8a53-9df24ea86bc4";


	ret = minfo_get_meta_info(handle, media_uuid, &mt);

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to get meta record of media Mitem. error code->%d", ret);
	minfo_destroy_mtype_item(mt);
}


/**
* @brief 		This tests int minfo_get_meta_info() API with invalid parameter
* 			Get meta record of media Mitem.
* @par ID	utc_minfo_get_meta_info_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_meta_info_func_02()
{	
	int ret = -1;
	Mmeta* mt = NULL;
	const char *media_uuid = NULL;

	ret = minfo_get_meta_info(handle, media_uuid, &mt);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"getting meta record of media Mitem should be failed because of the media_id parameter.");
}

