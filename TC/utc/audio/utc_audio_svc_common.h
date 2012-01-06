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
* @file		uts_mmf_player_common.h
* @author	
* @brief	This is the implementaion file for the test case of mmfplayer_common 
* @version	Initial Creation Version 0.1
* @date 	2010-09-13
*/

#ifndef __UTS_AUDIO_SVC_COMMON_H_
#define __UTS_AUDIO_SVC_COMMON_H_

#include <audio-svc.h>
#include <audio-svc-error.h>
#include <string.h>
#include <tet_api.h>
#include <unistd.h>
#include <glib.h>
#include <stdbool.h>

#define MAX_STRING_LEN 256

#define UTC_MM_LOG(fmt, args...)	tet_printf("[%s(L%d)]:"fmt"\n", __FUNCTION__, __LINE__, ##args)

#define UTC_AUDIO_SVC_OPEN() \
do \
{ \
	int ret = AUDIO_SVC_ERROR_NONE; \
	ret = audio_svc_open(); \
	if (ret != AUDIO_SVC_ERROR_NONE) \
	{ \
		dts_fail("audio_svc_open", "fail to open music db"); \
	} \
} \
while(0);

#define UTC_AUDIO_SVC_CLOSE() \
do \
{ \
	int ret = AUDIO_SVC_ERROR_NONE; \
	ret = audio_svc_close(); \
	if (ret != AUDIO_SVC_ERROR_NONE) \
	{ \
		dts_fail("audio_svc_close", "fail to close music db"); \
	} \
} \
while(0);

#define	DEFAULT_FILE		"/opt/media/Sounds and music/Music/Over the horizon.mp3"
#define	TEST_FILE			"/opt/media/Sounds and music/test.mp3"

bool get_item_audio_id(char * audio_id_val);
bool get_playlist_id(int * playlist_id);
bool check_default_item_exist();
bool check_temp_item_file_exist();
bool check_default_playlist_exist();
bool check_playlist_has_item(int * playlist_id, char * audio_id_val);

#endif //__UTS_AUDIO_SVC_COMMON_H_

