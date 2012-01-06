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
 * This file is  main header file of media service.
 *
 * @file		media-svc-api.h
 * @author	Lin Chaolong(chaolong.lin@samsung.com)
 * @version 	1.0
 * @brief		This file is header file api.
 */

/**
* @ingroup MEDIA_SVC
* @defgroup	MEDIA_SVC_API out-house media service API
* @{
*/

#ifndef _MEDIA_SERVICE_API_H_
#define _MEDIA_SERVICE_API_H_

#include <glib.h>
#include "media-svc-structures.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @fn    int  mb_svc_insert_file(char* file_full_path, mb_svc_media_type_t content_type);
* This function insert file information into folder table, meida table, image_meta table/video_meta table and bookmark table
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    file_full_path           the full path of file
* @param[in]                    content_type      the file type, maybe video, image and so on
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int
mb_svc_insert_file(const char* file_full_path, minfo_file_type content_type);


/**
* @fn    int  mb_svc_delete_file(char* file_full_path, mb_svc_media_type_t type);
* This function deletes file information from folder table,media table and related tables
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                     file_full_path           full path of file
* @exception                     None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_delete_file(const char* file_full_path);

/**
* @fn    int  mb_svc_update_file(char* old_file_full_path, char *new_file_full_path, mb_svc_media_type_t type);
* This function updates file information for media table, image_meta table/video_meta table
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    old_file_full_path           full path of old file
* @param[in]                    new_file_full_path           full path of updated file
* @param[in]                    type                         file type,maybe image,video and so on
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_update_file(const char* old_file_full_path, const char *new_file_full_path, minfo_file_type content_type);

/**
* @fn    int  mb_svc_rename_file(char *old_file_full_path, char *new_file_full_path, mb_svc_media_type_t type, char* thumb_path);
* This function updates media table due to file name is changed
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    old_file_full_path           full path of old file
* @param[in]                    new_file_full_path           full path of renamed file
* @param[in]                    type                         file type,maybe image,video and so on
* @param[out]                   thumb_path                   new file thumb path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_rename_file(const char *old_file_full_path, const char *new_file_full_path, minfo_file_type content_type, char* thumb_path);

/**
* @fn    int  mb_svc_move_file(char *old_file_full_path, char *new_file_full_path, mb_svc_media_type_t type, char* thumb_path);
* This function updates media table and folder table due to the file is moved
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    old_file_full_path           old full path of file
* @param[in]                    new_file_full_path           new full path of file
* @param[in]                    type                         file type, maybe image,video and so on
* @param[out]                   thumb_path                   new file thumb path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_move_file(const char *old_file_full_path, const char *new_file_full_path, minfo_file_type content_type, char* thumb_path);

/**
* @fn    int  mb_svc_move_file_by_id(const char *src_media_id, const char *dst_cluster_id);
* This function moves a media to destination cluster
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    src_media_id           id of source media
* @param[in]                    dst_cluster_id        id of destination cluster
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_move_file_by_id(const char *src_media_id, const char *dst_cluster_id);

/**
* @fn    int  mb_svc_copy_file(char *old_file_full_path, char *new_file_full_path, mb_svc_media_type_t type, char* thumb_path);	
* This function updates folder table, media table and image_meta/video_meta table
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    old_file_full_path           old full path of file
* @param[in]                    new_file_full_path           new full path of file
* @param[in]                    type                         file type,may be image,video and so on
* @param[out]                   thumb_path                   new file thumb path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_copy_file(const char *old_file_full_path, const char *new_file_full_path, minfo_file_type content_type, char* thumb_path);

/**
* @fn    int  mb_svc_copy_file_by_id(const char *src_media_id, const char *dst_cluster_id);	
* This function copy a media to the destination cluster.
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    src_media_id           id of source media
* @param[in]                    dst_cluster_id        id of destination cluster
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_copy_file_by_id(const char *src_media_id, const char *dst_cluster_id);

/**
* @fn    int  mb_svc_update_cluster_name(const char *cluster_id, const char* new_name);
* This function update a name of the cluster.
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    cluster_id           id of cluster
* @param[in]                    new_name        	new name of the cluster
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_update_cluster_name(const char *cluster_id, const char* new_name);

void
mb_svc_sync_files_to_db_with_path(char* dir_full_path);

int
mb_svc_insert_items();

int
mb_svc_insert_file_batch(const char *file_full_path, minfo_file_type content_type);

//clock_t 
long mb_svc_get_clock(void);


/**
* @fn      int mb_svc_initialize();
* This function connects to media service. 
*
* @return                        This function returns 0 on success, and error code on failure.
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/
int mb_svc_initialize();
/**
* @fn      int mb_svc_finalize();
* This function disconnects to media service. 
*
* @return                        This function returns 0 on success, and error code on failure.
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/
int mb_svc_finalize();

/**
* @fn      int mb_svc_get_folder_media_list(int folder_id, int file_type, int favoriate, GList** p_record_list);
* This function gets folder media list from media table via folder ID. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder id of media file  
* @param[in]                    file_type         file type (0--image, 1--video, 2--image+video)
* @param[in]                    favoriate        favoriate image or video 
* @param[out]                  p_record_list     list of media file obtained
* @exception                    None.
* @remark                        list record type is   mb_svc_media_record_s.
*                                                             
*                                                          
*/

int mb_svc_get_folder_media_list(int folder_id, minfo_file_type content_type, int favorite,GList** p_record_list);



/**
* @fn      int  mb_svc_get_video_record_by_media_id(int media_id, mb_svc_video_meta_record_s* video_meta_record);
* This function gets a video record  from video meta table via media ID.
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id        media id of video file
* @param[out]                  video_meta_record     video record obtained
* @exception                    None.
* @remark                        E.g.
*
*
*/

int  mb_svc_get_video_record_by_media_id(const char *media_id, mb_svc_video_meta_record_s* video_meta_record);

/**
* @fn      int  mb_svc_get_image_record_by_media_id(int media_id, mb_svc_image_meta_record_s* image_meta_record);
* This function gets a image record  from image meta table via media ID.
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id        media id of video file
* @param[out]                  image_meta_record     image record obtained
* @exception                    None.
* @remark                        E.g.
*
*
*/
int  mb_svc_get_image_record_by_media_id(const char *media_id, mb_svc_image_meta_record_s* image_meta_record);

/**
* @fn      int mb_svc_get_video_list(int folder_id, int favoriate, GList** p_record_list);
* This function gets all the video file in some folder via folder ID. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder id of media file  
* @param[in]                    favoriate        favoriate video
* @param[out]                  p_record_list     list of media video file obtained
* @exception                    None.
* @remark                        list record type is   mb_svc_video_meta_record_s.
*                                                             
*                                                          
*/
int mb_svc_get_video_list(int folder_id, int favorite, GList** p_record_list);

/**
* @fn     int  mb_svc_get_folder_fullpath_by_folder_id( int folder_id, char* folder_fullpath);	
* This function gets some folder's full path. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        		folder ID
* @param[out]                  folder_fullpath     	folder  path
* @max_length[in]             max_length        	the maximum length of the @p folder_fullpath
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/
int 
mb_svc_get_folder_fullpath_by_folder_id(const char *folder_id, char* folder_fullpath, int max_length);


/**
* @fn     int  mb_svc_get_folder_id_by_full_path( const char* folder_full_path,int* folder_id, int max_length);	
* This function gets some folder's full path. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                  	folder_fullpath     	folder  path
* @param[out]                  folder_id        		folder ID
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/

int 
mb_svc_get_folder_id_by_full_path( const char* folder_full_path, char *folder_id, int max_length);

/**
* @fn     int  mb_svc_get_folder_id_by_web_album_id(const char* web_album_id,int* folder_id);	
* This function could get folder id of a cluster, using its web album ID. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    web_album_id     	web album ID of a web cluster.
* @param[out]                  folder_id        		folder ID
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/

int 
mb_svc_get_folder_id_by_web_album_id(const char* web_album_id, char *folder_id);


/**
* @fn    int mb_svc_get_media_fullpath(const char *folder_id, char* media_display_name, char* media_fullpath);
* This function gets some media file's full path. 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder ID
* @param[in]                    media_display_name        meida file name
* @param[out]                  media_fullpath     media file  path
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/
int mb_svc_get_media_fullpath(const char *folder_id, char* media_display_name, char* media_fullpath);



/**
* @fn     int mb_svc_get_media_image_item_list(int folder_id, int media_type, int favoriate, GList** p_item_list)
* This function gets all the media item or image item from  record of media table  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder id 
* @param[in]                    favoriate       favoriate media 
* @param[out]                  p_item_list     media item list
* @exception                    None.
* @remark                        This function provide general API to get image, or both video and image list.  
*						Different list correpsonding to different item type.
*						image list item type is mb_svc_image_item_t.
*						media(video and image) list item type is mb_svc_media_item_t.

*                                                             
*                                                          
*/
int mb_svc_get_media_image_item_list(int folder_id, minfo_file_type content_type, int favorite, GList** p_item_list);


/**
* @fn     int mb_svc_get_video_item_list(int folder_id, int favoriate, GList** p_item_list)
* This function gets video item from  record of media table  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder id 
* @param[in]                    media_type        media type 
* @param[in]                    favoriate       favoriate media 
* @param[out]                  p_item_list    video item list
* @exception                    None.
* @remark                        list item type is mb_svc_video_item_t.
*                                                             
*                                                          
*/
int mb_svc_get_video_item_list(int folder_id, int favorite, GList** p_item_list);

/**
* @fn     int mb_svc_get_media_item_list(int folder_id, int media_type, int favoriate, GList** p_item_list)
* This function gets all the media item or image item or video item from  record of media table  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id        folder id 
* @param[in]                    media_type        media type 
* @param[in]                    favoriate       favoriate media 
* @param[out]                  p_item_list     media item list
* @exception                    None.
* @remark                        This function provide general API to get video, or image, or both video and image list.  
*						Different list correpsonding to different item type.
*						video list item type is mb_svc_video_item_t.
*						image list item type is mb_svc_image_item_t.
*						media(video and image) list item type is mb_svc_media_item_t.
*                                                             
*                                                          
*/
int mb_svc_get_media_item_list(int folder_id, minfo_file_type content_type, int favorite, GList** p_item_list);

/**
* @fn     int mb_svc_get_all_favorite_media_item_list(int media_type, GList** p_item_list);
* This function gets all the favorite media item or image item or video item from  record of media table  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_type        media type 
* @param[out]                  p_item_list     media item list
* @exception                    None.
* @remark                        list item type is mb_svc_media_item_t.
*                                                             
*                                                          
*/
int 
mb_svc_get_all_favorite_media_item_list(minfo_file_type content_type, GList** p_item_list);



/**
* @fn   int mb_svc_get_media_record_cnt_by_folder_id (int folder_id, int* count);
* This function gets record count in some folder  
*
* @return                        This function returns record count.
* @param[in]                    folder_id        folder id 
* @param[out]                  count             returned count of records
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/
int mb_svc_get_media_record_cnt_by_folder_id(int folder_id, int* count);

/**
* @fn   int mb_svc_get_folder_list(GList** p_record_list);
* This function gets all the folders  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[out]                  p_record_list     folder record list
* @exception                    None.
* @remark                        list record type is   mb_svc_folder_record_s.
*
*
*/
int mb_svc_get_folder_list(GList** p_record_list);

/**
* @fn  int mb_svc_get_folder_item_list(GList** p_item_list);
* This function gets all the folder items 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[out]                  p_record_list     folder item list
* @exception                    None.
* @remark                        list item type is mb_svc_folder_item_t. 
*                                                             
*                                                          
*/
int mb_svc_get_folder_item_list(GList** p_item_list);

/**
* @fn    int  mb_svc_get_bookmark_record_by_id(int record_id, mb_svc_bookmark_record_s* record);
* This function gets the bookmark  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record_id        bookmark ID 
* @param[out]                  record     record returned
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/
int  mb_svc_get_bookmark_record_by_id(int record_id, mb_svc_bookmark_record_s* record);

/**
* @fn    int  mb_svc_get_media_tag_by_id(int _id, mb_svc_tag_record_s *mtag_record);
* This function gets the tag record according to its ID
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    _id        		tag record ID 
* @param[out]                  mtag_record     tag record returned
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_get_media_tag_by_id(int _id, mb_svc_tag_record_s *mtag_record);


/**
* @fn    int mb_svc_get_bookmark_record_by_media_id(int media_id, mb_svc_bookmark_record_s* bookmark_record);
* This function gets the bookmark  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id        media ID 
* @param[out]                  record     	record returned
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int  
mb_svc_get_bookmark_record_by_media_id(int media_id, mb_svc_bookmark_record_s* bookmark_record);



/**
* @fn   int  mb_svc_get_image_list_by_location(double longitude, double latitude, GList** p_record_list);
* This function gets all the image records at some location  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    longitude        longitude 
* @param[in]                    latitude        latitude 
* @param[out]                  p_record_list    image record list
* @exception                    None.
* @remark                        list record type is   mb_svc_image_meta_record_s.
*
*
*/
int  mb_svc_get_image_list_by_location(double longitude, double latitude, GList** p_record_list);

/**
* @fn   int  mb_svc_get_video_list_by_location(double longitude, double latitude, GList** p_record_list);
* This function gets all the video records at some location  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    longitude        longitude 
* @param[in]                    latitude        latitude 
* @param[out]                  p_record_list    video record list
* @exception                    None.
* @remark                        list record type is   mb_svc_video_meta_record_s.
*
*
*/
int  mb_svc_get_video_list_by_location(double longitude, double latitude, GList** p_record_list);

/**
* @fn   int mb_svc_get_media_item_list_by_location(double longitude, double latitude, int media_type, int favoriate, GList** p_item_list);

* This function gets all the media item at some location  
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    longitude        longitude 
* @param[in]                    latitude        latitude 
* @param[in]                    media_type        media type 
* @param[in]                    favoriate        favoriate media 
* @param[out]                  p_item_list     media item list
* @exception                    None.
* @remark                        list item type is mb_svc_media_location_item_t.
*                                                             
*                                                          
*/
int mb_svc_get_media_item_list_by_location(double longitude, double latitude, minfo_file_type content_type, int favorite, GList** p_item_list);


/**
* @fn   int  mb_svc_get_web_streaming_record_by_folder_id(int folder_id, GList** p_record_list);
* This function gets the web streaming records list mapped to folder ID . 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id    folder ID
* @param[out]                  p_record_list    web streaming record list
* @exception                    None.
* @remark                        E.g.  
*                                                             
*                                                          
*/
int  mb_svc_get_web_streaming_record_by_folder_id(int folder_id, GList** p_record_list);

/**
* @fn   int  mb_svc_get_web_album_list_by_web_account(char *web_account,GList** p_record_list);
* This function gets the web streaming records list via web account 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    web_account   web account
* @param[out]                  p_record_list    web streaming record list
* @exception                    None.
* @remark                        list record type is   mb_svc_web_streaming_record_s.
*
*
*/
int  mb_svc_get_web_album_list_by_web_account(char *web_account,GList** p_record_list);

/**
* @fn   int  mb_svc_get_web_streaming_item_by_web_account(char *web_account,GList** p_item_list);
* This function gets the web streaming item list via web account 
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    web_account   web account
* @param[out]                  p_item_list    web streaming item list
* @exception                    None.
* @remark                        list item type is mb_svc_web_streaming_item_t.
*                                                             
*                                                          
*/
int  mb_svc_get_web_streaming_item_by_web_account(char *web_account,GList** p_item_list);

/**
* @fn    int  mb_svc_update_favorite_by_media_id(const char *media_id, int favorite);
* This function update rate field of media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id    folder id
* @param[in]                    display_name    file name
* @param[in]                    favorite    favorite
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int
mb_svc_update_favorite_by_media_id(const char *media_id, int favorite);

/**
* @fn    int  mb_svc_get_media_record_by_fid_name(int folder_id, char* display_name, mb_svc_media_record_s* m_record);
* This function gets media record by folder id and file name
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id     folder id
* @param[in]                    display_name  file name
* @param[out]                   m_record      media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_get_media_record_by_fid_name(const char *folder_id, const char* display_name, mb_svc_media_record_s* m_record);

/**
* @fn    int  mb_svc_get_folder_content_count_by_folder_id(int folder_id);
* This function gets matched media record count with specified folder id
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    folder_id           specified folder_id field in media table
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_get_folder_content_count_by_folder_id(const char *folder_id);

/**
* @fn    int  mb_svc_media_iter_start(int folder_id, mb_svc_media_list_filter* filter, minfo_folder_type folder_type, int valid,  GList* p_folder_id_list, mb_svc_iterator_s* mb_svc_iterator );
* This function gets media record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    folder_id             specified folder_id field in media table
* @param[in]                    filter                specified filter qualification to get matched record
* @param[in]                    folder_type       specified foler type
* @param[in]                    valid		       valid or invalid
* @param[out]                   mb_svc_iterator       pointer to media record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_media_iter_start(int folder_id, minfo_item_filter* filter, minfo_folder_type folder_type, int valid, GList* p_folder_id_list, mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_media_iter_start_new(int folder_id, mb_svc_media_list_filter* filter, minfo_folder_type folder_type, int valid,  GList* p_folder_id_list, mb_svc_iterator_s* mb_svc_iterator );
* This function gets media record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    folder_id             specified folder_id field in media table
* @param[in]                    filter                specified filter qualification to get matched record
* @param[in]                    folder_type       specified foler type
* @param[in]                    valid		       valid or invalid
* @param[out]                   mb_svc_iterator       pointer to media record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_media_iter_start_new(const char *folder_id, minfo_item_filter* filter, minfo_folder_type folder_type, int valid, GList* p_folder_id_list, mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_media_search_iter_start(minfo_search_field_t search_field, const char *search_str, const minfo_item_filter filter, mb_svc_iterator_s *mb_svc_iterator );
* This function gets media record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    search_field             specified field to search
* @param[in]                    search_str       specified string to search
* @param[in]                    folder_type       specified foler type
* @param[in]                    filter                specified filter qualification to get matched record
* @param[out]                   mb_svc_iterator       pointer to media record iterator
* @exception                    None.
*/

int 
mb_svc_media_search_iter_start(minfo_search_field_t search_field, const char *search_str, minfo_folder_type folder_type, minfo_item_filter filter, mb_svc_iterator_s *mb_svc_iterator);

/**
* @fn    int  mb_svc_media_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_media_record_s *record);
* This function gets next media record
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator             pointer to media record iterator
* @param[out]                   record                      pointer to next media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_media_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_media_record_s *record);

/**
* @fn    int  mb_svc_folder_iter_start(minfo_cluster_filter* cluster_filter, mb_svc_iterator_s* mb_svc_iterator);
* This function gets folder record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    cluster_filter             pointer to filter qualification to get matched folder record
* @param[out]                   mb_svc_iterator            pointer to folder record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_folder_iter_start(minfo_cluster_filter* cluster_filter, mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_folder_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_folder_record_s *record);
* This function gets next folder record
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator             pointer to folder record iterator
* @param[out]                   record                      pointer to next folder record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_folder_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_folder_record_s *record);

/**
* @fn    int  mb_svc_iter_finish(mb_svc_iterator_s* mb_svc_iterator);
* This function finalize iterator usage
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator             pointer to folder record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_iter_finish(mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_get_media_record_by_id(int media_id,mb_svc_media_record_s *media_record);
* This function gets media record matched with field _id
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    media_id                    specified fielde "_id" in media table
* @param[out]                   media_record                pointer to matched media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int
mb_svc_get_media_record_by_id(const char *media_id, mb_svc_media_record_s *media_record);

/**
* @fn    int  mb_svc_get_folder_record_by_id(const char *folder_id, mb_svc_folder_record_s *folder_record);
* This function gets folder record matched with field _id
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    folder_id                   specified "_id" in folder table
* @param[out]                   folder_record               pointer to matched folder record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int
mb_svc_get_folder_record_by_id(const char *folder_id, mb_svc_folder_record_s *folder_record);

/**
* @fn    int  mb_svc_get_video_id_by_media_id(int media_id, int* video_id);	
* This function gets video record id by media_id filed in video_meta table
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    media_id             specified media_id field
* @param[out]                   video_id             pointer to field "_id" in video_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int  
mb_svc_get_id_by_media_id(int media_id, char* table_name, int* video_id);	

/**
* @fn    int  mb_svc_delete_folder(const char *folder_id, minfo_store_type storage_type);
* This function delete folder record,matched media record,image_meta record or video_meta record and bookmark record by field "_id"
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    folder_id            specified "_id" field in folder table
* @param[in]                    store_type         specified "store_type" field in folder table
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int
mb_svc_delete_folder(const char *folder_id, minfo_store_type storage_type);

/**
* @fn    int  mb_svc_webstreaming_iter_start(mb_svc_iterator_s* mb_svc_iterator);
* This function gets webstreaming record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[out]                   mb_svc_iterator      pointer to webstreaming record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_webstreaming_iter_start(mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_webstreaming_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_web_streaming_record_s *webstreaming_record);
* This function gets next webstreaming record
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator            pointer to webstreaming record iterator
* @param[out]                   webstreaming_record        pointer to next webstreaming record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_webstreaming_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_web_streaming_record_s *webstreaming_record);


/**
* @fn    int  mb_svc_bookmark_iter_start(mb_svc_iterator_s* mb_svc_iterator);
* This function gets bookmark record iterator
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    media_id                  media_id
* @param[out]                   mb_svc_iterator            pointer to bookmark record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_bookmark_iter_start(const char *media_id, mb_svc_iterator_s* mb_svc_iterator);


/**
* @fn    int  mb_svc_bookmark_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_bookmark_record_s *record);
* This function gets next bookmark record
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator             pointer to bookmark record iterator
* @param[out]                   record                      pointer to next bookmark record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int
mb_svc_bookmark_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_bookmark_record_s *record);

/**
* @fn    mb_svc_tag_iter_start(const char *tag_name, int media_id, mb_svc_iterator_s* mb_svc_iterator);
* This function gets tag record iterator, according tag_name, if any, or media_id.
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    tag_name                  tag name
* @param[in]                    media_id                  media_id
* @param[out]                   mb_svc_iterator            pointer to bookmark record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_tag_iter_start(const char *tag_name, const char *media_id, mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    mb_svc_tag_iter_with_filter_start(const char *tag_name, int media_id, minfo_tag_filter, filter, mb_svc_iterator_s* mb_svc_iterator);
* This function gets tag record iterator, according tag_name, if any, or media_id.
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    tag_name                  tag name
* @param[in]                    media_id                  media_id
* @param[in]                    filter                    minfo_tag_filter
* @param[out]                   mb_svc_iterator            pointer to bookmark record iterator
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_tag_iter_with_filter_start(const char *tag_name, minfo_tag_filter filter, mb_svc_iterator_s* mb_svc_iterator);

/**
* @fn    int  mb_svc_tag_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_tag_record_s *record);
* This function gets next tag record
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_iterator             pointer to tag record iterator
* @param[out]                   record                      pointer to next tag record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_tag_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_tag_record_s *record);

int 
mb_svc_media_id_list_by_tag_iter_next(mb_svc_iterator_s* mb_svc_iterator, mb_svc_tag_record_s *record);

int 
mb_svc_delete_record_tag(const char *tag_name, const char *media_id);

int 
mb_svc_rename_record_tag(const char* src_tagname, const char* dst_tag_name);

int 
mb_svc_rename_record_tag_by_id(const char *media_id, const char* src_tagname, const char* dst_tag_name);

int
mb_svc_get_tagid_by_tagname(const char* tag_name);

int 
mb_svc_add_web_streaming_folder(char *folder_id);

int 
mb_svc_get_web_streaming_folder_id();

int
mb_svc_get_web_streaming_folder_uuid(char *folder_uuid, int max_length);

int
mb_svc_get_media_id_by_full_path(const char* file_full_path, char *media_id);

int
mb_svc_get_media_id_by_http_url(const char* http_url, char* media_id);

int
mb_svc_get_media_record_by_full_path(const char* file_full_path, mb_svc_media_record_s* record);

int  
mb_svc_get_web_album_cluster_record(int sns_type, const char* name, const char *account_id, const char *album_id, mb_svc_folder_record_s* folder_record);

int
mb_svc_delete_invalid_media_records(const minfo_store_type storage_type);

int
mb_svc_get_folder_name_by_id(const char *folder_id, char *folder_name, int max_length);

int 
mb_svc_get_folder_list_by_web_account_id(char *web_account, GList** p_record_list);

int 
mb_svc_geo_media_iter_start(const char *folder_id,
						minfo_folder_type store_filter,
						minfo_item_filter* filter,
						mb_svc_iterator_s* mb_svc_iterator,
						double min_longitude, 
						double max_longitude, 
						double min_latitude, 
						double max_latitude );

int
mb_svc_get_all_item_count(int *cnt);

int
mb_svc_get_media_count_by_tagname(const char* tagname, int* count);

int
mb_svc_delete_tagmap_by_media_id(const char *media_id);



#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @}
 */


#endif /*_MEDIA_SERVICE_API_H_*/

