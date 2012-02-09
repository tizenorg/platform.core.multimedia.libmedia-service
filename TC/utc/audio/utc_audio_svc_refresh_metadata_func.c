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
* @file 	utc_audio_svc_refresh_metadata_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_refresh_metadata API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2010-09-13
*/
#include "utc_audio_svc_refresh_metadata_func.h"


void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}

/**
* @brief	This tests int audio_svc_refresh_metadata() API with valid parameter
* 		Create a player handle with valid parameter & Test the handle by playing
* @par ID	utc_audio_svc_refresh_metadata_func_01
* @param	[in] &player = handle of player to be populated
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_refresh_metadata_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	char audio_id[AUDIO_SVC_UUID_SIZE+1] = {0, };

	retval = get_item_audio_id(audio_id);
	if(!retval)
	{
		dts_fail("get_item_audio_id","fail to get audio_id.");
	}
	
	ret = audio_svc_refresh_metadata(db_handle, audio_id);
	dts_check_eq("audio_svc_refresh_metadata", ret, AUDIO_SVC_ERROR_NONE, "failed to refresh metadata.");

}


/**
* @brief 		This tests int audio_svc_refresh_metadata() API with invalid parameter
* 			Create a player handle with a NULL out param
* @par ID	utc_audio_svc_refresh_metadata_func_02
* @param	[in] &player = NULL
* @return	error code on success
*/
void utc_audio_svc_refresh_metadata_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_refresh_metadata(db_handle, NULL);
	if (ret !=  AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_playlist_set_val","abnormal condition test for invalid audio_id parameter.");
	}
	else
	{
		dts_fail("audio_svc_refresh_metadata","Refresh metadata should be failed because of the invalid parameter.");
	}

}
