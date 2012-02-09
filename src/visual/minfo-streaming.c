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

#include "minfo-streaming.h"
#include "media-svc-api.h"
#include "visual-svc-util.h"

static void _minfo_mstreaming_init(Mstreaming *mstreaming);

int minfo_mstreaming_load(Mstreaming *mstreaming)
{
	return 0;
}

Mstreaming *minfo_mstreaming_new(int id)
{
	Mstreaming *mstreaming = NULL;
	int ret = 0;

	mstreaming = (Mstreaming *) malloc(sizeof(Mstreaming));
	if (mstreaming == NULL) {
		return NULL;
	}

	_minfo_mstreaming_init(mstreaming);
	if (id != -1) {
		mstreaming->_id = id;
		ret = minfo_mstreaming_load(mstreaming);
		if (ret < 0) {
			free(mstreaming);
			return NULL;
		}
	}

	return mstreaming;
}

void minfo_mstreaming_destroy(Mstreaming *mstreaming)
{
	if (mstreaming != NULL) {
		/* todo free resource */
		free(mstreaming);
	}
}

static void _minfo_mstreaming_init(Mstreaming *mstreaming)
{
	mstreaming->gtype = MINFO_TYPE_MSTREAMING;

	mstreaming->_id = -1;
	mstreaming->cluster_id = 0;
	mstreaming->http_url = NULL;
	mstreaming->thumb_url = NULL;
	mstreaming->duration = 0;
	mstreaming->title = NULL;
	mstreaming->description = NULL;
}

