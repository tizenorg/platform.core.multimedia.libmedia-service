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
* @file 	utc_minfo_init_func.c
* @brief 	This is a suit of unit test cases to test minfo_init API function
* @author 		
* @version 	Initial Creation Version 0.1
* @date 	2010-10-13
*/

#include "utc_minfo_init_func.h"


/**
* @brief	This tests int minfo_init() API with valid parameter
* 		Open media, metadata, etc, database tables.
* @par ID	utc_minfo_init_func_01
* @param	[in] 
* @return	This function returns zero on success, or negative value with error code
*/
void utc_minfo_init_func_01()
{
	int ret = 0;
	
	ret = minfo_init();
	
	dts_check_ge(API_NAME, ret, MB_SVC_ERROR_NONE, "unable to open media db. error code->%d", ret);
	
	return;
}


