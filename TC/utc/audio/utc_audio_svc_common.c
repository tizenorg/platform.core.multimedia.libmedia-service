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

#include "utc_audio_svc_common.h"

bool copy_file(const char* srcPath, const char* destPath);

bool copy_file(const char* srcPath, const char* destPath)
{
	FILE* fs = NULL;
	FILE* fd = NULL;
	char buff[4096] = {0};
	int n = 0;
	int m = 0;
	bool result = FALSE;
	bool remove_dest = FALSE;

	fs=fopen(srcPath, "rb");
	if(fs == NULL)
	{
		dts_message("fopen", "failed to open source file: %s", srcPath);
		return FALSE;
	}


	fd=fopen(destPath, "wb");
	if(fd == NULL)
	{
		dts_message("fopen", "ailed to open dest file: %s", destPath);

		fclose(fs);
		return FALSE;
	}


	while (1)
	{
		result = feof(fs);
		if (!result)
		{
			n = fread(buff, sizeof(char), sizeof(buff), fs);
			if(n > 0 && n <= sizeof(buff))
			{
				m = fwrite(buff, sizeof(char), n, fd);
				if (m <= 0)
				{
					dts_message("fwrite", "fwrite = %d \n", m);

					result = FALSE;
					remove_dest = TRUE;
					goto CATCH;
				}
			}
			else
			{
				dts_message("fread", "fread = %d \n", n);
				result = TRUE;
				goto CATCH;
			}
		}
		else
		{
			result = TRUE;
			goto CATCH;
		}


	}

CATCH:
	fflush(fd);
	fsync(fileno(fd));
	fclose(fd);
	fclose(fs);

	if(remove_dest)
	{
		remove(destPath);
		sync();
	}

	dts_message("copy_file", "copying file is successful\n");
	return result;
}

bool get_item_audio_id(char * audio_id_val)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	AudioHandleType *item;
	char * audio_id = NULL;
	int size = 0;

	retval = check_default_item_exist();
	if(!retval)
	{
		dts_message("check_default_item_exist","fail to check default item exist.");
		return FALSE;
	}

	ret = audio_svc_item_new(&item);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_item_new","fail to audio_svc_item_new.");
		return FALSE;
	}
	
	ret = audio_svc_get_item_by_path(db_handle, DEFAULT_FILE, item);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_item_free(item);
		dts_message("audio_svc_get_item_by_path","fail to audio_svc_get_item_by_path.");
		return FALSE;
	}
	
	ret = audio_svc_item_get_val(item, AUDIO_SVC_TRACK_DATA_AUDIO_ID, &audio_id, &size, -1);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_item_free(item);
		dts_message("audio_svc_item_get_val","fail to audio_svc_item_get_val.");
		return FALSE;
	}

	if(audio_id == NULL)
	{
		audio_svc_item_free(item);
		dts_message("audio_svc_item_get_val","wrong audio_id.");
		return FALSE;
	}
	
	strncpy(audio_id_val, audio_id, AUDIO_SVC_UUID_SIZE+1);

	ret = audio_svc_item_free(item);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_item_free","fail to audio_svc_item_free.");
		return FALSE;
	}
	
	return TRUE;
	
}

bool get_playlist_id(int * playlist_id)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	AudioHandleType*playlists = NULL;
	int count = -1;
	int plst_id = 0;
	
	retval = check_default_playlist_exist();
	if(!retval)
	{
		dts_message("check_default_playlist_exist","fail to check default playlist exist.");
		return FALSE;
	}

	ret = audio_svc_count_playlist(db_handle, "", "", &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_count_playlist","unable to get playlist.");
		return FALSE;
	}
	
	ret = audio_svc_playlist_new(&playlists, count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_count_playlist","there is no playlist.");
		return FALSE;
	}
	
	//get all the playlists in db.
	ret = audio_svc_get_playlist(db_handle, 
				NULL, //filter_string,
				NULL, //filter_string2,
				0, //offset,
				count,
				playlists);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_message("audio_svc_get_playlist","unable to get playlist.");
		return FALSE;
	}
	
	//get the playlist id and playlist name of each playlist
	ret = audio_svc_playlist_get_val(playlists, 0, AUDIO_SVC_PLAYLIST_ID, &plst_id, -1);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		audio_svc_playlist_free(playlists);
		dts_message("audio_svc_get_playlist","unable to get value for playlist.");
		return FALSE;
	}

	if(plst_id < 1)
	{
		audio_svc_playlist_free(playlists);
		dts_message("audio_svc_get_playlist","unable to get value for playlist.");
		return FALSE;
	}
	
	*playlist_id = plst_id;

	ret = audio_svc_playlist_free(playlists);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_playlist_free","unable to audio_svc_playlist_free.");
		return FALSE;
	}
	
	return TRUE;
}	

bool check_default_item_exist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	audio_svc_storage_type_e storage = AUDIO_SVC_STORAGE_PHONE;

	ret = audio_svc_check_item_exist(db_handle, DEFAULT_FILE);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		ret = audio_svc_insert_item(db_handle, storage, DEFAULT_FILE, AUDIO_SVC_CATEGORY_MUSIC);
		if (ret != AUDIO_SVC_ERROR_NONE)
		{
			dts_message("audio_svc_insert_item","fail to insert item.");
			return FALSE;
		}
	}
	
	return TRUE;
}

bool check_temp_item_file_exist()
{
	int ret = AUDIO_SVC_ERROR_NONE;

	ret = audio_svc_check_item_exist(db_handle, TEST_FILE);
	if(ret != AUDIO_SVC_ERROR_NONE)
	{
		copy_file(DEFAULT_FILE, TEST_FILE);
	}
	else
	{
		ret = audio_svc_delete_item_by_path(db_handle, TEST_FILE);
		if (ret != AUDIO_SVC_ERROR_NONE)
		{
			dts_message("audio_svc_delete_item_by_path","fail to delete item.");
			return FALSE;
		}
	}

	return TRUE;
}

bool check_default_playlist_exist()
{
	int ret = AUDIO_SVC_ERROR_NONE;
	int count = -1;

	ret = audio_svc_count_playlist(db_handle, "", "", &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_count_playlist","unable to get playlist.");
		return FALSE;
	}

	if(count < 1)
	{
		int playlist_id = 0;
		ret = audio_svc_add_playlist(db_handle, "plst_test", &playlist_id);
		if (ret != AUDIO_SVC_ERROR_NONE)
		{
			dts_message("audio_svc_add_playlist","fail to add playlist");
			return FALSE;
		}
		
		ret = audio_svc_count_playlist(db_handle, "", "", &count);
		if (ret != AUDIO_SVC_ERROR_NONE)
		{
			dts_message("audio_svc_count_playlist","unable to get playlist.");
			return FALSE;
		}
		if(count < 1)
		{
			dts_message("audio_svc_count_playlist","there is no playlist");
			return FALSE;
		}
	}
	
	return TRUE;
}

bool check_playlist_has_item(int * playlist_id, char * audio_id_val)
{
	int ret = AUDIO_SVC_ERROR_NONE;
	bool retval = FALSE;
	int playlist_idx = 0;
	char audio_id[AUDIO_SVC_UUID_SIZE+1] = {0, };
	int count = -1;
	
	retval = get_playlist_id(&playlist_idx);
	if(!retval)
	{
		dts_message("get_playlist_id","fail to get playlist id.");
		return FALSE;
	}

	retval = get_item_audio_id(audio_id);
	if(!retval)
	{
		dts_message("get_item_audio_id","fail to get item audio_id.");
		return FALSE;
	}

	ret = audio_svc_check_duplicate_insertion_in_playlist(db_handle, playlist_idx, audio_id, &count);
	if (ret != AUDIO_SVC_ERROR_NONE)
	{
		dts_message("audio_svc_count_playlist","unable to get playlist.");
		return FALSE;
	}

	if(count < 1)
	{
		ret = audio_svc_add_item_to_playlist(db_handle, playlist_idx, audio_id);
		if (ret != AUDIO_SVC_ERROR_NONE)
		{
			dts_message("audio_svc_add_item_to_playlist","unable to audio_svc_add_item_to_playlist.");
			return FALSE;
		}
	}

	*playlist_id = playlist_idx;
	strncpy(audio_id_val, audio_id, AUDIO_SVC_UUID_SIZE+1);
	
	return TRUE;
}
