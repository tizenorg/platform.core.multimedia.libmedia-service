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
* @file 	utc_minfo_get_bookmark_list_func.c
* @brief 	This is a suit of unit test cases to test minfo_get_bookmark_list API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_get_bookmark_list_func.h"

static int _minfo_bm_ite_fn( Mbookmark *bookmark, void *user_data )
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, bookmark );

	return 0;
}

/**
* @brief	This tests int minfo_get_bookmark_list() API with valid parameter
* 		get all of bookmark for a media content.
* @par ID	utc_minfo_get_bookmark_list_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_get_bookmark_list_func_01()
{
	int ret = -1;

	const char *media_uuid = "2f08863e-52fd-eaf8-269c-3d0798e7aa0e";
	GList *p_list = NULL;

	ret = minfo_get_bookmark_list(media_uuid, _minfo_bm_ite_fn, &p_list);

	if (ret == MB_SVC_ERROR_DB_NO_RECORD) {
		dts_pass(API_NAME, "No record. This is normal operation");
	}

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to get all of bookmark for a media content. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_get_bookmark_list() API with invalid parameter
* 			get all of bookmark for a media content.
* @par ID	utc_minfo_get_bookmark_list_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_get_bookmark_list_func_02()
{
	int ret = -1;

	const char *media_uuid = NULL;
	GList *p_list = NULL;

	ret = minfo_get_bookmark_list(media_uuid, NULL, &p_list);
		
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE,"get all of bookmark for a media content should be failed because of the p_list non-NULL.");
}
