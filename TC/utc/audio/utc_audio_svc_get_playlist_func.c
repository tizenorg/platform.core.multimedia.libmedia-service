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
* @file 	utc_audio_svc_get_playlist_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_get_playlist API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2010-09-13
*/

#include "utc_audio_svc_get_playlist_func.h"





void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}


/**
* @brief	This tests int audio_svc_get_playlist() API with valid parameter
* 		Create a player handle with valid parameter & Test the handle by playing
* @par ID	utc_audio_svc_get_playlist_func_01
* @param	[in] &player = handle of player to be populated
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_get_playlist_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	AudioHandleType *result_records = NULL;
	int count = -1;

	retval = check_default_playlist_exist();
	if(!retval)
	{
		dts_fail("check_default_playlist_exist","fail to check default playlist.");
	}

	ret = audio_svc_count_playlist(db_handle, "", "", &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_count_playlist","unable to get playlist.");
	}
	
	ret = audio_svc_playlist_new(&result_records, count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_playlist_new","unable to create playlist.");
	}
	
	ret = audio_svc_get_playlist(db_handle, "", "", 0, count, result_records);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(result_records);
		dts_fail("audio_svc_get_playlist","unable to get playlist.");
	}

	ret = audio_svc_playlist_free(result_records);
	dts_check_eq("audio_svc_playlist_free", ret, AUDIO_SVC_ERROR_NONE, "failed to free list memory.");
	
}


/**
* @brief 		This tests int audio_svc_get_playlist() API with invalid parameter
* 			Create a player handle with a NULL out param
* @par ID	utc_audio_svc_get_playlist_func_02
* @param	[in] &player = NULL
* @return	error code on success
*/
void utc_audio_svc_get_playlist_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *result_records = NULL;
	
	ret = audio_svc_get_playlist(db_handle, "", "", 0, ret, result_records);

	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_get_playlist","abnormal condition test for invalid NULL parameter.");
	}
	else
	{
		dts_fail("audio_svc_get_playlist","Getting playlists should be failed because of the NULL out parameter.");
	}

}
