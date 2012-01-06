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



#ifndef __MD5_HASH_H__
#define __MD5_HASH_H__
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Generate '\0'-terminated hashcode string of 'file'.
 *         The hashcode consists of 32-byte [0-9a-f] chars.
 *         You should not free returned result, it's internally
 *         static allocated. You may need to make a copy of result
 *         because the buffer will be modified by subsequent calling of
 *         this function.
 * @param file  Full pathname, or generally char string.
 * @return  Generated hashcode string,
 *          or NULL if parameter 'file' is NULL.
 */
char* _audio_svc_generate_hash(const char* file);

/**
 * @brief  Generate corresponding thumbnail file's full pathname.
 *         You should not free returned result, it's internally
 *         static allocated. You may need to make a copy of result
 *         because the buffer will be modified by subsequent calling of
 *         this function.
 * @param full_pathname  Origin file full pathname.
 * @return  Generated thumbnail file's full pathname, or
 *          NULL if full_pathname is NULL or full_pathname is not valid
 *          for file manager service.
 */



#ifdef __cplusplus
}
#endif

#endif /*__MD5_HASH_H__*/
