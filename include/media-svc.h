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
#include <media-util-noti.h>
#include <time.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif


int media_svc_connect(MediaSvcHandle **handle, uid_t uid, bool need_write);
int media_svc_disconnect(MediaSvcHandle *handle);
int media_svc_create_table(MediaSvcHandle *handle, uid_t uid);
int media_svc_check_item_exist_by_path(MediaSvcHandle *handle, const char *path);
int media_svc_insert_folder(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, const char *path, uid_t uid);
int media_svc_insert_item_begin(MediaSvcHandle *handle, int with_noti, int data_cnt, int from_pid);
int media_svc_insert_item_end(MediaSvcHandle *handle, uid_t uid);
int media_svc_insert_item_bulk(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, const char *path, int is_burst, uid_t uid);
int media_svc_insert_item_immediately(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, const char *path, uid_t uid);
int media_svc_move_item_begin(MediaSvcHandle *handle, int data_cnt);
int media_svc_move_item_end(MediaSvcHandle *handle, uid_t uid);
int media_svc_move_item(MediaSvcHandle *handle, media_svc_storage_type_e src_storage, const char *src_path, media_svc_storage_type_e dest_storage, const char *dest_path, uid_t uid);
int media_svc_set_item_validity_begin(MediaSvcHandle *handle, int data_cnt);
int media_svc_set_item_validity_end(MediaSvcHandle *handle, uid_t uid);
int media_svc_set_item_validity(MediaSvcHandle *handle, const char *path, int validity, uid_t uid);
int media_svc_delete_item_by_path(MediaSvcHandle *handle, const char *storage_id, const char *path, uid_t uid);
int media_svc_delete_all_items_in_storage(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, uid_t uid);
int media_svc_delete_invalid_items_in_storage(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, uid_t uid);
int media_svc_delete_invalid_items_in_folder(MediaSvcHandle *handle, const char *folder_path, uid_t uid);
int media_svc_set_all_storage_items_validity(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, int validity, uid_t uid);
int media_svc_set_folder_items_validity(MediaSvcHandle *handle, const char *folder_path, int validity, int recursive, uid_t uid);
int media_svc_refresh_item(MediaSvcHandle *handle, media_svc_storage_type_e storage_type, const char *storage_id, const char *path, uid_t uid);
int media_svc_rename_folder(MediaSvcHandle *handle, const char *storage_id, const char *src_path, const char *dst_path, uid_t uid);
int media_svc_request_update_db(const char *db_query, uid_t uid);
int media_svc_get_storage_type(const char *path, media_svc_storage_type_e *storage_type, uid_t uid);
int media_svc_get_file_info(MediaSvcHandle *handle, const char *path, time_t *modified_time, unsigned long long *size);
int media_svc_send_dir_update_noti(MediaSvcHandle *handle, const char *dir_path);
int media_svc_count_invalid_items_in_folder(MediaSvcHandle *handle, const char *folder_path, int *count);
int media_svc_check_db_upgrade(MediaSvcHandle *handle, uid_t uid);
int media_svc_check_db_corrupt(MediaSvcHandle *handle);
int media_svc_get_folder_list(MediaSvcHandle *handle, char *start_path, char ***folder_list, time_t **modified_time_list, int **item_num_list, int *count);
int media_svc_update_folder_time(MediaSvcHandle *handle, const char *folder_path, uid_t uid);
int media_svc_publish_noti(MediaSvcHandle *handle, media_item_type_e update_item, media_item_update_type_e update_type, const char *path, media_type_e media_type, const char *uuid, const char *mime_type);
int media_svc_get_pinyin(MediaSvcHandle *handle, const char *src_str, char **pinyin_str);
int media_svc_check_pinyin_support(bool *support);
int media_svc_update_item_begin(MediaSvcHandle *handle, int data_cnt);
int media_svc_update_item_end(MediaSvcHandle *handle, uid_t uid);
int media_svc_update_item_meta(MediaSvcHandle *handle, const char *file_path, int storage_type, uid_t uid);
int media_svc_insert_item_immediately_with_data(MediaSvcHandle *handle, media_svc_content_info_s *content_info, uid_t uid);
void media_svc_destroy_content_info(media_svc_content_info_s *content_info);
char *media_info_generate_uuid(void);
int media_svc_insert_storage(MediaSvcHandle *handle, const char *storage_id, const char *storage_name, const char *storage_path, const char *storage_account, media_svc_storage_type_e storage_type, uid_t uid);
int media_svc_delete_storage(MediaSvcHandle *handle, const char *storage_id, uid_t uid);
int media_svc_request_extract_all_thumbs(uid_t uid);




#ifdef __cplusplus
}
#endif

#endif /*_MEDIA_SVC_H_*/
