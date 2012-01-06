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
* @file 	utc_audio_svc_check_duplicate_insertion_in_playlist_playlist_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_add_playlist API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2010-09-13
*/

#include "utc_audio_svc_check_duplicate_insertion_in_playlist_func.h"





void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}


/**
* @brief	This tests int audio_svc_add_playlist() API with valid parameter
* 		Create a player handle with valid parameter & Test the handle by playing
* @par ID	utc_audio_svc_check_duplicate_insertion_in_playlist_playlist_func_01
* @param	[in] &player = handle of player to be populated
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_check_duplicate_insertion_in_playlist_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	int playlist_id = 0;
	char audio_id[AUDIO_SVC_UUID_SIZE+1] = {0, };
	int count = -1;

	retval = get_playlist_id(&playlist_id);
	if(!retval)
	{
		dts_fail("get_playlist_id","fail to get playlist id.");
	}

	retval = check_default_item_exist();
	if(!retval)
	{
		dts_fail("check_default_item_exist","fail to check default item.");
	}

	retval = get_item_audio_id(audio_id);
	if(!retval)
	{
		dts_fail("get_item_audio_id","fail to get audio_id.");
	}
	
	ret = audio_svc_check_duplicate_insertion_in_playlist(playlist_id, audio_id, &count);
	dts_check_eq("audio_svc_check_duplicate_insertion_in_playlist", ret, AUDIO_SVC_ERROR_NONE, "unable to check duplicate insertion.");

}


/**
* @brief 		This tests int audio_svc_add_playlist() API with invalid parameter
* 			Create a player handle with a NULL out param
* @par ID	utc_audio_svc_check_duplicate_insertion_in_playlist_playlist_func_02
* @param	[in] &player = NULL
* @return	error code on success
*/
void utc_audio_svc_check_duplicate_insertion_in_playlist_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int playlist_id = 1;
	char * audio_id = "550e8400-e29b-41d4-a716-446655440000";

	ret = audio_svc_check_duplicate_insertion_in_playlist(playlist_id, audio_id, NULL);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_check_duplicate_insertion_in_playlist","abnormal condition test for invalid parameter.");
	}
	else
	{
		dts_fail("audio_svc_check_duplicate_insertion_in_playlist","check insertion should be failed because of the NULL parameter.");
	}

}
