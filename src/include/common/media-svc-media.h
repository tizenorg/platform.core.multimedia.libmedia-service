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

#ifndef _MEDIA_SVC_MEDIA_H_
#define _MEDIA_SVC_MEDIA_H_

#include <sqlite3.h>
#include <stdbool.h>
#include "media-svc-types.h"
#include "media-svc-env.h"

int _media_svc_count_record_with_path(sqlite3 *handle, const char *path, int *count);
int _media_svc_insert_item_with_data(sqlite3 *handle, media_svc_content_info_s *content_info, bool stack_query);
int _media_svc_update_item_with_data(sqlite3 *handle, media_svc_content_info_s *content_info);
int _media_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *path, char *thumbnail_path);
int _media_svc_get_media_type_by_path(sqlite3 *handle, const char *path, int *media_type);
int _media_svc_delete_item_by_path(sqlite3 *handle, const char *path);
int _media_svc_truncate_table(sqlite3 *handle, media_svc_storage_type_e storage_type);
int _media_svc_delete_invalid_items(sqlite3 *handle, media_svc_storage_type_e storage_type);
int _media_svc_update_valid_of_music_records(sqlite3 *handle, media_svc_storage_type_e storage_type, int validity);
int _media_svc_update_item_validity(sqlite3 *handle, const char *path, int validity, bool stack_query);
int _media_svc_update_item_by_path(sqlite3 *handle, const char *src_path, media_svc_storage_type_e dest_storage, const char *dest_path, const char *file_name, int modified_time, const char *folder_uuid, const char *thumb_path, bool stack_query);
int _media_svc_list_query_do(sqlite3 *handle, media_svc_query_type_e query_type);
int _media_svc_get_media_id_by_path(sqlite3 *handle, const char *path, char *media_uuid, int max_length);
int _media_svc_update_thumbnail_path(sqlite3 *handle, const char *path, const char *thumb_path);

#endif /*_MEDIA_SVC_MEDIA_H_*/
