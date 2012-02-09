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

#include <audio-svc.h>
#include <media-svc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

void insert_into_db(const char * dir_path);
void msg_print(int line, char *msg);

int main()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	char * audio_id = NULL;
	int size = 0;
	int idx=0, j;
	int rows = -1;
	AudioHandleType *tracks = NULL;
	AudioHandleType *item = NULL;
	AudioHandleType  *groups = NULL;
	MediaSvcHandle * db_handle = NULL;
	
	//db open ==================================================
	ret = media_svc_connect(&db_handle);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to open music database");
		return -1;
	}
#if 0
	//create table test ==================================================
	ret = audio_svc_create_table();
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to create table");
		return -1;
	}
#endif
#if 0
	//insert music files to db ==================================================
	ret = audio_svc_delete_all(AUDIO_SVC_STORAGE_PHONE);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to delete all items on phone");
		return -1;
	}
	//insert_into_db("/opt/media/Sounds/Music");
#endif
	//iterate all tracks and get the info of tracks ==================================================
	msg_print(__LINE__, "iterate all tracks");
	ret = audio_svc_count_list_item(db_handle, AUDIO_SVC_TRACK_ALL, "", "", "", "", &rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to delete all items on phone");
		return -1;
	}

	if(rows < 1) {
		msg_print(__LINE__, "there is no item");
		return -1;
	}
	else
		fprintf(stderr, "rows = [%d]\n", rows);
	
	ret = audio_svc_list_item_new(&tracks, rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to alloc memory for list item");
		return -1;
	}
	
	ret = audio_svc_get_list_item(db_handle, AUDIO_SVC_TRACK_ALL, //item_type,
		NULL, //type_string,
		NULL, //type_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		rows, //rows,
		tracks
		);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to get list item");
		return -1;
	}
	
	ret = audio_svc_item_new(&item);
	for (idx = 0; idx < rows; idx++)
	{
		ret = audio_svc_list_item_get_val(tracks, idx, AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size, -1);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_get_val");
			return -1;
		}
		fprintf(stderr, "[audio_id] = %s \n", audio_id);
		ret = audio_svc_get_item_by_audio_id(db_handle, audio_id, item);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error to get item by audio_id");
			return -1;
		}
		
		{
			char *audio_id = NULL, *path = NULL, *title = NULL, *album = NULL, *artist = NULL, *thumbname = NULL, *year = NULL;
			char thumb_path[AUDIO_SVC_PATHNAME_SIZE] = {0};
			int size = -1;
			ret = audio_svc_item_get_val(item,
				AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size,
				AUDIO_SVC_TRACK_DATA_PATHNAME, &path, &size,
				AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
				AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
				AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
				AUDIO_SVC_TRACK_DATA_THUMBNAIL_PATH, &thumbname, &size,
				AUDIO_SVC_TRACK_DATA_YEAR, &year, &size,
				-1);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_item_get_val");
				return -1;
			}
			fprintf(stderr, "**** ITEM INFO[%d] ****\n", idx);
			fprintf(stderr, "	**audio_id = %s\n", audio_id);
			fprintf(stderr, "	**path = %s\n", path);
			fprintf(stderr, "	**title = %s\n", title);
			fprintf(stderr, "	**album = %s\n", album);
			fprintf(stderr, "	**artist = %s\n", artist);
			fprintf(stderr, "	**thumb = %s\n", thumbname);
			fprintf(stderr, "	**album = %s\n", album);
			fprintf(stderr, "	**year = %s\n", year);
			

			ret = audio_svc_get_thumbnail_path_by_path(db_handle, path, thumb_path, AUDIO_SVC_PATHNAME_SIZE);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_item_get_val");
				return -1;
			}
			fprintf(stderr, "	**thumb_path = %s\n\n", thumb_path);
		}

	}
	
	ret = audio_svc_item_free(item);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_item_free");
		return -1;
	}
	ret = audio_svc_list_item_free(tracks);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_list_item_free");
		return -1;
	}

#if 0
	ret = audio_svc_update_item_metadata(audio_id, 
			AUDIO_SVC_TRACK_DATA_PLAYED_COUNT, 5,
			AUDIO_SVC_TRACK_DATA_PLAYED_TIME, 5,
			AUDIO_SVC_TRACK_DATA_ADDED_TIME, 5,
			AUDIO_SVC_TRACK_DATA_RATING,	 AUDIO_SVC_RATING_5,
			AUDIO_SVC_TRACK_DATA_TITLE, "Test title", strlen("Test title"),
			AUDIO_SVC_TRACK_DATA_ARTIST, NULL, 0,
			AUDIO_SVC_TRACK_DATA_ALBUM, "Test album", strlen("Test album"),
			AUDIO_SVC_TRACK_DATA_GENRE, "Test genre", strlen("Test genre"),
			AUDIO_SVC_TRACK_DATA_AUTHOR, "Test author", strlen("Test author"),
			AUDIO_SVC_TRACK_DATA_DESCRIPTION, "Test description", strlen("Test description"),
			AUDIO_SVC_TRACK_DATA_YEAR, 2011,
			AUDIO_SVC_TRACK_DATA_TRACK_NUM, 10,
			AUDIO_SVC_TRACK_DATA_ALBUM_RATING, AUDIO_SVC_RATING_5,
			-1);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_update_item_metadata");
		return -1;
	}
#endif
	//iterate all albums and its tracks ==================================================
	msg_print(__LINE__, "iterate all albums and its tracks");

	ret = audio_svc_count_group_item(db_handle, AUDIO_SVC_GROUP_BY_ALBUM, NULL, NULL, NULL, NULL, &rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_count_group_item");
		return -1;
	}
	if(rows < 1) {
		msg_print(__LINE__, "there is no group item");
		return -1;
	}
	else
		fprintf(stderr, "rows = [%d]\n", rows);
	
	ret = audio_svc_group_item_new(&groups, rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_list_item_new");
		return -1;
	}
	
	ret = audio_svc_get_group_item(db_handle, AUDIO_SVC_GROUP_BY_ALBUM, //group_type,
		NULL, //limit_string1,
		NULL, //limit_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		rows, //rows,
		groups);

	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_get_group_item");
		return -1;
	}
	
	for (idx = 0; idx < rows; idx++) {
		char *main_info = NULL, *sub_info = NULL, *thumbname = NULL;
		int album_rating = 0;
		int size = 0;
		int count = 0;
		
		ret = audio_svc_group_item_get_val(groups, idx , 
			AUDIO_SVC_GROUP_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_GROUP_ITEM_MAIN_INFO, &main_info, &size,
			AUDIO_SVC_GROUP_ITEM_SUB_INFO, &sub_info, &size,
			AUDIO_SVC_GROUP_ITEM_RATING, &album_rating,
			-1);

		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_group_item_get_val");
			return -1;
		}
		
		fprintf(stderr, "**** GROUP INFO[%d] ****\n", idx);
		fprintf(stderr, "	**main_info = %s\n", main_info);
		fprintf(stderr, "	**sub_info = %s\n", sub_info);
		fprintf(stderr, "	**thumbname = %s\n", thumbname);
		fprintf(stderr, "	**album_rating = %d\n\n", album_rating);
		
		//iterate tracks of albums ==================================================
		ret = audio_svc_count_list_item(db_handle, AUDIO_SVC_TRACK_BY_ALBUM, main_info, "", "", "", &count);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_count_list_item");
			return -1;
		}
		if(count < 1) {
			msg_print(__LINE__, "there is no item");
			return -1;
		}
		else
			fprintf(stderr, "	rows = [%d]\n", count);

		ret = audio_svc_list_item_new(&tracks, count);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_new");
			return -1;
		}

		ret = audio_svc_get_list_item(db_handle, AUDIO_SVC_TRACK_BY_ALBUM, main_info, NULL, NULL, NULL, 0, count, tracks);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_new");
			ret = audio_svc_group_item_free(groups);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_list_item_free");
				return -1;
			}
			return -1;
		}
		
		for (j = 0; j < count; j++)
		{
			char *audio_id = NULL, *title = NULL, *artist = NULL, *thumbname = NULL, *pathname = NULL;
			int rating = 0;
			int duration = 0;
			int size = 0;
						
			ret = audio_svc_list_item_get_val(tracks, j , 
				AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size,
				AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH, &thumbname, &size,
				AUDIO_SVC_LIST_ITEM_TITLE, &title, &size,
				AUDIO_SVC_LIST_ITEM_ARTIST, &artist, &size,
				AUDIO_SVC_LIST_ITEM_PATHNAME, &pathname, &size,
				AUDIO_SVC_LIST_ITEM_DURATION, &duration,
				AUDIO_SVC_LIST_ITEM_RATING, &rating,
				-1);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_list_item_get_val");
				ret = audio_svc_list_item_free(tracks);
				if (ret != AUDIO_SVC_ERROR_NONE) {
					msg_print(__LINE__, "error audio_svc_list_item_free");
					return -1;
				}
				return -1;
			}

			fprintf(stderr, "	**audio_id = %s\n", audio_id);
			fprintf(stderr, "	**thumbnail_path = %s\n", thumbname);
			fprintf(stderr, "	**title = %s\n", title);
			fprintf(stderr, "	**artist = %s\n", artist);
			fprintf(stderr, "	**path = %s\n", pathname);
			fprintf(stderr, "	**duration = %d\n", duration);
			fprintf(stderr, "	**rating = %d\n\n", rating);
		}
		
		ret = audio_svc_list_item_free(tracks);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_free");
			return -1;
		}

	}
	
	ret = audio_svc_group_item_free(groups);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_list_item_free");
		return -1;
	}


	//iterate all Folder and its tracks ==================================================
	msg_print(__LINE__, "iterate all Folder and its tracks");
	//AudioHandleType  *groups = NULL;
	ret = audio_svc_count_group_item(db_handle, AUDIO_SVC_GROUP_BY_FOLDER, NULL, NULL, NULL, NULL, &rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_count_group_item");
		return -1;
	}
	if(rows < 1) {
		msg_print(__LINE__, "there is no group item");
		return -1;
	}
	else
		fprintf(stderr, "rows = [%d]\n", rows);
	
	ret = audio_svc_group_item_new(&groups, rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_group_item_new");
		return -1;
	}
	
	ret = audio_svc_get_group_item(db_handle, AUDIO_SVC_GROUP_BY_FOLDER, //group_type,
		NULL, //limit_string1,
		NULL, //limit_string2,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		rows, //rows,
		groups);
	if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_get_group_item of get folder");
			ret = audio_svc_list_item_free(tracks);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_list_item_free");
				return -1;
			}
			return -1;
	}
	
	for (idx = 0; idx < rows; idx++) {
		char *main_info = NULL, *sub_info = NULL, *thumbname = NULL;
		int rating = 0;
		int size = 0;
		int count = 0;

		ret = audio_svc_group_item_get_val(groups, idx , 
			AUDIO_SVC_GROUP_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_GROUP_ITEM_MAIN_INFO, &main_info, &size,
			AUDIO_SVC_GROUP_ITEM_SUB_INFO, &sub_info, &size,
			AUDIO_SVC_GROUP_ITEM_RATING, &rating,
			-1);

		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_get_val of get folder");
			ret = audio_svc_list_item_free(tracks);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_list_item_free");
				return -1;
			}
			return -1;
		}
		
		fprintf(stderr, "**** FOLDER INFO[%d] ****\n", idx);
		fprintf(stderr, "	**main_info = %s\n", main_info);
		fprintf(stderr, "	**sub_info = %s\n", sub_info);
		fprintf(stderr, "	**thumbname = %s\n", thumbname);
		fprintf(stderr, "	**rating = %d\n\n", rating);
		
		//iterate tracks of albums ==================================================
		ret = audio_svc_count_list_item(db_handle, AUDIO_SVC_TRACK_BY_FOLDER, sub_info, "", "", "", &count);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_count_list_item");
			return -1;
		}
		if(count < 1) {
			msg_print(__LINE__, "there is no item");
			return -1;
		}
		else
			fprintf(stderr, "	rows = [%d]\n\n", count);

		ret = audio_svc_list_item_new(&tracks, count);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_new");
			return -1;
		}

		ret = audio_svc_get_list_item(db_handle, AUDIO_SVC_TRACK_BY_FOLDER, sub_info, NULL, NULL, NULL, 0, count, tracks);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_new");
			return -1;
		}
		
		for (j = 0; j < count; j++)
		{
			char *audio_id = NULL, *title = NULL, *artist = NULL, *thumbname = NULL, *pathname = NULL;
			int rating = 0;
			int duration = 0;
			int size = 0;
						
			ret = audio_svc_list_item_get_val(tracks, j , 
				AUDIO_SVC_LIST_ITEM_AUDIO_ID, &audio_id, &size,
				AUDIO_SVC_LIST_ITEM_THUMBNAIL_PATH, &thumbname, &size,
				AUDIO_SVC_LIST_ITEM_TITLE, &title, &size,
				AUDIO_SVC_LIST_ITEM_ARTIST, &artist, &size,
				AUDIO_SVC_LIST_ITEM_PATHNAME, &pathname, &size,
				AUDIO_SVC_LIST_ITEM_DURATION, &duration,
				AUDIO_SVC_LIST_ITEM_RATING, &rating,
				-1);
			if (ret != AUDIO_SVC_ERROR_NONE) {
				msg_print(__LINE__, "error audio_svc_list_item_get_val");
				ret = audio_svc_list_item_free(tracks);
				if (ret != AUDIO_SVC_ERROR_NONE) {
					msg_print(__LINE__, "error audio_svc_list_item_free");
					return -1;
				}
				return -1;
			}

			fprintf(stderr, "	**audio_id = %s\n", audio_id);
			fprintf(stderr, "	**thumbnail_path = %s\n", thumbname);
			fprintf(stderr, "	**title = %s\n", title);
			fprintf(stderr, "	**artist = %s\n", artist);
			fprintf(stderr, "	**path = %s\n", pathname);
			fprintf(stderr, "	**duration = %d\n", duration);
			fprintf(stderr, "	**rating = %d\n\n", rating);
		}
		
		ret = audio_svc_list_item_free(tracks);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_free");
			return -1;
		}

	}
	
	ret = audio_svc_group_item_free(groups);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_list_item_free");
		return -1;
	}

	//iterate all playlist and its tracks ==================================================
	int plst_idx = -1;
	int plst_count = -1;
	AudioHandleType*playlists = NULL;
	char plst_name[AUDIO_SVC_PLAYLIST_NAME_SIZE] = {0};

	ret = audio_svc_count_playlist(db_handle, NULL, NULL, &plst_count);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_count_playlist");
		return -1;
	}
	if(plst_count < 1) {
		msg_print(__LINE__, "there is no playlist");
		return -1;
	}
	else
		fprintf(stderr, "plst_count = [%d]\n", plst_count);
	
	ret = audio_svc_playlist_new(&playlists, plst_count);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_playlist_new");
		return -1;
	}

	ret = audio_svc_get_playlist(db_handle, 
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				plst_count, //rows
				playlists);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_playlist_new");
		return -1;
	}

	int plst_id = -1;
	for (idx = 0; idx < plst_count; idx++) {
		int size;
		char *playlist_name = NULL;
		char *playlist_thumbnail_path = NULL;
		
		audio_svc_playlist_get_val(playlists, idx, AUDIO_SVC_PLAYLIST_ID, &plst_id, AUDIO_SVC_PLAYLIST_NAME, &playlist_name, &size, AUDIO_SVC_PLAYLIST_THUMBNAIL_PATH, &playlist_thumbnail_path, &size,-1);
		fprintf(stderr, "**** Playlist Item Info****\n");
		fprintf(stderr, "** plst id: [%d], name: [%s], thumbnail_path: [%s]\n\n", plst_id, playlist_name, playlist_thumbnail_path);
	}

	ret = audio_svc_playlist_free(playlists);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_playlist_new");
		return -1;
	}

	ret = audio_svc_count_playlist_item(db_handle, plst_id, "", "", &rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_count_list_item");
		return -1;
	}

	if(rows < 1) {
		msg_print(__LINE__, "there is no item");
		return -1;
	}
	else
		fprintf(stderr, "rows = [%d]\n", rows);

	ret = audio_svc_playlist_item_new(&tracks, rows);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to alloc memory for list item");
		return -1;
	}
	
	ret = audio_svc_get_playlist_item(db_handle, 
		plst_id,
		NULL, //filter_string,
		NULL, //filter_string2,
		0, //offset,
		rows, //rows,
		tracks
		);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to get list item");
		return -1;
	}

	for (idx = 0; idx < rows; idx++) {
		char *audio_id = NULL, *title = NULL, *artist = NULL, *thumbname = NULL, *pathname = NULL;
		int uid = -1;
		int rating = 0;
		int duration = 0;
		int size = 0;
		int play_order = 0;	
					
		ret = audio_svc_playlist_item_get_val(tracks, idx , 
			AUDIO_SVC_PLAYLIST_ITEM_UID, &uid,
			AUDIO_SVC_PLAYLIST_ITEM_AUDIO_ID, &audio_id, &size,
			AUDIO_SVC_PLAYLIST_ITEM_THUMBNAIL_PATH, &thumbname, &size,
			AUDIO_SVC_PLAYLIST_ITEM_TITLE, &title, &size,
			AUDIO_SVC_PLAYLIST_ITEM_ARTIST, &artist, &size,
			AUDIO_SVC_PLAYLIST_ITEM_PATHNAME, &pathname, &size,
			AUDIO_SVC_PLAYLIST_ITEM_DURATION, &duration,
			AUDIO_SVC_PLAYLIST_ITEM_RATING, &rating,
			AUDIO_SVC_PLAYLIST_ITEM_PLAY_ORDER, &play_order,
			-1);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			msg_print(__LINE__, "error audio_svc_list_item_get_val");
			return -1;
		}

		fprintf(stderr, "	**uid = %d\n", uid);
		fprintf(stderr, "	**audio_id = %s\n", audio_id);
		fprintf(stderr, "	**thumbnail_path = %s\n", thumbname);
		fprintf(stderr, "	**title = %s\n", title);
		fprintf(stderr, "	**artist = %s\n", artist);
		fprintf(stderr, "	**path = %s\n", pathname);
		fprintf(stderr, "	**duration = %d\n", duration);
		fprintf(stderr, "	**rating = %d\n", rating);
		fprintf(stderr, "	**play_order = %d\n\n", play_order);
	}	

	ret = audio_svc_playlist_item_free(tracks);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_list_item_free");
		return -1;
	}
	
	ret = audio_svc_get_playlist_name_by_playlist_id(db_handle, plst_id, plst_name, AUDIO_SVC_PLAYLIST_NAME_SIZE);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error audio_svc_get_playlist_name_by_playlist_id");
		return -1;
	}
	fprintf(stderr,"playlist id = %d, playlist name = %s\n", plst_id, plst_name);

	//db close ==================================================
	ret = media_svc_disconnect(db_handle);
	if (ret != AUDIO_SVC_ERROR_NONE) {
		msg_print(__LINE__, "error to close music database");
		return -1;
	}

	return 0;
}


void insert_into_db(const char * dir_path)
{
	int audio_id = 300;
	char thumb_path[AUDIO_SVC_PATHNAME_SIZE+1] = {0};

	struct dirent *dp;

	DIR *dir = opendir(dir_path);
	while ((dp=readdir(dir)) != NULL) {
		char fpath[_POSIX_PATH_MAX];

		if (dp->d_name[0] == '.') {
			continue;
		}
		snprintf(fpath, sizeof(fpath), "%s/%s", dir_path, dp->d_name);

		fprintf(stderr,"[file path] : %s\n", fpath);

		#if 0
		int ret = audio_svc_insert_item(AUDIO_SVC_STORAGE_PHONE, fpath, AUDIO_SVC_CATEGORY_MUSIC);
		if (ret != AUDIO_SVC_ERROR_NONE) {
			fprintf(stderr,"[error to insert music] : %s\n", fpath);
		}
		#endif
		audio_id++;
	}


}

void msg_print(int line, char *msg)
{
	//fprintf(stderr, "\n");
	//fprintf(stderr,"%s:%d\n", __FUNCTION__, __LINE__);
	//fprintf(stderr, "+++++++++++++++++++++++++++++++++\n");
	fprintf(stderr, "[%d]%s +++++\n", line, msg);
	//fprintf(stderr, "+++++++++++++++++++++++++++++++++\n");
	//fprintf(stderr, "+++++++++++++++++++++++++++++++++\n");
	//fprintf(stderr, "\n");
}

