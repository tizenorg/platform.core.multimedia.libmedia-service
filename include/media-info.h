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



#ifndef _MEDIA_INFO_H_
#define _MEDIA_INFO_H_

#include "media-info-types.h"
#include "media-info-error.h"
#include <sqlite3.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
	@defgroup	MEDIA_INFO	Media Information Service
	 @{
	  * @file			media-info.h
	  * @brief		This file defines API's for media information service.
	  * @version	 	1.0
 */

/**
        @defgroup MP_DB_API    Database Manager API
        @{

        @par
        manage the service database.
 */


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

/**
 *	mediainfo_connect_db_with_handle:
 *	Connect to the media database. This is the function that an user who wants to get a handle to access the media database. 
 * 	This function connects to the media database.
 *
 *  @param 		db_handle [out]		Handle to access database using sqlite3 libs
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		mediainfo_disconnect_db_with_handle
 *	@pre		None.
 *	@post		call mediainfo_connect_db_with_handle to connect to the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void connect_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3* my_handle = NULL;

	// connect to the media database
	ret = mediainfo_connect_db_with_handle(&my_handle);

	if (ret < 0)
	{
		printf("Fatal error to connect DB\n");
		return;
	}

	return;
}

 * 	@endcode
 */
int mediainfo_connect_db_with_handle(sqlite3** db_handle);


/**
 *	mediainfo_disconnect_db_with_handle:
 *	Disconnect to the media database. This is the function that an user who wants to disconnect the media database. 
 * 	This function disconnects to the media database.
 *
 *  @param 		db_handle [out]		Handle to access database using sqlite3 libs
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		mediainfo_connect_db_with_handle
 *	@pre		None.
 *	@post		call mediainfo_disconnect_db_with_handle to disconnect to the media database
 *	@remark	The database name is "/opt/dbspace/.media.db".
 * 	@par example
 * 	@code

#include <media-info.h>

void disconnect_media_db()
{
	int ret = MEDIA_INFO_ERROR_NONE;
	sqlite3* my_handle = NULL;

	// connect to the media database
	ret = mediainfo_connect_db_with_handle(&my_handle);

	if (ret < 0)
	{
		printf("Fatal error to connect DB\n");
		return;
	}

	//
	// Do something using my_handle
	//
	

	ret = mediainfo_disconnect_db_with_handle(my_handle);
	if (ret < 0)
	{
		printf("Fatal error to disconnect DB\n");
	}

	return;
}

 * 	@endcode
 */
int mediainfo_disconnect_db_with_handle(sqlite3* db_handle);

/**
 *	mediainfo_register_file:
 *	This function registers multimedia file to media DB
 *  When you did some file operations such as Create, Copy, Move, Rename, and Delete in phone or mmc storage, media-server registers the result to database automatically by inotify mechanism.
 *  However, automatic registration will have a little delay because the method is asynchronous.
 *  If you want to register some files to database immediately, you should use this API.
 *
 *  @param 		file_full_path [in]		full path of file for register
 *	@return		This function returns zero(MEDIA_INFO_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'media-info-error.h' to know the exact meaning of the error.
 *	@see		None.
 *	@pre		None.
 *	@post		None.
 *	@remark	The database name is "/opt/dbspace/.media.db".
 *                   You have to use this API only for registering multimedia files. If you try to register no multimedia file, this API returns error.
 * 	@par example
 * 	@code

#include <media-info.h>

int main()
{
	int result = -1;

	result = mediainfo_register_file("/opt/media/test.txt");
	if( result < 0 )
	{
		printf("FAIL to mediainfo_register_file\n");
		return 0;
	}
	else
	{
		printf("SUCCESS to register file\n");
	}
	
	return 0;
}

 * 	@endcode
 */
DEPRECATED_API int mediainfo_register_file(const char *file_full_path);

typedef GArray* minfo_list;

DEPRECATED_API int mediainfo_list_new(minfo_list *list);

DEPRECATED_API int mediainfo_list_add(minfo_list list, const char* file_full_path);

DEPRECATED_API int mediainfo_list_free(minfo_list list);

DEPRECATED_API int mediainfo_register_files(const minfo_list list);


/** @} */

/**
	@}
 */

#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_INFO_H_*/
