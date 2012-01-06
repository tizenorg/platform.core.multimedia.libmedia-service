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
* @file		utc_minfo_get_geo_item_list_func.h
* @author	
* @brief	This is the implementaion file for the test case of minfo_get_geo_item_list API function
* @version	Initial Creation Version 0.1
* @date		2010-10-13
*/

#ifndef __UTS_minfo_get_geo_item_list_FUNC_H_
#define __UTS_minfo_get_geo_item_list_FUNC_H_


#include "utc_visual_svc_common.h"

/* Initialize TCM data structures */
void (*tet_startup)() = startup;
void (*tet_cleanup)() = cleanup;

void utc_minfo_get_geo_item_list_func_01();
void utc_minfo_get_geo_item_list_func_02();

struct tet_testlist tet_testlist[] = {
	{utc_minfo_get_geo_item_list_func_01, 1},
	{utc_minfo_get_geo_item_list_func_02, 2},	
	{NULL, 0}
};


#endif //__UTS_minfo_get_geo_item_list_FUNC_H_
