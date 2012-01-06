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
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <util-func.h>
#include "media-info.h"
#include "media-info-debug.h"
#include "media-info-env.h"
#include "media-info-util.h"
#include "media-svc.h"
#include "audio-svc.h"
#include "audio-svc-error.h"

#define TIMEOUT_SEC		10

int mediainfo_open(void)
{
	mediainfo_dbg("");
	int err = -1;

	err = minfo_init();
	if (err < MB_SVC_ERROR_NONE) {
		mediainfo_dbg("minfo_init falis");
	}

	err = audio_svc_open();
	if (err < AUDIO_SVC_ERROR_NONE) {
		mediainfo_dbg("audio_svc_open falis");
	}

	if (err < 0) return MEDIA_INFO_ERROR_DATABASE_CONNECT;

	mediainfo_dbg("Success");
	return err;
}

int mediainfo_close(void)
{
	mediainfo_dbg("");
	int err = -1;

	err = minfo_finalize();
	if (err < MB_SVC_ERROR_NONE) {
		mediainfo_dbg("minfo_finalize falis");
	}

	err = audio_svc_close();
	if (err < AUDIO_SVC_ERROR_NONE) {
		mediainfo_dbg("audio_svc_close falis");
	}

	if (err < 0) return MEDIA_INFO_ERROR_DATABASE_DISCONNECT;

	mediainfo_dbg("Success");
	return err;
}

int mediainfo_connect_db_with_handle(sqlite3 **db_handle)
{
	mediainfo_dbg("");
	int err = -1;
	err =
	    db_util_open(MEDIA_INFO_DATABASE_NAME, db_handle,
			 DB_UTIL_REGISTER_HOOK_METHOD);

	if (SQLITE_OK != err) {
		*db_handle = NULL;

		return MEDIA_INFO_ERROR_DATABASE_CONNECT;
	}

	return MEDIA_INFO_ERROR_NONE;
}

int mediainfo_disconnect_db_with_handle(sqlite3 *db_handle)
{
	mediainfo_dbg("");
	int err = -1;
	err = db_util_close(db_handle);

	if (SQLITE_OK != err) {
		db_handle = NULL;

		return MEDIA_INFO_ERROR_DATABASE_DISCONNECT;
	}

	return MEDIA_INFO_ERROR_NONE;
}

#define MINFO_REGISTER_PORT 1001

static bool _mediainfo_is_valid_path(const char *path)
{
       if (path == NULL)
               return false;

	if (strncmp(path, MEDIAINFO_PHONE_ROOT_PATH, strlen(MEDIAINFO_PHONE_ROOT_PATH)) == 0) {
		return true;
	} else if (strncmp(path, MEDIAINFO_MMC_ROOT_PATH, strlen(MEDIAINFO_MMC_ROOT_PATH)) == 0) {
		return true;
	} else
		return false;

       return true;
}

DEPRECATED_API int mediainfo_register_file(const char *file_full_path)
{
	int exist;
	int err;
	int sockfd;
	int recv_msg = MEDIA_INFO_ERROR_NONE;
	int server_addr_size;
	struct sockaddr_in server_addr;
	struct timeval tv_timeout = { TIMEOUT_SEC, 0 };

	if(!_mediainfo_is_valid_path(file_full_path)) {
		mediainfo_dbg("Invalid path : %s", file_full_path);
		return MEDIA_INFO_ERROR_INVALID_PATH;
	}

	exist = open(file_full_path, O_RDONLY);
	if(exist < 0) {
		mediainfo_dbg("Not exist path : %s", file_full_path);
		return MEDIA_INFO_ERROR_INVALID_PATH;
	}
	close(exist);

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		mediainfo_dbg("socket create fail");
		return MEDIA_INFO_ERROR_SOCKET_CONN;
	}

	/*add timeout : timeout is 10 sec.*/
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout)) == -1) {
		mediainfo_dbg("setsockopt failed");
		return MEDIA_INFO_ERROR_SOCKET_CONN;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(MINFO_REGISTER_PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	err = sendto(sockfd, file_full_path, strlen(file_full_path), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (err < 0) {
		mediainfo_dbg("sendto error");
		perror("sendto error : ");
		return MEDIA_INFO_ERROR_SOCKET_SEND;
	} else {
		mediainfo_dbg("SEND OK");
	}

	server_addr_size = sizeof(server_addr);
	err = recvfrom(sockfd, &recv_msg, sizeof(recv_msg), 0 , (struct sockaddr*)&server_addr, (socklen_t *)&server_addr_size);
	if (err < 0) {
		if (errno == EWOULDBLOCK) {
			mediainfo_dbg("recvfrom timeout");
			return MEDIA_INFO_ERROR_SOCKET_RECEIVE_TIMEOUT;
		} else {
			mediainfo_dbg("recvfrom error");
			perror("recvfrom error : ");
			return MEDIA_INFO_ERROR_SOCKET_RECEIVE;
		}
	} else {
		mediainfo_dbg("RECEIVE OK");
		mediainfo_dbg("client receive: %d", recv_msg);
	}	
	
	close(sockfd);

	return recv_msg;
}

DEPRECATED_API int mediainfo_list_new(minfo_list *list)
{
	*list = g_array_new(TRUE, TRUE, sizeof(char*));

	return MB_SVC_ERROR_NONE;
}

DEPRECATED_API int mediainfo_list_add(minfo_list list, const char* file_full_path)
{
	mediainfo_dbg("");

	if (!list) {
		mediainfo_dbg("list == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	if (!file_full_path) {
		mediainfo_dbg("file_full_path == NULL");
		return MB_SVC_ERROR_INVALID_PARAMETER;
	}

	minfo_list ret_list = NULL;
	char *path = strdup(file_full_path);

	int len = list->len + 1;
	int i;
	char *data = NULL;

	ret_list = g_array_append_val(list, path);
	if(ret_list == NULL) {
		mediainfo_dbg("g_array_append_val fails");
		return MB_SVC_ERROR_UNKNOWN;
	}

	list = ret_list;

	for(i = 0; i < len; i++) {
		data = g_array_index(list, char*, i);
		mediainfo_dbg("%d, %s", i, data);
	}

	return MB_SVC_ERROR_NONE;
}

DEPRECATED_API int mediainfo_list_free(minfo_list list)
{
	if (!list)
		return MB_SVC_ERROR_INVALID_PARAMETER;

	int len = list->len + 1;
	int i;
	char *data = NULL;

	for(i = 0; i < len; i++) {
		data = g_array_index(list, char*, i);
		free(data);
	}

	g_array_free(list, TRUE);

	return MB_SVC_ERROR_NONE;
}

DEPRECATED_API int mediainfo_register_files(const minfo_list list)
{
	if (!list)
		return MB_SVC_ERROR_INVALID_PARAMETER;

	int len = list->len + 1;
	int i;
	char *data;

	for(i = 0; i < len; i++) {
		data = g_array_index(list, char*, i);
		mediainfo_register_file(data);
	}

	return MB_SVC_ERROR_NONE;
}
