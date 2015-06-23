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

#ifndef _MEDIA_SVC_STORAGE_H_
#define _MEDIA_SVC_STORAGE_H_

#include <sqlite3.h>

int _media_svc_init_storage(sqlite3 *handle, uid_t uid);
int _media_svc_append_storage(sqlite3 *handle, const char *storage_id, const char *storage_name, const char *storage_path, const char *storage_account, media_svc_storage_type_e storage_type, uid_t uid);
int _media_svc_delete_storage(sqlite3 *handle, const char *storage_id, uid_t uid);
int _media_svc_update_storage_validity(sqlite3 *handle, const char *storage_id, int validity, uid_t uid);

#endif /*_MEDIA_SVC_STORAGE_H_*/
