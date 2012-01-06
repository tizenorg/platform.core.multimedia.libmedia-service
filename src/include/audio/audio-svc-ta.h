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

#ifndef _AUDIO_SVC_TA_H_
#define _AUDIO_SVC_TA_H_


#define ENABLE_AUDIO_SVC_TA
#ifdef ENABLE_AUDIO_SVC_TA

// defs.
#define AUDIO_SVC_TA_MAX_CHECKPOINT	500
#define AUDIO_SVC_TA_MAX_ACCUM		500

typedef struct _audio_svc_ta_checkpoint
{
	unsigned long timestamp;
	char* name;
} audio_svc_ta_checkpoint;

typedef struct _audio_svc_ta_accum_item
{
	unsigned long elapsed_accum;
	unsigned long num_calls;
	unsigned long elapsed_min;
	unsigned long elapsed_max;
	unsigned long first_start;
	unsigned long last_end;

	char* name;

	unsigned long timestamp;
	int on_estimate;
	int num_unpair;
} audio_svc_ta_accum_item;

#define AUDIO_SVC_TA_SHOW_STDOUT	0
#define AUDIO_SVC_TA_SHOW_STDERR	1
#define AUDIO_SVC_TA_SHOW_FILE	2
#define AUDIO_SVC_TA_RESULT_FILE "/tmp/audio-svc-ta.log"

/////////////////////////////
// COMMON
int audio_svc_ta_init(void);
int audio_svc_ta_release(void);
void audio_svc_ta_set_enable(int enable);
char* audio_svc_ta_fmt(const char* fmt, ...);

/////////////////////////////
// CHECK POINT
int audio_svc_ta_add_checkpoint(char* name, int show, char* filename, int line);
void audio_svc_ta_show_checkpoints(void);
void audio_svc_ta_show_diff(char* name1, char* name2);

int audio_svc_ta_get_numof_checkpoints();
unsigned long audio_svc_ta_get_diff(char* name1, char* name2);
//char* audio_svc_ta_get_name(int idx);


/////////////////////////////
// ACCUM ITEM
int audio_svc_ta_accum_item_begin(char* name, int show, char* filename, int line);
int audio_svc_ta_accum_item_end(char* name, int show, char* filename, int line);
void audio_svc_ta_accum_show_result(int direction);

// macro.
#define AUDIO_SVC_TA_INIT()							(	audio_svc_ta_init()										)
#define AUDIO_SVC_TA_RELEASE()						(	audio_svc_ta_release()									)
#define AUDIO_SVC_TA_SET_ENABLE(enable)				(	audio_svc_ta_set_enable(enable)							)

// checkpoint handling
#define AUDIO_SVC_TA_ADD_CHECKPOINT(name,show)		(	audio_svc_ta_add_checkpoint(name,show,__FILE__,__LINE__)		)
#define AUDIO_SVC_TA_SHOW_CHECKPOINTS()				(	audio_svc_ta_show_checkpoints()							)
#define AUDIO_SVC_TA_SHOW_DIFF(name1, name2)			(	audio_svc_ta_show_diff(name1, name2)						)
#define AUDIO_SVC_TA_GET_NUMOF_CHECKPOINTS()		(	audio_svc_ta_get_numof_checkpoints()						)
#define AUDIO_SVC_TA_GET_DIFF(name1, name2)			(	audio_svc_ta_get_diff(name1, name2)						)
//#define AUDIO_SVC_TA_GET_NAME(idx)				(	audio_svc_ta_get_name(idx)							)

// accum item handling
#define AUDIO_SVC_TA_ACUM_ITEM_BEGIN(name,show)		(	audio_svc_ta_accum_item_begin(name,show,__FILE__,__LINE__)		)
#define AUDIO_SVC_TA_ACUM_ITEM_END(name,show)		(	audio_svc_ta_accum_item_end(name,show,__FILE__,__LINE__)		)
#define AUDIO_SVC_TA_ACUM_ITEM_SHOW_RESULT()		(	audio_svc_ta_accum_show_result(AUDIO_SVC_TA_SHOW_STDOUT)	)
#define AUDIO_SVC_TA_ACUM_ITEM_SHOW_RESULT_TO(x)	(	audio_svc_ta_accum_show_result(x)							)
/*
#define __audio_svc_ta__(name, x) \
AUDIO_SVC_TA_ACUM_ITEM_BEGIN(name, 0); \
x \
AUDIO_SVC_TA_ACUM_ITEM_END(name, 0);

#define __mm_tafmt__(fmt, args...) 			(	audio_svc_ta_audio_svc_ta_fmt(fmt, ##args)	)
*/

#else //#ifdef ENABLE_AUDIO_SVC_TA

#define AUDIO_SVC_TA_INIT()
#define AUDIO_SVC_TA_RELEASE()
#define AUDIO_SVC_TA_SET_ENABLE(enable)

// checkpoint handling
#define AUDIO_SVC_TA_ADD_CHECKPOINT(name,show)
#define AUDIO_SVC_TA_SHOW_CHECKPOINTS()
#define AUDIO_SVC_TA_SHOW_DIFF(name1, name2)
#define AUDIO_SVC_TA_GET_NUMOF_CHECKPOINTS()
#define AUDIO_SVC_TA_GET_DIFF(name1, name2)
//#define AUDIO_SVC_TA_GET_NAME(idx)

// accum item handling
#define AUDIO_SVC_TA_ACUM_ITEM_BEGIN(name,show)
#define AUDIO_SVC_TA_ACUM_ITEM_END(name,show)
#define AUDIO_SVC_TA_ACUM_ITEM_SHOW_RESULT()
#define AUDIO_SVC_TA_ACUM_ITEM_SHOW_RESULT_TO(x)
/*
#define __audio_svc_ta__(name, x)
#define __mm_tafmt__(fmt, args...)
*/
#endif //#ifdef ENABLE_AUDIO_SVC_TA

#endif /*_AUDIO_SVC_TA_H_*/

