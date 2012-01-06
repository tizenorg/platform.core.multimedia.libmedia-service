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
* @file		utc_visual_svc_common.h
* @author	
* @brief	This is the implementaion file for the test case of media service

* @version	Initial Creation Version 0.1
* @date		2010-10-13
*/

#ifndef __UTS_MINFO_SVC_COMMON_H_
#define __UTS_MINFO_SVC_COMMON_H_

#include <media-svc.h>
#include <string.h>
#include <tet_api.h>
#include <unistd.h>
#include <glib.h>

#define MAX_STRING_LEN 256

#define UTC_MM_LOG(fmt, args...)	tet_printf("[%s(L%d)]:"fmt"\n", __FUNCTION__, __LINE__, ##args)
#define API_NAME "libmedia-info"

#define UTC_MINFO_INIT() \
do \
{ \
	int ret = 0; \
	ret = minfo_init(); \
	if (ret < MB_SVC_ERROR_NONE) \
	{ \
		UTC_MM_LOG( "unable to open media db. error code->%d", ret); \
		tet_result(TET_FAIL); \
		return; \
	} \
} \
while(0);

#define UTC_MINFO_FINALIZE() \
do \
{ \
	int ret = 0; \
	ret = minfo_finalize(); \
	if (ret < MB_SVC_ERROR_NONE) \
	{ \
		UTC_MM_LOG( "unable to close media db. error code->%d", ret); \
		tet_result(TET_FAIL); \
		return; \
	} \
} \
while(0);

void startup()
{
	UTC_MINFO_INIT()
}

void cleanup()
{
	UTC_MINFO_FINALIZE()
}


#endif //__UTS_MINFO_SVC_COMMON_H_
