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



#ifndef _MEDIA_SVC_H_
#define _MEDIA_SVC_H_

#include "media-svc-types.h"
#include "media-svc-error.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
	@defgroup	MEDIA_SVC	Media Information Service
	 @{
	  * @file			media-svc.h
	  * @brief		This file defines API's for media service.
	  * @version	 	1.0
 */

/**
        @defgroup MEDIA_SVC_API    Media Database API
        @{

        @par
        manage the service database.
 */

#if 0
/**
 *	mediainfo_open:
 *	Open media information service library. This is the function that an user who wants to use media information service calls first.
 * 	This function connects to the media database.
 *
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		mediainfo_close
 *	@pre		None.
 *	@post		call mediainfo_close() to close the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void open_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	// open media database
	ret = mediainfo_open();
	// open failed
	if (ret < 0)
	{
		printf( "Cannot open media db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int mediainfo_open(void);

/**
 *	mediainfo_close:
 *	Open media information service library. This is the function that an user who wants to finalize media information service calls before closing the application.
 * 	This function disconnects to the media database.
 *
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		mediainfo_open
 *	@pre		None.
 *	@post		call mediainfo_open() to open the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void close_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	// close media database
	ret = mediainfo_close();
	// close failed
	if (ret < 0)
	{
		printf( "Cannot close media db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int mediainfo_close(void);
#endif
/**
 *	media_svc_connect:
 *	Connect to the media database. This is the function that an user who wants to get a handle to access the media database. 
 * 	This function connects to the media database.
 *
 *  @param 		handle [out]		Handle to access database.
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		media_svc_disconnect
 *	@pre		None.
 *	@post		call media_svc_connect to connect to the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void connect_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	MediaSvcHandle* my_handle = NULL;

	// connect to the media database
	ret = media_svc_connect(&my_handle);

	if (ret < 0)
	{
		printf("Fatal error to connect DB\n");
		return;
	}

	return;
}

 * 	@endcode
 */
int media_svc_connect(MediaSvcHandle **handle);


/**
 *	media_svc_disconnect:
 *	Disconnect to the media database. This is the function that an user who wants to disconnect the media database. 
 * 	This function disconnects to the media database.
 *
 *  @param 		handle [out]		Handle to access database.
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		media_svc_connect
 *	@pre		None.
 *	@post		call media_svc_disconnect to disconnect to the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void disconnect_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	MediaSvcHandle* my_handle = NULL;

	// connect to the media database
	ret = media_svc_connect(&my_handle);

	if (ret < 0)
	{
		printf("Fatal error to connect DB\n");
		return;
	}

	//
	// Do something using my_handle
	//
	

	ret = media_svc_disconnect(my_handle);
	if (ret < 0)
	{
		printf("Fatal error to disconnect DB\n");
	}

	return;
}

 * 	@endcode
 */
int media_svc_disconnect(MediaSvcHandle *handle);


/** @} */

/**
	@}
 */

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_H_*/
