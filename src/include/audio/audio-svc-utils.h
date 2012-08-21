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



#ifndef _AUDIO_SVC_UTILS_H_
#define _AUDIO_SVC_UTILS_H_


/**
 * @file       	audio-svc-utils.h
 * @version 	0.1
 * @brief     	This file defines utilities for Audio Service.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "audio-svc-types.h"
#include "audio-svc-types-priv.h"


#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#if !defined( min )
    #define min(a, b) ((a)<(b)?(a):(b))
#endif

#define str_2_year(year)	((!strcmp(year,  AUDIO_SVC_TAG_UNKNOWN) ) ? -1 : atoi(year))
#define STRING_VALID(str)	\
	((str != NULL && strlen(str) > 0) ? TRUE : FALSE)

char * _year_2_str(int year);
//void _strncpy_safe(char *x_dst, const char *x_src, int max_len);
void _strlcat_safe(char *x_dst, char *x_src, int max_len);
void _audio_svc_get_parental_rating(const char *path, char *parental_rating);
int _audio_svc_extract_metadata_audio(audio_svc_storage_type_e storage_type, const char *path, audio_svc_audio_item_s *item);
int _audio_svc_remove_all_files_in_dir(const char *dir_path);
int _audio_svc_get_drm_mime_type(const char *path, char *mime_type);
bool _audio_svc_possible_to_extract_title_from_file(const char *path);
bool _audio_svc_get_thumbnail_path(audio_svc_storage_type_e storage_type, char *thumb_path, const char *pathname, const char *img_format);
bool _audio_svc_copy_file(const char* srcPath, const char* destPath);
bool _audio_svc_make_file(const char* path);
bool _audio_svc_remove_file(const char* path);
bool _audio_svc_make_directory(const char* path);
char * _audio_svc_get_title_from_filepath (const char *path);
unsigned int _audio_svc_print_elapse_time(int start_time, const char* log_msg);
int _audio_svc_get_order_field_str(audio_svc_search_order_e order_field, char* output_str, int len);
int _audio_svc_get_file_dir_modified_date(const char *full_path);

#endif /*_AUDIO_SVC_UTILS_H_*/
