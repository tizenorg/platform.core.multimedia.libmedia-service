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



#ifndef _AUDIO_SVC_H_
#define _AUDIO_SVC_H_

#include <stddef.h>
#include "audio-svc-types.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
	@defgroup	AUDIO_SVC	Audio Service
	@{
	 * @file			audio-svc.h
	 * @brief		This file defines API's for audio service.
	 * @version	 	1.0
 */

/**
        @defgroup AUDIO_SVC_DB_API    Database Manager API
        @{

        @par
        manage the service database.
 */


/**
 *	audio_svc_open:\n
 *	Open audio service library. This is the function that an user who wants to use music-service calls first.
 * 	This function connects with the music database and initialize efreet mime libary.
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-types.h' to know the exact meaning of the error.
 *	@see		audio_svc_close
 *	@pre		None.
 *	@post		call audio_svc_close() to close music database
 *	@remark	The database name is "/opt/dbspace/.music.db".
 * 	@par example
 * 	@code

#include <audio-svc.h>

void open_music_db()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// open music database
	ret = audio_svc_open();
	// open failed
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_open(void);


/**
 *    audio_svc_close:\n
 *	Close audio service library. This is the function need to call before close the application.
 *	This function disconnects with the music database and shutdown the efreet mime libary.
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-types.h' to know the exact meaning of the error.
 *	@see		audio_svc_open
 *	@pre		music database already is opened.
 *	@post 		None
 *	@remark	memory free before you call this function to close database.
 * 	@par example
 * 	@code

#include <audio-svc.h>

void close_music_db()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// close music database
	ret = audio_svc_close();
	// close failed
	if (ret < 0)
	{
		printf( "unable to close music db. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_close(void);


/**
 *    audio_svc_create_table:\n
 *	Create the tables in music database. File manager service need to call this function before it register media data into
 *	music database. In all music database consists of 5 tables, music table for Phone tracks, music table for MMC tracks,
 *	music view for both Phone and MMC tracks, playlist table, and playlist items table.\n
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-types.h' to know the exact meaning of the error.
 *	@remark	This function is called only by file manager service library.
 *	@pre   		open music database firstly.
 *	@post 		None.
 *	@see		audio_svc_open
 * 	@par example
 * 	@code

#include <audio-svc.h>

void create_music_table()
{
	int ret = AUDIO_SVC_ERROR_NONE;

	// create muisc tables of phone type
	ret = audio_svc_create_table();
	if (ret < 0)
	{
		printf( "unable to create table. error code->%d", ret);
		return;
	}

	return ret;
}

 * 	@endcode
 */
int	audio_svc_create_table(void);

/**
 * 	audio_svc_set_db_valid:\n
 * 	This function set whether all the tracks in a storage are valid.
 * 	Actually audio service filter all the tracks query from database by the track validation.\n
 *	This function is always used for MMC card insert/inject operation, in file manager service library.
 * 	When inject a MMC card, the track records for MMC are not deleted really, but are set to be invalid.
 *
 * 	@param[in]   storage_type		information for storage type
 * 	@param[in]	valid				whether the track item is valid.
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 * 			Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see		audio_svc_set_item_valid
 *	@pre		None
 *	@post		None
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void set_db_valid(bool valid)
{
	int ret = AUDIO_SVC_ERROR_NONE;

	//set the validation of tracks in MMC storage in db.
	ret = audio_svc_set_db_valid(AUDIO_SVC_STORAGE_MMC, valid);
	if (ret < 0)
	{
		printf( "failed to set db invalid. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_set_db_valid(audio_svc_storage_type_e storage_type, int valid);

/**
 *	audio_svc_delete_all:\n
 *	This function deletes all  items in a storage, from both the tracks table (phone/mmc table) and the playlist items table.
 *  	This function also delete the existing thumbnail image files associated with the tracks.
 *
 * 	@param[in]	storage_type	storage type to delete
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre   		None
 *	@post 		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void delete_all()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// delete all the tracks in phone storage.
	ret = audio_svc_delete_all(AUDIO_SVC_STORAGE_PHONE);
	if (ret < 0)
	{
		printf( "failed to delete phone storage. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */
int audio_svc_delete_all(audio_svc_storage_type_e storage_type);


/** @} */

/**
        @defgroup AUDIO_SVC_GROUP_API     music group manager API
        @{
        @par
        manage group list and its contents
 */


/**
 *	 audio_svc_count_group_item:\n
 * 	This function counts the number of unique group name according to group type with filter condition.
 *	This parameter "limit_string1" is necessary to specify parent group. It should be defined in case group type is
 *	AUDIO_SVC_GROUP_BY_ARTIST_ALBUM, AUDIO_SVC_GROUP_BY_GENRE_ARTIST, AUDIO_SVC_GROUP_BY_GENRE_ALBUM,
 *	AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM.
 *	This parameter "limit_string2" is necessary to specify subgroup. It should be defined in case group type
 *	is AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM.
 *
 * 	@param[in]		group_type		group type (refer to audio_svc_group_type_e)
 * 	@param[in]		limit_string1		It is possible to make a subgroup within a specific group.
 * 	@param[in]		limit_string2		It is possible to make a small group of subgroup.
 * 	@param[in]		filter_string		the filter condition for group name.
 * 	@param[in]		filter_string2		the filter condition for group name.
 * 	@param[out]		count		the returned group count.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@par example 	 audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ARTIST_ALBUM, "abc", NULL)
 *					=> It counts unique album which artist name is "abc".
 * 	@par example  	audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM, "abc", "123")
 *					=> It counts unique album which genre name is "abc" and artist name is "123".
 *	@pre   			None
 *	@post 			None
 *	@see			audio_svc_get_group_item
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void get_group_count()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;

	// count the groups by artist
	ret = audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ARTIST, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get groups. error code->%d", ret);
		return;
	}

	printf("group count is %d", count);
	return;
}

 * 	@endcode
 */

int audio_svc_count_group_item(audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2, int *count);


/**
 *	 audio_svc_get_group_item:\n
 * 	This function retrieves unique group name according to group type.
 *	This parameter "limit_string1" is necessary to specify parent group, It should be defined in case group type
 *	is AUDIO_SVC_GROUP_BY_ARTIST_ALBUM, AUDIO_SVC_GROUP_BY_GENRE_ARTIST, AUDIO_SVC_GROUP_BY_GENRE_ALBUM,
 *	or AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM.\n
 *	This parameter "limit_string2" is necessary to specify subgroup. It should be defined in case group type is
 *	AUDIO_SVC_GROUP_BY_GENRE_ARTIST_ALBUM.
 *
 * 	@param[in]		group_type		group type (refer to audio_svc_group_type_e)
 * 	@param[in]		limit_string1		It is possible to make a subgroup within a specific group.
 * 	@param[in]		limit_string2		It is possible to make a small group of subgroup.
 * 	@param[in]		filter_string		the filter condition for group name.
 * 	@param[in]		filter_string2		the filter condition for group name.
 * 	@param[in]		offset			start position to fetch the records satisfied with given condition.
 * 	@param[in]		rows			the number of required records which is satisfied with given condition.
 * 	@param[out]		result_records	reference pointer of result records.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 			Please refer 'audio-svc-error.h' to know the exact meaning of the error.
  *	@pre		None
 *	@post		None
 *	@see		audio_svc_get_list_item
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_groups()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	AudioHandleType *handle = NULL;

	//count the albms with name "Unplugged"
	ret = audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of groups. error code->%d", ret);
		return;
	}

	if(count > 0)
	{
		// allocate the result records with count
		ret = audio_svc_group_item_new(&handle, count);
		if (ret < 0)
		{
			printf( "failed to allocate handle. error code->%d", ret);
			return;
		}

		ret = audio_svc_get_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", 0, ret, handle);
		if (ret < 0)
		{
			printf( "failed to get groups. error code->%d", ret);
		}
		audio_svc_group_item_free(handle);
	}
	else
	{
		printf( "There is no track items");
	}

	return;
}

 * 	@endcode
 */

int audio_svc_get_group_item(audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2,int offset, int rows, AudioHandleType *result_records);


/**
 * 	audio_svc_group_item_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType for group items.
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements.
 *
 * 	@param[in]		record		The handle for service group items
 * 	@param[in]		count		the elements number
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None.
 *	@post			None
 *	@see			audio_svc_group_item_free
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void group_item_new()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//allocate the memory of type group item with count
	ret = audio_svc_group_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_group_item_new(AudioHandleType **record, int count);


/**
 * 	audio_svc_group_item_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType for group items.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_group_item_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for service list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None
 *	@see		audio_svc_group_item_new.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void group_item_free()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//allocate the memory of type group item with count
	ret = audio_svc_group_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free the list item memory.
	ret = audio_svc_group_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}


 * 	@endcode
 */

int audio_svc_group_item_free(AudioHandleType *record);


/**
 * 	audio_svc_group_item_get_val:\n
 * 	This function allows to retrieve a group attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	index		The element index in array
 * 	@param[in]	first_field_name	the variable arguements list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None.
 *	@see		audio_svc_group_item_new, audio_svc_group_item_free.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_group_item_get_value()
{
	int count = 0;
	AudioHandleType *handle = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of groups. error code->%d", ret);
		return;
	}

	if(count > 0)
	{
		// allocate the result records with count
		ret = audio_svc_group_item_new(&handle, count);
		if (ret < 0)
		{
			printf( "failed to allocate handle. error code->%d", ret);
			return;
		}

		ret = audio_svc_get_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", 0, count, handle);
		if (ret < 0)
		{
			printf( "failed to get groups. error code->%d", ret);
			audio_svc_group_item_free(handle);
			return;
		}
	}
	else
	{
		printf( "There is no track items");
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *main_info = NULL, *sub_info = NULL, *thumbname = NULL;
		int album_rating = AUDIO_SVC_RATING_NONE;
		int size = 0;
		int count = 0;

		//get the group info
		ret = audio_svc_group_item_get_val(groups, i ,
			AUDIO_SVC_GROUP_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_GROUP_ITEM_MAIN_INFO, &main_info, &size,
			AUDIO_SVC_GROUP_ITEM_SUB_INFO, &sub_info, &size,
			AUDIO_SVC_GROUP_ITEM_RATING, &album_rating,
			-1);

		if (ret < 0)
		{
			printf( "failed to get group items. error code->%d", ret);
			audio_svc_group_item_free(handle);
			return;
		}
	}

	ret = audio_svc_group_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */

int audio_svc_group_item_get_val(AudioHandleType *record, int index, audio_svc_group_item_type_e first_field_name, ...);


/**
 * 	audio_svc_group_item_get:\n
 * 	This function is used to retrieve the element from elements arrary by index.
 *
 * 	@param[in]		record		The handle for service list
 * 	@param[in]		index		The element index
 * 	@param[out]	item		the element to be retrieved.
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_group_item()
{
	int count = 0;
	AudioHandleType *handle = NULL;
	AudioHandleType *item = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of groups. error code->%d", ret);
		return;
	}

	if(count > 0)
	{
		// allocate the result records with count
		ret = audio_svc_group_item_new(&handle, count);

		ret = audio_svc_get_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "Unplugged", "", "", "", 0, count, handle);
		if (ret < 0)
		{
			printf( "failed to allocate handle. error code->%d", ret);
			return;
		}

		if (ret < 0)
		{
			printf( "failed to get groups. error code->%d", ret);
			audio_svc_group_item_free(handle);
			return;
		}
	}
	else
	{
		printf( "There is no track items");
		return;
	}

	ret = audio_svc_group_item_get(handle,0, &item);
	if (ret < 0)
	{
		printf( "failed to get group item. error code->%d", ret);
		return;
	}

	ret = audio_svc_group_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_group_item_get(AudioHandleType *record, int index, AudioHandleType **item);


/**
 *	audio_svc_count_list_item:\n
 *	 This function counts the number of items according to item type.\n
 *	The param type_string defines the group name contains the tracks. If item type is AUDIO_SVC_TRACK_BY_ALBUM, AUDIO_SVC_TRACK_BY_ARTIST,
 *	AUDIO_SVC_TRACK_BY_GENRE, AUDIO_SVC_TRACK_BY_YEAR or AUDIO_SVC_TRACK_BY_COMPOSER the type_string should be defined. If item type is AUDIO_SVC_TRACK_BY_PLAYLIST, the type_string
 *	should be entered as type string after converting playlist index to character type.\n
 * 	The filter string act as the LIKE cluase for item title in the SQL condition search. The parameter "rows" and "offset"
 *	is used as the LIMIT expression and OFFSET expression.
 *
 * 	@param[in]		item_type		item retrieval type or order type (refer to audio_svc_track_type_e)
 * 	@param[in]		type_string		This parameter is necessayto retrieve items belonging to specific group.
 * 	@param[in] 		type_string2		This parameter is necessayto retrieve items belonging to specific group.
 * 	@param[in]		filter_string		the filter condition for track title.
 * 	@param[in]		filter_string2	 	the filter condition for track title.
 * 	@param[out]		count			the returned tracks count.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see			audio_svc_count_group_item
 *	@pre			None
 *	@post			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_item_count()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	// get the count of all tracks in db.
	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	printf("item count is %d", count);

	return;
}

 * 	@endcode
 */

int audio_svc_count_list_item(audio_svc_track_type_e item_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int *count);


/**
 *	audio_svc_get_list_item:\n
 * 	This function retrieves items according to item type. The param type_string defines the group name contains the tracks.
 *	If item type is AUDIO_SVC_TRACK_BY_ALBUM, AUDIO_SVC_TRACK_BY_ARTIST, AUDIO_SVC_TRACK_BY_GENRE,
 *	AUDIO_SVC_TRACK_BY_YEAR or AUDIO_SVC_TRACK_BY_COMPOSER the type_string should be defined. If item type is AUDIO_SVC_TRACK_BY_PLAYLIST, the type_string
 *	should be entered as type string after converting playlist index to character type.\n
 * 	The filter string act as the LIKE cluase for item title in the SQL condition search. The parameter "rows" and "offset"
 *	is used as the LIMIT expression and OFFSET expression.
 *
 * 	@param[in] 		item_type		item retrieval type or order type (refer to audio_svc_track_type_e)
 * 	@param[in] 		type_string		This parameter is necessayto retrieve items belonging to specific group.
 * 	@param[in] 		type_string2		This parameter is necessayto retrieve items belonging to specific group.
 * 	@param[in] 		filter_string		the filter condition for track title.
 * 	@param[in] 		filter_string2		the filter condition for track title.
 * 	@param[in]		offset	   		start position to fetch the records satisfied with given condition.
 * 	@param[in]		rows 			the number of required records which is satisfied with given condition.
 * 	@param[out]		track			reference pointer of result records.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see			audio_svc_count_list_item
 *	@pre			None
 *	@post			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_track_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = -1;

	// get the count of all tracks
	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	if(count > 0)
	{
		// allocate result records
		ret = audio_svc_list_item_new(&handle, count);
		if (ret < 0)
		{
			printf( "failed to allocate handle. error code->%d", ret);
			return;
		}

		// get items
		ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", 0, ret, handle);

		if (ret < 0)
		{
			printf( "failed to  get items. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}
		audio_svc_list_item_free(handle);
	}
	else
	{
		printf( "There is no track items");
	}

	return;
}

 * 	@endcode
 */

int audio_svc_get_list_item(audio_svc_track_type_e item_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *track);


/**
 * 	audio_svc_list_item_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType.
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements.
 *
 * 	@param[in]		record		The handle for service list
 * 	@param[in]		count		the elements number
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None.
 *	@post			None
 *	@see			audio_svc_list_item_free
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void list_item_new()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//allocate the memory of type list item with count
	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_list_item_new(AudioHandleType **record, int count);

/**
 * 	audio_svc_list_item_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_list_item_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for service list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None
 *	@see		audio_svc_list_item_new.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void list_item_free()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//allocate the memory of type list item with count
	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free the list item memory.
	ret = audio_svc_list_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}


 * 	@endcode
 */

int audio_svc_list_item_free(AudioHandleType *record);

/**
 * 	audio_svc_list_item_get_val:\n
 * 	This function allows to retrieve a list of attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	index		The element index in array
 * 	@param[in]	first_field_name	the variable arguements list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None
 *	@see		audio_svc_list_item_new, audio_svc_list_item_free.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_list_item_get_value()
{
	int count = 0;
	AudioHandleType *handle = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get the all track items.
	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //rows,
		handle
		);

	if (ret < 0)
	{
		audio_svc_list_item_free(handle);
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *audio_id = NULL, *title = NULL, *artist = NULL, *thumbname = NULL, *pathname = NULL;
		int rating = AUDIO_SVC_RATING_NONE;
		int duration = 0;
		int size = 0;

		ret = audio_svc_list_item_get_val(handle, i ,
			AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size,
			AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_LIST_ITEM_TITLE, &title, &size,
			AUDIO_SVC_LIST_ITEM_ARTIST, &artist, &size,
			AUDIO_SVC_LIST_ITEM_PATHNAME, &pathname, &size,
			AUDIO_SVC_LIST_ITEM_DURATION, &duration,
			AUDIO_SVC_LIST_ITEM_RATING, &rating,
			-1);

		if (ret < 0)
		{
			printf( "failed to get list items. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}
	}

	ret = audio_svc_list_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_list_item_get_val(AudioHandleType *record, int index, audio_svc_list_item_type_e first_field_name, ...);


/**
 * 	audio_svc_list_item_get:\n
 * 	This function is used to retrieve the element from elements arrary by index.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	index		The element index
 * 	@param[out]	item		the element to be retrieved.
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_svc_item()
{
	AudioHandleType *handle = NULL;
	AudioHandleType *item = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = 0;
	int i = 0

	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get the all tracks item.
	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //rows,
		handle
		);

	if (ret < 0)
	{
		printf( "failed to get items. error code->%d", ret);
		audio_svc_list_item_free(handle);
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *audio_id = NULL;
		int size = 0;
		//get the list item with index "i"
		ret = audio_svc_list_item_get(handle, i, &item);
		if (ret < 0)
		{
			printf( "failed to get list items. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}

		ret = audio_svc_list_item_get_val(item, 0, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get list items value. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}
	}

	return;
}

 * 	@endcode
 */

int audio_svc_list_item_get(AudioHandleType *record, int index, AudioHandleType **item);


/** @} */

/**
        @defgroup AUDIO_SVC_ITEM_API     music item manager API
        @{
        @par
        manage music item.
 */

/**
 * 	audio_svc_item_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType,
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements of type audio_svc_audio_item_s.
 *
 * 	@param[in]	record		The handle for service item
 * 	@param[in]	count		the elements number
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_item_free
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void new_svc_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;

	//create the svc item object.
	ret = audio_svc_item_new(&handle);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int audio_svc_item_new(AudioHandleType **record);

/**
 * 	audio_svc_item_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_item_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for service item
 *
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_item_new
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void free_svc_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;

	//create svc item object, object number is count.
	ret = audio_svc_item_new(&handle);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free the svc item object.
	ret = audio_svc_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_item_free(AudioHandleType *record);


/**
 * 	audio_svc_item_get_val:\n
 * 	This function allows to retrieve a list of attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	first_field_name	the variable arguements list
 *
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_item_set_val
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void get_svc_item_value()
{
	int count = 0;
	AudioHandleType*handle = NULL;

	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get all tracks from db.
	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //count,
		handle
		);

	if (ret < 0)
	{
		printf( "failed to get items. error code->%d", ret);
		audio_svc_list_item_free(handle);
		return;
	}

	ret = audio_svc_item_new(&item);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	for (i = 0; i < count; i++)
	{
		int size = 0;
		char *audio_id = NULL, *title = NULL, *album = NULL, *artist = NULL, *thumbname = NULL;
		//get the track audio_id with index "i" in handle array.
		ret = audio_svc_list_item_get_val(handle, i, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get list items. error code->%d", ret);
			audio_svc_item_free(item);
			audio_svc_list_item_free(handle);
			return;
		}

		ret = audio_svc_get_item_by_audio_id(audio_id, item);
		if (ret < 0)
		{
			printf( "failed to get items. error code->%d", ret);
			audio_svc_item_free(item);
			audio_svc_list_item_free(handle);
			return;
		}

		//get the property value of svc item
		ret = audio_svc_item_get_val(item,
			AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size, 
			AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
			AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
			AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
			AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH, &thumbname, &size,
			-1);
		if (ret < 0)
		{
			printf( "failed to get item value. error code->%d", ret);
			audio_svc_item_free(item);
			audio_svc_list_item_free(handle);
			return;
		}
	}
	ret = audio_svc_item_free(item);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_item_get_val(AudioHandleType *record, audio_svc_track_data_type_e  first_field_name, ...);

/**
 * 	audio_svc_search_item_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType to be searched,
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements of type audio_svc_audio_item_s.
 *
 * 	@param[in]	record		The handle for service item
 * 	@param[in]	count		the elements number
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_search_item_free, audio_svc_search_item_get
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void new_search_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;

	//create the search item object.
	ret = audio_svc_search_item_new(&handle, 10);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */

int audio_svc_search_item_new(AudioHandleType **record, int count);

/**
 * 	audio_svc_search_item_get:\n
 * 	This function is used to retrieve the element from searched elements arrary by index.
 *
 * 	@param[in]	record		The handle for search list
 * 	@param[in]	index		The element index
 * 	@param[out]	item		the element to be retrieved.
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_list_by_search, audio_svc_search_item_free, audio_svc_search_item_new
 *	@remark	None
 * 	@par example
 * 	@code
#include <audio-svc.h>

void test_audio_svc_list_by_search()
{
		int offset = 0, count = 10, i = 0;
		const char *str = "Sa";
		AudioHandleType *handle = NULL;

		err = audio_svc_search_item_new(&handle, count);
		if (err < 0) {
			printf("audio_svc_search_item_new failed:%d\n", err);
			return err;
		}

		err = audio_svc_list_by_search(handle, AUDIO_SVC_ORDER_BY_TITLE_ASC, offset, count, AUDIO_SVC_SEARCH_TITLE, str, strlen(str), AUDIO_SVC_SEARCH_ALBUM, str, strlen(str), AUDIO_SVC_SEARCH_ARTIST, str, strlen(str), -1);

		if (err != AUDIO_SVC_ERROR_NONE) {
			mediainfo_dbg("Fail to get items : %d", err);
			return err;
		}
		
		for (i = 0; i < count; i++) {
			AudioHandleType *item = NULL;
			err = audio_svc_search_item_get(handle, i, &item);
			char *audio_id = NULL, *title = NULL, *artist = NULL, *pathname = NULL, *album = NULL;
			int size = 0;
			if (err < 0) {
				printf("[%d] audio_svc_search_item_get failed : %d\n", i, err);
			} else {
				audio_svc_item_get_val(item,  
						AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size, 
						AUDIO_SVC_TRACK_DATA_PATHNAME, &pathname, &size,
						AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
						AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
						AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
						-1);

				if( audio_id == NULL ) break;

				printf("[%d] ID: %s\n", i, audio_id);
				printf("[%d] Path: %s\n", i, pathname);
				printf("[%d] Title: %s\n", i, title);
				printf("[%d] Artist: %s\n", i, artist);
				printf("[%d] Album: %s\n", i, album);
			}
		}

		audio_svc_search_item_free(handle);
}


 * 	@endcode
 */

int audio_svc_search_item_get(AudioHandleType *record, int index, AudioHandleType **item);

/**
 * 	audio_svc_search_item_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType to be searched.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_item_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for search item
 *
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_search_item_new, audio_svc_search_item_get
 *	@remark	None
 * 	@par example
 * 	@code

  #include <audio-svc.h>

void free_search_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;

	//create search item object, object number is count.
	ret = audio_svc_search_item_new(&handle, 10);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free the search item object.
	ret = audio_svc_search_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_search_item_free(AudioHandleType *record);

int audio_svc_insert_item_start(int data_cnt);
int audio_svc_insert_item_end(void);

/**
 *    audio_svc_insert_item:\n
 * 	Register music track into DB. The "Category" property is defined by file manager service, and Only
 *  	"MUSIC" type tracks can be listed in music app.
 * 	This function extract the metadata of track then insert it into the database.
 *
 *	@param[in]		storage_type	Information for storage type
 *	@param[in]		path         		Information for file path
 *	@param[in]		category			Information for file category, defined by file manager.
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@see		None
 *	@remark	None
 *	@pre   		music table is already created
 *	@post 		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void insert_item_to_db()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char *path = "/opt/media/Sounds/Music/Layla.mp3";
	int category = AUDIO_SVC_CATEGORY_MUSIC;
	// insert a track into music db
	ret = audio_svc_insert_item(AUDIO_SVC_STORAGE_PHONE, path, category);

	if (ret < 0)
	{
		printf( "unable to insert item, error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_insert_item(audio_svc_storage_type_e storage_type, const char *path, audio_svc_category_type_e category);

int audio_svc_move_item_start(int data_cnt);
int audio_svc_move_item_end(void);


/**
 *	audio_svc_move_item:\n
 * 	This function moves item with source path to destination.\n
 * 	This function is called by file manager service when user move music file in myfile app. Audio Service need
 * 	to create the thumbnail path of new item itself.
 *
 *	@param[in] 	src_storage		Storage type of the src_path
 * 	@param[in]	src_path			file full path before moving
 *	@param[in] 	dest_storage	Storage type of the dest_path
 * 	@param[in]	dest_path		file full path after moving
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@see		None
 *	@pre   		None
 *	@post 		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void move_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	audio_svc_storage_type_e storage = AUDIO_SVC_STORAGE_PHONE;
	const char *dest_path = "/opt/media/Sounds/BeyondSamsung.mp3";
	const char *src_path = "/opt/media/Sounds/Music/BeyondSamsung.mp3";
	// move the track to dest path
	ret = audio_svc_move_item(storage, src_path, storage, dest_path);

	if (ret < 0)
	{
		printf( "failed to move item. error code->%d", ret);
		return;
	}

	return;
}

* 	@endcode
*/

int audio_svc_move_item(audio_svc_storage_type_e src_storage, const char *src_path, audio_svc_storage_type_e dest_storage, const char *dest_path);


/**
 *    audio_svc_delete_item_by_path:\n
 * 	This function deletes item with file path from music database. deleting the track from phone/MMC track
 *  	table, the service also deleting the same track from playlist item table.
 *
 *	@param[in]	path			Full file path.
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@see		audio_svc_get_item_by_path
 *	@pre   		None
 *	@post 		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void delete_item_by_path()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	// delete music item by path
	ret = audio_svc_delete_item_by_path("test.mp3");
	if (ret < 0)
	{
		printf("failed to delete item by path. error code->%d", ret);
	}

	return;
}

 * 	@endcode
 */
int audio_svc_delete_item_by_path(const char *path);


/**
 *	audio_svc_delete_invalid_items:\n
 *	This function deletes invalid items in a storage, from both the tracks table (phone/mmc table).
 *  	This function also delete the existing thumbnail image files associated with the tracks.
 *
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre   		None
 *	@post 		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void delete_all()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	audio_svc_storage_type_e storage = AUDIO_SVC_STORAGE_PHONE;
	
	// delete all invalid item in phone storage.
	ret = audio_svc_delete_invalid_items(storage);
	if (ret < 0)
	{
		printf( "failed to delete invalid item. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */

int audio_svc_delete_invalid_items(audio_svc_storage_type_e storage);

int audio_svc_set_item_valid_start(int data_cnt);
int audio_svc_set_item_valid_end(void);

/**
 * 	audio_svc_set_item_valid:\n
 * 	This function set whether a track is valid in database.If a track is not valid, then it cannot be listed in music application.
 * 	Actually audio service filter all the tracks query from database by the track validation.\n
 * 	This function is always used for MMC card insert/inject operation, in file manager service library.
 *
 * 	@param[in]	path			file full path
 * 	@param[in]	valid			whether the track item is valid.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void set_item_valid()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char * test_path = "/opt/media/Sounds/Music/Layla.mp3";

	//set the track with "audio_id" in MMC storage to be valid.
	ret = audio_svc_set_item_valid(test_path, true);
	if (ret < 0)
	{
		printf( "failed to set item valid. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_set_item_valid(const char *path, int valid);


/**
 *    audio_svc_get_item_by_audio_id:\n
 * 	This function retrieves music track item with db index, querying from the virtual view including all phone
 *  	tracks and MMC tracks. The parameter "item" should be allocated before calling this function.
 *
 *	@param[in]		audio_id					Information for db index
 *	@param[out]	item_handle				Information for music track record.
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@see		audio_svc_get_item_by_path
 *	@remark	None
 *	@pre   		None
 *	@post		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void get_item_by_audio_id()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	char audio_id[AUDIO_SVC_UUID_SIZE+1] = "550e8400-e29b-41d4-a716-446655440000";

	ret = audio_svc_item_new(&handle);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	// retrieve the music item by audio_id.
	ret = audio_svc_get_item_by_audio_id(audio_id, handle);
	if (ret < 0)
	{
		printf("failed to get item by audio_id. error code->%d", ret);
		audio_svc_item_free(handle);
		return;
	}

	//free the music item
	ret = audio_svc_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_get_item_by_audio_id(const char *audio_id, AudioHandleType *item_handle);


/**
 *    audio_svc_get_item_by_path:\n
 * 	This function retrieves music track item with file path, querying from the virtual view including all phone
 *  	tracks and MMC tracks. The parameter "item" should be allocated before calling this function.
 *
 *	@param[out]		item_handle			Information for music track record.
 *	@param[in]		path		Information for file path.
 *	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@see		audio_svc_get_item_by_audio_id
 *	@pre   		None
 *	@post 		None
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void get_item_by_path()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char *path = "/opt/media/Sounds/Music/Layla.mp3";
	AudioHandleType *handle = NULL;

	ret = audio_svc_item_new(&handle);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	// retrieve the music item by file path.
	ret = audio_svc_get_item_by_path(path, handle);
	if (ret < 0)
	{
		printf("failed to get item by path. error code->%d", ret);
		audio_svc_item_free(handle);
		return;
	}

	//free the music item
	ret = audio_svc_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_get_item_by_path(const char *path, AudioHandleType *item_handle);


/**
 * 	audio_svc_check_item_exist:\n
 * 	This function check whether item(content) is exist or not in DB.
 * 	Application can use this when before delete item or update item and so on..
 *
 * 	@param[in]	path			the file path.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success or when item exist, or negative value with error code.
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

  #include <audio-svc.h>

void check_item_Exist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char *path = "/opt/media/Sounds/Music/The Last Laugh.mp3";

	//check item exist
	ret = audio_svc_check_item_exist(path);
	if (ret < 0)
	{
		printf("Item not found");
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_check_item_exist(const char *path);


/**
 * 	audio_svc_get_path_by_audio_id:\n
 * 	This function retrieves the track pathname by its db index.\n
 *	Sometimes application only knows the track's db index and want to extract its thumbnail image, then it can
 *	use this function to get path firstly.
 *
 * 	@param[in]	audio_id				Information for db index
 * 	@param[in]	path				Full path of track
  * 	@param[in]	max_path_length	Max path length
 * 	@return				This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *						Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre				None
 *	@post				None
 *	@see				None
 *	@remark			None
 * 	@par example
 * 	@code
 *
 #include <audio-svc.h>

char * get_path_by_audio_id (const char *audio_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	char pathname[AUDIO_SVC_PATHNAME_SIZE] = {0,};

	//retrieve the file path by track audio_id.
	ret = audio_svc_get_path_by_audio_id(audio_id, pathname, AUDIO_SVC_PATHNAME_SIZE);
	if (ret < 0)
	{
		printf( "failed to get path. error code->%d", ret);
		return NULL;
	}
	return pathname;
}
 * 	@endcode
 */
int audio_svc_get_path_by_audio_id(const char *audio_id, char *path, size_t max_path_length);


/**
 * 	audio_svc_get_audio_id_by_path:\n
 * 	This function retrieves the track index by track path.\n
 *	Application can use this api when only knows the track's path and want to know its db index.
 *
 * 	@param[in]		path			Full path of track
 * 	@param[out]	audio_id				Information for db index
 * 	@return							This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *									Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre							None
 *	@post							None
 *	@see							None
 *	@remark						None
 * 	@par example
 * 	@code
 *
 #include <audio-svc.h>

int get_audio_id_by_path ()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char * test_path = "/opt/media/Sounds/Music/Layla.mp3";
	char audio_id[AUDIO_SVC_UUID_SIZE+1] = {0,};

	//retrieve the file path by track audio_id.
	ret = audio_svc_get_audio_id_by_path(test_path, audio_id, AUDIO_SVC_UUID_SIZE);
	if (ret < 0)
	{
		printf( "failed to get audio_id. error code->%d", ret);
		return 0;
	}

	return audio_id;
}
 * 	@endcode
 */

int audio_svc_get_audio_id_by_path(const char *path, char *audio_id, size_t max_audio_id_length);

/**
 * 	audio_svc_get_thumbnail_path_by_path:\n
 * 	This function retrieves the thumbnail path by track path.\n
 *	Application can use this api when only knows the track's path.
 *
 * 	@param[in]		path						Full path of track
 * 	@param[in]		max_thumb_path_length		Max thumb path length
 * 	@param[out]	thumb_path					thumbnail path of track
 * 	@return							This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *									Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre							None
 *	@post							None
 *	@see							None
 *	@remark						None
 * 	@par example
 * 	@code
 *
 #include <audio-svc.h>

int get_thumbnail_path_by_path ()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	const char * test_path = "/opt/media/Sounds/Music/Layla.mp3";
	char thumb_path[AUDIO_SVC_PATHNAME_SIZE] = {0};

	//retrieve the thumbnail path by track path.
	ret = audio_svc_get_thumbnail_path_by_path(test_path, thumb_path, AUDIO_SVC_PATHNAME_SIZE);
	if (ret < 0)
	{
		printf( "failed to get thumbnail_path. error code->%d", ret);
		return ret;
	}

	return AUDIO_SVC_ERROR_NONE;
}
 * 	@endcode
 */

int audio_svc_get_thumbnail_path_by_path(const char *path, char *thumb_path, size_t max_thumb_path_length);


/**
 * 	audio_svc_update_item_metadata:\n
 * 	This function allows to update attributes for the specified element.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	audio_id				db index of item to update play time
 * 	@param[in]	first_field_name	the variable arguements list
 *
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void update_svc_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 0;

	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get all tracks from db.
	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //count,
		handle
		);

	if (ret < 0)
	{
		printf( "failed to get service list items. error code->%d", ret);
		audio_svc_list_item_free(handle);
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *audio_id = NULL;
		int size = 0;
		ret = audio_svc_list_item_get_val(handle, i, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get list items. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}

		ret = audio_svc_update_item_metadata(audio_id,
			AUDIO_SVC_TRACK_DATA_PLAYED_COUNT, 5,
			AUDIO_SVC_TRACK_DATA_PLAYED_TIME, 5,
			AUDIO_SVC_TRACK_DATA_ADDED_TIME, 5,
			AUDIO_SVC_TRACK_DATA_RATING,	 AUDIO_SVC_RATING_5,
			AUDIO_SVC_TRACK_DATA_TITLE, "Test title", strlen("Test title"),
			AUDIO_SVC_TRACK_DATA_ARTIST, "Test artist", strlen("Test artist"),
			AUDIO_SVC_TRACK_DATA_ALBUM, "Test album", strlen("Test album"),
			AUDIO_SVC_TRACK_DATA_GENRE, "Test genre", strlen("Test genre"),
			AUDIO_SVC_TRACK_DATA_AUTHOR, "Test author", strlen("Test author"),
			AUDIO_SVC_TRACK_DATA_DESCRIPTION, "Test description", strlen("Test description"),
			AUDIO_SVC_TRACK_DATA_YEAR, 2011,
			AUDIO_SVC_TRACK_DATA_TRACK_NUM, 10,
			AUDIO_SVC_TRACK_DATA_ALBUM_RATING, AUDIO_SVC_RATING_5,
			-1);

		if (ret < 0)
		{
			printf( "failed to update items. error code->%d", ret);
			audio_svc_list_item_free(handle);
			return;
		}

	}

	ret = audio_svc_list_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_update_item_metadata(const char *audio_id, audio_svc_track_data_type_e  first_field_name, ...);


/**
 * 	audio_svc_refresh_metadata:\n
 * 	This function extracts metadata from file and save them into database. \n
 * 	This function checks whether the metadata is already extracted, if so, then do nothing and return. If not, then extracted metadata by
 * 	mm-fileinfo middleware library API, and update the music record in database with the result metadata.
 *
 * 	@param[in] 	audio_id				db index of item to refresh metadata
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 * 				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@remark	This function will be called when receiving right object of DRM content.
 *				If it has already valid metadata, then it just returns TRUE without extracting metadata.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void
refresh_drm_metadata()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	char * test_audio_id = "550e8400-e29b-41d4-a716-446655440000";

	ret = audio_svc_refresh_metadata(test_audio_id);
	if (ret < 0)
	{
		printf( "failed to refresh metadata. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */
int audio_svc_refresh_metadata(const char *audio_id);


/** @} */

/**
        @defgroup AUDIO_SVC_PLST_API     Playlist API
        @{
        @par
        manage playlist and its contents.
 */


/**
 * 	audio_svc_count_playlist:\n
 * 	This function counts the playlists created by user by given filter condition. The filter string act as the LIKE cluase for playlist name in
 *	the SQL condition search, such as "where name like %s".
 *
 * 	@param[in]		filter_string		the filter for playlist name.
 * 	@param[in]		filter_string2		the filter for playlist name.
  * 	@param[out]		count			the return count of playlist.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			audio_svc_get_playlist
 *	@remark		None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void get_playlist_count()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;

	// get the count of all playlist in db.
	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlists. error code->%d", ret);
		return;
	}

	printf("playlist count is %d", count);

	return;
}

* 	@endcode
*/

int audio_svc_count_playlist(const char *filter_string, const char *filter_string2, int *count);


/**
 * 	audio_svc_get_playlist:\n
 * 	This function retrieves the playlists created by user with given filter condition. The filter string act as the LIKE cluase for playlist name in
 *	the SQL condition search. The parameter "rows" and "offset" is used as the LIMIT expression and OFFSET expression.
 *
 * 	@param[in]		filter_string		the filter for playlist name.
 * 	@param[in]		filter_string2		the filter for playlist name.
  * 	@param[in]		offset			start position to fetch the records
 * 	@param[in]		rows			the number of required records
 * 	@param[out]	playlists			reference pointer of result records. Its memory size should be rows*sizeof(audio_svc_playlist_s).
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see  			audio_svc_count_playlist
 *	@pre			None
 *	@post			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlists()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	AudioHandleType *handle = NULL;

	// get the count of all playlist in db.
	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlists. error code->%d", ret);
		return;
	}

	if(count < 1)
	{
		printf("there is no playlist");
		return;
	}

	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	ret = audio_svc_get_playlist("", "", 0, count, handle);
	if (ret < 0)
	{
		printf( "failed to get playlists. error code->%d", ret);
		audio_svc_playlist_free(handle);
		return;
	}

	ret = audio_svc_playlist_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_get_playlist(const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *playlists);


/**
 * 	audio_svc_playlist_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType,
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements of type audio_svc_playlist_s.
 *
 * 	@param[in]	record			The handle for playlist object.
 * 	@param[in]	count			The element count to be allocate
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_playlist_free
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void new_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//create playlists
	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */

int audio_svc_playlist_new(AudioHandleType **record, int count);

/**
 * 	audio_svc_playlist_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_list_playlist_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for playlist object.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_playlist_new
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void free_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//create playlist object, object number is "count".
	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free playlist object
	ret = audio_svc_playlist_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}
	return;
}

 * 	@endcode
 */

int audio_svc_playlist_free(AudioHandleType *record);

/**
 * 	audio_svc_playlist_get_val:\n
 * 	This function allows to get a list of attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	playlists	The handle for playlist
 * 	@param[in]	index		The element index in array
 * 	@param[in]	first_field_name	the variable arguements list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_playlist_set_val
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlist_value()
{
	AudioHandleType*handle = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	int i = 0;
	int count = 0;

	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist. error code->%d", ret);
		return;
	}

	if (count < 1)
	{
		printf( "there is no playlist");
		return;
	}

	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get all the playlists in db.
	ret = audio_svc_get_playlist(
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				count, //count
				handle);
	if (ret < 0)
	{
		printf( "failed to get playlist. error code->%d", ret);
		audio_svc_playlist_free(handle);
		return;
	}

	for (i = 0; i < count; i++) {
		char *p = NULL;
		int plst_id;
		int size;
		//get the playlist id and playlist name of each playlist
		ret = audio_svc_playlist_get_val(handle, i, AUDIO_SVC_PLAYLIST_ID, &plst_id, AUDIO_SVC_PLAYLIST_NAME, &p, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get playlist attribute value. error code->%d", ret);
			audio_svc_playlist_free(handle);
			return;
		}
	}

	ret = audio_svc_playlist_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_playlist_get_val(AudioHandleType *playlists, int index, audio_svc_playlist_e first_field_name, ...);

/**
 * 	audio_svc_playlist_set_val:\n
 * 	This function allows to retrieve a list of attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	playlists			The handle for playlist
 * 	@param[in]	index			The element index in array
 * 	@param[in]	first_field_name	the variable arguements list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		audio_svc_playlist_get_val
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void set_playlist_value()
{
	int count = 0;
	AudioHandleType *handle = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	char *p = NULL;
	int plst_id = 0;
	int size = AUDIO_SVC_METADATA_LEN_MAX;
	int i = 0;

	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist. error code->%d", ret);
		return;
	}

	if (count < 1)
	{
		printf( "there is no playlist");
		return;
	}

	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get all the playlists.
	ret = audio_svc_get_playlist(
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				count, //rows
				handle);
	if (ret < 0)
	{
		printf( "failed to get playlist. error code->%d", ret);
		audio_svc_playlist_free(handle);
		return;
	}

	//set the name of first playlist to "playlist_test_name"
	ret = audio_svc_playlist_set_val(handle, i, AUDIO_SVC_PLAYLIST_NAME, "playlist_test_name", size, -1);
	if (ret < 0)
	{
		printf( "failed to set playlist attribute value. error code->%d", ret);
		audio_svc_playlist_free(handle);
		return;
	}

	ret = audio_svc_playlist_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_playlist_set_val(AudioHandleType *playlists, int index, audio_svc_playlist_e  first_field_name, ...);

/**
 * 	audio_svc_set_item_valid:\n
 * 	This function is used to retrieve the element from elements arrary by index.
 *	Internally the element type is audio_svc_playlist_s.
 *
 * 	@param[in]		record		The handle for playlist
 * 	@param[in]		index		The element index
 * 	@param[out]		plst			the element to be retrieved.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			audio_svc_list_item_get
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlist_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType*handle = NULL;
	int count = 0;
	int i = 0;

	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist. error code->%d", ret);
		return;
	}

	if (count < 1)
	{
		printf( "there is no playlist");
		return;
	}

	ret = audio_svc_playlist_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	ret = audio_svc_get_playlist(
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				count, //rows
				handle);
	if (ret < 0)
	{
		printf( "failed to get playlist. error code->%d", ret);
		return;
	}

	for (i = 0; i < count; i++) {
		char *p = NULL;
		int plst_id;
		int size;
		AudioHandleType *plst = NULL;

		//get the playlist object with index "i"
		ret = audio_svc_playlist_get_item(handle, i, &plst);
		if (ret < 0)
		{
			printf( "failed to get playlist item. error code->%d", ret);
			audio_svc_playlist_free(handle);
			return;
		}

		//get the id and name of playlist object.
		ret = audio_svc_playlist_get_val(plst, 0, AUDIO_SVC_PLAYLIST_ID, &plst_id, AUDIO_SVC_PLAYLIST_NAME, &p, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get playlist attribute value. error code->%d", ret);
			audio_svc_playlist_free(handle);
			return;
		}
	}

	ret = audio_svc_playlist_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;

}

 * 	@endcode
 */

int audio_svc_playlist_get_item(AudioHandleType *record, int index, AudioHandleType **plst);


/**
 *	audio_svc_count_playlist_item:\n
 *	This function counts the number of playlist item.\n
 * 	The filter string act as the LIKE cluase for item title in the SQL condition search. The parameter "rows" and "offset"
 *	is used as the LIMIT expression and OFFSET expression.
 *
 * 	@param[in]		playlist_id		playlist id.
 * 	@param[in]		filter_string		the filter condition for track title.
 * 	@param[in]		filter_string2	 	the filter condition for track title.
 * 	@param[out]		count			the returned tracks count.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see			None
 *	@pre			None
 *	@post			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlist_item_count()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	int playlist_id = 1;
	// get the count of all tracks in db.
	ret = audio_svc_count_playlist_item(playlist_id, "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist items. error code->%d", ret);
		return;
	}

	printf("playlist item count is %d", count);

	return;
}

 * 	@endcode
 */

int audio_svc_count_playlist_item(int playlist_id, const char *filter_string, const char *filter_string2, int *count);


/**
 *	audio_svc_get_playlist_item:\n
 * 	This function retrieves items of playlist.
 * 	The filter string act as the LIKE cluase for item title in the SQL condition search. The parameter "rows" and "offset"
 *	is used as the LIMIT expression and OFFSET expression.
 *
 * 	@param[in] 		playlist_id		playlist id.
 * 	@param[in] 		filter_string		the filter condition for track title.
 * 	@param[in] 		filter_string2		the filter condition for track title.
 * 	@param[in]		offset	   		start position to fetch the records satisfied with given condition.
 * 	@param[in]		rows 			the number of required records which is satisfied with given condition.
 * 	@param[out]		track			reference pointer of result records.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see			audio_svc_count_playlist_item
 *	@pre			None
 *	@post			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_track_item()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = -1;
	int playlist_id = 1;

	// get the count of all tracks in db.
	ret = audio_svc_count_playlist_item(playlist_id, "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist items. error code->%d", ret);
		return;
	}

	if(count < 0)
	{
		printf( "There is no items");
		return;
	}

	ret = audio_svc_playlist_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get the playlist items.
	ret = audio_svc_get_playlist_item(plst_id,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //rows,
		handle
		);

	if (ret < 0)
	{
		audio_svc_playlist_item_free(handle);
		return;
	}

	audio_svc_playlist_item_free(handle);
	return;
}

 * 	@endcode
 */

int audio_svc_get_playlist_item(int playlist_id, const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *playlist_item);


/**
 * 	audio_svc_playlist_item_new:\n
 * 	This function is used to allocate  a block (an array) of elements of type AudioHandleType.
 *	where number of elements is speicified by the second parameter "count". Therefore, the first parameter
 *	"record" points to a valid block of memory with space for "count" elements.
 *
 * 	@param[in]		record		The handle for service list
 * 	@param[in]		count		the elements number
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None.
 *	@post			None
 *	@see			audio_svc_count_playlist_item, audio_svc_playlist_item_free
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void playlist_item_new()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 0;

	ret = audio_svc_count_playlist_item(AUDIO_SVC_TRACK_ALL, "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of items. error code->%d", ret);
		return;
	}

	//allocate the memory of type list item with count
	ret = audio_svc_playlist_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_playlist_item_new(AudioHandleType **record, int count);


/**
 * 	audio_svc_playlist_item_free:\n
 * 	This function is used to free memory allocated for arrays of element with type AudioHandleType.
 *	The value passed as argument to delete must be either a pointer to a memory block previously allocated
 *	with audio_svc_playlist_item_new(), or a null pointer (in the case of a null pointer, this function produces no effect).
 *
 * 	@param[in]	record		The handle for service list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None
 *	@see		audio_svc_playlist_item_new.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void playlist_item_free()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *handle = NULL;
	int count = 5;

	//allocate the memory of type list item with count
	ret = audio_svc_playlist_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//free the list item memory.
	ret = audio_svc_playlist_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}


 * 	@endcode
 */

int audio_svc_playlist_item_free(AudioHandleType *record);


/**
 * 	audio_svc_playlist_item_get_val:\n
 * 	This function allows to retrieve a list of attributes for the specified element in the elements array by index.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	index		The element index in array
 * 	@param[in]	first_field_name	the variable arguements list
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None
 *	@see		audio_svc_playlist_item_new, audio_svc_playlist_item_free, audio_svc_count_playlist_item.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlist_item_get_value()
{
	int count = 0;
	AudioHandleType *handle = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = 1;

	ret = audio_svc_count_playlist_item(plst_id, "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist items. error code->%d", ret);
		return;
	}

	ret = audio_svc_playlist_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get the playlist items.
	ret = audio_svc_get_playlist_item(plst_id,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //rows,
		handle
		);

	if (ret < 0)
	{
		audio_svc_playlist_item_free(handle);
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *audio_id = NULL, *title = NULL, *artist = NULL, *thumbname = NULL, *pathname = NULL;
		int uid = -1;
		int rating = AUDIO_SVC_RATING_NONE;
		int duration = 0;
		int size = 0;
		int play_order = 0;

		ret = audio_svc_playlist_item_get_val(handle, i ,
			AUDIO_SVC_PLAYLIST_ITEM_UID, &uid,
			AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID, &audio_id, &size,
			AUDIO_SVC_PLAYLIST_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_PLAYLIST_ITEM_TITLE, &title, &size,
			AUDIO_SVC_PLAYLIST_ITEM_ARTIST, &artist, &size,
			AUDIO_SVC_PLAYLIST_ITEM_PATHNAME, &pathname, &size,
			AUDIO_SVC_PLAYLIST_ITEM_DURATION, &duration,
			AUDIO_SVC_PLAYLIST_ITEM_RATING, &rating,
			AUDIO_SVC_PLAYLIST_ITEM_PLAY_ORDER, &play_order,
			-1);

		if (ret < 0)
		{
			printf( "failed to get playlist items. error code->%d", ret);
			audio_svc_playlist_item_free(handle);
			return;
		}
	}

	ret = audio_svc_playlist_item_free(handle);
	if (ret < 0)
	{
		printf( "failed to free handle. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */

int audio_svc_playlist_item_get_val(AudioHandleType *record, int index, audio_svc_playlist_item_type_e first_field_name, ...);


/**
 * 	audio_svc_playlist_item_get:\n
 * 	This function is used to retrieve the element from elements arrary by index.
 *
 * 	@param[in]	record		The handle for service list
 * 	@param[in]	index		The element index
 * 	@param[out]	item		the element to be retrieved.
 * 	@return	This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void get_playlist_item()
{
	AudioHandleType *handle = NULL;
	AudioHandleType *item = NULL;
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = 0;
	int i = 0

	ret = audio_svc_count_playlist_item(plst_id, "", "", &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist items. error code->%d", ret);
		return;
	}

	ret = audio_svc_playlist_item_new(&handle, count);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	//get the playlist items.
	ret = audio_svc_get_playlist_item(plst_id,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		count, //rows,
		handle
		);

	if (ret < 0)
	{
		audio_svc_playlist_item_free(handle);
		return;
	}

	for (i = 0; i < count; i++)
	{
		char *audio_id = NULL;
		int size = 0;
		//get the playlist item with index "i"
		ret = audio_svc_playlist_item_get(handle, i, &item);
		if (ret < 0)
		{
			printf( "failed to get playlistlist items. error code->%d", ret);
			audio_svc_playlist_item_free(handle);
			return;
		}
		
		ret = audio_svc_playlist_item_get_val(item, 0, AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
		if (ret < 0)
		{
			printf( "failed to get playlist items value. error code->%d", ret);
			audio_svc_playlist_item_free(handle);
			return;
		}
	}

	return;
}

 * 	@endcode
 */

int audio_svc_playlist_item_get(AudioHandleType *record, int index, AudioHandleType **item);

/**
 * 	audio_svc_add_playlist:\n
 * 	This function saves new playlist. When user create a new playlist, application call this function to save it.
 *
 * 	@param[in]		playlist_name	playlist name
 * 	@param[out]	playlist_id		playlist index
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.\n
 * 					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@remark		To guarantee uniqueness of playlist name, call 'audio_svc_check_duplication_of_playlist_name()' before this function.
 *	@pre			None
 *	@post 			This created playlist index can be used
 *	@see			audio_svc_delete_playlist
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void add_playlist()
 {
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	// add playlist with name "plst_test_001"
	ret = audio_svc_add_playlist(playlist_name, &plst_id);

	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}
	// printf the playlist index
	printf("playlist index is %d", plst_id);

	return;
}

* 	@endcode
*/
int audio_svc_add_playlist(const char *playlist_name, int *playlist_id);


/**
 * 	audio_svc_delete_playlist:\n
 * 	This function deletes a playlist. Internally this function not only deletes the playlist records from playlist table,
 *	but also deletes track items from playlist item table belongs to the playlist.
 *
 * 	@param[in]	playlist_id	index of playlist. It will be allocated when saving a playlist.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see		audio_svc_add_playlist
 *	@pre		None
 *	@post		None
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void delete_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";

	//append a playlist with name "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	//delete the playlist "plst_test_001"
	ret = audio_svc_delete_playlist(plst_id);
	if (ret < 0)
	{
		printf( "failed to delete playlist. error code->%d", ret);
		return;
	}

	return;
}

 * 	@endcode
 */
int audio_svc_delete_playlist(int playlist_id);


/**
 * 	audio_svc_add_item_to_playlist:\n
 * 	This function adds a track item to playlist.. Internally this function insert a record into playlist item table.
 *
 * 	@param[in] 	playlist_id		playlist index
 * 	@param[in]	audio_id			db index of item to be added
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		the playlist with index "playlist_id" should exist, the track with "audio_id" should be valid.
 *	@post 		None
 *	@see		audio_svc_remove_item_from_playlist_by_uid
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void add_item_to_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	char * test_audio_id = "550e8400-e29b-41d4-a716-446655440000";

	// append a playlist with name "plst_test_001"
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	// append a track with test_audio_id into playlist "plst_test_001"
	ret = audio_svc_add_item_to_playlist(plst_id, test_audio_id);
	if (ret < 0)
	{
		printf( "failed to add item to playlist. error code->%d", ret);
		return;
	}

	return;

}

 * 	@endcode
 */
int audio_svc_add_item_to_playlist(int playlist_id, const char *audio_id);


/**
 * 	audio_svc_remove_item_from_playlist_by_uid:\n
 * 	This function removes a track item from playlist. Internally this function delete the record from playlist item table.
 *	In the playlist item table, the playlist index and audio_id both can identify a unique record.\n
 *	When user deletes a track from a playlist, application call this function to complete the task.
 * 	@param[in]	playlist_id		playlist index
 * 	@param[in]	uid 			db index of playlist item not audio_id
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void remove_item_from_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	int test_uid = 50;

	//append playlist "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	//remove music item with uid "50" from playlist "plst_test_001"
	ret = audio_svc_remove_item_from_playlist_by_uid(plst_id, test_uid);
	if (ret < 0)
	{
		printf( "failed to remove item to playlist. error code->%d", ret);
		return;
	}

	return;

}

 * 	@endcode
 */

int audio_svc_remove_item_from_playlist_by_uid(int playlist_id, int uid);


/**
 * 	audio_svc_remove_item_from_playlist_by_audio_id:\n
 * 	This function removes a track item from playlist. Internally this function delete the record from playlist item table.
 *	In the playlist item table, the playlist index and audio_id both can identify a unique record.\n
 *	When user deletes a track from a playlist, application call this function to complete the task.
 * 	@param[in]	playlist_id		playlist index
 * 	@param[in]	audio_id 		db index of playlist item
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

 void remove_item_from_playlist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	char * test_audio_id = "550e8400-e29b-41d4-a716-446655440000";

	//append playlist "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	//remove music item with uid "50" from playlist "plst_test_001"
	ret = audio_svc_remove_item_from_playlist_by_audio_id(plst_id, test_audio_id);
	if (ret < 0)
	{
		printf( "failed to remove item to playlist. error code->%d", ret);
		return;
	}

	return;

}

 * 	@endcode
 */

int audio_svc_remove_item_from_playlist_by_audio_id(int playlist_id, const char *audio_id);


/**
 * 	audio_svc_get_unique_playlist_name:\n
 * 	This function makes unique name and returns it after retrieving playlist table with original playlist name.
 * 	This function call audio_svc_count_playlist_by_name() to check whether the playlist with "orignal_name"
 * 	exist or not.
 *
 * 	@param[in]		orig_name					the original name to be checked
 * 	@param[in]		max_unique_name_length	max length of unique name 
 * 	@param[out]	unique_name				the unique playlist name based on the original name
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			None
 *	@remark		None
 * 	@par example
 * 	@code

#include <audio-svc.h>

gchar* get_new_playlist_name (void)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	char unique_name[24] = "\0";

	//get a unique playlist name, based on "My playlist"
	ret = audio_svc_get_unique_playlist_name("My playlist", unique_name, 24);
	if (ret < 0)
	{
		printf( "failed to get unique playlist name. error code->%d", ret);
		return NULL;
	}

	if(unique_name == NULL)
	{
		printf("playlist name is NULL");
		return NULL;
	}
	else
	{
		return g_strdup(unique_name);
	}

	return NULL;
}

 * 	@endcode
 */

int audio_svc_get_unique_playlist_name(const char* orig_name, char *unique_name, size_t max_unique_name_length);


/**
 * 	audio_svc_get_playlist_name_by_playlist_id:\n
 * 	This function retrieves the playlist name by playlist index.\n
 *	Internally in music database the playlist index is the primary key of playlist table.
 *
 * 	@param[out]	playlist_name				playlist name
 * 	@param[in]		max_playlist_name_length	max length of playlist name
 * 	@param[in]		playlist_id					playlist index
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 		Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			None
 *	@remark		None
 * 	@par example
 * 	@code
 *
 #include <audio-svc.h>

char * get_playlist_name_by_playlist_id (int playlist_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	char playlist_name[AUDIO_SVC_PATHNAME_SIZE] = {0,};

	//get the playlist name by playlist id "playlist_id"
	ret = audio_svc_get_playlist_name_by_playlist_id(playlist_id, playlist_name, AUDIO_SVC_PATHNAME_SIZE);
	if (ret < 0)
	{
		printf( "failed to get playlist name. error code->%d", ret);
		return NULL;
	}
	return playlist_name;
}
 * 	@endcode
 */

int audio_svc_get_playlist_name_by_playlist_id(int playlist_id, char *playlist_name, size_t max_playlist_name_length);


/**
 * 	audio_svc_get_playlist_id_by_playlist_name:\n
 * 	This function find playlist index by playlist name in music db.
 * 	playlist id and playlist name are unique value
 *
 * 	@param[in]		playlist_name	playlist name for find playlist index.
 * 	@param[out]	playlist_id		the unique playlist id.
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			None
 *	@remark		None
 * 	@par example
 * 	@code

#include <audio-svc.h>

int get_playlist_id (void)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int playlist_id = 0;

	//get a playlist index.
	ret = audio_svc_get_playlist_id_by_playlist_name("My playlist", &playlist_id);
	if (ret < 0)
	{
		printf( "failed to get playlist index. error code->%d", ret);
	}

	return ret;
}

 * 	@endcode
 */

int audio_svc_get_playlist_id_by_playlist_name(const char *playlist_name, int *playlist_id);


/**
 * 	audio_svc_update_playlist_name:\n
 * 	This function updates the playlist with index "playlist_id" to the new name " playlist name".
 *	Here the "playlist_id" is the value of "_id" field in playlist talbe, and the "_id" field is the primary key.
 *	So internally audio service can locate a playlist with "playlist_id".
 *
 * 	@param[in]	playlist_id			index of playlist. It will be allocated when saving a playlist.
 * 	@param[in]	new_playlist_name	changed playlist name
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 * 	@see		audio_svc_add_playlist
 *	@pre		None
 *	@post		None
 *	@remark	None
 * 	@par example
 * 	@code

#include <audio-svc.h>

void
update_playlist_name()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";

	//append a playlist "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	//rename playlist name to "plst_test_002"
	ret = audio_svc_update_playlist_name(plst_id, "plst_test_002");
	if (ret < 0)
	{
		printf( "failed to update playlist name. error code->%d", ret);
		return;
	}
}

 * 	@endcode
 */
int audio_svc_update_playlist_name(int playlist_id, const char *new_playlist_name);


/**
 * 	audio_svc_update_playlist_item_play_order:\n
 * 	This function update the playlist item play order.\n
 * 	This function is used for playlist item reorder. When user reorder the items in a playlist, app call this function to finished the reorder in database.
 * 	In music application, the track items are listed by the order of item index.
 *
 * 	@param[in]	playlist_id			playlist id.
 * 	@param[in]	uid					unique playlist item id. not audio_id.
 * 	@param[in]	new_play_order		new play order.
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *				Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@see		None
 *	@remark	None
 * 	@par example
 * 	@code

  #include <audio-svc.h>

void update_playlist_order()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int playlist_id = 1;
	int uid = 1;
	int new_play_order = 5;

	//update the item index in playlist
	ret = audio_svc_update_playlist_item_play_order(playlist_id, uid, new_play_order);
	if (ret < 0)
	{
		printf( "failed to update play order. error code->%d", ret);
		return;
	}
	return;
}
 * 	@endcode
 */

int audio_svc_update_playlist_item_play_order(int playlist_id, int uid, int new_play_order);


/**
 * 	audio_svc_count_playlist_by_name:\n
 * 	This function checks whether playlist with 'playlist_name' does exist or not in database.
 * 	If exist, the returned output parameter 'count' will be positive.
 *
 * 	@param[in]		playlist_name		playlist name
 * 	@param[out]		count				the count of playlists
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post			None
 *	@see			None
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

int
get_playlist_count(void)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	const char *playlist_name = "plst_test_001";

	//get the count of playlists whose name are "plst_test_001"
	ret = audio_svc_count_playlist_by_name(playlist_name, &count);
	if (ret < 0)
	{
		printf( "failed to get count of playlist . error code->%d", ret);
		return -1;
	}

	printf("playlist count is %d", count)

	return count;
}

 * 	@endcode
 */

int audio_svc_count_playlist_by_name(const char* playlist_name, int*count);


/**
 * 	audio_svc_check_duplicate_insertion_in_playlist:\n
 * 	This function checks whether a track with db index "audio_id" does exist or not in a playlist with index "playlist_id".
 * 	This function is alwayed used for avoiding inserting duplicate track into a playlist.
 *
 * 	@param[in]		playlist_id		playlist index
 * 	@param[in]		audio_id				db index of item to be added
 * 	@param[out]		count			the count of tracks in playlist
 * 	@return			This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *					Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre			None
 *	@post 			None
 *	@see			audio_svc_add_item_to_playlist
 *	@remark		None
 * 	@par example
 * 	@code

 #include <audio-svc.h>

bool check_item_exist (gint plst_id, gint key_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;
	char *audio_id = "550e8400-e29b-41d4-a716-446655440000";

	//get the count of music track with audio_id in playlist "plst_id"
	ret = audio_svc_check_duplicate_insertion_in_playlist(plst_id, audio_id, &count);
	if (ret < 0)
	{
		if(count > 0)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		printf( "failed to check duplicate insertion. error code->%d", ret);
		return ret;
	}
}

 * 	@endcode
 */
int audio_svc_check_duplicate_insertion_in_playlist(int playlist_id, const char *audio_id, int * count);

/**
 * 	audio_svc_list_by_search:\n
 * 	This function allows to search tracks by string to be searched.
 *	This function takes a variable number of arguments, the arguments format is pair of atrribute index and attribute value.
 *	The last parameter should be "-1", which tell the compiler that the arguments list is over.
 *
 * 	@param[in]	handle			The handle for search
 * 	@param[in]	order_field		field to order
 *			 	Please refer 'audio-svc-types.h', and see the enum audio_svc_search_order_e
 * 	@param[in]	offset		offset of list to be searched
 * 	@param[in]	count		count of list to be searched
 * 	@param[in]	first_field_name	the variable arguements list of field to search
 *			 	Please refer 'audio-svc-types.h', and see the enum audio_svc_serch_field_e
 * 	@return		This function returns zero(AUDIO_SVC_ERROR_NONE) on success, or negative value with error code.
 *			 	Please refer 'audio-svc-error.h' to know the exact meaning of the error.
 *	@pre		None.
 *	@post		None.
 *	@see		None.
 *	@remark	None.
 * 	@par example
 * 	@code

 #include <audio-svc.h>

void test_audio_svc_list_by_search()
{
		int offset = 0, count = 10, i = 0;
		const char *str = "Sa";
		AudioHandleType *handle = NULL;

		err = audio_svc_search_item_new(&handle, count);
		if (err < 0) {
			printf("audio_svc_search_item_new failed:%d\n", err);
			return err;
		}

		err = audio_svc_list_by_search(handle, AUDIO_SVC_ORDER_BY_TITLE_ASC, offset, count, AUDIO_SVC_SEARCH_TITLE, str, strlen(str), AUDIO_SVC_SEARCH_ALBUM, str, strlen(str), AUDIO_SVC_SEARCH_ARTIST, str, strlen(str), -1);

		if (err != AUDIO_SVC_ERROR_NONE) {
			mediainfo_dbg("Fail to get items : %d", err);
			return err;
		}
		
		for (i = 0; i < count; i++) {
			AudioHandleType *item = NULL;
			err = audio_svc_search_item_get(handle, i, &item);
			char *audio_id = NULL, *title = NULL, *artist = NULL, *pathname = NULL, *album = NULL;
			int size = 0;
			if (err < 0) {
				printf("[%d] audio_svc_search_item_get failed : %d\n", i, err);
			} else {
				audio_svc_item_get_val(item,  
						AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size,
						AUDIO_SVC_TRACK_DATA_PATHNAME, &pathname, &size,
						AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
						AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
						AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
						-1);

				if( audio_id == NULL ) break;

				printf("[%d] ID: %s\n", i, audio_id);
				printf("[%d] Path: %s\n", i, pathname);
				printf("[%d] Title: %s\n", i, title);
				printf("[%d] Artist: %s\n", i, artist);
				printf("[%d] Album: %s\n", i, album);
			}
		}

		audio_svc_search_item_free(handle);
}

 * 	@endcode
 */
int audio_svc_list_by_search(AudioHandleType *handle,
							audio_svc_search_order_e order_field,
							int offset,
							int count,
							audio_svc_serch_field_e first_field_name,
							...);


/** @} */

/**
	@}
 */

#ifdef __cplusplus
}
#endif

#endif /*_AUDIO_SVC_H_*/
