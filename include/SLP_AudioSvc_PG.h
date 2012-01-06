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
 *
 * @defgroup   AUDIO_SVC_PG Audio Service
    @ingroup SLP_PG
    @{


<h1 class="pg">Introduction</h1>
<h2 class="pg">Purpose</h2>

The purpose of this document is to give the application developer detailed information to access  music database using the Audio Service API's.


<h2 class="pg">Scope</h2>

The scope of this document is limited to Samsung platform Audio Service API usage.

<h2 class="pg">Abbreviations</h2>

 	<table>
 		<tr>
  			<td>
  			API
  			</td>
  			<td>
  			Application Programming Interface
  			</td>
  		</tr>
 		<tr>
  			<td>
			  Group
  			</td>
  			<td>
			 a collection of related audio or music tracks like Album, Artist, Genre,  Year, Composer
  			</td>
  		</tr>
 		<tr>
  			<td>
			  Playlist
  			</td>
  			<td>
  			a simple list of songs
  			</td>
  		</tr>
 		<tr>
  			<td>
			  Extractor
  			</td>
  			<td>
			  Extracts metadata associated with a particular media format from a file or records in the databases.
  			</td>
  		</tr>
		<tr>
  			<td>
			  File manager service
  			</td>
  			<td>
			  Detect and manage all of file system chagnes.
  			</td>
  		</tr>
	</table>

<h1 class="pg">Audio Service Architecture</h1>


Audio Service is designed to provide easy to use services for accessing music library database based on an embedded Linux platform.
Media Service is used to maintain music content in SQLite database. The content stored in database consists of music items from both MMC and Phone flash, playlist, and playlist elements.


<h2 class="pg">Features</h2>
You can use Audio Service to organize your digital music collection.
You can do a number of things with them, including playing music, extract metadata, creating playlists.

- Add items to the Music library
	- File manager service automatically searches certain default folders on your phone 	and MMC for music files, and then adds those files to your library. If you ever add or remove files from these
	folders, File manager service automatically updates the library accordingly.

- Delete items from the Music library
	- You can delete a music file from the library, and the song will be removed forever in your device.

- Find items in the Music library
	- Audio Service provides smart search for you to find items in the music library. To ensure that you can easily find items in the library, it is important that files have accurate and complete media information.
You can also find files those were recently added / recently played / Most played in the library.

- Using playlists
	- Playlists are a great way to control how you use the files in your Music library. By creating a playlist, you can group any songs into a list that you can then quickly
	play. Favorite playlist(playlist_id = 0) is default folder for you.

<h2 class="pg">Architecture diagram</h2>


<b>Modules</b>

- Database Manager

Database Manager component is responsible for database manipulation, such as open database, close database, create table, set the storage validation, and delete the storage contents.


- Music Group Manager

Music Group Manager component is responsible for manipulation of tracks by the group type, such as album, artist, genre, and etc.

- Music Item Manager

Music Item Manager component is responsible for music track manipulation, including add track, delete track, copy track, move track, refresh track metadata, and etc.

- Playlist Manager

Playlist Manager component is responsible for managing playlists and its contents, such as appending playlist, deleting playlist, and manage the tracks in playlist.

@image html SLP_MusicSVC_PG_image001.png

<h1 class="pg">Audio Service API usage</h1>
<h2 class="pg">Database Manager</h2>

This part describes the APIs of the Database Manager component.

<b>Database schema design</b>
@image html SLP_MusicSVC_PG_image002.png


<b>Functions</b>

The followings are full proto types of database manager functions.

- int audio_svc_open(void);

- int audio_svc_close(void);

- int audio_svc_create_table();

- int audio_svc_set_db_valid(audio_svc_storage_type_e storage_type, int valid);

- int audio_svc_delete_all(audio_svc_storage_type_e storage_type);

<b>Open music database and create tables</b>

- Purpose
	- Be prepared for registering music tracks into database.


Make music database and table by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Create table using audio_svc_create_table(). All table will be made by this API.

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;

	// open music database
	ret = audio_svc_open();
	// open failed
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	// create muisc tables of phone type
	ret = audio_svc_create_table();

	if (ret < 0)
	{
		printf( "unable to create music phone table. error code->%d", ret);
		return;
	}

	return;

@endcode

<b>Close music database</b>

- Purpose
	- Disconnects with the music database.

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;

	// close music database
	ret = audio_svc_close();
	// close failed
	if (ret < 0)
	{
		printf( "unable to close music db. error code->%d", ret);
		return ret;
	}


@endcode

<h2 class="pg">Music Group Manager</h2>

This section describes APIs used for managing music groups.

The following are supported functions in the group manager module.
		- int 	audio_svc_list_item_new(AudioHandleType **record, int count);

		- int 	audio_svc_list_item_free(AudioHandleType *record);

		- int 	#audio_svc_list_item_get_val(AudioHandleType *record, int index, audio_svc_list_item_type_e first_field_name, ...);

		- int 	audio_svc_list_item_get(AudioHandleType *record, int index, AudioHandleType **item);

		- int 	audio_svc_count_group_item(audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2, int *count);

		- int 	audio_svc_get_group_item(audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *result_records);

		- int 	audio_svc_count_list_item(audio_svc_track_type_e item_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int *count);

		- int 	audio_svc_get_list_item(audio_svc_track_type_e item_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *track);


<b>Get Album groups</b>

- Purpose
	- Find all albums in the music database.


Find all albums in music database and get album and artist name information from database by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Count albums using audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "", "", "", "", &count).
-# Make array to get all album info using audio_svc_list_item_new(&groups, count).
-# Get the all album info using audio_svc_get_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "", "", "", "", 0, count, groups).
-# Get each album's attribute value using audio_svc_list_item_get_val(groups, i, AUDIO_SVC_LIST_ITEM_TITLE, &first_name, &size,  -1).
-# free memory using audio_svc_list_item_free(groups).

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;
	int i = 0, count = -1, size = -1;
	AudioHandleType  *groups = NULL;
	char * first_name = NULL, second_name = NULL, thumbnail_path = NULL;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	//Count album
	ret = audio_svc_count_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "", "", "", "", &count);
	if (ret < 0)
	{
		printf( "Fail to count groups. error code->%d", ret);
		return;
	}

	if(count == 0)
	{
		DEBUG_TRACE("There is no item");
		return FALSE;
	}

	ret = audio_svc_list_item_new(&groups, count);
	if (ret < 0)
	{
		printf( "Fail to allocate memory for list. error code->%d", ret);
		return;
	}

	ret = audio_svc_get_group_item(AUDIO_SVC_GROUP_BY_ALBUM, "", "", "", "", 0, count, groups);
	if (ret < 0)
	{
		printf( "Fail to get groups. error code->%d", ret);
		return;
	}

	for(i=0; i < count; i++)
	{
		//Get album name
		audio_svc_list_item_get_val(groups, i, AUDIO_SVC_LIST_ITEM_TITLE, &first_name, &size,  -1);
		//Get artist name of album
		audio_svc_list_item_get_val(groups, i, AUDIO_SVC_LIST_ITEM_ARTIST, &second_name, &size,  -1);
		//Get thumbnail path of first item in album
		audio_svc_list_item_get_val(groups, i, AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH, &thumbnail_path, &size,  -1);
	}

	ret = audio_svc_list_item_free(groups);
	if (ret < 0)
	{
		printf( "Fail to free memory for list. error code->%d", ret);
	}

	return;

@endcode


<b>Search and get all tracks in music library</b>

- Purpose
	- Find all tracks and get metadata in the music database.


Find all tracks in music database and get metadata information from database by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Count tracks using audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &rows).
-# Make array to get all tracks info using audio_svc_list_item_new(&tracks, rows).
-# Get the all tracks info using audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, NULL, NULL, NULL, NULL, 0, rows, tracks).
-# Get each tracks attribute value using audio_svc_item_get_val(item, AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, AUDIO_SVC_TRACK_DATA_TITLE, &title, &size, AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size, AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size, AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH, &thumbname, &size, AUDIO_SVC_TRACK_DATA_YEAR, &year, &size, -1);
-# Free memory allocation using audio_svc_list_item_free(tracks).


The following is a sample code:


@code

	AudioHandleType *item;
	int ret = AUDIO_SVC_ERROR_NONE;
	int audio_id;
	int i, j;
	int rows;
	AudioHandleType*tracks = NULL;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	// Count all tracks
	ret = audio_svc_count_list_item(AUDIO_SVC_TRACK_ALL, "", "", "", "", &rows);
	if (ret < 0)
	{
		printf( "failed to  count items. error code->%d", ret);
		return;
	}

	ret = audio_svc_list_item_new(&tracks, rows);
	if (ret < 0)
	{
		printf( "failed to allocate handle. error code->%d", ret);
		return;
	}

	ret = audio_svc_get_list_item(AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		rows, //rows,
		tracks
		);

	if (ret == AUDIO_SVC_ERROR_NONE) {
		audio_svc_item_new(&item);
		for (i = 0; i < rows; i++)
		{
			audio_svc_list_item_get_val(tracks, i, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, -1);
			ret = audio_svc_get_item_by_audio_id(audio_id, item);
			if (ret == AUDIO_SVC_ERROR_NONE) {
				int audio_id;
				char *title, *album, *artist, *thumbname, *year;
				int size;
				audio_svc_item_get_val(item,
					AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id,
					AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
					AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
					AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
					AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH, &thumbname, &size,
					AUDIO_SVC_TRACK_DATA_YEAR, &year, &size,
					-1);

				printf(" [audio_id]:%d, [title] :%s, [album] : %s, [artist] : %s, [thumb] : %s, [year] : %s\n",
					audio_id, title, album, artist, thumbname, year);
			}
			else
			{
				printf("error to get item by audio_id : %d\n", audio_id);
			}
		}
		audio_svc_item_free(item);
	}
	else
	{
		printf( "failed to  get items. error code->%d", ret);
	}

	ret = audio_svc_list_item_free(tracks);
	if (ret < 0)
	{
		printf( "Fail to free memory, error code->%d", ret);
	}

	return;

@endcode


<h2 class="pg">Muisc Item Manager</h2>

Muisc Item Manager module provides APIs to manage music tracks in library.

The following are supported functions

		- int 	audio_svc_item_new(AudioHandleType **record);

		- int 	audio_svc_item_free(AudioHandleType *record);

  		- int 	#audio_svc_item_set_val(AudioHandleType *record, audio_svc_track_data_type_e  first_field_name, ...);

		- int 	#audio_svc_item_get_val(AudioHandleType *record, audio_svc_track_data_type_e  first_field_name, ...);

		- int 	audio_svc_insert_item(audio_svc_storage_type_e storage_type, const char *path, int category);

		- int 	audio_svc_get_item_by_audio_id(int audio_id, AudioHandleType *item_handle);

		- int 	audio_svc_get_item_by_path(const char *path, AudioHandleType *item_handle);

		- int 	audio_svc_delete_item_by_path(const char *path);

		- int 	audio_svc_move_item(audio_svc_storage_type_e src_storage, const char *src_path, audio_svc_storage_type_e dest_storage, const char *dest_path);

		- int 	audio_svc_get_path_by_audio_id(int audio_id, char *path);

		- int 	audio_svc_set_item_valid(const char *path, int valid);

		- int 	audio_svc_refresh_metadata(int audio_id);

		- int	audio_svc_check_item_exist(const char *path);

<b>Insert music item to music database</b>

- Purpose
	- Insert music item into database with path. audio service extract metadata and register it in music database.


Insert music item by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# define path for music item.
-# insert item to music database using audio_svc_insert_item(AUDIO_SVC_STORAGE_PHONE, path, category).

The following is a sample code

@code

	int ret = AUDIO_SVC_ERROR_NONE;
	const char *path = "/opt/media/Sounds/Music/Layla.mp3";
	int category = AUDIO_SVC_CATEGORY_MUSIC;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	// insert a track into music db
	ret = audio_svc_insert_item(AUDIO_SVC_STORAGE_PHONE, path, category);

	if (ret < 0)
	{
		printf( "unable to insert item, error code->%d", ret);
		return;
	}

	return;

@endcode


<h2 class="pg">Music Playlist Manager</h2>

Music Playlist Manager module provides APIs to manage playlist and its contents.

The following are supported functions :

	- int 	audio_svc_playlist_new(AudioHandleType **record, int count);

	- int 	audio_svc_playlist_free(AudioHandleType *record);

	- int 	#audio_svc_playlist_get_val(AudioHandleType *playlists, int index, audio_svc_playlist_e first_field_name, ...);

	- int 	#audio_svc_playlist_set_val(AudioHandleType *playlists, int index, audio_svc_playlist_e  first_field_name, ...);

	- int 	audio_svc_playlist_get_item(AudioHandleType *record, int index, AudioHandleType **plst);

	- int 	audio_svc_add_playlist(const char *playlist_name, int *playlist_id);

	- int 	audio_svc_count_playlist(const char *filter_string, const char *filter_string2, int *count);

	- int 	audio_svc_get_playlist(const char *filter_string, const char *filter_string2, int offset, int rows, AudioHandleType *playlists);

	- int 	audio_svc_delete_playlist(int playlist_id);

	- int 	audio_svc_get_unique_playlist_name(const char *orig_name, char *unique_name);

	- int 	audio_svc_update_playlist_name(int playlist_id, const char *new_playlist_name);

	- int 	audio_svc_count_playlist_by_name(const char *playlist_name, int *count);

	- int 	audio_svc_add_item_to_playlist(int playlist_id, int audio_id);

	- int 	audio_svc_remove_item_from_playlist_by_uid(int playlist_id, int audio_id);

	- int 	audio_svc_check_duplicate_insertion_in_playlist(int playlist_id, int audio_id, int *count);

	- int 	audio_svc_get_playlist_name_by_playlist_id(int playlist_id, char *playlist_name);

	- int 	audio_svc_update_playlist_item_idx(int playlist_id, int audio_id, int new_idx);


<b>Make new playlist</b>

- Purpose
	-  User can make own playlist and resiger it to database.


Make and Add new playlist with unique name into database by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Add playlist into database using audio_svc_add_playlist(playlist_name, &plst_id).
-# User can handle playlist by using plst_id.

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	int test_audio_id = 50;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	//append playlist "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	return;

@endcode


<b>Add and Remove items from playlist</b>

- Purpose
	-  Add music items and Remove items from user made playlist. music items 	just removed from playlist and not deleted on filesystem.


Add and Remove music items from playlist and register it into database by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Add playlist into database using audio_svc_add_playlist(playlist_name, &plst_id).
-# Add music item to playlist with plst_id and music item audio_id using audio_svc_add_item_to_playlist(plst_id, test_audio_id).
-# Remove music item from playlist with plst_id and music item audio_id using audio_svc_add_item_to_playlist(plst_id, test_audio_id).

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;
	int plst_id = -1;
	const char *playlist_name = "plst_test_001";
	int test_audio_id = 50;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	//append playlist "plst_test_001" into db.
	ret = audio_svc_add_playlist(playlist_name, &plst_id);
	if (ret < 0)
	{
		printf( "failed to add playlist. error code->%d", ret);
		return;
	}

	//add music item with audio_id "50" to playlist "plst_test_001"
	ret = audio_svc_add_item_to_playlist(plst_id, test_audio_id);
	if (ret < 0)
	{
		printf( "failed to add item to playlist. error code->%d", ret);
		return;
	}

	//remove music item with audio_id "50" from playlist "plst_test_001"
	ret = audio_svc_remove_item_from_playlist(plst_id, test_audio_id);
	if (ret < 0)
	{
		printf( "failed to remove item to playlist. error code->%d", ret);
		return;
	}

	return;

@endcode


<b> Get all user playlist info</b>

- Purpose
	-  Get all user playlist name and playlist id from music database.


Get user playlist info from database by using the following steps:

-# Make database or connect to database using audio_svc_open().
-# Count all user playlist using audio_svc_count_playlist("", "", &count).
-# Allocate memory block to get the playlist using audio_svc_playlist_new(&playlists, count).
-# Get playlist items using audio_svc_get_playlist("", "", 0, count, playlists).
-# Get attribute values using audio_svc_playlist_get_val(playlists, i, AUDIO_SVC_PLAYLIST_ID, &plst_id, -1);
-# free memory using audio_svc_playlist_free(playlists);

The following is a sample code:

@code

	int ret = AUDIO_SVC_ERROR_NONE;
	AudioHandleType *playlists;
	char *name = NULL;
	int i = 0, count = -1, size = -1, plst_id = -1;

	// open music database
	ret = audio_svc_open();
	if (ret < 0)
	{
		printf( "Cannot open music db. error code->%d", ret);
		return;
	}

	// count all user playlist
	ret = audio_svc_count_playlist("", "", &count);
	if (ret < 0)
	{
		printf( "failed to get playlist count. error code->%d", ret);
		return;
	}

	if(count == 0)
	{
		DEBUG_TRACE("there is no user playlist");
		return;
	}

	// allocate memory block for playlist
	ret = audio_svc_playlist_new(&playlists, count);
	if (ret < 0)
	{
		printf( "failed to make playlist. error code->%d", ret);
		return;
	}

	//get playlist items
	ret = audio_svc_get_playlist("", "", 0, count, playlists);
	if (ret < 0)
	{
		printf( "failed to get playlist. error code->%d", ret);
		audio_svc_playlist_free(playlists)
		return;
	}

	// get each playlist's attribute value
	for(i = 0; i < count; i++)
	{
		audio_svc_playlist_get_val(playlists, i, AUDIO_SVC_PLAYLIST_ID, &plst_id, -1);
		audio_svc_playlist_get_val(playlists, i, AUDIO_SVC_PLAYLIST_NAME, &name, &size, -1);

	}

	ret = audio_svc_playlist_free(playlists);
	if (ret < 0)
	{
		printf( "failed to free memoy. error code->%d", ret);
	}

	return;

@endcode

/**
*@}
*/
