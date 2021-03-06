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

#include "visual-svc-debug.h"
#include "media-img-codec.h"
#include <string.h>

unsigned int *ImgGetFirstFrameAGIFAtSize(const char *szFileName,
					 ImgImageInfo *image_info)
{
	AGifFrameInfo *pFrameInfo = 0;
	void *pDecodedRGB888Buf = 0;
	unsigned char *raw_data = NULL;

	if (szFileName == NULL) {
		mb_svc_debug
		    ("ImgGetFirstFrameAGIFAtSize: Input File Name is NULL");
		return NULL;
	}

	if (image_info == NULL) {
		mb_svc_debug
		    ("ImgGetFirstFrameAGIFAtSize: Input ImgImageInfo is NULL");
		return NULL;
	}

	pFrameInfo =
	    ImgCreateAGIFFrame(szFileName, image_info->width,
			       image_info->height, 0, FALSE);

	if (pFrameInfo && pFrameInfo->pOutBits) {
		ImgGetNextAGIFFrame(pFrameInfo, TRUE);

		if (ImgConvertRGB565ToRGB888
		    (pFrameInfo->pOutBits, &pDecodedRGB888Buf,
		     pFrameInfo->width, pFrameInfo->height)) {
			if (pDecodedRGB888Buf) {
				free(pFrameInfo->pOutBits);
				pFrameInfo->pOutBits = pDecodedRGB888Buf;
				unsigned char *src =
				    ((unsigned char *)(pFrameInfo->pOutBits));
				int i =
				    image_info->width * image_info->height * 3;
				raw_data = (unsigned char *)malloc(i);
				memset(raw_data, 0, i);
				unsigned char *dest = raw_data;
				while (i--) {
					if (dest != NULL) {
						*dest =
						    *((unsigned char *)(src));
						dest++;
						src++;
					}
				}
			}
		}
	} else {
		mb_svc_debug("ImgDecodeAGIFToPixbufFromFile :: Error");
	}

	if (pFrameInfo) {
		ImgDestroyAGIFFrame(pFrameInfo);
	}

	return (unsigned int *)raw_data;
}

int ImgConvertRGB565ToRGB888(void *pBuf_rgb565, void **pBuf_rgb888, int width,
			     int height)
{
	unsigned short *rgb565buf = 0;
	unsigned char *rgb888Buf = 0;
	unsigned char red, green, blue;
	int i;

	rgb565buf = (unsigned short *)pBuf_rgb565;
	if (rgb565buf == NULL) {
		mb_svc_debug("rgb565buf is NULL: Error !!!");
		return FALSE;
	}

	rgb888Buf = (unsigned char *)malloc(width * height * 3);

	if (rgb888Buf == NULL) {
		mb_svc_debug("rgb888Buf is NULL: Error !!!");
		return FALSE;
	}

	memset(rgb888Buf, 0, (width * height * 3));

	for (i = 0; i < width * height; i++) {
		red = ((rgb565buf[i] >> 11) & 0x1F) << 3;
		green = ((rgb565buf[i] >> 5) & 0x3F) << 2;
		blue = (rgb565buf[i] & 0x1F) << 3;
		rgb888Buf[3 * i] = red;
		rgb888Buf[3 * i + 1] = green;
		rgb888Buf[3 * i + 2] = blue;
	}

	*pBuf_rgb888 = (void *)rgb888Buf;

	return TRUE;
}

/**
 * This function is wrapper function for "ImgFastCreateAGIFFrameData".
 *
 * @param 	szFileName[in] Specifies the read image data.
 * @param	 	width[in] Specifies a width of the image to be created.
 * @param 	height[in] Specifies a height of the image to be created.
 * @param 	bgColor[in] Specifies background color of output buffer.
 * @param 	bLoop[in] Specifies looping condition whether infinte or once play.
 * @return	This fucntion returns AGifFrameInfo Structure's pointer.
 * @see		ImgFastCreateAGIFFrameData.
 */

AGifFrameInfo *ImgCreateAGIFFrame(const char *szFileName, unsigned int width,
				  unsigned int height, unsigned int bgColor,
				  BOOL bLoop)
{
	HFile hFile;
	FmFileAttribute fileAttrib;
	unsigned long size;
	unsigned char *pEncodedData = NULL;
	int cFileSize;
	int mem_alloc_size;

	if (szFileName == NULL) {
		mb_svc_debug("Input File Name is NULL");
		return FALSE;
	}

	SysRequireEx(szFileName != NULL, FALSE);

	hFile = DrmOpenFile(szFileName);
	if (hFile == (HFile) INVALID_HOBJ) {
		mb_svc_debug("ImgCreateAGIFFrame: Cannot open file");
		return NULL;
	}

	DrmGetFileAttributes(szFileName, &fileAttrib);

	if (fileAttrib.fileSize == 0) {
		mb_svc_debug("Zero File Size");
		DrmCloseFile(hFile);
		return NULL;
	}

	cFileSize = fileAttrib.fileSize;
	/* A size of allocated memory - w * h *2 means RGB565 and 4096 means the max of header length */
	mem_alloc_size = width * height * 2 + MAX_GIF_HEADER_SIZE;

	if ((pEncodedData = (unsigned char *)malloc(mem_alloc_size)) == NULL) {
		mb_svc_debug("Memory Allocation to pEncodedData failed");
		DrmCloseFile(hFile);
		return NULL;
	}
	/* coverity[ -tainted_data_argument : pEncodedData ] */
	if (DrmReadFile(hFile, pEncodedData, mem_alloc_size, &size) == FALSE) {
		mb_svc_debug("DrmReadFile was failed");
		DrmCloseFile(hFile);

		if (pEncodedData) {
			free(pEncodedData);
			pEncodedData = NULL;
		}

		return NULL;
	}

	mb_svc_debug("ImgCreateAGIFFrame: file (%s) read...", szFileName);

	DrmCloseFile(hFile);

	return FastImgCreateAGIFFrameData(width, height, pEncodedData,
					  cFileSize, bgColor, bLoop);
}

/**
 * This function is wrapper function for "ImgFastDestroyAGIFFrameData".
 *
 * @param 	pFrameData[in]
 * @return	void
 * @see		ImgFastDestroyAGIFFrameData.
 * @see		ImgFastDestroyAGIFFrameData.
 *
 * @note 		ImgFastDestroyAGIFFrameData function set free all memory in AGifFrameInfo structure
 *			even pEncodedData
 *
 * @author	Hyunjun Ko <zzoon.ko@samsung.com>
 */

void ImgDestroyAGIFFrame(AGifFrameInfo *pFrameData)
{
	SysAssert(pFrameData);

	FastImgDestroyAGIFFrameData(pFrameData);
}

/**
 * This function is wrapper function for "ImgFastGetNextFrameAGIF".
 *
 * @param 	pFrameData[in]
 * @param		bCenterAlign Specifies true if you want center align in output buffer, else left top align
 * @return	This fucntion returns True or False as decoding result.
 * @see		ImgFastGetNextFrameAGIF.
 *
 * @note 		This function returns one frame data it decoded.
 *			next time, returns next decoded frame data... and so on.
 *
 * @author	rubric(sinjae4b.lee@samsung.com)
 */

ImgFastCodecInfo ImgGetNextAGIFFrame(AGifFrameInfo *gFrameData,
				     BOOL bCenterAlign)
{
	int iResult;

	if (gFrameData == NULL) {
		mb_svc_debug("Input gFrameData is NULL");
		return IMG_INFO_DECODING_FAIL;
	}

	iResult = FastImgGetNextFrameAGIF(gFrameData, bCenterAlign);

	if (iResult == 1) {
		return IMG_INFO_DECODING_SUCCESS;
	} else if (iResult == 2) {
		return IMG_INFO_AGIF_LAST_FRAME;
	} else {
		return IMG_INFO_DECODING_FAIL;
	}

}
