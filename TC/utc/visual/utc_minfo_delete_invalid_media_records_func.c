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
* @file 	utc_minfo_delete_invalid_media_records.c
* @brief 	This is a suit of unit test cases to test minfo_delete_invalid_media_records API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_delete_invalid_media_records_func.h"


/**
* @brief	This tests int minfo_delete_invalid_media_records() API with valid parameter
* 			Set status for lock to DB
* @par ID	utc_minfo_delete_invalid_media_records_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_delete_invalid_media_records_func_01()
{
	int ret = -1;
	minfo_store_type storage_type = MINFO_PHONE;

    ret = minfo_delete_invalid_media_records(storage_type );

	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "failed to delete invalid records. error code->%d", ret);
}


/**
* @brief 		This tests int minfo_delete_invalid_media_records() API with invalid parameter
* 			Set status for lock to DB
* @par ID	utc_minfo_delete_invalid_media_records_func_02
* @param	[in] 
* @return	error code on success 
*/
void utc_minfo_delete_invalid_media_records_func_02()
{
	int ret = -1;
	minfo_store_type storage_type = MINFO_SYSTEM;

    ret = minfo_delete_invalid_media_records(storage_type);
	dts_check_lt(API_NAME, ret, MB_SVC_ERROR_NONE, "deleting invalid records should be failed because of the status is invalid.");
}

