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
* @file 		utc_audio_svc_check_item_exist_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_check_item_exist API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2011-03-07
*/
#include "utc_audio_svc_check_item_exist_func.h"


void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}

/**
* @brief	This tests int audio_svc_check_item_exist() API with valid parameter
* @par ID	utc_audio_svc_check_item_exist_func_01
* @param	[in] storage_type	storage type to delete invalid item
*			[in]	path			the file path
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_check_item_exist_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_check_item_exist(DEFAULT_FILE);
	if(ret ==  AUDIO_SVC_ERROR_INVALID_PARAMETER)
	{
		dts_fail("audio_svc_check_item_exist","fail to check item.");
	}
	else
	{
		dts_pass("audio_svc_check_item_exist","Success to check item.");
	}
}


/**
* @brief 	This tests int audio_svc_check_item_exist() API with invalid parameter
* @par ID	utc_audio_svc_check_item_exist_func_02
* @param	[in] storage_type	storage type to delete invalid item
*			[in]	path			the file path
* @return	error code on success
*/
void utc_audio_svc_check_item_exist_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_check_item_exist(NULL);
	if (ret ==  AUDIO_SVC_ERROR_INVALID_PARAMETER)
	{
		dts_pass("audio_svc_check_item_exist","abnormal condition test for invalid path parameter.");
	}
	else
	{
		dts_fail("audio_svc_check_item_exist","this API should be failed because of the invalid path parameter.");
	}

	return ;
}

