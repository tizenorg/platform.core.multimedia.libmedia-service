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

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <media-svc.h>

#define PLUGIN_SO_FILE_NAME  "/usr/lib/libmedia-svc-plugin.so"
void *funcHandle = NULL;

static void msg_print(int line, char *msg);

int (*svc_check_item)			(const char *file_path, const char * mime_type, char ** err_msg);
int (*svc_connect)				(void ** handle, char ** err_msg);
int (*svc_disconnect)			(void * handle, char ** err_msg);
int (*svc_check_item_exist)		(void* handle, const char *file_path, int storage_type, char ** err_msg);
int (*svc_insert_item_immediately)	(void* handle, const char *file_path, int storage_type, const char * mime_type, char ** err_msg);

int __load_functions()
{
	msg_print(__LINE__, "__load_functions");

	funcHandle = dlopen (PLUGIN_SO_FILE_NAME, RTLD_LAZY);
	if (!funcHandle) {
		fprintf (stderr,"error: %s\n", dlerror());
	}

	svc_check_item		= dlsym (funcHandle, "check_item");
	svc_connect			= dlsym (funcHandle, "connect");
	svc_disconnect		= dlsym (funcHandle, "disconnect");
	svc_check_item_exist	= dlsym (funcHandle, "check_item_exist");
	svc_insert_item_immediately	= dlsym (funcHandle, "insert_item_immediately");

	if ( !svc_check_item ||
		 !svc_connect ||
		 !svc_disconnect ||
		 !svc_insert_item_immediately ||
		 !svc_check_item_exist) {
		fprintf(stderr,"error: %s\n", dlerror());
		return -1;
	}

	return 0;
}

int __unload_functions(void)
{
	msg_print(__LINE__, "__unload_functions");

	if (funcHandle)
	{
		dlclose (funcHandle);
	}

	return 0;
}

int main()
{
	int ret = 0;
	MediaSvcHandle * db_handle = NULL;
	char * err_msg = NULL;
	char path[1024] = {0,};
	char type[1024] = {0,};

	ret = __load_functions();
	if(ret < 0) {
		msg_print(__LINE__, "__load_functions error");
		return -1;
	} else {
		msg_print(__LINE__, "__load_functions success");
	}

	//check_item ================================================
	ret = svc_check_item("/opt/media/Music/Over the horizon.mp3", "audio/mpeg", &err_msg);
	if(ret < 0) {
		msg_print(__LINE__, "svc_check_item error");
		if(err_msg != NULL) {
			printf("err_msg[%s]\n", err_msg);
			free(err_msg);
			err_msg = NULL;
		}
		__unload_functions();
		return -1;
	} else {
		msg_print(__LINE__, "svc_check_item success");
	}

	//db open ==================================================
	ret = svc_connect(&db_handle, &err_msg);
	if(ret < 0) {
		msg_print(__LINE__, "svc_connect error");
		if(err_msg != NULL) {
			printf("err_msg[%s]\n", err_msg);
			free(err_msg);
			err_msg = NULL;
		}
		__unload_functions();
		return -1;
	} else {
		msg_print(__LINE__, "svc_connect success");
	}

#if 1
	ret = media_svc_create_table(db_handle);
	if (ret < 0) {
		msg_print(__LINE__, "table already exists");
	} else {
		msg_print(__LINE__, "table create success");
	}
#endif

	while (1) {

	printf("Enter path and mimetype ( ex. /opt/media/a.jpg image ) : ");
	scanf("%s %s", path, type);

	//check_item_exist ============================================
	ret = svc_check_item_exist(db_handle, path, 0, &err_msg);
	if(ret < 0) {
		msg_print(__LINE__, "svc_check_item_exist error");
		if(err_msg != NULL) {
			printf("err_msg[%s]\n", err_msg);
			free(err_msg);
			err_msg = NULL;
		}
		//__unload_functions();
		//return -1;
	} else {
		msg_print(__LINE__, "svc_check_item_exist success");
	}

	// svc_check_item_exist ============================================
	ret = svc_insert_item_immediately(db_handle, path, 0, type, &err_msg);
	if(ret < 0) {
		msg_print(__LINE__, "svc_insert_item_immediately error");
		if(err_msg != NULL) {
			printf("err_msg[%s]\n", err_msg);
			free(err_msg);
			err_msg = NULL;
		}
		//__unload_functions();
		//return -1;
	} else {
		msg_print(__LINE__, "svc_insert_item_immediately success");
	}
	} // End of While

	ret = media_svc_insert_folder(db_handle, 0,  path);
	if(ret < 0) {
		msg_print(__LINE__, "media_svc_insert_folder error ");
	} else {
		msg_print(__LINE__, "media_svc_insert_folder success");
	}

	//db close ==================================================
	ret = svc_disconnect(db_handle, &err_msg);
	if(ret < 0) {
		msg_print(__LINE__, "svc_disconnect error");
		if(err_msg != NULL) {
			printf("err_msg[%s]\n", err_msg);
			free(err_msg);
			err_msg = NULL;
		}
		__unload_functions();
		return -1;
	} else {
		msg_print(__LINE__, "svc_disconnect success");
	}

	__unload_functions();

	return 0;
}


static void msg_print(int line, char *msg)
{
	fprintf(stderr, "[%d]%s\n", line, msg);
}

