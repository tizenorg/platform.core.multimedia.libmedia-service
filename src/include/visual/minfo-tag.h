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
 * @file       	minfo-tag.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */

#ifndef _MINFO_TAG_H_
#define _MINFO_TAG_H_

#include "visual-svc-types.h"
#include "media-svc-structures.h"
#include "media-svc-types.h"

#define MINFO_TYPE_MTAG		(0x55555)
#define MINFO_MTAG(obj)		((Mtag*)(obj))
#define MINFO_MTAG_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MTAG(obj)              	(MINFO_TYPE_MTAG == MINFO_MTAG_GET_TYPE(MINFO_MTAG(obj)))

/**
* @fn    Mtag* minfo_media_tag_new	(int id, mb_svc_tag_record_s *p_tag_record);
* This function creates Mtag minfo
*
* @return                        This function returns mtag minfo
* @param[in]                    id          new mtag id
* @param[in]                    p_tag_record          input tag record from db if any
* @exception                    None.
* @remark
*
*
*/

Mtag*
minfo_media_tag_new	(MediaSvcHandle *mb_svc_handle, int id, mb_svc_tag_record_s *p_tag_record);


/**
* @fn    void minfo_media_tag_destroy(Mtag* i_tag);
* This function destroies Mtag minfo
*
* @param[in]                    i_tag           pointer to Mtag minfo
* @exception                    None.
* @remark
*
*
*/

void
minfo_media_tag_destroy(Mtag* i_tag);


/**
* @}
*/


#endif /*_MINFO_TAG_H_*/




