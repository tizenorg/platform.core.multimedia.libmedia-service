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
 * This file defines the error code of media service
 *
 * @file       	media-svc-util.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines the error code of media service 
 */

/**
* @ingroup MEDIA-SVC
* @defgroup	MEDIA_SVC_UTIL error code table
* @{
*/


#ifndef _MEDIA_SVC_UTIL_H_
#define _MEDIA_SVC_UTIL_H_
	
#include <glib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB_SVC_PATH_PHONE 	"/opt/media" 	/**< File path prefix of files stored in phone */
#define MB_SVC_PATH_MMC 	"/opt/storage/sdcard"	/**< File path prefix of files stored in mmc card */


/**
* @fn    int _mb_svc_get_file_display_name (const char* file_path, char* file_name);
* This function gets file name by file full path
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    file_path           file full path
* @param[out]                   file_name      file name
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	

bool	
_mb_svc_get_file_display_name (const char* file_path, char* file_name);

 
 /**
 * @fn	  int _mb_svc_get_file_parent_path(const char* file_path, char* parent_path);
 * This function gets file parent path
 *
 * @return						  This function returns 0 on success, and -1 on failure.
 * @param[in]					 file_path 		  file full path
 * @param[out]					 parent_path	  file parent path
 * @exception					 None.
 * @remark						  
 *															   
 *															
 */  

bool
_mb_svc_get_file_parent_path(const char* file_path, char* parent_path);

/**
* @fn    int _mb_svc_get_dir_display_name (const char* dir_path, char* dir_name);
* This function gets directory name
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    dir_path           directory path
* @param[out]                   dir_name     directory name
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	
bool	
_mb_svc_get_dir_display_name (const char* dir_path, char* dir_name);

/**
* @fn    int _mb_svc_get_dir_parent_path(const char* dir_path, char* parent_path);
* This function gets directory parent path
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    dir_path           directory path
* @param[out]                   parent_path      directory parent path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	 
bool
_mb_svc_get_dir_parent_path(const char* dir_path, char* parent_path);

/**
* @fn    int _mb_svc_get_file_dir_modified_date(const char* full_path);
* This function gets file modified date
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    full_path           file full path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	

int 
_mb_svc_get_file_dir_modified_date(const char* full_path);

/**
* @fn    int _mb_svc_get_store_type_by_full(const char* full_path);
* This function gets file store type
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    full_path           file full path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	 
int
_mb_svc_get_store_type_by_full(const char* full_path);

/**
* @fn    int _mb_svc_get_rel_path_by_full(const char* full_path, char* path);
* This function gets file relative path
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    full_path           file full path
* @param[out]                   path     file relative path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	  
int
_mb_svc_get_rel_path_by_full(const char* full_path, char* path);

/**
* @fn    int _mb_svc_get_file_ext (const char* file_path, char* file_ext);
* This function gets file extension
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    file_path           file full path
* @param[out]                   file_ext       pointer to file extention
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	 
bool
_mb_svc_get_file_ext (const char* file_path, char* file_ext);

/**
* @fn    int _mb_svc_glist_free(GList** glist, bool is_free_element);
* This function frees glist and maybe free each items in glist
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    glist           the glist to be freed
* @param[in]                 is_free_element    falg to declare if to free items in glist
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/	

bool
_mb_svc_glist_free(GList** glist, bool is_free_element);

/**
* @fn    int _mb_svc_get_file_type(char* file_full_path);
* This function gets file type
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    file_full_path           file full path
 * @exception                    None.
* @remark
*
*
*/

int
_mb_svc_get_file_type(const char* file_full_path);

bool
_mb_svc_get_path_by_full(const char* full_path, char* path, int max_length);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*_MEDIA_SVC_UTIL_H_*/

/**
* @}
*/

