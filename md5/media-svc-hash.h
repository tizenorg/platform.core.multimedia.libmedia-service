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

#ifndef _MEDIA_SVC_HASH_
#define _MEDIA_SVC_HASH_

#ifndef LIBMEDIA_SVC_EXPORT_API
#define LIBMEDIA_SVC_EXPORT_API
#endif // LIBMEDIA_SVC_EXPORT_API


LIBMEDIA_SVC_EXPORT_API int mb_svc_generate_hash_code(const char *origin_path, char *hash_code, int max_length);

#endif /*MEDIA_SVC_HASH_*/
