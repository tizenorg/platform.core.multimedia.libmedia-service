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
* @file 	utc_audio_svc_playlist_get_val_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_playlist_new API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2010-09-13
*/
#include "utc_audio_svc_playlist_get_val_func.h"


void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}

/**
* @brief	This tests int audio_svc_playlist_new() API with valid parameter
* 		Create a player handle with valid parameter & Test the handle by playing
* @par ID	utc_audio_svc_playlist_get_val_func_01
* @param	[in] &player = handle of player to be populated
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_playlist_get_val_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	int count = -1;
	AudioHandleType*playlists = NULL;
	char *p = NULL;
	int plst_id = -1;
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
		dts_fail("audio_svc_count_playlist","unable to get playlist.");
	}
	
	ret = audio_svc_playlist_new(&playlists, count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_count_playlist","there is no playlist.");
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
	ret = audio_svc_playlist_get_val(playlists, i, AUDIO_SVC_PLAYLIST_ID, &plst_id, AUDIO_SVC_PLAYLIST_NAME, &p, &size, -1);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_fail("audio_svc_get_playlist","unable to get value for playlist.");
	}

	ret = audio_svc_playlist_free(playlists);
	dts_check_eq("audio_svc_playlist_free", ret, AUDIO_SVC_ERROR_NONE, "failed to free list memory.");
	
}

/**
* @brief 		This tests int audio_svc_playlist_new() API with invalid parameter
* 			Create a player handle with a NULL out param
* @par ID	utc_audio_svc_playlist_get_val_func_02
* @param	[in] &player = NULL
* @return	error code on success
*/
void utc_audio_svc_playlist_get_val_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int i = 0;
	int size;
	char *name;

	ret = audio_svc_playlist_get_val(handle, i, AUDIO_SVC_PLAYLIST_NAME, &name, &size, -1);
	if (ret !=  AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_playlist_get_val","abnormal condition test for invalid NULL parameter.");
	}
	else
	{
		dts_fail("audio_svc_playlist_get_val","Playlist get value should be failed because of the null parameter.");
	}

}
