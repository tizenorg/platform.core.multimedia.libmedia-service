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



#ifndef _AUDIO_SVC_DEBUG_H_
#define _AUDIO_SVC_DEBUG_H_

#ifndef DBG_PREFIX
#define DBG_PREFIX		"AUDIO_SVC"
#endif


#include <stdio.h>
#include <stdbool.h>
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "AUDIO_SVC"

#define audio_svc_debug(fmt, arg...) do{ if (TRUE) { \
				LOGD("[%s : %s-%d]\n"fmt"",__FILE__, __FUNCTION__, __LINE__,##arg);     \
			} } while (0)

#define audio_svc_error(fmt, arg...) do{ if (TRUE) { \
				LOGE("[%s : %s-%d]\n"fmt"",__FILE__, __FUNCTION__, __LINE__,##arg);     \
			} } while (0)

#define audio_svc_debug_func() do{ if (TRUE) { \
				LOGD("[%s : %s-%d]\n",__FILE__, __FUNCTION__, __LINE__);     \
			} } while (0)

#define audio_svc_debug_time() do{ if (TRUE) { \
				LOGD("[%s]\n",__TIME__);     \
			} } while (0)

#define audio_svc_ret_if(expr) do { \
		if(expr) { \
			return; \
		} \
	} while (0)
#define audio_svc_retv_if(expr, val) do { \
		if(expr) { \
			return (val); \
		} \
	} while (0)

#define audio_svc_retm_if(expr, fmt, arg...) do { \
		if(expr) { \
			LOGE("[%s : %s-%d]\n"fmt"",__FILE__, __FUNCTION__, __LINE__,##arg);     \
			return; \
		} \
	} while (0)

#define audio_svc_retvm_if(expr, val, fmt, arg...) do { \
		if(expr) { \
			LOGE("[%s : %s-%d]\n"fmt"",__FILE__, __FUNCTION__, __LINE__,##arg);     \
			return (val); \
		} \
	} while (0)


#endif /*_AUDIO_SVC_DEBUG_H_*/
