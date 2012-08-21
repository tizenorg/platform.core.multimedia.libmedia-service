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

#include "visual-svc-util.h"
#include "visual-svc-debug.h"
#include "visual-svc-error.h"
#include "media-svc-structures.h"
#include "media-svc-types.h"

#include <vconf.h>
#include <vconf-keys.h>
#include <drm_client.h>
#include <string.h>
#include <aul/aul.h>
#include <sys/stat.h>

bool _mb_svc_get_file_display_name(const char *file_path, char *file_name)
{
	char *result = NULL;

	if ((result = strrchr(file_path, '/'))) {
		strncpy(file_name, (result + 1), MB_SVC_FILE_NAME_LEN_MAX + 1);
		return TRUE;
	}

	strncpy(file_name, file_path, MB_SVC_FILE_NAME_LEN_MAX + 1);
	file_name[MB_SVC_FILE_NAME_LEN_MAX] = '\0';

	return TRUE;
}

bool _mb_svc_get_file_parent_path(const char *file_path, char *parent_path)
{
	char file_name[MB_SVC_FILE_NAME_LEN_MAX + 1] = { 0 };

	_mb_svc_get_file_display_name(file_path, file_name);
	strncpy(parent_path, file_path, MB_SVC_FILE_PATH_LEN_MAX + 1);
	parent_path[strlen(file_path) - strlen(file_name) - 1] = '\0';

	if (strlen(parent_path) == 0) {
		strncpy(parent_path, "/", MB_SVC_FILE_PATH_LEN_MAX + 1);
	}

	return TRUE;
}

bool _mb_svc_get_dir_display_name(const char *dir_path, char *dir_name)
{
	char path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char *result = NULL;

	strncpy(path, dir_path, sizeof(path));

	if ((result = strrchr(path, '/'))) {
		if (*(result + 1) == '\0') {
			*result = '\0';
		}
	}

	if (strncmp(dir_path, MB_SVC_PATH_MMC, strlen(dir_path)) == 0
	    || strncmp(dir_path, MB_SVC_PATH_PHONE, strlen(dir_path)) == 0) {
		mb_svc_debug("dir path is empty because path is root");
		strncpy(dir_name, "", MB_SVC_FILE_NAME_LEN_MAX + 1);
		return true;
	}

	if ((result = strrchr(path, '/'))) {
		strncpy(dir_name, (result + 1), MB_SVC_FILE_NAME_LEN_MAX + 1);
		return TRUE;
	}

	strncpy(dir_name, path, MB_SVC_FILE_NAME_LEN_MAX + 1);
	dir_name[MB_SVC_FILE_NAME_LEN_MAX] = '\0';

	return TRUE;
}

/*
** in this funtion, if dir_path equals "/01/02/03", parent path is "/01/02/", not "01/02"
** if dir_path equals "/01", its parent path is "/"
*/
bool _mb_svc_get_dir_parent_path(const char *dir_path, char *parent_path)
{
	char dir_name[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };

	_mb_svc_get_dir_display_name(dir_path, dir_name);
	strncpy(parent_path, dir_path, MB_SVC_DIR_PATH_LEN_MAX + 1);
	parent_path[strlen(parent_path) - strlen(dir_name)] = '\0';
	mb_svc_debug("parent_path is %s", parent_path);
	return TRUE;
}

int _mb_svc_get_file_dir_modified_date(const char *full_path)
{
	struct stat statbuf;
	int fd = 0;
	int err = 0;

	memset(&statbuf, 0, sizeof(struct stat));
	fd = stat(full_path, &statbuf);
	if (fd == -1) {
		err = errno;
		mb_svc_debug("stat(%s) fails. err[%d]", full_path, err);
		return MB_SVC_ERROR_INTERNAL;
	}

	return statbuf.st_mtime;
}

bool
_mb_svc_get_full_path(const char *path, minfo_store_type storage_type,
		      char *full_path)
{
	if (path == NULL || full_path == NULL) {
		mb_svc_debug("path == NULL || full_path == NULL ");
		return false;
	}

	switch (storage_type) {
	case MINFO_MMC:
		strncpy(full_path, MB_SVC_PATH_MMC, MB_SVC_FILE_PATH_LEN_MAX + 1);
		break;
	case MINFO_PHONE:
	default:
		strncpy(full_path, MB_SVC_PATH_PHONE, MB_SVC_FILE_PATH_LEN_MAX + 1);
		break;
	}
	if (strncmp(path, "/", MB_SVC_FILE_PATH_LEN_MAX + 1) != 0) {
		strcat(full_path, path);
	}

	return true;
}

bool _mb_svc_is_valid_path(const char *full_path)
{
	char phone_root_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };
	char mmc_root_path[MB_SVC_DIR_PATH_LEN_MAX + 1] = { 0 };

	if (strlen(full_path) == 0) {
		return FALSE;
	}

	_mb_svc_get_full_path("/", MINFO_PHONE, phone_root_path);
	_mb_svc_get_full_path("/", MINFO_MMC, mmc_root_path);

	if (strncmp(full_path, phone_root_path, strlen(phone_root_path)) == 0) {
		/* like "/mnt/ums/.message" isn't valid mesage, shoud filter */
		if (strlen(full_path) > strlen(phone_root_path) + 1 && full_path[strlen(phone_root_path) + 1] == '.') {
			return FALSE;
		} else {
			return TRUE;
		}
	}

	if (strncmp(full_path, mmc_root_path, strlen(mmc_root_path)) == 0) {
		return TRUE;
	}

	return FALSE;
}

int _mb_svc_get_store_type_by_full(const char *full_path)
{
	if (full_path != NULL) {
		if (strncmp
		    (full_path, MB_SVC_PATH_PHONE,
		     strlen(MB_SVC_PATH_PHONE)) == 0) {
			return MINFO_PHONE;
		} else
		    if (strncmp
			(full_path, MB_SVC_PATH_MMC,
			 strlen(MB_SVC_PATH_MMC)) == 0) {
			return MINFO_MMC;
		}
	}

	return MB_SVC_ERROR_INTERNAL;
}

int _mb_svc_get_rel_path_by_full(const char *full_path, char *path)
{
	int root_len = 0;
	minfo_store_type store_type = 0;

	store_type = _mb_svc_get_store_type_by_full(full_path);

	switch (store_type) {
	case MINFO_PHONE:
		root_len = strlen(MB_SVC_PATH_PHONE);
		break;
	case MINFO_MMC:
		root_len = strlen(MB_SVC_PATH_MMC);
		break;
	default:
		return MB_SVC_ERROR_INTERNAL;
	}
	if (*(full_path + root_len) != '\0') {
		strncpy(path, full_path + root_len, MB_SVC_FILE_PATH_LEN_MAX + 1);
	} else {
		strncpy(path, "/", MB_SVC_FILE_PATH_LEN_MAX + 1);
	}

	return 0;
}

bool _mb_svc_get_file_ext(const char *file_path, char *file_ext)
{
	int i = 0;

	for (i = strlen(file_path); i >= 0; i--) {
		if ((file_path[i] == '.') && (i < MB_SVC_FILE_PATH_LEN_MAX)) {
			strncpy(file_ext, &file_path[i + 1],
				MB_SVC_FILE_EXT_LEN_MAX + 1);
			return TRUE;
		}

		if (file_path[i] == '/') { /* meet the dir. no ext */
			return TRUE;
		}
	}
	return TRUE;
}

bool _mb_svc_glist_free(GList **glist, bool is_free_element)
{
	int length = 0;
	int i = 0;
	void *p = NULL;

	if (*glist == NULL) {
		return TRUE;
	}

	if (is_free_element) {
		length = g_list_length(*glist);
		for (i = 0; i < length; i++) {
			p = g_list_nth_data(*glist, i);
			free(p);
			p = NULL;
		}
	}

	if (*glist != NULL) {
		g_list_free(*glist);
		*glist = NULL;
	}
	return TRUE;
}

int _mb_svc_get_file_type(const char *file_full_path)
{
	int ret = 0;
	drm_bool_type_e drm_type;
	drm_file_type_e drm_file_type;
	char mimetype[255];

	if (file_full_path == NULL)
		return MB_SVC_ERROR_INVALID_PARAMETER;

	ret = drm_is_drm_file(file_full_path, &drm_type);
	if (ret < 0) {
		mb_svc_debug("drm_is_drm_file falied : %d", ret);
		drm_type = DRM_FALSE;
	}

	if (drm_type == DRM_TRUE) {
		drm_file_type = DRM_TYPE_UNDEFINED;

		ret = drm_get_file_type(file_full_path, &drm_file_type);
		if (ret < 0) {
			mb_svc_debug("drm_get_file_type falied : %d", ret);
			return MINFO_ITEM_NONE;
		}

		if (drm_file_type == DRM_TYPE_UNDEFINED) {
			return MINFO_ITEM_NONE;
		} 
		else {
			drm_content_info_s contentInfo;
			memset(&contentInfo, 0x00, sizeof(drm_content_info_s));

			ret = drm_get_content_info(file_full_path, &contentInfo);
			if (ret != DRM_RETURN_SUCCESS) {
				mb_svc_debug("drm_get_content_info() fails. : %d", ret);
				return MINFO_ITEM_NONE;
			}
			mb_svc_debug("DRM mime type: %s", contentInfo.mime_type);

			strncpy(mimetype, contentInfo.mime_type, sizeof(mimetype));
		}
	} else {
		/* get content type and mime type from file. */
		ret =
			aul_get_mime_from_file(file_full_path, mimetype, sizeof(mimetype));
		if (ret < 0) {
			mb_svc_debug
				("aul_get_mime_from_file fail.. Now trying to get type by extension");
	
			char ext[MB_SVC_FILE_EXT_LEN_MAX + 1] = { 0 };
			_mb_svc_get_file_ext(file_full_path, ext);
	
			if (strcasecmp(ext, "JPG") == 0 ||
				strcasecmp(ext, "JPEG") == 0 ||
				strcasecmp(ext, "PNG") == 0 ||
				strcasecmp(ext, "GIF") == 0 ||
				strcasecmp(ext, "AGIF") == 0 ||
				strcasecmp(ext, "XWD") == 0 ||
				strcasecmp(ext, "BMP") == 0 ||
				strcasecmp(ext, "TIF") == 0 ||
				strcasecmp(ext, "TIFF") == 0 ||
				strcasecmp(ext, "WBMP") == 0) {
				return MINFO_ITEM_IMAGE;
			} else if (strcasecmp(ext, "AVI") == 0 ||
				strcasecmp(ext, "MPEG") == 0 ||
				strcasecmp(ext, "MP4") == 0 ||
				strcasecmp(ext, "DCF") == 0 ||
				strcasecmp(ext, "WMV") == 0 ||
				strcasecmp(ext, "ASF") == 0 ||
				strcasecmp(ext, "DIVX") == 0 ||
				strcasecmp(ext, "3GPP") == 0 ||
				strcasecmp(ext, "3GP") == 0) {
				return MINFO_ITEM_VIDEO;
			} else {
				return MINFO_ITEM_NONE;
			}
		}
	}

	mb_svc_debug("mime type : %s", mimetype);

	/* categorize from mimetype */
	if (strstr(mimetype, "image") != NULL) {
		return MINFO_ITEM_IMAGE;
	} else if (strstr(mimetype, "video") != NULL) {
		return MINFO_ITEM_VIDEO;
	}

	return MINFO_ITEM_NONE;
}

