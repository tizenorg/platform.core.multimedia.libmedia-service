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
 * This file defines the error code of media service
 *
 * @file       	media-svc-error.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines the error code of media service 
 */

/**
* @ingroup MINFO_SVC_API
* @defgroup	MEDIA_SVC_ERROR Media service error code table
* @{
*/

#ifndef _MEDIA_SVC_ERROR_H_
#define _MEDIA_SVC_ERROR_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//Error types definition
#define MB_SVC_ERROR_NONE					0				/**< base */
#define MB_SVC_ERROR_INVALID_PARAMETER 		-1  			/**< invalid parameter(s) */
#define MB_SVC_ERROR_INVALID_MEDIA  	 	-2				/**< invalid or unknown media */
#define MB_SVC_ERROR_FILE_NOT_EXSITED 		-3				/**< file doesn't exist */
#define MB_SVC_ERROR_DIR_NOT_EXSITED 		-4				/**< folder doesn't exist */

#define MB_SVC_ERROR_FILE_IO 				-11				/**< file I/O error  */
#define MB_SVC_ERROR_OUT_OF_MEMORY         	-12				/**< memory allocation error*/

#define MB_SVC_ERROR_CREATE_THUMBNAIL       -101			/**< create thumbnail */
#define MB_SVC_ERROR_COPY_THUMBNAIL    		-102			/**< copy thumbnail */
#define MB_SVC_ERROR_MOVE_THUMBNAIL    		-103			/**< move thumbnail */

#define MB_SVC_ERROR_DB_CONNECT 			-201			/**< connect DB error */
#define MB_SVC_ERROR_DB_DISCONNECT 			-202			/**< disconnect DB error  */
#define MB_SVC_ERROR_DB_CREATE_TABLE 		-203			/**< create table error */
#define MB_SVC_ERROR_DB_NO_RECORD 			-204			/**< No record */
#define MB_SVC_ERROR_DB_OUT_OF_RANGE 		-205			/**< DB out of table records range*/
#define MB_SVC_ERROR_DB_INTERNAL	 		-206			/**< internal db error  */

#define MB_SVC_ERROR_NOT_IMPLEMENTED		-997			/**< Not implemented */
#define MB_SVC_ERROR_INTERNAL				-998			/**< internal error */
#define MB_SVC_ERROR_UNKNOWN				-999			/**< Unknown error */


#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
* @}
*/

#endif /*_MEDIA_SVC_ERROR_H_*/



