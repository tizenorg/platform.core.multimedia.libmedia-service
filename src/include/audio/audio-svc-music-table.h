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



#ifndef _AUDIO_SVC_MUSIC_TABLE_H_
#define _AUDIO_SVC_MUSIC_TABLE_H_

/**
 * This file defines structure and functions related to database.
 *
 * @file       	audio-svc-music-table.h
 * @version 	0.1
 * @brief     	This file defines the functions related to DB.
 */

#include <stdbool.h>
#include <sqlite3.h>
#include "audio-svc-types.h"
#include "audio-svc-types-priv.h"

typedef struct{
	char thumbnail_path[AUDIO_SVC_PATHNAME_SIZE];
}mp_thumbnailpath_record_t;

int _audio_svc_create_music_table(sqlite3 *handle);
int _audio_svc_truncate_music_table(sqlite3 *handle, audio_svc_storage_type_e storage_type);
int _audio_svc_create_folder_table(sqlite3 *handle);
int _audio_svc_delete_folder(sqlite3 *handle, audio_svc_storage_type_e storage_type, const char *folder_id);
int _audio_svc_select_music_record_by_audio_id(sqlite3 *handle, const char *audio_id, audio_svc_audio_item_s *item);
int _audio_svc_select_music_record_by_path(sqlite3 *handle, const char *path, audio_svc_audio_item_s *item);
int _audio_svc_delete_music_record_by_audio_id(sqlite3 *handle, const char *audio_id);
int _audio_svc_update_metadata_in_music_record(sqlite3 *handle, const char *audio_id, audio_svc_audio_item_s *item);
int _audio_svc_update_path_in_music_record(sqlite3 *handle, const char *src_path, const char *path, const char *title);
int _audio_svc_update_path_and_storage_in_music_record(sqlite3 *handle, const char *src_path, const char *path, audio_svc_storage_type_e storage_type);
int _audio_svc_update_folder_id_in_music_record(sqlite3 *handle, const char *path, const char *folder_id);
int _audio_svc_update_thumb_path_in_music_record(sqlite3 *handle, const char *file_path, const char *path);
int _audio_svc_update_rating_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_playtime_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_playcount_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_addtime_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_track_num_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_album_rating_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_year_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_update_title_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_artist_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_album_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_genre_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_author_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_description_in_music_record(sqlite3 *handle, const char *audio_id, const char * changed_value);
int _audio_svc_update_favourite_in_music_record(sqlite3 *handle, const char *audio_id, int changed_value);
int _audio_svc_count_music_group_records(sqlite3 *handle, audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2, int *count);
int _audio_svc_get_music_group_records(sqlite3 *handle, audio_svc_group_type_e group_type, const char *limit_string1, const char *limit_string2, const char *filter_string, const char *filter_string2,int offset, int rows, audio_svc_group_item_s *group);
int _audio_svc_count_music_track_records(sqlite3 *handle, audio_svc_track_type_e track_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int *count);
int _audio_svc_get_music_track_records(sqlite3 *handle, audio_svc_track_type_e track_type, const char *type_string, const char *type_string2, const char *filter_string, const char *filter_string2, int offset, int rows, audio_svc_list_item_s *track);
int _audio_svc_search_audio_id_by_path(sqlite3 *handle, const char *path, char *audio_id);
int _audio_svc_update_valid_of_music_records(sqlite3 *handle, audio_svc_storage_type_e storage_type, int valid);
int _audio_svc_count_record_with_path(sqlite3 *handle, const char *path);
int _audio_svc_delete_invalid_music_records(sqlite3 *handle, audio_svc_storage_type_e storage_type);
int _audio_svc_update_valid_in_music_record(sqlite3 *handle, const char *path, int valid);
int _audio_svc_update_valid_in_music_record_query_add(sqlite3 *handle, const char *path, int valid);
int _audio_svc_move_item_query_add(sqlite3 *handle, const char *src_path, const char *path, audio_svc_storage_type_e storage_type, const char *folder_id);
int _audio_svc_list_query_do(sqlite3 *handle, audio_svc_query_type_e query_type);
int _audio_svc_get_path(sqlite3 *handle, const char *audio_id, char *path);
int _audio_svc_delete_music_track_groups(sqlite3 *handle, audio_svc_group_type_e group_type, const char *type_string);
int _audio_svc_check_and_update_folder_table(sqlite3 *handle, const char* path_name);
int _audio_svc_check_and_update_albums_table(sqlite3 *handle, const char *album);
int _audio_svc_insert_item_with_data(sqlite3 *handle, audio_svc_audio_item_s *item, bool stack_query);
int _audio_svc_update_folder_table(sqlite3 *handle);
int _audio_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *path, char *thumb_path);
int _audio_svc_get_and_append_folder_id_by_path(sqlite3 *handle, const char *path, audio_svc_storage_type_e storage_type, char *folder_id);
int _audio_svc_get_folder_id_by_foldername(sqlite3 *handle, const char *folder_name, char *folder_id);
int _audio_svc_append_audio_folder(sqlite3 *handle, audio_svc_storage_type_e storage_type, const char *folder_id, const char *path_name, const char *folder_name, int modified_date);
char * _audio_svc_get_thumbnail_path_by_album_id(sqlite3 *handle, int album_id);
int _audio_svc_check_and_remove_thumbnail(sqlite3 *handle, const char * thumbnail_path);
int _audio_svc_list_search(sqlite3 *handle, audio_svc_audio_item_s *item,
							char *where_query,
							audio_svc_search_order_e order_field,
							int offset,
							int count
							);

#endif /*_AUDIO_SVC_MUSIC_TABLE_H_*/
