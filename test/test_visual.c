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
#include "media-svc.h"
#include <glib.h>
#include <glib-object.h>
#include <glib/gstdio.h>
#include "media-svc-debug.h"

static int _ite_fn( Mitem* item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, item );

	return 0;
}

static int _cluster_ite_fn( Mcluster* cluster, void* user_data)
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, cluster );

	return 0;
}

static int _cover_ite_fn( char* thumb_path, void* user_data)
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, thumb_path );

	return 0;
}

static int _minfo_bm_ite_fn( Mbookmark *bookmark, void *user_data )
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, bookmark );

	return 0;
}

static int _ite_tag_fn(Mtag* t_item, void* user_data) 
{
	GList** list = (GList**) user_data;
	*list = g_list_append( *list, t_item );

	return 0;
}


int main(int argc, char *argv[])
{
    int test_sequence = 0;
	int media_id = 0;
	Mmeta* mt = NULL;
	Mcluster* cluster = NULL;
	Mitem* mitem = NULL;
	int type = 0;
    int favorite_level = 0;
    int cluster_id = 0;
    int position = 0;
    int bookmark_id = 0;
    int duration= 0,id = 0;
    int err = -1, ret = -1;
	int streaming_id = 0;
    int img_cnt = 0;
	int cnt = 0, i = 0;
	char file_url[MB_SVC_FILE_PATH_LEN_MAX] ={0};
	char new_file_url[MB_SVC_FILE_PATH_LEN_MAX] ={0};
	double min_longitude = 0.0;
	double max_longitude = 0.0;
	double min_latitude = 0.0;
	double max_latitude = 0.0;
    int sns_type = 0;
    GList *p_list = NULL;
		
    printf("################################################\n");
	printf("---this is self-test program for media service---\n");
    printf("################################################\n");

	
	if ((argc <2)||(argv[0] == NULL)||(argv[1] == NULL))
	{
		printf("parameter count typed in console is wrong!!\n");
		return -1;
	}

	g_type_init(); 

	err = minfo_init();
	if(err<0)
	{
		printf("minfo_init error\n");
        return -1;
	}

    test_sequence =atoi(argv[1]);
    //printf("test_sequence is %d\n",test_sequence);                
	
    switch(test_sequence)
	{
	case 0: 
		printf("reset database of media service.......\n");
		//mb_svc_reset_db();
		printf("sync image and video files to media service database.......\n");
		break;
	 
	case 1: 
		printf("test minfo_get_bookmark_list.......\n");
		if(argv[2] != NULL)
		{
		    media_id = atoi(argv[2]);
			printf("parameter media_id is %d\n",media_id);
			
			ret = minfo_get_bookmark_list(media_id, _minfo_bm_ite_fn, &p_list);

		    if( ret < 0)
		    {
		        printf("minfo get bookmark failed, media_id is %d\n",media_id);
		        return -1;
		    } 
			else {
				int i;
				Mbookmark* bm = NULL;
				for( i = 0; i < g_list_length(p_list); i++ ) {
					bm = (Mbookmark*)g_list_nth_data(p_list, i);
					printf("Thumb[%d]:%s\n", i, bm->thumb_url );
				}
			}
			//minfo_free_glist(&p_list);		
		}
		else 
		{
		    printf("minfo get bookmark: media_id is null\n");
			return -1;		 
		}
	break;
	 
	case 2:   
		printf("test minfo_add_media.......\n");
		if((argv[2] != NULL)&&(argv[3] != NULL))//minfo_add_media
		{                                       //type: 1-image, 2-video
			 memcpy(file_url,argv[2],MB_SVC_FILE_PATH_LEN_MAX);
			 type = atoi(argv[3]);
			 printf("parameter file url is %s, type is %d\n",file_url,type);
			 err = minfo_add_media(file_url, type);
			 if( err < 0)
			 {
			    printf("minfo_add_media failed\n");
			    return -1;
			 }
		}
		else 
		{
		    printf("minfo_add_media: file url or type  is null\n");
			return -1;
		 
		}

	break; 

	case 3: 
		printf("test minfo_delete_media.......\n");
		if(argv[2] != NULL) //minfo_delete_media
		{

			memcpy(file_url,argv[2],MB_SVC_FILE_PATH_LEN_MAX);
			printf("parameter file url is %s\n",file_url);
			err = minfo_delete_media(file_url);
			if( err < 0)
			{
				printf("minfo_delete_media failed\n");
				return -1;
			}
		}
		else 
		{
			printf("minfo_delete_media: file url is null\n");
			return -1;
		}
		break;

	case 4:                                  
		printf("test minfo_delete_media_id.......\n");
		if(argv[2] != NULL) //minfo_delete_media_id
		{
			media_id = atoi(argv[2]);
			printf("parameter media_id is %d\n",media_id);
			err = minfo_delete_media_id(media_id);
			if( err < 0)
			{
				printf("minfo_delete_media_id failed\n");
				return -1;
			}
		}
		else 
		{
			printf("minfo_delete_media_id: media id  is null\n");
			return -1;

		}
	
	break;
	 

	case 5:           
		printf("minfo_move_media.......\n");
		if((argv[2] != NULL)&&(argv[3] !=NULL)&&(argv[4] != NULL))//minfo_update_media
		{

			memcpy(file_url,argv[2],MB_SVC_FILE_PATH_LEN_MAX);
			memcpy(new_file_url,argv[3],MB_SVC_FILE_PATH_LEN_MAX);
			type = atoi(argv[4]);
			printf("parameter old_file_url is %s, new_file_url is %s, type is %d\n",file_url,new_file_url,type);
			err = minfo_move_media(file_url, new_file_url, type);
			if( err < 0)
			{
				printf("minfo_update_media failed\n");
				return -1;
			}
		}
		else 
		{
			printf("minfo_update_media: old file url or new file url or type  is null\n");
			return -1;
		}
		printf("minfo_update_media leave\n");
		break;	 

	case 6:                              
		printf("minfo_update_media_name.......\n");

		if((argv[2] != NULL)&&(argv[3] !=NULL))//minfo_update_media_name
		{
			media_id = atoi(argv[2]);
			printf("parameter media_id is %d, new name is %s",media_id,argv[3]);
			err = minfo_update_media_name(media_id, argv[3]);
			if( err < 0)
			{
			printf("minfo_update_media_name failed\n");
			return -1;
			}
		}
		else 
		{
			printf("minfo_update_media_name: media id or new file name  is null\n");
			return -1;

		}		
		break;
	 
	 
	case 7:                         //minfo update meida favorite     
		printf("minfo_update_media_favorite.......\n");

		if((argv[2] != NULL)&&(argv[3] !=NULL))
		{
			media_id = atoi(argv[2]);
			favorite_level = atoi(argv[3]);
			printf("parameter media_id is %d, favorite_level is %d",cluster_id,favorite_level);
			err = minfo_update_media_favorite(media_id, favorite_level);
			if( err < 0)
			{
			printf("minfo_update_media_favorite failed\n");
			return -1;
			}
		}
		else 
		{
			printf("minfo_update_media_favorite: media id or favorite_level  is null\n");
			return -1;

		}
	 break;
	 

	case 8:                                   //minfo_delete_cluster
		printf("minfo_update_media_favorite.......\n");
		if(argv[2] != NULL) 
		{
			cluster_id = atoi(argv[2]);
			printf("parameter cluster_id is %d",cluster_id);
			err = minfo_delete_cluster(cluster_id);
			if( err < 0)
			{
				printf("minfo_delete_cluster failed\n");
				return -1;
			}
		}    
		else
		{
			printf("minfo_update_media_favorite: cluster id is null\n");
			return -1;
		}
		break;  
	 
	case 9:                                   //minfo_update_cluster_name
		printf("minfo_update_cluster_name.......\n");
		if((argv[2] != NULL)&&(argv[3] != NULL)) 
		{
			cluster_id = atoi(argv[2]);
			printf("parameter cluster_id is %d, new name is %s",cluster_id,argv[3]);
			err = minfo_update_cluster_name(cluster_id,argv[3]);
			if( err < 0)
			{
				printf("minfo_update_cluster_name failed\n");
				return -1;
			}
		}    
		else
		{
			printf("minfo_update_cluster_name: cluster id or new cluster name is null\n");
			return -1;
		}
	 break;  

	case 10:                                   //minfo_add_bookmark
		printf("minfo_add_bookmark.......\n");
		if((argv[2] != NULL)&&(argv[3] != NULL)&&(argv[4] != NULL)) 
		{
			media_id = atoi(argv[2]);
			position = atoi(argv[3]);
			printf("parameter media_id is %d,position is %d,thumb path is %s",media_id,position,argv[4]);
			err = minfo_add_bookmark(media_id,position,argv[4]);
			if( err < 0)
			{
				printf("minfo_add_bookmark failed\n");
				return -1;
			}
		}    
		else
		{
			printf("minfo_add_bookmark: media id or position or new thumb path is null\n");
			return -1;
		}
	 break;   

	case 11:                                   //minfo_delete_bookmark
		printf("minfo_delete_bookmark.......\n");
		if(argv[2] != NULL)
		{
			bookmark_id = atoi(argv[2]);
			printf("parameter bookmark_id is %d",bookmark_id);
			err = minfo_delete_bookmark(bookmark_id);
			if( err < 0)
			{
			printf("minfo_delete_bookmark failed\n");
			return -1;
			}
		}    
		else
		{
			printf("minfo_delete_bookmark: bookmark id is null\n");
			return -1;
		}
		break; 


	case 12:                                   //minfo_add_streaming
		printf("minfo_add_streaming.......\n");
		if((argv[2] != NULL)&&(argv[3] != NULL)&&(argv[4] != NULL)&&(argv[5] != NULL))
		{
			duration = atoi(argv[4]);
			printf("parameter duration is %d",duration);
			err = minfo_add_streaming(argv[2], argv[3], duration, argv[5],&id);
			printf("id is %d\n",id);
			if( err < 0)
			{
			printf("minfo_add_streaming failed\n");
			return -1;
			}
		}    
		else
		{
		printf("minfo_add_streaming: title or url or duration or thumb path is null\n");
		return -1;
		}
		printf("minfo_add_streaming leave\n");
	 break; 


	case 13:                                   //minfo_delete_streaming
		printf("minfo_delete_streaming.......\n");
		if(argv[2] != NULL)
		{
			streaming_id = atoi(argv[2]);
			printf("parameter streaming_id is %d",streaming_id);
			err = minfo_delete_streaming(streaming_id);
			if( err < 0)
			{
				printf("minfo_delete_streaming failed\n");
				return -1;
			}
		}    
		else
		{
			printf("minfo_delete_streaming: streaming id is null\n");
			return -1;
		}
	 break;		 				

	case 14:                                   //minfo_get_item_list
		printf("minfo_get_item_list.......\n");
		if(argv[2] != NULL)
		{
			minfo_item_filter item_filter1 = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,false,MINFO_MEDIA_FAV_ALL};
			//minfo_item_filter item_filter1 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_DESC,0,1,FALSE,FALSE};
			minfo_item_filter item_filter2 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,0,2,true,MINFO_MEDIA_FAV_ALL};
			minfo_item_filter item_filter3 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,1,10,true,MINFO_MEDIA_FAV_ALL};
			minfo_item_filter item_filter4 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,1,2,true,MINFO_MEDIA_FAV_ONLY};

			cluster_id = atoi(argv[2]);	
			err = minfo_get_item_list(cluster_id, item_filter1, _ite_fn, &p_list);

			if( err < 0)
			{
				printf("minfo_get_item_list failed\n");
				//return -1;
			}
			img_cnt = g_list_length(p_list);
			printf("img_cnt1 is %d,p_list is %p\n",img_cnt,p_list);
			
			for(i=0; i<img_cnt; i++)
			{
				mitem = (Mitem*)g_list_nth_data(p_list, i);
				printf("mitem ID=%d, %s\n",mitem->_id, mitem->file_url);
				printf("case14 count is %d\n",((GObject *)mitem)->ref_count); 
				//printf("mitem->meta_info->type =%d,\n",mitem->meta_info->type);	
				//printf("mitem->meta_info->video_info =%p,\n",mitem->meta_info->video_info);	
			}

// 			minfo_free_glist(&p_list);

/*
			err = minfo_get_item_list(cluster_id, item_filter2, &p_list);
			if( err < 0)
			{
				printf("minfo_get_item_list failed\n");
				//return -1;
			}
			img_cnt = g_list_length(p_list);
			printf("img_cnt2 is %d,p_list is %p\n",img_cnt,p_list);
			
			for(i=0; i<img_cnt; i++)
			{
				mitem = (Mitem*)g_list_nth_data(p_list, i);
				printf("mitem ID=%d, %p\n",mitem->_id, mitem);
				printf("mitem->meta_info =%p,\n",mitem->meta_info);
			}
// 			minfo_free_glist(&p_list);	


 
           err = minfo_get_item_list(cluster_id, item_filter3, &p_list);
			if( err < 0)
			{
				printf("minfo_get_item_list failed\n");
				//return -1;
			}
			img_cnt = g_list_length(p_list);
			printf("img_cnt3 is %d,p_list is %p\n",img_cnt,p_list);
			
			for(i=0; i<img_cnt; i++)
			{
				mitem = (Mitem*)g_list_nth_data(p_list, i);
				printf("mitem ID=%d, %p\n",mitem->_id, mitem);
				printf("mitem->meta_info =%p,\n",mitem->meta_info);
			}
			minfo_free_glist(&p_list);	



            err = minfo_get_item_list(cluster_id, item_filter4, &p_list);
			if( err < 0)
			{
				printf("minfo_get_item_list failed\n");
				return -1;
			}
			img_cnt = g_list_length(p_list);
			printf("img_cnt4 is %d,p_list is %p\n",img_cnt,p_list);
			
			for(i=0; i<img_cnt; i++)
			{
				mitem = (Mitem*)g_list_nth_data(p_list, i);
				printf("mitem ID=%d, %p\n",mitem->_id, mitem);
				printf("mitem->meta_info =%p,\n",mitem->meta_info);
			}
// 			minfo_free_glist(&p_list);	
*/

		}    
		else
		{
			printf("minfo_get_item_list: cluster id is null\n");
			return -1;
		}
		break;	

	case 15:									//minfo_get_cluster_cover
		printf("minfo_get_cluster_cover.......\n");
		if((argv[2] != NULL)&&(argv[3] != NULL))
		{
			cluster_id = atoi(argv[2]);
			img_cnt = atoi(argv[3]);
			printf("parameter cluster_id is %d, img_cnt is %d\n",cluster_id,img_cnt);

			err = minfo_get_cluster_cover(cluster_id, img_cnt, _cover_ite_fn, &p_list);

			if( err < 0)
			{
			printf("minfo_get_cluster_cover failed\n");
			return -1;
			}
			for(i=0; i<img_cnt; i++)
			{
			printf("thumb_path=%s\n",(char *)g_list_nth_data(p_list, i));
			}
		}    
		else
		{
			printf("minfo_get_cluster_cover: cluster id or img_cnt is null\n");
			return -1;
		}
	break;  

	case 16:									//minfo_get_item_cnt
		printf("minfo_get_item_cnt.......\n");
		if(argv[2] != NULL)
		{
			cluster_id = atoi(argv[2]);
			minfo_item_filter filter = {MINFO_ITEM_VIDEO,MINFO_MEDIA_SORT_BY_DATE_ASC,-1,10,true,true};
			printf("parameter cluster_id is %d\n",cluster_id);
			err = minfo_get_item_cnt(cluster_id, filter, &cnt);
			printf("cnt is %d\n",cnt);
			if( err < 0)
			{
			printf("minfo_get_item_cnt failed\n");
			return -1;
			}
		}    
		else
		{
			printf("minfo_get_item_cnt: cluster id is null\n");
			return -1;
		}
		printf("minfo_get_item_cnt leave\n");
	break;  


	case 17:								 //minfo_get_cluster_cnt
		printf("minfo_get_cluster_cnt.......\n");
		minfo_cluster_filter cluster_filter ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_DATE_ASC,0,10};

		err = minfo_get_cluster_cnt(cluster_filter, &cnt);
		printf("cnt is %d\n",cnt);
		if( err < 0)
		{
			printf("minfo_get_cluster_cnt failed\n");
			return -1;
		}
		printf("minfo_get_cluster_cnt leave\n");
		break; 


	case 18:								 //minfo_get_cluster_list
		printf("test minfo_get_cluster_list...\n");
		minfo_cluster_filter cluster_filter1 ={MINFO_CLUSTER_TYPE_ALL,MINFO_CLUSTER_SORT_BY_DATE_ASC,0,10};
		minfo_cluster_filter cluster_filter2 ={MINFO_CLUSTER_TYPE_LOCAL_ALL,MINFO_CLUSTER_SORT_BY_NAME_DESC,0,10};
			
		err = minfo_get_cluster_list(cluster_filter1, _cluster_ite_fn, &p_list);

		if( err < 0)
		{
			 printf("minfo_get_cluster_list failed\n");
			 return -1;
		}
		img_cnt = g_list_length(p_list);
		for(i=0; i<img_cnt; i++)
		{
			cluster = (Mcluster*)g_list_nth_data(p_list, i);
			printf("%p: cluster ID=%d, sns_type = %d, display_name= %s, \n", cluster, cluster->_id, cluster->sns_type,cluster->display_name);
		}		
// 		minfo_free_glist(&p_list);
		
		err = minfo_get_cluster_list(cluster_filter2, _cluster_ite_fn, &p_list);

		if( err < 0)
		{
			 printf("minfo_get_cluster_list failed\n");
			 return -1;
		}

		img_cnt = g_list_length(p_list);
		for(i=0; i<img_cnt; i++)
		{
			cluster = (Mcluster*)g_list_nth_data(p_list, i);
			printf("%p: cluster ID=%d, sns_type = %d, display_name= %s, \n", cluster, cluster->_id, cluster->sns_type,cluster->display_name);
		}		
// 		minfo_free_glist(&p_list);
		break;

	case 19:								 //minfo_get_geo_item_list
		printf("test minfo_get_geo_item_list...\n");
		minfo_item_filter filter ={0};

		for (i=0;i<5;i++)
		{
			if (argv[2+i] == NULL)
			{
			    printf("at least one parameter is null\n");
				return -1;
			}
		}
		cluster_id = atoi(argv[2]);
		min_longitude = atof(argv[3]);
		max_longitude = atof(argv[4]);
		min_latitude = atof(argv[5]);
		max_latitude = atof(argv[6]);

		minfo_folder_type folder_type = MINFO_CLUSTER_TYPE_LOCAL_ALL;

		minfo_item_filter item_filter1 = {MINFO_ITEM_ALL,MINFO_MEDIA_SORT_BY_NONE,-1,-1,true,MINFO_MEDIA_FAV_ALL};
			//minfo_item_filter item_filter1 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_DESC,0,1,FALSE,FALSE};
		minfo_item_filter item_filter2 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,0,2,true,MINFO_MEDIA_FAV_ALL};
		minfo_item_filter item_filter3 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,1,10,true,MINFO_MEDIA_FAV_ALL};
		minfo_item_filter item_filter4 = {MINFO_ITEM_IMAGE,MINFO_MEDIA_SORT_BY_DATE_ASC,1,2,true,MINFO_MEDIA_FAV_ONLY};

		err = minfo_get_geo_item_list(cluster_id, 
									folder_type,
						            item_filter1, 
						            min_longitude, 
						            max_longitude, 
						            min_latitude, 
						            max_latitude,
									_ite_fn,
						            &p_list);

		if( err < 0)
		{
			 printf("minfo_get_geo_item_list failed\n");
			 return -1;
		}

		img_cnt = g_list_length(p_list);
		printf("minfo_get_geo_item_list len = %d\n",img_cnt);
		
		for(i=0; i<img_cnt; i++)
		{
			mitem = (Mitem*)g_list_nth_data(p_list, i);
			printf("item ID=%d, %s, %p\n",mitem->_id, mitem->file_url, mitem);
			minfo_mitem_destroy(mitem);
		}
		_mb_svc_glist_free(&p_list, false);
		printf("minfo_get_geo_item_list leave\n");
		break;  


	case 20:								 //minfo_delete_web_cluster
			printf("test minfo_delete_web_cluster...\n");
			if(argv[2] != NULL)
			{
				cluster_id = atoi(argv[2]);
				printf("parameter cluster_id is %d\n",cluster_id);
				err = minfo_delete_web_cluster(cluster_id);
				if( err < 0)
				{
					printf("minfo_delete_web_cluster failed\n");
					return -1;
				}
			}    
			else
			{
				printf("minfo_delete_web_cluster: cluster id is null\n");
				return -1;
			}
			printf("minfo_delete_web_cluster leave\n");
			break;



	case 21:								 //minfo_add_web_media
			printf("test minfo_delete_web_cluster...\n");
			if((argv[2] != NULL)&&(argv[3] != NULL)&&(argv[4] != NULL)&&(argv[5] != NULL))
			{
				cluster_id = atoi(argv[2]);
				printf("parameter cluster_id is %d, http_url is %s,file name is %s,thumb path is %s\n",cluster_id,argv[3],argv[4],argv[5]);
				err = minfo_add_web_media(cluster_id, argv[3], argv[4], argv[5]);
				if( err < 0)
				{
				printf("minfo_add_web_media failed\n");
				return -1;
				}
			}    
			else
			{
				printf("minfo_add_web_media: at least one parameter is null\n");
				return -1;
			}
			break; 
	case 22:								 //minfo_add_web_cluster
			printf("test minfo_add_web_cluster...\n");
			if((argv[2] != NULL)&&(argv[3] != NULL)&&(argv[4] != NULL))
			{
				sns_type = atoi(argv[2]);
				printf("parameter sns_type is %d, name is %s,web_account_id is %s\n",sns_type,argv[3],argv[4]);
				err = minfo_add_web_cluster(sns_type, argv[3],argv[4],&id);
				if( err < 0)
				{
				printf("minfo_add_web_cluster failed\n");
				return -1;
				}
			}    
			else
			{
				printf("minfo_add_web_cluster: at least one parameter is null\n");
				return -1;
			}
	break; 



	case 23:								 //minfo_add_cluster
			printf("test minfo_add_cluster...\n");
			if(argv[2] != NULL)
			{
				printf("parameter cluster_url is %s\n",argv[2]);
				err = minfo_add_cluster(argv[2],&id);
				if( err < 0)
				{
				printf("minfo_add_cluster failed\n");
				return -1;
				}
			}    
			else
			{
				printf("minfo_add_cluster: cluster_url is null\n");
				return -1;
			}
	break; 
	case 24:								 //minfo_delete_web_cluster
			printf("test minfo_delete_web_cluster...\n");
			if(argv[2] != NULL)
			{
				cluster_id = atoi(argv[2]); 
				printf("parameter cluster_id is %d\n",cluster_id);
				err = minfo_delete_web_cluster(cluster_id);
				if( err < 0)
				{
				printf("minfo_delete_web_cluster failed\n");
				return -1;
				}
			}    
			else
			{
				printf("minfo_delete_web_cluster: cluster_id is null\n");
				return -1;
			}
	break; 


	case 25:								 //minfo_get_meta_info
			printf("test minfo_delete_web_cluster...\n");
			if(argv[2] != NULL)
			{

				media_id = atoi(argv[2]); 
				printf("parameter media_id is %d\n",media_id);
				ret = minfo_get_meta_info(media_id, &mt);
				if( ret < 0)
				{
					printf("minfo_get_meta_info failed\n");
					return -1;
				}
				if(mt->type == MINFO_ITEM_IMAGE) {
				printf("item_id =%d, mt->image_info--address%p", mt->item_id, mt->image_info);
				}
				else if(mt->type == MINFO_ITEM_VIDEO) {
				printf("item_id =%d, mt->video_info--address%p, duration+%d", mt->item_id, mt->video_info, mt->video_info->duration);
			}    
			}    
			else
			{
				printf("minfo_get_meta_info: media_id is null\n");
				return -1;
			}
			break; 

	case 26:
/*
			_mb_svc_drop_tbl();	
			_mb_svc_create_tbl();
			test_insert_record_bookmark();
			test_insert_record_folder();
			test_insert_record_media();
			test_insert_record_video_meta();
			test_insert_record_image_meta();
			test_insert_record_webstreaming();
*/
			break; 
	case 27:
		printf("test minfo_move_media...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			printf("parameter old path is %s\n",argv[2]);
			printf("parameter new path is %s\n",argv[3]);
			minfo_move_media( argv[2], argv[3], MINFO_ITEM_IMAGE);
		}
		break;
	case 28:
		printf("minfo_copy_mediaminfo_move_media...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			printf("parameter old path is %s\n",argv[2]);
			printf("parameter new path is %s\n",argv[3]);
			minfo_copy_media( argv[2], argv[3], MINFO_ITEM_IMAGE);
		}
		break;	
	case 29:
		printf("minfo_mv_media...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			media_id = atoi(argv[2]);
			cluster_id = atoi(argv[3]);
			printf("parameter media idis %d\n",media_id);
			printf("parameter cluster id is %d\n",cluster_id);
			minfo_mv_media( media_id,  cluster_id);
		}
		break;
	case 30:
		printf("minfo_cp_media...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			media_id = atoi(argv[2]);
			cluster_id = atoi(argv[3]);
			printf("parameter media idis %d\n",media_id);
			printf("parameter cluster id is %d\n",cluster_id);
			minfo_cp_media( media_id,  cluster_id);
		}
		break;
	case 31:
		printf("test minfo_get_item \n");
		if(argv[2] != NULL )
		{
			Mitem* mi = NULL;
			ret = minfo_get_item(argv[2], &mi);
			if(ret < 0)
			{
				printf("minfo_get_item fail: %s \n", argv[2]);
				return -1;
			}

			printf("minfo_get_item:%s, %s \n", mi->thumb_url,mi->display_name);
		}
		break;
	case 32:
		printf("test minfo_get_cluster \n");
		if(argv[2] != NULL )
		{
			Mcluster* mc = NULL;
			ret = minfo_get_cluster(argv[2], 0, &mc);
			if(ret < 0)
			{
				printf("minfo_get_cluster fail: %s \n", argv[2]);
				return -1;
			}

			printf("minfo_get_cluster:%s, %s \n", mc->thumb_url,mc->display_name);
		}
	break;

	case 33:
		printf("test minfo_set_cluster_lock_status \n");
		
		ret = minfo_set_cluster_lock_status(2, 1);
		if(ret < 0)
		{
			printf("minfo_set_cluster_lock_status fail: \n" );
			return -1;
		}

		printf("minfo_set_cluster_lock\n");
	break;

	case 34:
		printf("test minfo_get_cluster_lock_status \n");
		int status = -1;
		ret = minfo_get_cluster_lock_status(2, &status);
		if(ret < 0)
		{
			printf("minfo_get_cluster_lock_status fail\n" );
			return -1;
		}

		printf("minfo_get_cluster_lock_status : %d\n", status);
	break;

	case 35:
		printf("test minfo_get_media_path \n");
		char media_path[255] = {0,};

		ret = minfo_get_media_path(MINFO_PHONE, media_path, 255);
		if(ret < 0)
		{
			printf("minfo_get_media_path fails\n" );
			return -1;
		}

		printf("minfo_get_media_path : %s\n", media_path);
	break;

	case 36:
		printf("test minfo_get_thumb_path \n");
		char thumb_path[255] = {0,};

		if(argv[2] && argv[3]) {
			int type = atoi(argv[3]);
			if( type == 1 ) {
				ret = minfo_get_thumb_path(argv[2], thumb_path, 255);
			} else if( type == 2) {
				ret = minfo_get_thumb_path_for_video(argv[2], thumb_path, 255);
			} else {
				printf("minfo_get_thumb_path fails( invalid type )\n" );
				return -1;
			}
			if(ret < 0)
			{
				printf("minfo_get_thumb_path fails\n" );
				return -1;
			}
		}

		printf("minfo_get_thumb_path : %s\n", thumb_path);
	break;

	case 37:
		printf("test minfo_set_db_valid...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			printf("parameter one is %s\n",argv[2]);
			printf("parameter two is %s\n",argv[3]);
			minfo_set_db_valid(atoi(argv[2]), atoi(argv[3]));
		}
			
	break;


	case 38:
		printf("test minfo_get_cluster_name_by_id...\n");
		int ret = -1;
		char folder_name[100];

		if(argv[2] != NULL )
		{
			printf("parameter id is %s\n",argv[2]);
			
			ret = minfo_get_cluster_name_by_id(atoi(argv[2]), folder_name, 100 );
			if( ret < 0 ) {
				printf("Fail : %d", ret );
			} 
			else {
				printf("Success : folder name is %s\n", folder_name);
			}
		}
			
	break;

	case 39:
		printf("test minfo_set_item_valid...\n");
		if(argv[2] != NULL && argv[3] != NULL)
		{
			printf("parameter store type is %s\n",argv[2]);
			printf("parameter valid is %s\n",argv[3]);
			minfo_set_item_valid(atoi(argv[2]), "/opt/media/Frinsd/seo1.jpg", atoi(argv[3]));
		}
			
	break;

	case 40:
		printf("test minfo_delete_invalid_media_records...\n");
		if(argv[2] != NULL )
		{
			printf("parameter store type is %s\n",argv[2]);
			ret = minfo_delete_invalid_media_records(atoi(argv[2]));
			if( ret < 0 ) {
				printf("minfo_delete_invalid_media_records fails : %d\n", ret );
			}
		}
			
	break;

	case 41:
		printf("test minfo_add_tag...\n");
		if (argc < 4) {
			printf("test minfo_add_tag the count of arguments is wrong!\n");
			break;
		}
			
		if (argv[2] != NULL)
		{
			printf("parameter store type is %s\n",argv[2]);
			ret = minfo_add_tag(atoi(argv[2]), argv[3]);
			if( ret < 0 ) {
				printf("minfo_add_tag fails : %d\n", ret );
			}
		}
			
	break;

	case 42:
		printf("test minfo_get_media_list_by_tagname...\n");
			
		if (argv[2] != NULL)
		{
			printf("parameter store type is %s\n",argv[2]);
			ret = minfo_get_media_list_by_tagname(argv[2], FALSE, _ite_fn, &p_list);
			if( ret < 0 ) {
				printf("minfo_get_media_list_by_tagname fails : %d\n", ret );
			}
		}
			
	break;

	case 43:
		printf("test minfo_get_tag_list_by_media_id...\n");
			
		if (argv[2] != NULL)
		{
			printf("parameter store type is %s\n",argv[2]);
			ret = minfo_get_tag_list_by_media_id(atoi(argv[2]), _ite_tag_fn, &p_list);
			if( ret < 0 ) {
				printf("minfo_get_tag_list_by_media_id fails : %d\n", ret );
			}
		}
			
	break;

	case 44:
		printf("test minfo_get_item_by_id \n");
		if(argv[2] != NULL )
		{
			Mitem* mi = NULL;
			ret = minfo_get_item_by_id(atoi(argv[2]), &mi);
			if(ret < 0)
			{
				printf("minfo_get_item_by_id fail: %s \n", argv[2]);
				return -1;
			}

			printf("minfo_get_item_by_id:%s, %s \n", mi->thumb_url,mi->display_name);
		}
		break;

	case 45:
		printf("test minfo_get_cluster_fullpath_by_id \n");
		char __folder_path[MB_SVC_FILE_PATH_LEN_MAX+1] = {0};
		if(argv[2] != NULL )
		{
			Mitem* mi = NULL;
			ret = minfo_get_cluster_fullpath_by_id(atoi(argv[2]), __folder_path, sizeof(__folder_path));
			if(ret < 0)
			{
				printf("minfo_get_cluster_fullpath_by_id fail: %d \n", ret);
				return -1;
			}

			printf("minfo_get_cluster_fullpath_by_id:%s\n", __folder_path);
		}
		break;

	
	case 46:
			printf("test minfo_rename_tag \n");
			if(argv[2] != NULL && argv[3] != NULL)
			{
				Mitem* mi = NULL;
				ret = minfo_rename_tag(argv[2], argv[3]);
				if(ret < 0)
				{
					printf("minfo_rename_tag fail: %d \n", ret);
					return -1;
				}
			}
			break;
			
	case 47:
			printf("test minfo_rename_tag_by_id! \n");
			if(argv[2] != NULL && argv[3] != NULL && argv[4] != NULL)
			{
				Mitem* mi = NULL;
				ret = minfo_rename_tag_by_id(atoi(argv[2]), argv[3], argv[4]);
				if(ret < 0)
				{
					printf("minfo_rename_tag_by_id fail: %d \n", ret);
					return -1;
				}
			}
			break;
			
	default:
		  printf("wrong test sequence, no matched test case exist!!\n");                          
		  return -1;
	} 	 

	err = minfo_finalize();

	if(err<0)
	{
		printf("minfo_finalize error\n");
		return -1;
	}
    printf("this is test programe------------leave\n");
    return 0;

	printf("################################################\n");
	printf("---------------self-test problem succeed---------\n");
	printf("################################################\n");
}

