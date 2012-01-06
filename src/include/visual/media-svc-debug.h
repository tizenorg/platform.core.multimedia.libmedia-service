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
 * This file defines synchronize apis for phone explorer.
 *
 * @file       	media-svc-debug.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @ingroup MEDIA_SVC
  * @defgroup MEDIA_SVC_DEBUG in-house media service API
  * @{
  */



#ifndef _MEDIA_SVC_DEBUG_H_
#define _MEDIA_SVC_DEBUG_H_


#include <stdio.h>
#include <stdlib.h>
#include <dlog.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "Media-SVC"

#ifndef DEBUG_PREFIX
#define DEBUG_PREFIX "Media-SVC"
#endif

//#define MB_SVC_LOG_FILE  "/opt/mb-svc.log"

#define MB_DEBUG

#ifdef MB_DEBUG


#ifndef LOG_TAG


#ifdef MB_SVC_LOG_FILE
#define pb_svc_debug(fmt, arg...)	\
	{	\
		FILE *fp;	\
		fp = fopen(MB_SVC_LOG_FILE, "a");	\
		fprintf(fp, "[%s] ", DEBUG_PREFIX);	\
		fprintf(fp, fmt, ##arg);	\
		fclose(fp);	\
		fprintf(stderr, "[%s] ", DEBUG_PREFIX);	\
		fprintf(stderr, fmt, ##arg);	\
	}

#else /*MB_SVC_LOG_FILE*/
#define mb_svc_debug(fmt, arg...)	\
{	\
	fprintf(stderr, "[%s] ", DEBUG_PREFIX);	\
	fprintf(stderr, fmt, ##arg);	\
}
#endif /*MB_SVC_LOG_FILE*/


#else /*LOG_TAG*/
#include <unistd.h>
#include <asm/unistd.h>
#include <pthread.h>
static pid_t gettid(void)
{
	return syscall(__NR_gettid);
}

#define mb_svc_debug(fmt, arg...)	 LOGD("[%d] [%s : %d] " fmt "\n", gettid(), __FUNCTION__, __LINE__, ##arg)



#endif /*LOG_TAG*/

#else /*MB_DEBUG*/

#define mb_svc_debug(fmt, arg...)

#endif /*MB_DEBUG*/

#endif /*_MEDIA_SVC_DEBUG_H_*/

/**
* @}
*/


