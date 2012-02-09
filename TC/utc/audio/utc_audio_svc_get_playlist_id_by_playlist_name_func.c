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
* @file 		utc_audio_svc_get_playlist_id_by_playlist_name_func.c
* @brief 	This is a suit of unit test cases to test utc_audio_svc_get_playlist_id_by_playlist_name_func API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2011-03-07
*/
#include "utc_audio_svc_get_playlist_id_by_playlist_name_func.h"


void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}

/**
* @brief	This tests int audio_svc_get_playlist_id_by_playlist_name() API with valid parameter
* @par ID	utc_audio_svc_get_playlist_id_by_playlist_name_func_01
* @param	[in] 	playlist_name	playlist name for find playlist index
			[out]	playlist_id		the unique playlist id
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_get_playlist_id_by_playlist_name_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	int playlist_id = -1;
	int count = -1;
	AudioHandleType*playlists = NULL;
	char *p = NULL;
	int size = -1;
	int i = 0;

	retval = check_default_playlist_exist();
	if(!retval)
	{
		dts_fail("check_default_playlist_exist","fail to check default playlist.");
	}

	ret = audio_svc_count_playlist(db_handle, "", "", &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_count_playlists","unable to get playlist.");
	}
	
	ret = audio_svc_playlist_new(&playlists, count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_count_playlists","there is no playlist.");
	}
	
	//get all the playlists in db.
	ret = audio_svc_get_playlist(db_handle, 
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				count,
				playlists);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_fail("audio_svc_get_playlist","unable to get playlist.");
	}
	
	//get the playlist id and playlist name of each playlist
	ret = audio_svc_playlist_get_val(playlists, i, AUDIO_SVC_PLAYLIST_NAME, &p, &size, -1);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_fail("audio_svc_playlist_get_val","unable to get value for playlist.");
	}
	if(p == NULL)
	{
		audio_svc_playlist_free(playlists);
		dts_fail("audio_svc_playlists_get_val","unable to get name of playlist.");
	}
	
	ret = audio_svc_get_playlist_id_by_playlist_name(db_handle, p, &playlist_id);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_fail("audio_svc_get_playlist_id_by_playlist_name","unable to get playlist id.");
	}
	
	ret = audio_svc_playlist_free(playlists);
	dts_check_eq("audio_svc_playlist_free", ret, AUDIO_SVC_ERROR_NONE, "failed to free list memory.");

}


/**
* @brief 	This tests int audio_svc_get_playlist_id_by_playlist_name() API with invalid parameter
* @par ID	utc_audio_svc_get_playlist_id_by_playlist_name_func_02
* @param	[in] 	playlist_name	playlist name for find playlist index
			[out]	playlist_id		the unique playlist id
* @return	error code on success
*/
void utc_audio_svc_get_playlist_id_by_playlist_name_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int playlist_id = -1;
	
	ret = audio_svc_get_playlist_id_by_playlist_name(db_handle, NULL, &playlist_id);
	if (ret !=  AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_get_playlist_id_by_playlist_name","abnormal condition test for invalid playlist name parameter.");
	}
	else
	{
		dts_fail("audio_svc_get_playlist_id_by_playlist_name","this API should be failed because of the invalid playlist name parameter.");
	}

	return ;
}

