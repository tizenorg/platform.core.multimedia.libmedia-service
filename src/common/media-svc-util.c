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

#include "uuid.h"
#include <stdlib.h>

char *_media_info_generate_uuid(void)
{
	uuid_t uuid_value;
	static char uuid_unparsed[50];	

	uuid_generate(uuid_value);
	uuid_unparse(uuid_value, uuid_unparsed);

	//mediainfo_dbg("UUID : %s", uuid_unparsed);
	return uuid_unparsed;
}

int _media_svc_check_escape_char(char ch)
{
	int i;
	char escape_char[3] = {'%', '_' ,'#'};

	for (i = 0; i < 3; i++) {
		if (ch == escape_char[i]) {
			return 1;
		}
	}

	return 0;
}

char *_media_svc_escape_str(char *input, int len)
{
	int i = 0;
	int j = 0;
	char *result = NULL;
	
	result = (char*)malloc(len * 2 * sizeof(char) + 1);
	if (result == NULL) {
		return NULL;
	}

	for (i = 0; i < len; i++, j++) {
		if (input[i] == '\0') break;

		if (_media_svc_check_escape_char(input[i])) {
			result[j] = '#';
			result[++j] = input[i];
		} else {
			result[j] = input[i];
		}
	}

	result[j] = '\0';
	
	return result;
}

