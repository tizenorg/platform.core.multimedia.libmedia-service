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

#include "minfo-tag.h"
#include "minfo-meta.h"
#include "media-svc-api.h"
#include "visual-svc-util.h"
#include "visual-svc-debug.h"
#include "visual-svc-error.h"
#include <string.h>

static bool _minfo_mtag_load(MediaSvcHandle *mb_svc_handle, Mtag *mtag, mb_svc_tag_record_s *p_tag_record)
{
	mb_svc_tag_record_s mtag_record = { 0 };
	int ret = 0;

	if (p_tag_record == NULL) {
		ret = mb_svc_get_media_tag_by_id(mb_svc_handle, mtag->_id, &mtag_record);
	} else {
		mtag_record = *p_tag_record;
	}

	if (ret < 0) {
		return ret;
	}

	mtag->tag_name = (char *)malloc(MB_SVC_ARRAY_LEN_MAX + 1);
	if (mtag->tag_name == NULL) {
		return MB_SVC_ERROR_OUT_OF_MEMORY;
	}
	strncpy(mtag->tag_name, mtag_record.tag_name, MB_SVC_ARRAY_LEN_MAX + 1);

	return 0;
}

Mtag *minfo_media_tag_new(MediaSvcHandle *mb_svc_handle, int id, mb_svc_tag_record_s * p_tag_record)
{
	Mtag *mtag = NULL;
	int ret = 0;

	mtag = (Mtag *) malloc(sizeof(Mtag));
	if (mtag == NULL) {
		return NULL;
	}

	mtag->gtype = MINFO_TYPE_MTAG;
	mtag->tag_name = NULL;

	if (p_tag_record) {
		//mtag->media_id = p_tag_record->media_id;
		mtag->_id = p_tag_record->_id;
		ret = _minfo_mtag_load(mb_svc_handle, mtag, p_tag_record);
	} else if (id != -1) {
		mtag->_id = id;
		ret = _minfo_mtag_load(mb_svc_handle, mtag, NULL);
		if (ret < 0) {
			minfo_media_tag_destroy(mtag);
			return NULL;
		}
	}

	return mtag;
}

void minfo_media_tag_destroy(Mtag *i_tag)
{
	if (i_tag != NULL && IS_MINFO_MTAG(i_tag)) {
		mb_svc_debug("do free resource %p\n", i_tag);
		if (i_tag->tag_name) {
			free(i_tag->tag_name);
		}

		i_tag->gtype = 0;
		free(i_tag);

		i_tag = NULL;
	}
}

