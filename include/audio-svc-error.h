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



#ifndef _AUDIO_SVC_ERROR_H_
#define _AUDIO_SVC_ERROR_H_

/**
	@addtogroup AUDIO_SVC
	@{
	* @file		audio-svc-error.h
	* @brief		This file defines error codes for audio service.

 */

/**
        @defgroup AUDIO_SVC_COMMON  Global data structure and error code
        @{

        @par
         type definition and error code
 */


#define AUDIO_SVC_ERROR_NONE							0			/**< No Error */

#define AUDIO_SVC_ERROR_INVALID_PARAMETER			-1			/**< Invalid parameter */
#define AUDIO_SVC_ERROR_INVALID_MEDIA				-2			/**< Invalid media */
#define AUDIO_SVC_ERROR_FILE_NOT_EXIST					-3			/**< File not exist */

#define AUDIO_SVC_ERROR_FILE_IO						-11			/**< File IO error */
#define AUDIO_SVC_ERROR_OUT_OF_MEMORY				-12			/**< Out of memory */

#define AUDIO_SVC_ERROR_MAKE_PLAYLIST_NAME_FAILED	-101		/**< fail to make new playlist name */

#define AUDIO_SVC_ERROR_DB_CONNECT					-201		/**< DB connect error */
#define AUDIO_SVC_ERROR_DB_DISCONNECT				-202		/**< DB disconnect error */
#define AUDIO_SVC_ERROR_DB_CREATE_TABLE				-203		/**< DB create table error */
#define AUDIO_SVC_ERROR_DB_NO_RECORD				-204		/**< Item not found in DB */
#define AUDIO_SVC_ERROR_DB_OUT_OF_RANGE				-205		/**< Invalid range */
#define AUDIO_SVC_ERROR_DB_INTERNAL					-206		/**< DB internal error */

#define AUDIO_SVC_ERROR_NOT_IMPLEMENTED				-997		/**< Not implemented */
#define AUDIO_SVC_ERROR_INTERNAL						-998		/**< Internal error */
#define AUDIO_SVC_ERROR_UNKNOWN						-999		/**< Unknown error */

/**
	@}
*/

/**
	@}
*/

#endif /*_AUDIO_SVC_ERROR_H_*/
