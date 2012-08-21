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
#include "media-img-codec-osal.h"
#include "visual-svc-debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <drm_client.h>

void *IfegMemAlloc(unsigned int size)
{
	void *pmem;
	pmem = malloc(size);
	return pmem;
}

void IfegMemFree(void *pMem)
{
	free(pMem);
	pMem = 0;
}

void *IfegMemcpy(void *dest, const void *src, unsigned int count)
{
	return memcpy(dest, src, count);
}

void *IfegMemset(void *dest, int c, unsigned int count)
{
	return memset(dest, c, count);
}

ULONG IfegGetAvailableMemSize(void)
{
	return 1;
}

int IfegMemcmp(const void *pMem1, const void *pMem2, size_t length)
{
	return memcmp(pMem1, pMem2, length);
}

HFile DrmOpenFile(const char *szPathName)
{
	return NULL;
}

BOOL DrmReadFile(HFile hFile, void *pBuffer, ULONG bufLen, ULONG * pReadLen)
{
	return TRUE;
}

long DrmTellFile(HFile hFile)
{
	return 0;
}

BOOL DrmSeekFile(HFile hFile, long position, long offset)
{
	return TRUE;
}

BOOL DrmGetFileAttributes(const char *szPathName, FmFileAttribute * pFileAttr)
{
	FILE *f = NULL;

	f = fopen(szPathName, "r");

	if (f == NULL) {
		return FALSE;
	}

	fseek(f, 0, SEEK_END);
	pFileAttr->fileSize = ftell(f);
	fclose(f);

	return TRUE;
}

BOOL DrmCloseFile(HFile hFile)
{
	return TRUE;
}
