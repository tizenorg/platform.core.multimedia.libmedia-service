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


/**
 * This file includes API sets for syncronization with other module.
 *
 * @file       	mb-svc-thumb.h
 * @author 	  	Hyunjun Ko <zzoon.ko@samsung.com>
 * @version		1.0
 * @brief		Thumbnail management APIs.
 */

/**
* @ingroup MEDIA-SVC
* @defgroup	MEDIA_SVC_THUMB error code table
* @{
*/

#ifndef _MB_SVC_THUMB_H_
#define _MB_SVC_THUMB_H_

#include "media-svc-structures.h"
#include "media-svc-types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MB_SVC_THUMB_PATH				"/opt/data/file-manager-service"
#define MB_SVC_THUMB_PHONE_PATH			MB_SVC_THUMB_PATH"/.thumb/phone"
#define MB_SVC_THUMB_MMC_PATH			MB_SVC_THUMB_PATH"/.thumb/mmc"
#define DEFAULT_IMAGE_THUMB				MB_SVC_THUMB_PATH"/.thumb/thumb_default.png"
#define DEFAULT_VIDEO_THUMB				MB_SVC_THUMB_PATH"/.thumb/thumb_default.png"


/**
 * @fn  int 
 _mb_svc_thumb_generate_hash_name(char *file_full_path, char* thumb_hash_path, size_t max_thumb_path);
 * @brief  generate thumbnail path to value  media record thumbnail_path field
* @return error code.
 */
int 
_mb_svc_thumb_generate_hash_name(const char *file_full_path, char* thumb_hash_path, size_t max_thumb_path);


/**
 * @fn int mb_svc_image_create_thumb(char *file_fullpath, char* thumb_path_hash, bool force, mb_svc_image_meta_record_s *img_meta_record);
 * @brief  generate thumbnail path for image file
* @return error code.
 */

int 
mb_svc_image_create_thumb(const char *file_fullpath, char* thumb_path_hash, size_t max_thumb_length, bool force, mb_svc_image_meta_record_s *img_meta_record);


/**
 * @fn int mb_svc_image_create_thumb_new(char *file_fullpath, char* thumb_path_hash, mb_svc_image_meta_record_s *img_meta_record);
 * @brief  generate thumbnail path for image file
* @return error code.
 */

int 
mb_svc_image_create_thumb_new(const char *file_fullpath, char* thumb_path_hash, size_t max_thumb_length, mb_svc_image_meta_record_s *img_meta_record);

/**
 * @fn int mb_svc_video_create_thumb(char *file_fullpath, char* thumb_path_hash);
 * @brief  generate thumbnail path for video file
* @return error code.
*/
int 
mb_svc_video_create_thumb(const char *file_fullpath, char* thumb_path_hash, size_t max_thumb_length);

/**
* @fn    int  mb_svc_get_video_meta(MediaSvcHandle *mb_svc_handle, char* file_full_path, mb_svc_video_meta_record_s *video_record);
* This function gets video_meta record by filefull path
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    file_full_path           file full path
* @param[out]                   video_record      pointer to vidoe_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	

int
mb_svc_get_video_meta(MediaSvcHandle *mb_svc_handle, const char* file_full_path, mb_svc_video_meta_record_s *video_record);


/**
* @fn    int mb_svc_get_image_meta(MediaSvcHandle *mb_svc_handle, char* file_full_path, mb_svc_image_meta_record_s *image_record, bool *thumb_done);
* This function gets image_meta record by filefull path
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    file_full_path           file full path
* @param[out]                   record by filefull path      pointer to image_meta record
* @param[out]                   thumb_done      			Flag whether thumbnail is generated or not
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	

int
mb_svc_get_image_meta(MediaSvcHandle *mb_svc_handle, const char* file_full_path, mb_svc_image_meta_record_s *image_record, bool *thumb_done);

/**
* @brief  Delete thumbnail file of original file.
* @param  file_full_path  original file.
* @return 0 ok, -1 failed.
*/
int
_mb_svc_thumb_delete(const char* file_full_path);

/**
* @brief  Delete a specified thumbnail file directly.
* @param  file_full_path  the fullpath of thumbnail file.
* @return 0 ok, -1 failed.
*/
int
_mb_svc_thumb_rm(char *file_full_path);


/**
* @brief  Copy thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_copy(const char* src_file_full_path,
								const char* dest_file_full_path,
								char* dest_thumb_path);

/**
* @brief  Move thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_move(const char* src_file_full_path,
								const char* dest_file_full_path,
								char* dest_thumb_path);

/**
* @brief  Rename thumbnail of src to the one of dest.
* @param[in]   src_file_full_path    src original file.
* @param[in]   dest_file_full_path   dest original file.
* @param[out]  dest_thumb_path       thumbnail file of dest orignal file.
* @return  0 ok, -1 failed.
*/
int
_mb_svc_thumb_rename(const char* src_file_full_path,
								const char* dest_file_full_path,
								char* dest_thumb_path);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*_MEDIA_SVC_THUMB_H_*/

/**
* @}
*/

