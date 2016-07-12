/*
 * Contacts Service
 *
 * Copyright (c) 2010 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
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

#ifndef __MEDIA_SERVICE_LOCALIZE_TW_H__
#define __MEDIA_SERVICE_LOCALIZE_TW_H__

#define MAX_BPMF_NAME_LENGTH 30
typedef struct {
	char *bpmf_initial;
	char *bpmf_name;
} media_svc_bpmf_name_s;

int _media_svc_get_bopomofo(const char *src, char **dest);
int _media_svc_convert_chinese_to_bpmf(const char *src, media_svc_bpmf_name_s **dest);
void _media_svc_bpmf_name_destroy(media_svc_bpmf_name_s *bpmf);
bool _media_svc_bpmf_is_bpmf(const char *src);
char _media_svc_bpmf_get_fuzzy_number(const char *src);

#endif		/*__MEDIA_SERVICE_LOCALIZE_TW_H__ */