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
 * @file       	minfo-streaming.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @addtogroup MINFO_TYPES
  * @{
  */


#ifndef _MINFO_STREAMING_H_
#define _MINFO_STREAMING_H_


#define MINFO_TYPE_MSTREAMING		(0x55548)
#define MINFO_MSTREAMING(obj)		((Mbookmark*)(obj))
#define MINFO_MSTREAMING_GET_TYPE(obj)	((obj)->gtype)
#define IS_MINFO_MSTREAMING(obj)          (MINFO_TYPE_MSTREAMING == MINFO_MSTREAMING_GET_TYPE(MINFO_MSTREAMING(obj)))

/**
* @struct _Mstreaming
* This structure defines _Mstreaming, same with Mstreaming
*/

struct _Mstreaming
{
	int gtype;
	
	unsigned int _id;					/**< web streaming id */
	unsigned int cluster_id;			/**< cluster id */
	char *http_url;						/**< http url */
	char *thumb_url;					/**< thumbnail full path */
	unsigned int duration;				/**< durantion */
	char *title;						/**< title */
	char *description;					/**< description */
};

typedef struct _Mstreaming	Mstreaming;


/**
* @fn    Mstreaming*  minfo_mstreaming_new(int id);
* This function creates mstreaming minfo
*
* @return                        This function returns mstreaming minfo
* @param[in]                    id          new mstreaming id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

Mstreaming* 
minfo_mstreaming_new(int id);


/**
* @fn    void minfo_mstreaming_destroy(Mstreaming* item);
* This function destroies mstreaming minfo
*
* @param[in]                    item        pointor to mstreaming minfo
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

void
minfo_mstreaming_destroy(Mstreaming* item);


/**
* @}
*/


#endif /*_MINFO_STREAMING__H_*/



