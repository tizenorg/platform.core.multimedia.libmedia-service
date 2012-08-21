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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include <pthread.h>
#include <sqlite3.h>
#include "media-svc.h"
#include "media-svc-util.h"
#include "media-svc-debug.h"

#include "audio-svc.h"
#include "visual-svc.h"

void test_connect_disconn();
int test_query(sqlite3* handle);
void *do_newjob();

static int _ite_cluster_fn( Mcluster* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

static int _ite_tag_fn(Mtag* t_item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, t_item );

	return 0;
}

static int _bm_ite_fn( Mbookmark *bookmark, void *user_data )
{
	if (!bookmark) {
		printf("bookmark is NULL!\n");
		return -1;
	}

	GList** list = (GList**) user_data;
	*list = g_list_append(*list, bookmark);

	return 0;
}

int main(int argc, char *argv[])
{
	int err = -1;
	int test_case = -1;
	int i;
	int count = 0;
    GList *p_list = NULL;
	Mtag* tag = NULL;
	Mitem* item = NULL;
	Mcluster* cluster = NULL;
	minfo_cluster_filter cluster_filter;
	minfo_item_filter item_filter;


	if(argc < 2) return -1;

	MediaSvcHandle *handle = NULL;
	err = media_svc_connect(&handle);
	if(err < 0) {
		media_svc_debug("media_svc_connect fails");
	}

	media_svc_debug("media_svc_connect succeeds");

	test_case = atoi(argv[1]);

	switch( test_case ) {
		case 0: 
			test_connect_disconn();
			break;
		case 1: 
			media_svc_create_table(handle);
			break;
		
		case 2: 
			// Test for tag

			//err = minfo_add_tag("1bf63a42-0530-3cb8-94a1-564d603d85e8", "ZZOON");
			//err = minfo_add_tag("1bf63a42-0530-3cb8-94a1-564d603d85e8", "HELLO");

			//err = minfo_add_tag("2ba2cd51-e93f-af54-c1c2-b29a19fd97d0", "ZZOON");
			//err = minfo_rename_tag("HIHI", "ZZOON");
			//err = minfo_rename_tag_by_id("2ba2cd51-e93f-af54-c1c2-b29a19fd97d0", "ZZOON", "HELLOHELLO");
			//err = minfo_delete_tag("2ba2cd51-e93f-af54-c1c2-b29a19fd97d0", "HELLOHELLO");


			// Test for tag or media list
			err = minfo_get_media_list_by_tagname(handle, "ZZOON", FALSE, _ite_fn, &p_list);
			if( err < 0 ) {
				printf("minfo_get_media_list_by_tagname fails : %d\n", err );
				break;
			}

			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, item->uuid );
				printf("path[%d]:%s\n", i, item->file_url );
				printf("display_name[%d]:%s\n", i, item->display_name );
			}

			g_list_free(p_list);
			p_list = NULL;

			printf("\n-------------------------------------------\n");

			err = minfo_get_tag_list_by_media_id(handle, "2ba2cd51-e93f-af54-c1c2-b29a19fd97d0", _ite_tag_fn, &p_list);
			if( err < 0 ) {
				printf("minfo_get_media_list_by_tagname fails : %d\n", err );
				break;
			}

			for( i = 0; i < g_list_length(p_list); i++ ) {
				tag = (Mtag*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, tag->media_uuid );
				printf("tag name[%d]:%s\n", i, tag->tag_name );
				printf("count[%d]:%d\n", i, tag->count );
			}
			g_list_free(p_list);
			p_list = NULL;

			printf("\n-------------------------------------------\n");

			err = minfo_get_tag_list_by_media_id(handle, "1bf63a42-0530-3cb8-94a1-564d603d85e8", _ite_tag_fn, &p_list);
			if( err < 0 ) {
				printf("minfo_get_media_list_by_tagname fails : %d\n", err );
				break;
			}

			for( i = 0; i < g_list_length(p_list); i++ ) {
				tag = (Mtag*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, tag->media_uuid );
				printf("tag name[%d]:%s\n", i, tag->tag_name );
				printf("count[%d]:%d\n", i, tag->count );
			}
			g_list_free(p_list);

			break;

		case 3:
			// Test for cluster
			printf("Test for cluster\n");
			memset(&cluster_filter, 0x00, sizeof(minfo_cluster_filter));
			//filter.sort_type =  MINFO_CLUSTER_SORT_BY_NAME_ASC;
			cluster_filter.sort_type =  MINFO_CLUSTER_SORT_BY_NAME_DESC;
			//filter.start_pos = filter.end_pos = -1;
			cluster_filter.start_pos = 0;
			cluster_filter.end_pos = 5;

			err = minfo_get_cluster_list(handle, cluster_filter, _ite_cluster_fn, &p_list);
			if( err < 0 ) {
				printf("minfo_get_cluster_list fail : %d\n", err);
				break;
			}

			for( i = 0; i < g_list_length(p_list); i++ ) {
				cluster = (Mcluster*)g_list_nth_data(p_list, i);
				printf("cluster UUID[%d]:%s\n", i,  cluster->uuid );
				printf("display_name[%d]:%s\n", i, cluster->display_name );
			}

			break;

		case 4:
			// Test for multi-threads
			printf("Test for multi-threads\n");
			pthread_t p_thread[1];
			int thr_id;
			int status;
			//int media_id = 1;
			const char *media_id = "2b0a4efe-c3cb-cb62-fe12-3f4f7aef4ab9";

			thr_id = pthread_create(&p_thread[0], NULL, do_newjob, NULL);

			pthread_join(p_thread[0], (void **) &status);

			err = minfo_update_media_name(handle, media_id, "New.JPG");
			if( err < 0)
			{
				printf("minfo_update_media_name failed\n");
				return err;
			}
			
			break;

		case 5:
			printf("Test minfo_get_item_list\n");
			memset(&item_filter, 0x00, sizeof(minfo_item_filter));

			item_filter.file_type =  MINFO_ITEM_ALL;
			item_filter.sort_type = MINFO_MEDIA_SORT_BY_DATE_ASC;
			item_filter.start_pos = 0;
			item_filter.end_pos = 10;
			item_filter.with_meta = 0;
			item_filter.favorite = MINFO_MEDIA_FAV_ALL;
			const char *_id = "fae82467-6e74-475c-8414-40f011130c6d";

			//get a set of items
			err = minfo_get_item_list(handle, _id, item_filter, _ite_fn, &p_list);
			if( err < 0)
			{
				printf("minfo_get_item_list failed\n");
				return err;
			}

			if( p_list == NULL ) printf("failed!\n");
	
			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, item->uuid );
				printf("path[%d]:%s\n", i, item->file_url );
				printf("display_name[%d]:%s\n", i, item->display_name );
				printf("size[%d]:%d\n", i, item->size );
				printf("thumb url[%d]:%s(length:%d)\n", i, item->thumb_url, strlen(item->thumb_url) );
			}
	
			/* delete list to avoid memory leak */
			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				minfo_destroy_mtype_item(item);
			}
	
			g_list_free(p_list);
			p_list = NULL;

			break;
		
		case 6:
			printf("Test minfo_get_all_item_list\n");
			memset(&item_filter, 0x00, sizeof(minfo_item_filter));

			item_filter.file_type = MINFO_ITEM_ALL;
			item_filter.sort_type = MINFO_MEDIA_SORT_BY_DATE_ASC;
			item_filter.start_pos = 0;
			item_filter.end_pos = 10;
			item_filter.with_meta = true;
			item_filter.favorite = MINFO_MEDIA_FAV_ALL;

			//get a set of items
			err = minfo_get_all_item_list(handle, MINFO_CLUSTER_TYPE_ALL, item_filter, _ite_fn, &p_list);
			if( err < 0)
			{
				printf("minfo_get_all_item_list failed\n");
				return err;
			}

			if( p_list == NULL ) printf("failed!\n");
	
			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, item->uuid );
				printf("path[%d]:%s\n", i, item->file_url );
				printf("display_name[%d]:%s\n", i, item->display_name );
				printf("size[%d]:%d\n", i, item->size );
				printf("thumb url[%d]:%s(length:%d)\n", i, item->thumb_url, strlen(item->thumb_url) );

				if (item->type == MINFO_ITEM_VIDEO) {
					printf("genre : %s\n", item->meta_info->video_info->genre);
				}
			}
	
			/* delete list to avoid memory leak */
			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				minfo_destroy_mtype_item(item);
			}
	
			g_list_free(p_list);
			p_list = NULL;
			break;

		case 7:
			printf("Test minfo_delete_media_id\n");
			
			err = minfo_delete_media_id(handle, "12ca468c-994d-f62c-7229-a361c3a6c2a1");
			if( err < 0)
			{
				printf("minfo_delete_media_id failed\n");
				return err;
			}
			break;

		case 8:
			// Test for cluster
			printf("Test for minfo_get_cluster\n");
			Mcluster *cluster = NULL;

			err = minfo_get_cluster(handle, NULL, NULL, &cluster);
			if( err < 0 ) {
				printf("minfo_get_cluster_list fail : %d\n", err);
				break;
			}

			break;

		case 9:
			printf("Test minfo_get_media_list_by_tagname_with_filter\n");

			GList *p_list = NULL;
			minfo_tag_filter filter;
		
			filter.start_pos = 1;
			filter.end_pos = 3;
			filter.file_type =  MINFO_ITEM_VIDEO;
			filter.with_meta = 0;
		
			//get a media items' list who are included to the same tag with 'test tag'.
			err = minfo_get_media_list_by_tagname_with_filter(handle, "gd", filter, _ite_fn, &p_list);
			if (err < 0)
			{
				printf( "failed to get a media items' list. error code->%d", err);
				break;
			}
			
			for( i = 0; i < g_list_length(p_list); i++ ) {
				item = (Mitem*)g_list_nth_data(p_list, i);
				printf("media ID[%d]:%s\n", i, item->uuid );
				printf("path[%d]:%s\n", i, item->file_url );
				printf("display_name[%d]:%s\n", i, item->display_name );
			}

			g_list_free(p_list);
			p_list = NULL;

			break;

		case 10:
			printf("Test get count\n");
			count = 0;
			const char *cluster_id = "8ddcdba9-9df4-72b4-4890-8d21d13854ad";
			minfo_cluster_filter c_filter;
			minfo_item_filter m_filter;

			memset(&c_filter, 0x00, sizeof(minfo_cluster_filter));
			c_filter.cluster_type = MINFO_CLUSTER_TYPE_ALL;
			c_filter.sort_type =  MINFO_CLUSTER_SORT_BY_NAME_DESC;
			c_filter.start_pos = c_filter.end_pos = -1;

			memset(&m_filter, 0x00, sizeof(minfo_item_filter));
			m_filter.file_type =  MINFO_ITEM_ALL;
			m_filter.sort_type = MINFO_MEDIA_SORT_BY_DATE_ASC;
			m_filter.start_pos = -1;
			m_filter.end_pos = -1;
			m_filter.with_meta = 0;
			m_filter.favorite = MINFO_MEDIA_FAV_ALL;

			err = minfo_get_cluster_cnt(handle, c_filter, &count);

			if (err < 0) {
				printf( "failed to get a media items' list. error code->%d", err);
			} else {
				printf( "Clouster count : %d\n", count);
			}

			err = minfo_get_item_cnt(handle, cluster_id, m_filter, &count);

			if (err < 0) {
				printf( "failed to get a media items' list. error code->%d", err);
			} else {
				printf( "media count in cluster[%s] : %d\n", cluster_id, count);
			}

			break;

	case 11:
		printf("test minfo_add_media\n");
		char path[255];
		int type;
		while(1) {
			printf("Enter path and type(  \"exit 0\" to quit ) : ");
			scanf("%s %d", path, &type);

			if( strcmp(path, "exit") == 0 ) break;

			struct timeval time1;
			struct timeval time2;
			gettimeofday(&time1, NULL);

			err = minfo_add_media(handle, path, type);
			if( err < 0 ) {
				printf("minfo_add_media fails ( path : %s, type : %d )\n", path, type );
			} else {
				printf("minfo_add_media succeed ( path : %s, type : %d )\n", path, type );
			}

			gettimeofday(&time2, NULL);

			printf("Time : %ld\n", (time2.tv_sec * 1000000 + time2.tv_usec) - 
									(time1.tv_sec * 1000000 + time1.tv_usec));
		}

		break;

	case 12:
		printf("Test minfo_delete_media_id\n");

		char inserted_media_uuid[256] = {0,};
		printf("Enter media id: ");
		scanf("%s", inserted_media_uuid);

		err = minfo_delete_media_id(handle, inserted_media_uuid);
		if( err < 0 ) {
			printf("minfo_delete_media_id fails\n");
		} else {
			printf("minfo_delete_media_id succeeds\n");
		}

		break;

	case 13:
	{
		printf("test minfo_add_web_media\n");
		//char path[255];
		//int type;
		const char *cluster_id = "8ddcdba9-9df4-72b4-4890-8d21d13854ad";

		//add a web media to a web album.
		err = minfo_add_web_media_with_type(handle, cluster_id, "http://user/specifying/address",  "web_media", MINFO_ITEM_IMAGE, "thumbnail name");
		if (err < 0)
		{
			printf( "failed to add to a web album. error code->%d", err);
		}
	}
	break;

	case 14:
	{
		printf("test minfo_update_cluster_name\n");
		//char path[255];
		//int type;

		//add a web media to a web album.
		err = minfo_update_cluster_name(handle, "8ddcdba9-9df4-72b4-4890-8d21d13854ad", "hey");
		if (err < 0)
		{
			printf( "failed to add to a web album. error code->%d", err);
		}
	}
	break;

	case 15:
	{
		printf("test to get the latest item in camera shots\n");		
		char cluster_id[256] = {0,};

		//add a web media to a web album.
		err = minfo_get_cluster_id_by_url(handle, "/opt/media/Images and videos/hey", cluster_id, sizeof(cluster_id));
		if (err < 0)
		{
			printf( "failed to minfo_get_cluster_id_by_url. error code->%d", err);
			break;
		}

		printf("Cluster ID : %s\n", cluster_id);

		memset(&item_filter, 0x00, sizeof(minfo_item_filter));

		item_filter.file_type = MINFO_ITEM_ALL;
		item_filter.sort_type = MINFO_MEDIA_SORT_BY_DATE_DESC;
		item_filter.start_pos = 0;
		item_filter.end_pos = 1;
		item_filter.with_meta = false;
		item_filter.favorite = MINFO_MEDIA_FAV_ALL;
		p_list = NULL;
		//get a set of items
		err = minfo_get_item_list(handle, cluster_id, item_filter, _ite_fn, &p_list);
		if( err < 0)
		{
			printf("minfo_get_item_list failed\n");
			return err;
		}

		if( p_list == NULL ) printf("failed!\n");

		for( i = 0; i < g_list_length(p_list); i++ ) {
			item = (Mitem*)g_list_nth_data(p_list, i);
			printf("media ID[%d]:%s\n", i, item->uuid );
			printf("path[%d]:%s\n", i, item->file_url );
			printf("display_name[%d]:%s\n", i, item->display_name );
		}

		/* delete list to avoid memory leak */
		for( i = 0; i < g_list_length(p_list); i++ ) {
			item = (Mitem*)g_list_nth_data(p_list, i);
			minfo_destroy_mtype_item(item);
		}

		g_list_free(p_list);
		p_list = NULL;
	}
	break;
	
	case 16:
	{
		printf("Test minfo_get_item_list_search\n");
		memset(&item_filter, 0x00, sizeof(minfo_item_filter));

		const char *search_str = "Ima";
		minfo_search_field_t search_field = MINFO_SEARCH_BY_HTTP_URL | MINFO_SEARCH_BY_PATH | MINFO_SEARCH_BY_NAME;
		minfo_folder_type folder_type = MINFO_CLUSTER_TYPE_ALL;

		item_filter.file_type =  MINFO_ITEM_ALL;
		item_filter.sort_type = MINFO_MEDIA_SORT_BY_NAME_ASC;
		item_filter.start_pos = 0;
		item_filter.end_pos = 9;
		item_filter.with_meta = 0;
		item_filter.favorite = MINFO_MEDIA_FAV_ALL;
		p_list = NULL;

		//get a set of items
		err = minfo_get_item_list_search(handle, search_field, search_str, folder_type, item_filter, _ite_fn, &p_list);
		if (err < 0) {
			printf("minfo_get_item_list_search failed\n");
			return err;
		}

		if (p_list == NULL) printf("failed!\n");
		else printf("success\n");

		for (i = 0; i < g_list_length(p_list); i++) {
			item = (Mitem*)g_list_nth_data(p_list, i);
			printf("media ID[%d]:%s\n", i, item->uuid );
			printf("path[%d]:%s\n", i, item->file_url );
			printf("display_name[%d]:%s\n", i, item->display_name );
			printf("thumb url[%d]:%s(length:%d)\n", i, item->thumb_url, strlen(item->thumb_url) );
		}

		/* delete list to avoid memory leak */
		for (i = 0; i < g_list_length(p_list); i++) {
			item = (Mitem*)g_list_nth_data(p_list, i);
			minfo_destroy_mtype_item(item);
		}

		g_list_free(p_list);
		p_list = NULL;

		break;
	}

	case 17:
	{
		printf("test minfo_update_image_meta_info_int\n");
		const char *media_uuid = "12ca468c-994d-f62c-7229-a361c3a6c2a1";
		int width = 640;
		int height = 480;

		//add a web media to a web album.
		err = minfo_update_image_meta_info_int(handle, media_uuid, MINFO_IMAGE_META_WIDTH, width,
												MINFO_IMAGE_META_HEIGHT, height, -1);
		if (err < 0) {
			printf( "minfo_update_image_meta_info_int failed->%d\n", err);
		} else {
			printf( "minfo_update_image_meta_info_int success\n");
		}

		break;
	}

	case 18:
	{
		printf("test _media_info_generate_uuid\n");
		printf("UUID : %s\n", (char*)_media_info_generate_uuid());
		break;
	}

	case 19:
		printf("test minfo_get_bookmark_list\n");
    	GList *_list = NULL;
		const char *media_uuid = NULL;

		p_list = NULL;
		if (p_list) printf("p list does exist\n");
		if (argv[2]) {
			media_uuid = argv[2];
		} else {
			printf("please insert media id\n");
			break;
		}

		//add a web media to a web album.
		err = minfo_get_bookmark_list(handle, media_uuid, _bm_ite_fn, &_list);
		if (err < 0) {
			printf( "minfo_get_bookmark_list failed->%d\n", err);
		} else {
			printf( "minfo_get_bookmark_list success\n");
			
			if (_list) {
			int i;
			Mbookmark* bm = NULL;
			for( i = 0; i < g_list_length(_list); i++ ) {
				bm = (Mbookmark*)g_list_nth_data(_list, i);
				printf("Thumb[%d]:%s\n", i, bm->thumb_url );
			}
			}
		}

		break;

	case 20:
		printf("test minfo_get_item\n");
		Mitem *mitem = NULL;
		const char *url = "/opt/media/Images and videos/My video clips/Helicopter.mp4";

		err = minfo_get_item(handle, url, &mitem);

		if (err < 0) {
			printf("minfo_get_item failed");
			return -1;
		}

		printf("mitem->path : %s\n", mitem->file_url);
		printf("mitem->display_name : %s\n", mitem->display_name);
		break;
	case 21:
		printf("test minfo_get_all_item_conut\n");
		int cnt = 0;
		minfo_folder_type f_type = MINFO_CLUSTER_TYPE_LOCAL_ALL;
		//minfo_folder_type f_type = MINFO_CLUSTER_TYPE_LOCAL_PHONE;
		//minfo_media_favorite_type fav_type = MINFO_MEDIA_FAV_ALL;
		minfo_media_favorite_type fav_type = MINFO_MEDIA_FAV_ONLY;
		
		minfo_file_type file_type = MINFO_ITEM_ALL;
		//minfo_file_type file_type = MINFO_ITEM_IMAGE;

		err = minfo_get_all_item_count(handle, f_type, file_type, fav_type, &cnt);

		if (err < 0) {
			printf("minfo_get_all_item_conut failed");
			return -1;
		}

		printf("count : %d\n", cnt);
		break;

	case 22:
		printf("test minfo_get_meta_info\n");
		const char *video_media_id = "77b876ed-5db9-4114-82c0-d08e814b5051";
		Mmeta *mmeta = NULL;

		err = minfo_get_meta_info(handle, video_media_id, &mmeta);

		if (err < 0) {
			printf("minfo_get_meta_info failed");
			return -1;
		}

		if (mmeta->video_info) {
			printf("Album name : %s\n", mmeta->video_info->album_name);
		}
		break;

	case 36:
		printf("test minfo_get_thumb_path \n");
		char thumb_path[255] = {0,};

		if(argv[2] && argv[3]) {
			int type = atoi(argv[3]);
			if( type == 1 ) {
				err = minfo_get_thumb_path(handle, argv[2], thumb_path, 255);
			} else if( type == 2) {
				err = minfo_get_thumb_path_for_video(handle, argv[2], thumb_path, 255);
			} else {
				printf("minfo_get_thumb_path fails( invalid type )\n" );
				return -1;
			}
			if(err < 0)
			{
				printf("minfo_get_thumb_path fails\n" );
				return -1;
			}
		}

		printf("minfo_get_thumb_path : %s\n", thumb_path);
		break;

	case 37:
		printf("test minfo_delete_invalid_media_records \n");
		
		err = minfo_delete_invalid_media_records(handle, 1);

		if(err < 0)
		{
			printf("minfo_delete_invalid_media_records fails\n");
			return -1;
		}
		

		printf("minfo_delete_invalid_media_records succeeds.\n");
		break;

	/* ------------------- Test for audio ---------------- */
	case 101:
	{
		printf("test audio_svc_insert_item\n");
		char path[255];

		while(1) {
			printf("Enter path (  \"exit\" to quit ) : ");
			scanf("%s", path );

			if( strcmp(path, "exit") == 0 ) break;

			err = audio_svc_insert_item(handle, AUDIO_SVC_STORAGE_PHONE, path, AUDIO_SVC_CATEGORY_MUSIC);
			if (err != AUDIO_SVC_ERROR_NONE) {
				fprintf(stderr,"[errer to insert music] : %s\n", path);
			}
		}
	}
	break;

	case 102:
	{
		printf("test audio_svc_delete_item_by_path\n");
		char path[255];

		while(1) {
			printf("Enter path (  \"exit\" to quit ) : ");
			scanf("%s", path );

			if( strcmp(path, "exit") == 0 ) break;

			err = audio_svc_delete_item_by_path(handle, path);
			if (err != AUDIO_SVC_ERROR_NONE) {
				fprintf(stderr,"[errer to delete music] : %s\n", path);
			}
		}
	}
	break;

	case 103:
	{
		printf("test audio_svc_get_list_item - AUDIO_SVC_TRACK_BY_SEARCH\n");
		int offset = 0, count = 10, i = 0;
		const char *str = "Sa";
		AudioHandleType *a_handle = NULL;

		err = audio_svc_search_item_new(&a_handle, count);
		if (err < 0) {
			printf("audio_svc_search_item_new failed:%d\n", err);
			return err;
		}

		err = audio_svc_list_by_search(handle, a_handle, AUDIO_SVC_ORDER_BY_TITLE_ASC, offset, count, AUDIO_SVC_SEARCH_TITLE, str, strlen(str), AUDIO_SVC_SEARCH_ALBUM, str, strlen(str), AUDIO_SVC_SEARCH_ARTIST, str, strlen(str), -1);

		if (err != AUDIO_SVC_ERROR_NONE) {
			media_svc_debug("Fail to get items : %d", err);
			return err;
		}
		
		for (i = 0; i < count; i++) {
			AudioHandleType *item = NULL;
			err = audio_svc_search_item_get(a_handle, i, &item);
			char *title = NULL, *artist = NULL, *pathname = NULL, *album = NULL;
			char *audio_id = NULL;
			int size = 0;

			if (err < 0) {
				printf("[%d] audio_svc_search_item_get failed : %d\n", i, err);
			} else {
				audio_svc_item_get_val(item,  
						AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size,
						AUDIO_SVC_TRACK_DATA_PATHNAME, &pathname, &size,
						AUDIO_SVC_TRACK_DATA_TITLE, &title, &size,
						AUDIO_SVC_TRACK_DATA_ARTIST, &artist, &size,
						AUDIO_SVC_TRACK_DATA_ALBUM, &album, &size,
						-1);

				if( audio_id == NULL ) break;

				printf("[%d] ID: %s\n", i, audio_id);
				printf("[%d] Path: %s\n", i, pathname);
				printf("[%d] Title: %s\n", i, title);
				printf("[%d] Artist: %s\n", i, artist);
				printf("[%d] Album: %s\n", i, album);
			}
		}

		audio_svc_search_item_free(a_handle);
	}
	break;

	case 104:
	{
#if 0
			printf("Test audio_svc_update_item_metadata\n");
			count = 4;
			
			err = audio_svc_update_item_metadata(31, AUDIO_SVC_TRACK_DATA_PLAYED_COUNT, count, -1);

			if( err < 0)
			{
				printf("audio_svc_update_item_metadata failed\n");
				return err;
			} else {
				printf("play count is now %d\n", count);
			}
#endif
	}	
	break;

	default:
		break;
	}

	err = media_svc_disconnect(handle);
	if(err < 0) {
		media_svc_debug("media_svc_disconnect fails");
	}

	printf("End of Test\n");
    return err;
}

void *do_newjob()
{
	int err;
	
	MediaSvcHandle *handle = NULL;
	err = media_svc_connect(&handle);
	if(err < 0) {
		media_svc_debug("media_svc_connect fails");
	}

	const char *media_id = "2b0a4efe-c3cb-cb62-fe12-3f4f7aef4ab9";

	err = minfo_update_media_name(handle, media_id, "New1.JPG");
	if( err < 0)
	{
		printf("minfo_update_media_name failed\n");
		return NULL;
	}

	printf("Calling media_svc_disconnect in do_newjob \n");
	err = media_svc_disconnect(handle);
	if(err < 0) {
		media_svc_debug("media_svc_disconnect fails");
		return NULL;
	}

	printf("do_newjob done\n");

	return NULL;
}


void test_connect_disconn()
{
	media_svc_debug("");
	int err = -1;
	MediaSvcHandle *handle = NULL;
	err = media_svc_connect(&handle);
	if( err < 0 ) {
		media_svc_debug("Error");
		return;
	} else { media_svc_debug("success"); }

	if(handle == NULL) media_svc_debug("NULL!!!!!");
	test_query((sqlite3 *)handle);

	err = media_svc_disconnect(handle);
	if( err < 0 ) {
		media_svc_debug("Error");
		return;
	}

	media_svc_debug("test_connect_disconn success");

	return;
}

int test_query(sqlite3* handle)
{
	int err = -1;
	sqlite3_stmt* stmt = NULL;
	char query_string[255];
	memset(query_string, 0, sizeof(query_string) );

	if( handle == NULL ) {
		media_svc_debug( "handle is NULL" );
		return -1;
	}

	snprintf(query_string, sizeof(query_string), "select * from visual_folder where storage_type = 0");

	err = sqlite3_prepare_v2(handle, query_string, strlen(query_string), &stmt, NULL);	

	if (SQLITE_OK != err) 
	{
		 media_svc_debug ("prepare error [%s]\n", sqlite3_errmsg(handle));
		 media_svc_debug ("query string is %s\n", query_string);
		 return -1;
	}
	err = sqlite3_step(stmt);	
	if (err != SQLITE_ROW) 
	{
		 media_svc_debug ("end of row [%s]\n", sqlite3_errmsg(handle));
		 media_svc_debug ("query string is %s\n", query_string);
		 sqlite3_finalize(stmt);
	 	 return -1;
	}

	media_svc_debug("ID: %d", sqlite3_column_int (stmt, 0));
	media_svc_debug("Display name: %s", (const char*)sqlite3_column_text (stmt, 2));

	if(err < 0)
	{
		media_svc_debug ("mb-svc load data failed");
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);

	return 0;
}

