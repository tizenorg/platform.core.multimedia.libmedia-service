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
 * @file       	minfo-bookmark.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */

#include "minfo-types.h"

#ifndef _MINFO_BOOKMARK_H_
#define _MINFO_BOOKMARK_H_

#define MINFO_TYPE_MBOOKMARK		(0x55554)
#define MINFO_MBOOKMARK(obj)		((Mbookmark*)(obj))
#define MINFO_MBOOKMARK_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MBOOKMARK(obj)          (MINFO_TYPE_MBOOKMARK == MINFO_MBOOKMARK_GET_TYPE(MINFO_MBOOKMARK(obj)))

/**
* @fn    Mbookmark*  minfo_mbookmark_new(int id);
* This function creates mbookmark minfo
*
* @return                        This function returns mbookmark minfo
* @param[in]                     id          new mbookmark   id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

Mbookmark* 
minfo_mbookmark_new(int id);

/**
* @fn    void minfo_mbookmark_destroy(Mbookmark* bookmark);
* This function destroies mbookmark minfo
*
* @param[in]                    bookmark           pointer to Mbookmark minfo
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

void
minfo_mbookmark_destroy(Mbookmark* bookmark);

/**
* @}
*/


#endif /*_MINFO_BOOKMARK_H_*/


