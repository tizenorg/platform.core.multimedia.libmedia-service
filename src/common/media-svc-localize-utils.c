/*
 * Media Service
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "media-util-err.h"
#include "media-svc-debug.h"
#include "media-svc-localize-utils.h"

#define SAFE_STRLEN(src) ((src)?strlen(src):0)

int _media_svc_check_utf8(char c)
{
	if ((c & 0xff) < (128 & 0xff))
		return 1;
	else if ((c & (char)0xe0) == (char)0xc0)
		return 2;
	else if ((c & (char)0xf0) == (char)0xe0)
		return 3;
	else if ((c & (char)0xf8) == (char)0xf0)
		return 4;
	else if ((c & (char)0xfc) == (char)0xf8)
		return 5;
	else if ((c & (char)0xfe) == (char)0xfc)
		return 6;
	else
		return MS_MEDIA_ERR_INVALID_PARAMETER;
}

int SAFE_SNPRINTF(char **buf, int *buf_size, int len, const char *src)
{
	int remain;
	int temp_len;

	if (len < 0)
		return -1;

	remain = *buf_size - len;
	if (remain > strlen(src) + 1) {
		temp_len = snprintf((*buf)+len, remain, "%s", src);
		return temp_len;
	}
	else {
		char *temp;
		while(1) {
			temp = realloc(*buf, *buf_size*2);
			if (NULL == temp)
				return -1;
			*buf = temp;
			*buf_size = *buf_size * 2;
			remain = *buf_size - len;
			if (remain > strlen(src) + 1)
				break;
		}
		temp_len = snprintf((*buf)+len, remain, "%s", src);
		return temp_len;
	}
}
