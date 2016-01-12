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
#include "media-svc-noti.h"

int _media_svc_count_record_with_path(sqlite3 *handle, const char *storage_id, const char *path, int *count);
int _media_svc_insert_item_with_data(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid);
int _media_svc_update_item_with_data(const char *storage_id, media_svc_content_info_s *content_info, uid_t uid);
int _media_svc_get_thumbnail_path_by_path(sqlite3 *handle, const char *storage_id, const char *path, char *thumbnail_path);
int _media_svc_get_media_type_by_path(sqlite3 *handle, const char *storage_id, const char *path, int *media_type);
int _media_svc_get_burst_id(sqlite3 *handle, const char *storage_id, int *id);
int _media_svc_delete_item_by_path(const char *storage_id, const char *path, bool stack_query, uid_t uid);
int _media_svc_truncate_table(const char *storage_id, media_svc_storage_type_e storage_type, uid_t uid);
int _media_svc_delete_invalid_items(sqlite3 *handle, const char *storage_id, media_svc_storage_type_e storage_type, uid_t uid);
int _media_svc_delete_invalid_folder_items(sqlite3 *handle, const char *storage_id, const char *folder_path, bool is_recursive, uid_t uid);
int _media_svc_update_storage_item_validity(const char *storage_id, media_svc_storage_type_e storage_type, int validity, uid_t uid);
int _media_svc_update_folder_item_validity(sqlite3 *handle, const char *storage_id, const char *folder_path, int validity, uid_t uid);
int _media_svc_update_recursive_folder_item_validity(const char *storage_id, const char *folder_path, int validity, uid_t uid);
int _media_svc_update_item_validity(const char *storage_id, const char *path, int validity, bool stack_query, uid_t uid);
int _media_svc_update_item_by_path(const char *storage_id, const char *src_path, media_svc_storage_type_e dest_storage, const char *dest_path, const char *file_name, int modified_time, const char *folder_uuid, const char *thumb_path, bool stack_query, uid_t uid);
int _media_svc_list_query_do(media_svc_query_type_e query_type, uid_t uid);
int _media_svc_update_thumbnail_path(const char *storage_id, const char *path, const char *thumb_path, uid_t uid);
int _media_svc_get_noti_info(sqlite3 *handle, const char *storage_id, const char *path, int update_item, media_svc_noti_item **item);
int _media_svc_count_invalid_folder_items(sqlite3 *handle, const char *storage_id, const char *folder_path, int *count);
int _media_svc_get_thumbnail_count(sqlite3 *handle, const char *storage_id, const char *thumb_path, int *count);
char *_media_svc_get_thumb_default_path(uid_t uid);
int _media_svc_get_fileinfo_by_path(sqlite3 *handle, const char *storage_id, const char *path, time_t *modified_time, unsigned long long *size);
int _media_svc_update_meta_with_data(media_svc_content_info_s *content_info);

int _media_svc_change_validity_item_batch(const char *storage_id, const char *path, int des_validity, int src_validity, uid_t uid);
int _media_svc_insert_item_pass1(sqlite3 *handle, const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid);
int _media_svc_insert_item_pass2(const char *storage_id, media_svc_content_info_s *content_info, int is_burst, bool stack_query, uid_t uid);

int _media_svc_append_query_list(const char *query, uid_t uid);

#endif /*_MEDIA_SVC_MEDIA_H_*/
