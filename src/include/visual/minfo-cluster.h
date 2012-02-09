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
 * @file       	minfo-cluster.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */

#include "visual-svc-types.h"
#include "media-svc-types.h"

#ifndef _MINFO_CLUSTER
#define _MINFO_CLUSTER

#define MINFO_TYPE_MCLUSTER		(0x55549)
#define MINFO_MCLUSTER(obj)		((Mcluster*)(obj))
#define MINFO_MCLUSTER_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MCLUSTER(obj)          (MINFO_TYPE_MCLUSTER == MINFO_MCLUSTER_GET_TYPE(MINFO_MCLUSTER(obj)))


/**
* @fn    Mcluster* minfo_mcluster_new(int id);
* This function creates mcluster item
*
* @return                        This function returns mcluster item
* @param[in]                    id          new mcluster id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

Mcluster* 
minfo_mcluster_new(MediaSvcHandle *mb_svc_handle, const char *uuid);

/**
* @fn    void minfo_mcluster_destroy(Mcluster* cluster);
* This function destroies mcluster minfo
*
* @param[in]                    cluster    cluster minfo
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

void
minfo_mcluster_destroy(Mcluster* cluster);

/**
* @}
*/


#endif /*_MINFO_CLUSTER_H_*/



