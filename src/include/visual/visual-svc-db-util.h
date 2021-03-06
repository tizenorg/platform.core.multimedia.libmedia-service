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
 * @file       	visual-svc-db-util.h
 * @author 		Hyunjun Ko <zzoon.ko@samsung.com>
 * @version 	1.0
 * @brief     	This file defines in-house apis for media service.
 */

 
 /**
  * @ingroup MEDIA_SVC
  * @defgroup MEDIA_SVC_DB_UTIL in-house media service API
  * @{
  */


#ifndef _MEDIA_SVC_DB_UTIL_H_
#define _MEDIA_SVC_DB_UTIL_H_
	
#include <glib.h>
#include "media-svc-types.h"
	
#ifdef __cplusplus
extern "C" {
#endif

/**
*  query strings
*/
#define MB_SVC_TABLE_EXIST_QUERY_STRING  "select name from sqlite_master where name='%s';"
#define MB_SVC_TABLE_DROP_QUERY_STRING   "DROP TABLE %s;"
#define MB_SVC_RECORD_DELETE_QUERY_STRING   "DELETE FROM %s WHERE _id = %d;"
#define MB_SVC_TABLE_DELETE_QUERY_STRING  "DELETE FROM %s ;"
#define MB_SVC_TABLE_COUNT_QUERY_STRING  "SELECT count(*) FROM %s"
#define MB_SVC_TABLE_COUNT_BY_PATH_QUERY_STRING  "SELECT count(*) FROM %s WHERE path='%q'"

#define MB_SVC_TABLE_SELECT_BOOKMARK_BY_BID_QUERY_STRING   "SELECT _id, visual_uuid, marked_time, thumbnail_path FROM %s WHERE _id=%d;"
#define MB_SVC_TABLE_SELECT_WEB_CLUSTER_RECORD_QUERY_STRING   "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s WHERE sns_type = %d AND folder_name = '%q' AND web_account_id = '%q';"
#define MB_SVC_TABLE_SELECT_WEB_ALBUM_CLUSTER_RECORD_QUERY_STRING   "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s WHERE sns_type = %d AND folder_name = '%q' AND web_account_id = '%q' AND web_album_id = '%q';"
#define MB_SVC_TABLE_SELECT_MEDIA_ID_BY_HTTP_URL "SELECT visual_uuid FROM %s WHERE http_url = '%q';"
#define MB_SVC_UPDATE_FOLDER_PATH_TABLE "UPDATE %s SET path = REPLACE( path, '%q', '%q');"
#define MB_SVC_UPDATE_FOLDER_MODIFIED_DATE_TABLE "UPDATE %s SET modified_date = %d where path like '%q';"
#define MB_SVC_TABLE_SELECT_WEBSTREAMING_RECORD_BY_ID  "SELECT _id, title, duration, url, thumb_path FROM %s WHERE _id=%d;"

#define MB_SVC_TABLE_SELECT_TAG_BY_TID_QUERY_STRING   "SELECT _id, tag_name FROM %s WHERE _id=%d;"
#define MB_SVC_TABLE_UPDATE_TAG_NAME_QUERY_STRING  "UPDATE %s SET tag_name='%s' WHERE tag_name='%s';"
#define MB_SVC_TABLE_UPDATE_TAG_NAME_QUERY_STRING_BY_TAG_ID  "UPDATE %s SET tag_name='%q' WHERE _id=%d;"
#define MB_SVC_TABLE_UPDATE_TAG_MAP_QUERY_STRING_BY_TAG_ID  "UPDATE %s SET tag_id=%d WHERE tag_id=%d;"
#define MB_SVC_TABLE_SELECT_TAG_COUNT_QUERY_STRING "SELECT _id from %s where tag_name='%q'"

#define MB_SVC_TABLE_DELETE_TAG_MAP_BY_TAGNAME "DELETE FROM %s WHERE tag_id=%d"
#define MB_SVC_TABLE_DELETE_TAG_BY_TAGID "DELETE FROM %s WHERE _id=%d"
#define MB_SVC_TABLE_SELECT_MEDIA_CNT_BY_TAGID "SELECT count(*) FROM %s WHERE tag_id=%d"

/* SELECT Query for uuid */
#define MB_SVC_TABLE_SELECT_MEDIA_BY_MEDIA_UUID  "SELECT visual_uuid, path, folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size FROM %s WHERE visual_uuid = '%s' AND valid = 1;"
#define MB_SVC_TABLE_SELECT_FOLDER_RECORD_BY_UUID  "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s WHERE folder_uuid = '%s';"
#define MB_SVC_TABLE_SELECT_VIDEO_BY_MUUID  "SELECT _id, visual_uuid, album, artist, title, genre, description, youtube_category, last_played_time, duration, longitude, latitude, width, height, datetaken FROM %s WHERE visual_uuid = '%s';"
#define MB_SVC_TABLE_SELECT_IMAGE_BY_MUUID  "SELECT _id, visual_uuid, longitude, latitude, description, width, height, orientation, datetaken FROM %s WHERE visual_uuid='%s';"
#define MB_SVC_TABLE_SELECT_BOOKMARK_ALL_BY_MUUID  "SELECT _id, visual_uuid, marked_time, thumbnail_path FROM %s where visual_uuid = '%s';"
#define MB_SVC_TABLE_SELECT_MEDIA_BY_PATH  "SELECT visual_uuid, path, folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size FROM %s WHERE path = '%q';"
#define MB_SVC_FOLDER_CONTENT_COUNT_BY_FUUID   "SELECT count(*) FROM %s WHERE folder_uuid = '%s'"
#define MB_SVC_SELECT_MEDIA_RECORD_BY_FOLDER_UUID_AND_DISPLAY_NAME   "SELECT visual_uuid, path, folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size FROM %s WHERE folder_uuid = '%s' AND display_name = '%q';"
#define MB_SVC_TABLE_SELECT_FOLDER_URI_BY_FUUID  "SELECT path, folder_name, storage_type FROM %s WHERE folder_uuid = '%s';"
#define MB_SVC_TABLE_SELECT_FOLDER_BY_PATH_INFO   "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s WHERE path = '%q' AND folder_name = '%q' AND storage_type = %d;"
#define MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_WEB_ALBUM_ID_QUERY_STRING   "SELECT folder_uuid FROM %s WHERE web_album_id = '%q';"
#define MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_WEB_STREAMING  "SELECT folder_uuid FROM %s WHERE storage_type = %d;"
#define MB_SVC_TABLE_SELECT_FOLDER_UUID_BY_PATH_INFO   "SELECT folder_uuid FROM %s WHERE path = '%q' AND folder_name = '%q' AND storage_type = %d;"
#define MB_SVC_SELECT_MEDIA_UUID_BY_FOLDER_UUID_AND_DISPLAY_NAME   "SELECT visual_uuid FROM %s WHERE folder_uuid = '%s' AND display_name = '%q';"
#define MB_SVC_TABLE_SELECT_FOLDER_NAME_BY_UUID  "SELECT folder_name FROM %s WHERE folder_uuid = '%s';"
//#define MB_SVC_SELECT_ALL_ITEM_COUNT "select count(*) from %s as m INNER JOIN visual_folder AS f ON m.folder_uuid = f.folder_uuid and f.lock_status=0 and m.valid=1 and f.storage_type!=2;"
#define MB_SVC_SELECT_ALL_ITEM_COUNT "select count(*) from %s as m INNER JOIN visual_folder AS f ON m.folder_uuid = f.folder_uuid and f.lock_status=0 and m.valid=1 and %s %s %s;"
#define MB_SVC_TABLE_SELECT_FOLDER_NAME_BY_UUID  "SELECT folder_name FROM %s WHERE folder_uuid = '%s';"
#define MB_SVC_TABLE_SELECT_FOLDER_ALL_QUERY_STRING  "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s "
#define MB_SVC_SELECT_MEDIA_RECORD_BY_FOLDER_ID_AND_DISPLAY_NAME   "SELECT visual_uuid, path, folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size FROM %s WHERE folder_uuid = '%s' AND display_name = '%q';"
#define MB_SVC_SELECT_ALL_MEDIA  "SELECT m.visual_uuid, m.path, m.folder_uuid, display_name, content_type, rating, m.modified_date, thumbnail_path, http_url, size FROM %s as m INNER JOIN visual_folder AS f ON "
#define MB_SVC_SELECT_FOLDER_BY_PATH "SELECT folder_uuid, path, folder_name, modified_date, web_account_id, storage_type, sns_type, lock_status, web_album_id FROM %s where valid=1 and path='%q' or path like '%q';"
#define MB_SVC_SELECT_MEDIA_ID_BY_FOLDER_UUID_AND_DISPLAY_NAME   "SELECT visual_uuid FROM %s WHERE folder_uuid = '%s' AND display_name = '%q';"
#define MB_SVC_SELECT_INVALID_MEDIA_LIST "SELECT m.visual_uuid, m.path, m.folder_uuid, display_name, content_type, rating, m.modified_date, thumbnail_path, http_url, size FROM visual_media as m INNER JOIN visual_folder AS f ON   f.folder_uuid = m.folder_uuid and m.valid=0 and f.storage_type=%d"
#define MB_SVC_SELECT_ALL_MEDIA_LIST_BY_STORAGE "SELECT m.visual_uuid, m.path, m.folder_uuid, display_name, content_type, rating, m.modified_date, thumbnail_path, http_url, size FROM visual_media as m INNER JOIN visual_folder AS f ON f.folder_uuid = m.folder_uuid and f.storage_type=%d"

#define MB_SVC_TABLE_SELECT_GEO_LIST  "SELECT a.visual_uuid, path, a.folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size from ( SELECT m.visual_uuid, path, m.folder_uuid, display_name, content_type, rating, modified_date, thumbnail_path, http_url, size, valid FROM %s AS m INNER JOIN ( SELECT visual_uuid from image_meta where longitude between %f and %f and latitude between %f and %f union select visual_uuid from video_meta where longitude between %f and %f and latitude between %f and %f) AS meta ON meta.visual_uuid = m.visual_uuid ) a, (select folder_uuid, lock_status from visual_folder where valid=1 %s ) b where a.folder_uuid = b.folder_uuid and a.valid=1 "

#define MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_TAG_NAME_WITH_LOCK_STATUS "SELECT t._id, tm.visual_uuid from ( select _id, tag_name from visual_tag WHERE tag_name='%s' ORDER BY tag_name ASC  ) t, ( select visual_uuid, tag_id from visual_tag_map ) tm, ( select visual_uuid, folder_uuid from visual_media) m, ( select folder_uuid, lock_status from visual_folder where valid=1 ) f where tm.tag_id = t._id and m.visual_uuid = tm.visual_uuid and m.folder_uuid = f.folder_uuid and f.lock_status=%d;"

#define MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_WITH_LOCK_STATUS "select t._id, t.tag_name from ( select _id, tag_name from visual_tag ORDER BY tag_name ASC ) t, ( select visual_uuid, tag_id from visual_tag_map ) tm, ( select visual_uuid, folder_uuid from visual_media) m, ( select folder_uuid, lock_status from visual_folder where valid=1 ) f where tm.tag_id = t._id and m.visual_uuid = tm.visual_uuid and m.folder_uuid = f.folder_uuid and f.lock_status=%d UNION SELECT _id, tag_name from visual_tag ORDER BY tag_name ASC;"

#define MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_MEDIA_ID_WITH_LOCK_STATUS  "select t._id, t.tag_name from ( select _id, tag_name from visual_tag ORDER BY tag_name ASC ) t, ( select visual_uuid, tag_id from visual_tag_map where visual_uuid='%s') tm, ( select visual_uuid, folder_uuid from visual_media) m, ( select folder_uuid, lock_status from visual_folder where valid=1 ) f where tm.tag_id = t._id and m.visual_uuid = tm.visual_uuid and m.folder_uuid = f.folder_uuid and f.lock_status=%d;"

#define MB_SVC_TABLE_SELECT_TAG_ALL_QUERY_STRING_BY_TAG_NAME_WITH_LOCK_STATUS_AND_FILTER "SELECT t._id, tm.visual_uuid from ( select _id, tag_name from visual_tag WHERE tag_name='%s' ORDER BY tag_name ASC  ) t, ( select visual_uuid, tag_id from visual_tag_map ) tm, ( select visual_uuid, folder_uuid, content_type from visual_media) m, ( select folder_uuid, lock_status from visual_folder where valid=1 ) f where tm.tag_id = t._id and m.visual_uuid = tm.visual_uuid and m.folder_uuid = f.folder_uuid and f.lock_status=%d %s ;"

#define MB_SVC_TABLE_SELECT_COUNT_BY_TAG_NAME_WITH_LOCK_STATUS "SELECT count(*) from ( select _id, tag_name from visual_tag WHERE tag_name='%s' ORDER BY tag_name ASC  ) t, ( select visual_uuid, tag_id from visual_tag_map ) tm, ( select visual_uuid, folder_uuid from visual_media) m, ( select folder_uuid, lock_status from visual_folder where valid=1 ) f where tm.tag_id = t._id and m.visual_uuid = tm.visual_uuid and m.folder_uuid = f.folder_uuid and f.lock_status=%d;"

/* UPDATE Query for uuid */
#define MB_SVC_UPDATE_VIDEO_META_TABLE "UPDATE %s SET %s='%q', %s='%q', %s='%q', %s='%q', %s='%q', %s='%q', %s='%q', %s=%d, %s=%d, %s=%f, %s=%f, %s=%d, %s=%d, %s=%d WHERE _id = %d;"
#define MB_SVC_UPDATE_IMAGE_META_TABLE "UPDATE %s SET %s='%q', %s=%f, %s=%f, %s='%q', %s=%d, %s=%d, %s=%d, %s=%d WHERE _id = %d;"
#define MB_SVC_UPDATE_MEDIA_TABLE "UPDATE %s SET %s='%q', %s='%q', %s='%q', %s=%d, %s=%d, %s=%d, %s='%q', %s='%q', %s=%d WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_MEDIA_THUMB_PATH   "UPDATE %s SET thumbnail_path = '%q' WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_MEDIA_FAVORITE_BY_ID   "UPDATE %s SET rating = %d WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_MEDIA_DATE_BY_ID   "UPDATE %s SET modified_date = %d WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_MEDIA_ORIENTATION_BY_ID   "UPDATE %s SET orientation = %d WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_FOLDER_ALBUM_STATUS   "UPDATE %s SET lock_status = ? WHERE folder_uuid = ?;"
#define MB_SVC_UPDATE_MEDIA_VALID_BY_UUID   "UPDATE %s SET valid = %d WHERE visual_uuid = '%s';"
#define MB_SVC_UPDATE_FOLDER_VALID_BY_UUID   "UPDATE %s SET valid = %d WHERE folder_uuid = '%s';"
#define MB_SVC_UPDATE_FOLDER_TABLE "UPDATE %s SET %s='%q', %s='%q', %s=%d, %s='%q', %s=%d, %s=%d, %s=%d WHERE folder_uuid = '%s';"
#define MB_SVC_UPDATE_META_WIDTH_HEIGHT "UPDATE %s SET %s=%d, %s=%d WHERE visual_uuid = '%s';"
#define MB_SVC_TABLE_UPDATE_TAG_MAP_QUERY_STRING_BY_TAG_ID_AND_MEDIA_ID  "UPDATE %s SET tag_id=%d WHERE visual_uuid='%s' and tag_id=%d;"

/* INSERT Query for uuid */
#define MB_SVC_INSERT_FOLDER_TABLE "INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s) VALUES ('%q', '%q', '%q', %d, '%q', %d, %d, %d, '%q', %d );"
#define MB_SVC_INSERT_IMAGE_META_TABLE "INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s) VALUES ( '%q', %f, %f, '%q', %d, %d, %d, %d );"
#define MB_SVC_INSERT_MEDIA_TABLE "INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s ) VALUES ( '%q', '%q', '%q', '%q', %d, %d, %d, '%q', '%q', %d, %d );"
#define MB_SVC_INSERT_TAG_TABLE "INSERT INTO %s (%s, %s) VALUES ( %d, '%q' );"
#define MB_SVC_INSERT_TAG_MAP_TABLE "INSERT INTO %s (%s, %s, %s) VALUES ( %d, '%q', %d );"
#define MB_SVC_INSERT_VIDEO_META_TABLE "INSERT INTO %s (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s) VALUES ( '%q', '%q', '%q','%q', '%q', '%q','%q', %d, %d, %f, %f, %d, %d, %d );"
#define MB_SVC_INSERT_BOOKMARK_TABLE "INSERT INTO %s (%s, %s, %s) VALUES ( '%q', %d,'%q' );"
#define MB_SVC_INSERT_WEB_STREAMING_TABLE "INSERT INTO %s (%s, %s, %s, %s, %s, %s) VALUES ( %d, '%s', '%q', %d, '%q'. '%q' );"

/* DELETE Query for uuid */
#define MB_SVC_RECORD_FOLDER_DELETE_BY_UUID   "DELETE FROM %s WHERE folder_uuid = '%s';"
#define MB_SVC_RECORD_MEDIA_DELETE_BY_UUID   "DELETE FROM %s WHERE visual_uuid = '%s';"
#define MB_SVC_DELETE_MEDIA_RELATED_INFO_BY_MEDIA_UUID   "DELETE FROM %s WHERE visual_uuid = '%s' ;"
#define MB_SVC_DELETE_MEDIA_BY_FOLDER_ID_AND_DISPLAY_NAME   "DELETE FROM %s WHERE folder_uuid = '%s' AND display_name = '%q';"
#define MB_SVC_TABLE_DELETE_TAG_MAP_BY_TAGNAME_MEDIA_UUID "DELETE FROM %s WHERE visual_uuid='%s' and tag_id=%d"
#define MB_SVC_TABLE_DELETE_TAG_MAP_BY_MEDIA_UUID "DELETE FROM %s WHERE visual_uuid='%s'"

/**
* @fn    int  mb_svc_query_sql_gstring(MediaSvcHandle *mb_svc_handle, GString *query_string);
* This function executes the inputted GString sql statement
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_handle          Handle for media service
* @param[in]                    query_string           pointer to GString sql statement
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_query_sql_gstring(MediaSvcHandle *mb_svc_handle, GString *query_string);


/**
* @fn    int  mb_svc_query_sql(MediaSvcHandle *mb_svc_handle, char *query_str);
* This function executes inputted sql string
*
* @return                       This function returns 0 on success, and negative value on failure.
* @param[in]                    mb_svc_handle          Handle for media service
* @param[in]                    query_str           pointer to sql string
* @exception                    None.
* @remark                        
*                                                             
*                                                          
*/

int 
mb_svc_query_sql(MediaSvcHandle *mb_svc_handle, char *query_str);

int 
mb_svc_sqlite3_begin_trans(MediaSvcHandle *mb_svc_handle);

int 
mb_svc_sqlite3_commit_trans(MediaSvcHandle *mb_svc_handle);

int 
mb_svc_sqlite3_rollback_trans(MediaSvcHandle *mb_svc_handle);

void
mb_svc_sql_list_add(GList **list, char **sql);

void
mb_svc_sql_list_release(GList **sql_list);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_MEDIA_SVC_DB_UTIL_H_*/

/**
* @}
*/


