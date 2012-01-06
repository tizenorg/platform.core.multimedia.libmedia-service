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
* @file 	utc_audio_svc_item_get_val_func.c
* @brief 	This is a suit of unit test cases to test audio_svc_list_item_get_val API function
* @author
* @version 	Initial Creation Version 0.1
* @date 	2010-09-13
*/
#include "utc_audio_svc_item_get_val_func.h"

void startup()
{
	UTC_AUDIO_SVC_OPEN();
}


void cleanup()
{
	UTC_AUDIO_SVC_CLOSE();
}

/**
* @brief	This tests int audio_svc_list_item_get_val() API with valid parameter
* 		Create a player handle with valid parameter & Test the handle by playing
* @par ID	utc_audio_svc_item_get_val_func_01
* @param	[in] &player = handle of player to be populated
* @return	This function returns zero on success, or negative value with error code
*/
void utc_audio_svc_item_get_val_func_01()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	int count = -1;
	AudioHandleType *tracks = NULL;
	AudioHandleType *item = NULL;
	char *audio_id = NULL, *title = NULL, *album = NULL, *artist = NULL, *thumbname = NULL;
	int i = 0;
	int size = -1;

	retval = check_default_item_exist();
	if(!retval)
	{
		dts_fail("check_default_item_exist","fail to check default item.");
	}
	
	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_count_list_item","unable to get count.");
	}

	if(count < 1)
	{
		dts_fail("audio_svc_insert_item","there is no item.");
	}

	ret = audio_svc_list_item_new(&tracks, count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_fail("audio_svc_list_item_new","unable to make list item.");
	}

	//get all tracks from db.
	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count,
		tracks
		);

	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_list_item_free(tracks);
		dts_fail("audio_svc_get_list_item","unable to get items.");
	}

	ret = audio_svc_item_new(&item);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_list_item_free(tracks);
		dts_fail("audio_svc_item_new","unable to malloc.");	
	}
	
	//get the track audio_id with index "i" in tracks array.
	ret = audio_svc_list_item_get_val(tracks, i, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_list_item_free(tracks);
		dts_fail("audio_svc_list_item_get_val","unable to get count.");
	}
	
	ret = audio_svc_get_item_by_audio_id(audio_id, item);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_list_item_free(tracks);
		dts_fail("audio_svc_get_item_by_audio_id","unable to get audio_id.");
	}

	//get the property value of svc item
	ret = audio_svc_item_get_val(item,
		AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size,
		AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
		AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
		AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
		AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH, &thumbname, &size,
		-1);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_list_item_free(tracks);
		dts_fail("audio_svc_item_get_val","unable to get value for item.");
	}

	ret = audio_svc_list_item_free(tracks);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_item_free(item);
		dts_fail("audio_svc_list_item_free","unable to free.");
	}
	
	ret = audio_svc_item_free(item);
	dts_check_eq("audio_svc_item_free", ret, AUDIO_SVC_ERROR_NONE, "fail to free memory.");

}


/**
* @brief 		This tests int audio_svc_item_get_val() API with invalid parameter
* 			Create a player handle with a NULL out param
* @par ID	utc_audio_svc_item_get_val_func_02
* @param	[in] &player = NULL
* @return	error code on success
*/
void utc_audio_svc_item_get_val_func_02()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int i = 0;
	char * audio_id = NULL;

	ret = audio_svc_item_get_val(handle, i, AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, -1);
	if (ret !=  AUDIO_SVC_ERROR_NONE)
	{
		dts_pass("audio_svc_item_get_val","abnormal condition test for invalid NULL parameter.");
	}
	else
	{
		dts_fail("audio_svc_item_get_val","Svc item get value should be failed because of the null parameter.");
	}

}
