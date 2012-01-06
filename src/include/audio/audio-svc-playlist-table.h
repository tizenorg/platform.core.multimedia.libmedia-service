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



#ifndef _AUDIO_SVC_PLAYLIST_TABLE_H_
#define _AUDIO_SVC_PLAYLIST_TABLE_H_


/**
 * This file defines structure and functions related to database for managing playlists.
 *
 * @file       	audio-svc-playlist-table.h
 * @version 	0.1
 */

#include "audio-svc-types-priv.h"

int _audio_svc_create_playlist_table(void);
int _audio_svc_truncate_playlist_table(void);

/* playlist table */

/**
* This function inserts new playlist record into playlist table.
*/
int _audio_svc_insert_playlist_record(const char *playlist_name, int *playlist_id);

/**
* This function deletes playlist record from playlist table.
*/
int _audio_svc_delete_playlist_record(int playlist_id);

/**
* This function update playlist record with new name
*/
int _audio_svc_update_playlist_record_by_name(int playlist_id, const char *new_playlist_name);

/**
* This function returns the number of total playlist records in the playlist table.
*/
int _audio_svc_count_playlist_records(const char *filter_string, const char *filter_string2, int *count);

/**
* This function retrieves playlist record without any condition in the playlist table.
*/
int _audio_svc_get_playlist_records(int offset, int rows, const char *filter_string, const char *filter_string2, audio_svc_playlist_s *playlists);

int _audio_svc_count_playlist_item_records(int playlist_id, const char *filter_string, const char *filter_string2, int *count);
int _audio_svc_get_playlist_item_records(int playlist_id, const char *filter_string, const char *filter_string2,int offset, int rows, audio_svc_playlist_item_s *track);

/**
* This function returns the number of playlist with playlist_name.
* It is called when inserting a new playlist record to guarantee unique playlist name.
* */
int _audio_svc_count_playlist_records_by_name(const char *playlist_name, int * count);


/*****************************************************************/
/* playlist item table*/

/**
* This function inserts a new record with playlist index and audio_id into playlist item table.
*/
int _audio_svc_insert_playlist_item_record(int playlist_id, const char *audio_id);

/**
* This function removes item with audio_id from playlist.
* It is possible to add a same track to a playlist several times.
* So if user tries to remove one instance of track from playlist, this function should be called.
*/
int _audio_svc_delete_playlist_item_record_from_playlist_by_audio_id(int playlist_id, const char *audio_id);

/**
* When deleting a track, all instance of track will be removed from playlists
*/
int _audio_svc_delete_playlist_item_records_by_audio_id(const char *audio_id);
int _audio_svc_delete_playlist_item_record_from_playlist_by_uid(int playlist_id, int uid);
int _audio_svc_check_duplication_records_in_playlist(int playlist_id, const char *audio_id,  int * count);
int _audio_svc_get_playlist_id_by_name(const char *playlist_name, int *playlist_id);
int _audio_svc_get_playlist_name_by_playlist_id(int playlist_id, char *playlist_name);
int _audio_svc_get_audio_id_by_uid(int uid, char *audio_id);
int _audio_svc_update_item_play_order(int playlist_id, int uid, int new_play_order);

#endif /*_AUDIO_SVC_PLAYLIST_TABLE_H_*/