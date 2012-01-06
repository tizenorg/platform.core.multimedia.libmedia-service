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
 *
 * @defgroup   MB_SVC_PG Media Service
 * @ingroup SLP_PG
   @{


<h1 class="pg">Introduction</h1>
<h2 class="pg">Purpose</h2>

The purpose of this document is to give the application developer detailed information to access  media(images and videos) database using the Media Service API's.


<h2 class="pg">Scope</h2>

The scope of this document is limited to Samsung platform Media Service API usage.

<h2 class="pg">Abbreviations</h2>

 	<table>
 		<tr>
  			<td>
			  Media
  			</td>
  			<td>
			 In this document, Media items mean multimedia items including image and video, not music. If you want to manage music items, Please see Music Service Document
  			</td>
  		</tr>
 		<tr>
  			<td>
  			API
  			</td>
  			<td>
  			Application Programming Interface
  			</td>
  		</tr>
 		<tr>
  			<td>
			  Cluster
  			</td>
  			<td>
			 a collection of related images or videos like Album or Folder
  			</td>
  		</tr>
 		<tr>
  			<td>
			  extractor
  			</td>
  			<td>
			  Extracts metadata associated with a particular media format from a file or records in the databases.
  			</td>
  		</tr>
		<tr>
  			<td>
			  File manager service
  			</td>
  			<td>
			  Detect and manage all of file system chagnes.
  			</td>
  		</tr>
	</table>

<h1 class="pg"> Media Service Architecture</h1>


Media Service is designed to provide easy to use services for accessing media library database based on an embedded Linux platform.
Media Service is used to maintain media content in SQLite database. The content stored in database consists of media items from both MMC and Phone flash. 


<h2 class="pg">Features</h2>
You can use Media Service to organize your digital media collection. 
You can do a number of things with them, including playing, creating playlists.

- Add media items to the media database.
	- File manager service automatically searches certain default folders on your phone	and MMC for media files, and then adds those files to the system media database.  If you ever add or remove files from these folders, File manager service automatically updates the database accordingly.

- Delete media items from the media database.
	- You can delete a media file from the library, and it will be removed forever in your device.

- Find media items in the media database. 
	- Media Service provides smart search for you to find items in the media database. To ensure that you can easily find items in the database, it is important that files have accurate and complete media information.

- Organizing media items in the media database.
	- Media Service provides a great way to organize media items in the media database as you want. You can also sort by name, date, or etc.

<h2 class="pg">Architecture diagram</h2>


<b>Modules</b>

- User Interface
	- User interface is a group of api, which is open to public. User could access and control media data in the media database with using these apis. Also, user could extract thumbnail from media of image or video.

- List Organizer
	- List organizer performs querying db, organizing and returning results as a type of list. This component return a list, which is sorted by what user wants, for example, by name or date.

- Database Manager
	 - Database manager is responsible for database manipulation, such as open database, close database, create table, set the storage validation, and delete the storage contents.

- Thumbnail Extractor
	- Thumbnail extractor supports for user to extract a thumbnail from media of image or video. Also, user could get the path of the produced thumbnail.


@image html SLP_MediaSvc_PG_image001.png

<h1 class="pg">Database Schema Design</h1>

This image describes the design of the media database schema.

@image html SLP_MediaSvc_PG_image002.png

<h1 class="pg"> API descriptions </h2>

<h2 class="pg"> Open and close media service </h2>

<b> Purpose </b>
- To be prepared for accessing to the media database.
	- Use function minfo_init().
- To be finalized for accessing to the media database.
	- Use function minfo_finalize().
- Applications should call minfo_init() before using media-svc and call minfo_finalize() after finishing jobs in related to the media-svc.

The following is a sample code:

@code
	#include <media-svc.h>

	int ret = -1;

	//open a databse file.
	ret = minfo_init();
	if(ret< 0)
	{ 
		printf("minfo_init error\n");
		return ret;
	}

	//close database file.
	ret = minfo_finalize();

	if(ret< 0)
	{ 
		printf("minfo_finalize error\n");
		return ret;
	}

@endcode

<h2 class="pg"> Get a cluster list of media </h2>
<b> Purpose </b>
- To get a list of clusters in the media database.

<b> Usage </b>
- User can set the minfo_cluster_filter to get what user wants.
	- User can set minfo_folder_type
		- MINFO_CLUSTER_TYPE_ALL : All type of media
		- MINFO_CLUSTER_TYPE_LOCAL_ALL : lcoal both phone and mmc
		- MINFO_CLUSTER_TYPE_LOCAL_PHONE : lcoal phone only
		- MINFO_CLUSTER_TYPE_LOCAL_MMC : lcoal mmc only
		- MINFO_CLUSTER_TYPE_WEB : web album
		- MINFO_CLUSTER_TYPE_STREAMING : streaming album
	- User can set minfo_folder_sort_type
		- MINFO_CLUSTER_SORT_BY_NONE : No Sort
		- MINFO_CLUSTER_SORT_BY_NAME_DESC : Sort by display name descending
		- MINFO_CLUSTER_SORT_BY_NAME_ASC : Sort by display name ascending
		- MINFO_CLUSTER_SORT_BY_DATE_DESC : Sort by modified_date descending
		- MINFO_CLUSTER_SORT_BY_DATE_ASC : Sort by modified_date ascending 
	- User can set start position and end position.
		- If user want to get all clusters in the database, set -1 to both.
	- User should define a iterative callback to insert clusters to user list. It makes user to be able to use specific list type user wants.

The following is a sample code:

@code
#include <media-svc.h>

static int _cluster_ite_fn( Mcluster* cluster, void* user_data)
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, cluster );

	return 0;
}

int get_cluster_list_exam()
{
	int err = -1;
	int img_cnt = 0;
	int i;
	Mcluster* cluster;
	GList* list = NULL

	minfo_cluster_filter cluster_filter_all ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_DATE_ASC,0,10};
	minfo_cluster_filter cluster_filter_mmc ={MINFO_CLUSTER_TYPE_LOCAL_MMC,MINFO_CLUSTER_SORT_BY_NAME_DESC, -1, -1};
			
	err = minfo_get_cluster_list(cluster_filter_all, _cluster_ite_fn, &p_list);

	if( err < 0)
	{
		 printf("minfo_get_cluster_list failed\n");
		 return -1;
	}

	img_cnt = g_list_length(p_list);

	for(i=0; i<img_cnt; i++)
	{
		cluster = (Mcluster*)g_list_nth_data(p_list, i);
		printf("cluster ID=%d, sns_type = %d, display_name= %s, \n", cluster->_id, cluster->sns_type,cluster->display_name);
	}		

	err = minfo_get_cluster_list(cluster_filter_mmc, _cluster_ite_fn, &p_list);

	if( err < 0)
	{
		 printf("minfo_get_cluster_list failed\n");
		 return -1;
	}

	img_cnt = g_list_length(p_list);

	for(i=0; i<img_cnt; i++)
	{
		cluster = (Mcluster*)g_list_nth_data(p_list, i);
		printf("cluster ID=%d, sns_type = %d, display_name= %s, \n", cluster->_id, cluster->sns_type,cluster->display_name);
	}		

	return 0;
}

@endcode

<h2 class="pg"> Get a item list of media </h2>
<b> Purpose </b>
- To get a list of items in a cluster in the media database.

<b> Usage </b>
- User can set minfo_item_filter to get what user wants.
	- User can set minfo_file_type
		- MINFO_ITEM_NONE : none
		- MINFO_ITEM_IMAGE : image files
		- MINFO_ITEM_VIDEO : video files 
	- User can set minfo_media_sort_type
		- MINFO_MEDIA_SORT_BY_NONE : No Sort
		- MINFO_MEDIA_SORT_BY_NAME_DESC : Sort by display name descending
		- MINFO_MEDIA_SORT_BY_NAME_ASC : Sort by display name ascending
		- MINFO_MEDIA_SORT_BY_DATE_DESC : Sort by modified_date descending
		- MINFO_MEDIA_SORT_BY_DATE_ASC : Sort by modified_date ascending 
	- User can set minfo_media_favorite_type
		- MINFO_MEDIA_FAV_ALL : Includes all favorite and unfavorite media
		- MINFO_MEDIA_FAV_ONLY : Includes only favorite media
		- MINFO_MEDIA_UNFAV_ONLY : Includes only unfavorite media
	- User can set start position and end position.
		- If user want to get all items in the cluster, set -1 to both.
	- User should define a iterative callback to insert items to user list. It makes user to be able to use specific list type user wants.

The following is a sample code:

@code

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

int get_item_list_exam()
{
	int err = -1;
	int img_cnt = 0;
	int cluster_id = 0;
	int i;
	Mcluster* cluster;
	Mitem* cluster;
	GList* list = NULL

	minfo_item_filter item_filter_all = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,false,MINFO_MEDIA_FAV_ALL};
	minfo_item_filter item_filter_image = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_DESC,0,1,FALSE, MINFO_MEDIA_FAV_ONLY};

	// Id of the cluster in which user want to get items
	cluster_id = 1; 
	err = minfo_get_item_list(cluster_id, item_filter_all, _ite_fn, &p_list);

	if( err < 0)
	{
		printf("minfo_get_item_list failed\n");
		return -1;
	}

	img_cnt = g_list_length(p_list);
		
	for(i=0; i<img_cnt; i++)
	{
		mitem = (Mitem*)g_list_nth_data(p_list, i);
		printf("mitem ID=%d, %s\n",mitem->_id, mitem->file_url);
	}

	err = minfo_get_item_list(cluster_id, item_filter_image, &p_list);

	if( err < 0)
	{
		printf("minfo_get_item_list failed\n");
		return -1;
	}

	img_cnt = g_list_length(p_list);
			
	for(i=0; i<img_cnt; i++)
	{
		mitem = (Mitem*)g_list_nth_data(p_list, i);
		printf("mitem ID=%d, %p\n",mitem->_id, mitem);
	}
}

@endcode

<h2 class="pg"> Delete a user list, which is got by media-svc </h2>
<b> Purpose </b>
- To delete media-svc items that user get through media-svc api.

<b> Usage </b>
- To delete media-svc specified items, user should call minfo_destroy_mtype_item(). It can delete all media-svc specified types below:
	- Mcluster
	- Mitem
	- Mmeta
	- Mbookmark

The following is a sample code:

@code

// plist is a list which user get through media-svc api
int delete_all_mediasvc_items_exam(GList** plist)
{
	int count = 0;
	int i = 0;  
	void* item = NULL; 
				         
	if(p_list == NULL)
	{
		mb_svc_debug("p_list == NULL\n");
		return MB_SVC_ERROR_INVALID_ARG;
	}

	count = g_list_length(*p_list);

	for(i = 0; i < count; i++)
	{
		item = (void*)g_list_nth_data(*p_list, i);

		minfo_destroy_mtype_item(item);
		item = NULL; 
	}

	g_list_free(*p_list);
	*p_list = NULL; 
	return 0;
}


@endcode

<h2 class="pg"> Add a cluster to DB</h2>
<b> Purpose </b>
- To make and add new cluster to the media database.

<b> Usage </b>
- User must specify the url of the cluster path.
- User can get the id of the new cluster. 

The following is a sample code:

@code

int minfo_add_cluster_exam()
{
	const char* cluster_url = "/opt/media/Images and videos/new";
	int new_cluster_id;

	err = minfo_add_cluster(cluster_url, &new_cluster_id);

	if( err < 0)
	{
		printf("minfo_add_media failed\n");
		return -1;
	}
	else 
	{
		printf("minfo_add_media succeed. New cluster's id is %d\n", new_cluster_id);
		return 0;
	}
}

@endcode

<h2 class="pg"> Delete a cluster from DB</h2>
<b> Purpose </b>
- To delete a cluster in the media database.

<b> Usage </b>
- User can delete a media by the id of the cluster.

The following is a sample code:

@code

int minfo_delete_cluster_exam()
{
	int cluster_id = 1;

	err = minfo_delete_cluster(cluster_id);

	if( err < 0)
	{
		printf("minfo_delete_media failed\n");
		return -1;
	}
}

@endcode

<h2 class="pg"> Add a media to DB</h2>
<b> Purpose </b>
- To add user's media item to the media database.

<b> Usage </b>
- User must specify the url of the media file.
- User must specify a type of the media. ( image or video )

The following is a sample code:

@code

int minfo_add_media_exam()
{
	const char* file_url = "/opt/media/Images and videos/image.jpg";
	minfo_file_type type = MINFO_ITEM_IMAGE;

	err = minfo_add_media(file_url, type);

	if( err < 0)
	{
		printf("minfo_add_media failed\n");
		return -1;
	}
}

@endcode

<h2 class="pg"> Update a media to DB</h2>
<b> Purpose </b>
- To update media in the media database.

<b> Usage </b>
- To update a name of cluster, call minfo_update_cluster_name()
	-User must specify the id of the cluster and new name user wants.
- To update a favorite set of media item, call minfo_update_media_favorite()
	-User must specify the id of the media and a value of favorite ( true or false )
- To update a name of media item, call minfo_update_media_name()
	-User must specify the id of the media and new name user wants.
- To update meta information of integer type of video item, call minfo_update_video_meta_info_int()
	- User can set minfo_video_meta_field_t
		- MINFO_VIDEO_META_ID :  media medta ID field
		- MINFO_VIDEO_META_MEDIA_ID media medta ID field
		- MINFO_VIDEO_META_BOOKMARK_LAST_PLAYED : medta bookmark field
		- MINFO_VIDEO_META_DURATION : medta duration field
- To update meta information of string type of video item, call minfo_update_video_meta_info_string()
	- User can set minfo_video_meta_field_t
		- MINFO_VIDEO_META_ALBUM : medta album field
		- MINFO_VIDEO_META_ARTIST : medta artist field
		- MINFO_VIDEO_META_TITLE : medta title field
		- MINFO_VIDEO_META_DESCRIPTION : medta description field
		- MINFO_VIDEO_META_YOUTUBE_CATEGORY : medta youtube cat field

The following is a sample code:

@code

int minfo_update_cluster_name_exam()
{
	int err = -1;
	int cluster_id = 1;
	const char* new_name = "NewCluster";

	err = minfo_update_cluster_name(cluster_id, new_name);

	if( err < 0)
	{
		printf("minfo_update_cluster_name failed\n");
		return -1;
	}
}

int minfo_update_media_name_exam()
{
	int err = -1;
	int media_id = 1;
	const char* new_name = "NewMedia";

	err = minfo_update_media_name(media_id, new_name);

	if( err < 0)
	{
		printf("minfo_update_media_name failed\n");
		return -1;
	}
}

int minfo_update_media_favorite_exam()
{
	int err = -1;
	int media_id = 1;
	int favorite = TRUE;

	err = minfo_update_media_favorite(media_id, favorite);

	if( err < 0)
	{
		printf("minfo_update_media_favorite failed\n");
		return -1;
	}
}

int minfo_update_media_videometa_exam()
{
	int err = -1;
	int media_id = 1;
	minfo_video_meta_field_t meta_field;
	int last_played_time = 1259000000;
	const char* title = "New Title";

	// Update last played time
	err = minfo_update_video_meta_info_int(media_id, last_played_time);

	if( err < 0)
	{
		printf("minfo_update_video_meta_info_int failed\n");
		return -1;
	}

	// Update title
	err = minfo_update_video_meta_info_string(media_id, title);

	if( err < 0)
	{
		printf("minfo_update_video_meta_info_string failed\n");
		return -1;
	}
}

@endcode

<h2 class="pg">  Copy media </h2>
<b> Purpose </b>
- To copy media from original folder to the other folder, not only updating database but also operating system call.

<b> Usage </b>
- User can copy a media by the path of origin media and the path of destination cluster. In this case, user should call minfo_copy_media().
	- User must specify minfo_file_type.
		- MINFO_ITEM_IMAGE : image
		- MINFO_ITEM_VIDEO : video
- User can copy a media by the id of the origin media and the id of destination cluster. In this case, user should call minfo_cp_media().

The following is a sample code:

@code

int minfo_copy_exam_by_path()
{
	const char* file_url = "/opt/media/Images and videos/image.jpg";
	const char* dest_url = "/opt/media/Images and videos/second";
	minfo_file_type file_type = MINFO_ITEM_IMAGE;

	err = minfo_copy_media(file_url, dest_url, file_type);

	if( err < 0)
	{
		printf("minfo_copy_media failed\n");
		return -1;
	}
}

int minfo_copy_exam_by_id()
{
	int media_id = 1;
	int dest_cluster_id = 3;

	err = minfo_cp_media(media_id, dest_cluster_id );

	if( err < 0)
	{
		printf("minfo_copy_media failed\n");
		return -1;
	}
}

@endcode

<h2 class="pg">  Move media </h2>
<b> Purpose </b>
- To move media from original folder to the other folder, not only updating database but also operating system call.

<b> Usage </b>
- User can move a media by the path of origin media and the path of destination cluster. In this case, user should call minfo_move_media().
	- User must specify minfo_file_type.
		- MINFO_ITEM_IMAGE : image
		- MINFO_ITEM_VIDEO : video
- User can move a media by the id of the origin media and the id of destination cluster. In this case, user should call minfo_mv_media().

The following is a sample code:

@code

int minfo_move_exam_by_path()
{
	const char* file_url = "/opt/media/Images and videos/image.jpg";
	const char* dest_url = "/opt/media/Images and videos/second";
	minfo_file_type file_type = MINFO_ITEM_IMAGE;

	err = minfo_move_media(file_url, dest_url, file_type);

	if( err < 0)
	{
		printf("minfo_move_media failed\n");
		return -1;
	}
}

int minfo_move_exam_by_id()
{
	int media_id = 1;
	int dest_cluster_id = 3;

	err = minfo_mv_media(media_id, dest_cluster_id );

	if( err < 0)
	{
		printf("minfo_mv_media failed\n");
		return -1;
	}
}

@endcode

<h2 class="pg"> Get a thumbnail of media </h2>
<b> Purpose </b>
- To get thumbnail of the choosen media.

<b> Usage </b>
- User must specify the url of the video or image media .
- If thumbnail of the media exists, just set the filepath of the thumbnail. If not, create new thumbnail and set the filepath of the thumbnail.

The following is a sample code:

@code

int minfo_get_thumb_path_exam()
{
	int ret = -1;
	const char* file_url = "/opt/media/Images and videos/image.jpg";
	char thumb_path[255] = {0,};


	ret = minfo_get_thumb_path(file_url, thumb_path, 255);

	if(ret < 0)
	{
		printf("minfo_get_thumb_path fails\n" );
		return -1;
	}

	printf("minfo_get_thumb_path : %s\n", thumb_path);
}

@endcode

@}
*/

