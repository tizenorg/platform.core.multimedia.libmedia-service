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



#ifndef _MEDIA_INFO_DEBUG_H_
#define _MEDIA_INFO_DEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "Media-Info"

#define mediainfo_dbg(fmt, arg...)	 LOGD("[%s : %d] [%s] " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##arg)

#ifdef _USE_LOG_FILE_
void mediainfo_init_file_debug();
void mediainfo_close_file_debug();
FILE* get_fp();
#define mediainfo_file_dbg(fmt,arg...)      fprintf( get_fp(), "[%s: %d] [%s]" fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##arg)

#endif


#ifdef _PERFORMANCE_CHECK_
long
mediainfo_get_debug_time(void);
void
mediainfo_reset_debug_time(void);
void
mediainfo_print_debug_time(char* time_string);
void
mediainfo_print_debug_time_ex(long start, long end, const char* func_name, char* time_string);
#endif

#endif /*_MEDIA_INFO_DEBUG_H_*/
