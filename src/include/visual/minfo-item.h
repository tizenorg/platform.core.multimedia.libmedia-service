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
 * @file       	minfo-item.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */

#ifndef _MINFO_ITEM_H_
#define _MINFO_ITEM_H_

#include "minfo-types.h"
#include "media-svc-structures.h"

#define MINFO_TYPE_MITEM		(0x55550)
#define MINFO_MITEM(obj)		((Mitem*)(obj))
#define MINFO_MITEM_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MITEM(obj)              	(MINFO_TYPE_MITEM == MINFO_MITEM_GET_TYPE(MINFO_MITEM(obj)))

/**
* @fn    Mitem* minfo_mitem_new(int id, mb_svc_media_record_s *p_md_record);
* This function creates mitem minfo
*
* @return                        This function returns mitem minfo
* @param[in]                    id          new mitem id
* @param[in]                    p_md_record          input media record from db if any
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/
__attribute__((deprecated))
Mitem* 
minfo_mitem_new(const char *uuid); 

Mitem*
minfo_media_item_new	(const char *uuid, mb_svc_media_record_s *p_md_record);


/**
* @fn    void minfo_mitem_destroy(Mitem* item);
* This function destroies mitem minfo
*
* @param[in]                    item           pointer to mitem minfo
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

void
minfo_mitem_destroy(Mitem* item);

/**
* @}
*/


#endif /*_MINFO_ITEM_H_*/



