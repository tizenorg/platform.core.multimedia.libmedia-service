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
 * This file defines synchronize apis for phone explorer.
 *
 * @file       	media-svc-db.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @ingroup MEDIA_SVC
  * @defgroup MEDIA_SVC_DB in-house media service API
  * @{
  */


#ifndef _MEDIA_SVC_DB_H_
#define _MEDIA_SVC_DB_H_

#include "media-svc-structures.h"

#ifdef __cplusplus
extern "C" {
#endif

//table name macros
#define MB_SVC_TBL_NAME_BOOKMARK   		"video_bookmark"
#define MB_SVC_TBL_NAME_FOLDER   		"visual_folder"
#define MB_SVC_TBL_NAME_WEB_STREAMING 	"web_streaming"
#define MB_SVC_TBL_NAME_MEDIA			"visual_media"
#define MB_SVC_TBL_NAME_VIDEO_META		"video_meta"
#define MB_SVC_TBL_NAME_IMAGE_META		"image_meta"
#define MB_SVC_TBL_NAME_TAG_MAP			"visual_tag_map"
#define MB_SVC_TBL_NAME_TAG				"visual_tag"

//default query string len
#define MB_SVC_DEFAULT_QUERY_SIZE  1023 * 3

#define MB_SVC_DATABASE_NAME	"/opt/dbspace/.media.db"
#define MB_SVC_TABLE_NAME_MAX_LEN 1024
#define DELETE_FOLDER_RECORD_IF_NO_FILE_IN    //if folder contains no files, delete folder record.


/**
* @fn    int  mb_svc_connect_db(sqlite3** handle);
* This function connects to database server and creates tables, such as bookmark,folder and so on
*
* @param[out]                    handle           DB handle connected
* @return                        This function returns 0 on success, and negative value on failure.
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_connect_db(sqlite3** handle);

/**
* @fn    int int mb_svc_disconnect_db(sqlite3* handle);
* This function disconnects with database server
*
* @param[in]                    handle           DB handle to disconnect
* @return                        This function returns 0 on success, and negative value on failure.
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_disconnect_db(sqlite3* handle);

/**
* @fn    int  _mb_svc_truncate_tbl();
* This function truncates all created tables, such as bookmark,folder and so on
*
* @return                        This function returns 0 on success, and negative value on failure.
* @exception                     None.
* @remark                        
*                                                             
*                                                          
*/

int _mb_svc_truncate_tbl();

/**
* @fn    int  mb_svc_delete_record_bookmark_by_id(int id);
* This function deletes specified bookmark record by field "_id"
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    id           bookmark record id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_delete_record_bookmark_by_id(int id);

/**
* @fn    int  int mb_svc_delete_record_folder_by_id(const char *id);
* This function deletes specified folder record by id
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    id           folder record id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_delete_record_folder_by_id(const char *id);

/**
* @fn    int  mb_svc_delete_record_web_streaming_by_id(int id);
* This function deletes specified webstreaming record by id
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    id           webstreaming record id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_delete_record_web_streaming_by_id(int id);

/**
* @fn    int mb_svc_delete_record_media_by_id(const char *id);
* This function deletes specified media record by id
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    id          media record id
 * @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_delete_record_media_by_id(const char *id);

/**
* @fn    int mb_svc_delete_record_video_meta_by_id(int id);
* This function deletes specified video_meta record
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    id          video_meta record id
 * @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_delete_record_video_meta_by_id(int id);

/**
* @fn    int mb_svc_update_record_folder(mb_svc_folder_record_s *record);
* This function updates folder record
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    record      pointer to new folder record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_update_record_folder(mb_svc_folder_record_s *record);

/**
* @fn    int  mb_svc_update_record_media(mb_svc_media_record_s *record);
* This function updates media record
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    record      pointer to new media record
* @exception                    None.
* @remark
*
*
*/

int mb_svc_update_record_media(mb_svc_media_record_s *record);

/**
* @fn    int mb_svc_update_record_video_meta(mb_svc_video_meta_record_s *record);
* This function updates video_meta record
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    record      pointer to new video_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_update_record_video_meta(mb_svc_video_meta_record_s *record);

/**
* @fn    int mb_svc_update_record_image_meta(mb_svc_image_meta_record_s *record);
* This function updates image_meta record
*
* @return                        This function returns 0 on success, and negative value on failure.
* @param[in]                    record      pointer to new image_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_update_record_image_meta(mb_svc_image_meta_record_s *record);

/**
* @fn    int mb_svc_insert_record_bookmark(mb_svc_bookmark_record_s *record);
* This function inserts record into bookmark table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record           pointer to inserted record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int mb_svc_insert_record_bookmark(mb_svc_bookmark_record_s *record);

/**
* @fn    mb_svc_insert_record_folder(mb_svc_folder_record_s *record);
* This function inserts record into folder table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record      pointer to folder record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_insert_record_folder(mb_svc_folder_record_s *record);

/**
* @fn    int mb_svc_insert_record_web_streaming(mb_svc_web_streaming_record_s *record);
* This function inserts record into webstreaming table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record      pointer to web streaming reocrd
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_insert_record_web_streaming(mb_svc_web_streaming_record_s *record);

/**
* @fn    int  mb_svc_insert_record_media(mb_svc_media_record_s *record);
* This function inserts record into media table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record      pointer to media record
* exception                    None.
* @remark
*
*
*/

int mb_svc_insert_record_media(mb_svc_media_record_s *record, minfo_store_type storage_type);

/**
* @fn    int  mb_svc_insert_record_video_meta(mb_svc_video_meta_record_s *record, minfo_store_type storage_type);
* This function insert record into video_meta table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record      pointer to video_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_insert_record_video_meta(mb_svc_video_meta_record_s *record, minfo_store_type storage_type);

/**
* @fn    int  mb_svc_insert_record_image_meta(mb_svc_image_meta_record_s *record, minfo_store_type storage_type);
* This function inserts record into image_meta table
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record       pointer to image_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_insert_record_image_meta(mb_svc_image_meta_record_s *record, minfo_store_type storage_type);

/**
* @fn    int  mb_svc_load_record_bookmark(sqlite3_stmt* stmt, mb_svc_bookmark_record_s * record);
* This function gets bookmark record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt           sql stmt
* @param[out]                   record      pointer to bookmark record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int mb_svc_load_record_bookmark(sqlite3_stmt* stmt, mb_svc_bookmark_record_s * record);

/**
* @fn    int  mb_svc_load_record_folder(sqlite3_stmt* stmt, mb_svc_folder_record_s * record);
* This function gets folder record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt           sql stmt
* @param[out]                   record      pointer to folder record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_load_record_folder(sqlite3_stmt* stmt, mb_svc_folder_record_s * record);

/**
* @fn    int mb_svc_load_record_web_streaming(sqlite3_stmt* stmt, mb_svc_web_streaming_record_s * record);
* This function gets web streaming record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                  record      pointer to web streaming record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_load_record_web_streaming(sqlite3_stmt* stmt, mb_svc_web_streaming_record_s * record);

/**
* @fn    int  mb_svc_load_record_media(sqlite3_stmt* stmt, mb_svc_media_record_s * record);
* This function gets media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                   record      pointer to media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_load_record_media(sqlite3_stmt* stmt, mb_svc_media_record_s * record);

/**
* @fn    int mb_svc_load_record_video_meta(sqlite3_stmt* stmt, mb_svc_video_meta_record_s * record);
* This function gets video_meta record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                   record      pointer to video_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_load_record_video_meta(sqlite3_stmt* stmt, mb_svc_video_meta_record_s * record);

/**
* @fn    int  mb_svc_load_record_image_meta(sqlite3_stmt* stmt, mb_svc_image_meta_record_s * record);
* This function gets folder record by path information
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                   record      pointer to image_meta record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int mb_svc_load_record_image_meta(sqlite3_stmt* stmt, mb_svc_image_meta_record_s * record);

/**
* @fn    int  mb_svc_insert_record_tag(mb_svc_tag_record_s *record);
* This function inserts tag record which should be filled up the corresponding value
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    record        pointer to tag record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_insert_record_tag(mb_svc_tag_record_s *record);

/**
* @fn    int  mb_svc_insert_record_tag_map(const char *media_id, int tag_id);
* This function inserts tag_map record which should be filled up the corresponding value
*
* @return                       This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id       
* @param[in]                    tag_id        
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_insert_record_tag_map(const char *media_id, int tag_id);

/**
* @fn    int  mb_svc_load_record_tag(sqlite3_stmt* stmt, mb_svc_tag_record_s * record);
* This function gets tag record by tag name, if any, or media_id
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                   record      pointer to tag record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_load_record_tag(sqlite3_stmt* stmt, mb_svc_tag_record_s * record);

/**
* @fn    int  mb_svc_load_record_tagmap(sqlite3_stmt* stmt, mb_svc_tag_record_s * record);
* This function gets tag record by tag name, if any, or media_id
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    stmt        sql stmt
* @param[out]                   record      pointer to tag record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/
int 
mb_svc_load_record_tagmap(sqlite3_stmt* stmt, mb_svc_tag_record_s * record);

/**
* @fn    int  mb_svc_update_thumb_path_by_id(const char *media_id, const char* thumb_path);
* This function updates thumb path for media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id           media id
* @param[in]                    thumb_path  new thumb path
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int mb_svc_update_thumb_path_by_id(const char *media_id, const char* thumb_path);

/**
* @fn    int  mb_svc_delete_record_media(const char *folder_id, char* display_name);
* This function deletes media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    folder_id           folder id in media record field
* @param[in]                    display_name  file name
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_delete_record_media(const char *folder_id, char* display_name);


/**
* @fn    int mb_svc_delete_bookmark_meta_by_media_id(const char *media_id , mb_svc_media_type_t file_type);
* This function deletes media related record in other tables
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id           media id
* @param[in]                    file_type   media file type, maybe image,video and so on
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_delete_bookmark_meta_by_media_id(const char *media_id , minfo_file_type content_type);

/**
* @fn    int  mb_svc_update_favorite_by_id(const char *media_id, int favorite);
* This function updates favorite field in media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id           media id
* @param[in]                    favorite  favorite field in media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/


int 
mb_svc_update_favorite_by_id(const char *media_id, int favorite);

/**
* @fn    int mb_svc_delete_record_image_meta_by_id(int id);
* This function gets folder record by path information
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    id          image_meta record id
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_delete_record_image_meta_by_id(int id);


/**
* @fn    int  mb_svc_update_date_by_id(const char *media_id, int date);
* This function updates favorite field in media record
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    media_id           media id
* @param[in]                    date  modified date in media record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_update_date_by_id(const char *media_id, int date );

/**
* @fn    int  mb_svc_get_web_streaming_record_by_id(int webstreaming_id, mb_svc_web_streaming_record_s *webstreaming_record);
* This function gets webstreaming record by id
*
* @return                        This function returns 0 on success, and -1 on failure.
* @param[in]                    webstreaming_id           web streaming record id
* @param[out]                   webstreaming_record      pointer to webstreaming record
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/											 
int
mb_svc_get_web_streaming_record_by_id(int webstreaming_id,
										mb_svc_web_streaming_record_s *webstreaming_record);
int
mb_svc_set_media_records_as_valid(const minfo_store_type storage_type, int valid);

int
mb_svc_update_album_lock_status(const char *folder_id, int lock, minfo_store_type storage_type );

int
mb_svc_load_record_folder_name(sqlite3_stmt* stmt, char * folder_name, int max_length);

int
mb_svc_update_record_folder_path(char* old_path, char* new_path);

int
mb_svc_update_folder_modified_date(char* path, int date);

int 
mb_svc_update_width_and_height(const char *media_id, minfo_file_type content_type, int width, int height);

int
mb_svc_set_folder_as_valid_sql_add(const char *folder_id, int valid);

int
mb_svc_set_item_as_valid_sql_add(const char *full_path, int valid);

int
mb_svc_set_item_as_valid();

int
mb_svc_insert_record_folder_sql(mb_svc_folder_record_s *record, char **sql);

int
mb_svc_insert_record_image_meta_sql(mb_svc_image_meta_record_s *record,
				minfo_store_type storage_type,
				char **sql);

int
mb_svc_insert_record_media_sql(mb_svc_media_record_s *record,
					minfo_store_type storage_type,
					char **sql);

int
mb_svc_insert_record_video_meta_sql(mb_svc_video_meta_record_s *record,
				minfo_store_type storage_type,
				char **sql);

int
mb_svc_update_record_folder_sql(mb_svc_folder_record_s *record, char **sql);

int
mb_svc_update_record_media_sql(mb_svc_media_record_s *record, char **sql);

int
mb_svc_update_thumb_path_sql(const char *media_id, const char *thumb_path, char **sql);

int 
mb_svc_delete_record_folder_sql(const char *folder_id, char **sql);

int
mb_svc_delete_record_media_sql(const char *media_id, char **sql);

int
mb_svc_delete_record_image_meta_sql(const char *media_id, char **sql);

int
mb_svc_delete_record_video_meta_sql(const char *media_id, char **sql);

int
mb_svc_delete_tagmap_by_media_id_sql(const char *media_id, char **sql);

int
mb_svc_delete_bookmark_meta_by_media_id_sql(const char *media_id, char **sql);

int
mb_svc_update_width_and_height_sql(const char *media_id, minfo_file_type content_type,
			       int width, int height, char **sql);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_MEDIA_SVC_DB_H_*/

/**
* @}
*/
