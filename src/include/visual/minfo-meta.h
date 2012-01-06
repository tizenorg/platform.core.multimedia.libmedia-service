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
 * @file       	minfo-meta.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */

#include "minfo-types.h"
#include "media-svc-structures.h"

#ifndef _MINFO_META_H_
#define _MINFO_META_H_

/*---------------------------------Mvideo-----------------------------------------------------------------*/
#define MINFO_TYPE_MVIDEO		(0x55553)
#define MINFO_MVIDEO(obj)               	((Mvideo*)(obj))
#define MINFO_MVIDEO_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MVIDEO(obj)              	(MINFO_TYPE_MVIDEO == MINFO_MVIDEO_GET_TYPE(MINFO_MVIDEO(obj)))

Mvideo* 
minfo_mvideo_new(const char *id);

void
minfo_mvideo_destroy(Mvideo* video);

/*---------------------------------Mimage-----------------------------------------------------------------*/

#define MINFO_TYPE_MIMAGE		(0x55552)
#define MINFO_MIMAGE(obj)               	((Mimage*)(obj))
#define MINFO_MIMAGE_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MIMAGE(obj)        (MINFO_TYPE_MIMAGE == MINFO_MIMAGE_GET_TYPE(MINFO_MIMAGE(obj)))

Mimage* 
minfo_mimage_new(const char *id);

void
minfo_mimage_destroy(Mimage* image);


/*---------------------------------Mmeta-----------------------------------------------------------------*/


#define MINFO_TYPE_MMETA		(0x55551)
#define MINFO_MMETA(obj)               	((Mmeta*)(obj))
#define MINFO_MMETA_GET_TYPE(obj)		((obj)->gtype)
#define IS_MINFO_MMETA(obj)             (MINFO_TYPE_MMETA == MINFO_MMETA_GET_TYPE(MINFO_MMETA(obj)))


/**
* @fn    Mmeta* minfo_mmeta_new(int id, mb_svc_media_record_s *p_md_record);
* This function news mmeta minfo
*
* @return                        This function returns mmeta minfo
* @param[in]                    id          new mmeta id
* @param[in]                    p_md_record          input media record from db if any
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

Mmeta* 
minfo_mmeta_new(const char *media_uuid, mb_svc_media_record_s *p_md_record);

/**
* @fn    void minfo_mmeta_destroy(Mmeta* item);
* This function destroies mmeta minfo
*
* @param[in]                    item        pointer to mmeta minfo
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

void
minfo_mmeta_destroy(Mmeta* item);


/**
* @}
*/


#endif /*_MINFO_META_H_*/



