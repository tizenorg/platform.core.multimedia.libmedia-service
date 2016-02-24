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

#include <unistd.h>
#include <sys/stat.h>
#include <db-util.h>
#include <media-util.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include "media-svc-env.h"
#include "media-svc-debug.h"
#include "media-svc-util.h"
#include "media-svc-db-utils.h"
#include "media-util-err.h"
#include "media-util-db.h"
#include "media-svc-media.h"

static int __media_svc_db_upgrade(sqlite3 *db_handle, int cur_version, uid_t uid);
static int __media_svc_rebuild_view_query(sqlite3 *db_handle, uid_t uid);


static GHashTable *table;
static GSList *column_list[MEDIA_SVC_DB_LIST_MAX];

char *_media_svc_get_path(uid_t uid)
{
	char *result_passwd = NULL;
	struct group *grpinfo = NULL;
	if (uid == getuid()) {
		grpinfo = getgrnam("users");
		if (grpinfo == NULL) {
			media_svc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
		result_passwd = g_strdup(MEDIA_ROOT_PATH_INTERNAL);
	} else {
		char passwd_str[MEDIA_SVC_PATHNAME_SIZE] = {0, };
		struct passwd *userinfo = getpwuid(uid);
		if (userinfo == NULL) {
			media_svc_error("getpwuid(%d) returns NULL !", uid);
			return NULL;
		}
		grpinfo = getgrnam("users");
		if (grpinfo == NULL) {
			media_svc_error("getgrnam(users) returns NULL !");
			return NULL;
		}
		/* Compare git_t type and not group name */
		if (grpinfo->gr_gid != userinfo->pw_gid) {
			media_svc_error("UID [%d] does not belong to 'users' group!", uid);
			return NULL;
		}
		snprintf(passwd_str, sizeof(passwd_str), "%s/%s", userinfo->pw_dir, MEDIA_CONTENT_PATH);
		result_passwd = g_strdup(passwd_str);
	}

	return result_passwd;
}

int __media_svc_add_table_info(const char *name, const char *trigger_name, const char *event_table, const char *action_table, const char *view_name)
{
	table_info_s *tbl = NULL;

	media_svc_retvm_if(!STRING_VALID(name), MS_MEDIA_ERR_INVALID_PARAMETER, "name is NULL");

	if (STRING_VALID(trigger_name)) {
		media_svc_retvm_if(!STRING_VALID(event_table), MS_MEDIA_ERR_INVALID_PARAMETER, "event_table is NULL");
		media_svc_retvm_if(!STRING_VALID(action_table), MS_MEDIA_ERR_INVALID_PARAMETER, "action_table is NULL");
	}

	tbl = malloc(sizeof(table_info_s));
	if (tbl == NULL) {
		media_svc_error("MS_MEDIA_ERR_OUT_OF_MEMORY");
		return MS_MEDIA_ERR_OUT_OF_MEMORY;
	}

	memset(tbl, 0x00, sizeof(table_info_s));

	if (STRING_VALID(trigger_name)) {
		tbl->trigger_name = malloc(MEDIA_SVC_PATHNAME_SIZE);
		if (tbl->trigger_name == NULL) {
			media_svc_error("MS_MEDIA_ERR_OUT_OF_MEMORY");
			SAFE_FREE(tbl);
			return MS_MEDIA_ERR_OUT_OF_MEMORY;
		}

		memset(tbl->trigger_name, 0x00, MEDIA_SVC_PATHNAME_SIZE);
		snprintf(tbl->trigger_name, MEDIA_SVC_PATHNAME_SIZE, "%s_%s", trigger_name, event_table);

		tbl->event_table = strndup(event_table, strlen(event_table));
		tbl->action_table = strndup(action_table, strlen(action_table));
	}

	if (STRING_VALID(view_name))
		tbl->view_name = strndup(view_name, strlen(view_name));

	g_hash_table_insert(table, (gpointer)name, (gpointer)tbl);

	return MS_MEDIA_ERR_NONE;
}

int __media_svc_add_column_info(GSList **slist, const char *name, const char *type, const char *option, int version, const char *index_name, bool is_unique, bool is_trigger, bool is_view)
{
	column_info_s *col = NULL;
	col = malloc(sizeof(column_info_s));
	if (col == NULL) {
		media_svc_error("MS_MEDIA_ERR_OUT_OF_MEMORY");
		return MS_MEDIA_ERR_OUT_OF_MEMORY;
	}
	memset(col, 0, sizeof(column_info_s));

	col->name = strndup(name, strlen(name));
	col->type = strndup(type, strlen(type));
	if (option != NULL) {
		col->has_option = true;
		col->option = strndup(option, strlen(option));
	} else {
		col->has_option = false;
	}
	col->version = version;
	if (index_name != NULL) {
		col->is_index = true;
		col->index_name = strndup(index_name, strlen(index_name));
	} else {
		col->is_index = false;
	}
	col->is_unique = is_unique;
	col->is_trigger = is_trigger;
	col->is_view = is_view;
	*slist = g_slist_append(*slist, col);

	return MS_MEDIA_ERR_NONE;
}

static int __media_svc_rebuild_view_query(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	column_info_s *col_ptr = NULL;
	char *sql = NULL;
	char table_query[4096] = {0, };
	char temp[1024] = {0, };
	bool sflag = false;
	int i, len;
	/*media */
	_media_svc_update_media_view(db_handle, uid);

	/*drop playlist_view, tag_view */
	sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_DROP_VIEW, MEDIA_SVC_DB_VIEW_PLAYLIST);
	ret = _media_svc_sql_query(sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_DROP_VIEW, MEDIA_SVC_DB_VIEW_TAG);
	ret = _media_svc_sql_query(sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*create playlist_view */
	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST], i);
		if (col_ptr->is_view) {
			if (sflag == true) {
				if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_THUMBNAIL, strlen(MEDIA_SVC_DB_COLUMN_THUMBNAIL)) == 0)
					snprintf(temp, sizeof(temp), ", playlist.%s AS p_thumbnail_path", col_ptr->name);
				else
					snprintf(temp, sizeof(temp), ", playlist.%s", col_ptr->name);
				strncat(table_query, temp, strlen(temp));
			} else {
				snprintf(temp, sizeof(temp), "playlist.%s", col_ptr->name);
				strncpy(table_query, temp, strlen(temp));
				sflag = true;
			}
		}
		memset(temp, 0, sizeof(temp));
	}
	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], i);
		if (col_ptr->is_view) {
			if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_MAP_ID, strlen(MEDIA_SVC_DB_COLUMN_MAP_ID)) == 0)
				snprintf(temp, sizeof(temp), ", media_count IS NOT NULL AS media_count, playlist_map.%s AS pm_id", col_ptr->name);
			else
				snprintf(temp, sizeof(temp), ", playlist_map.%s", col_ptr->name);
			strncat(table_query, temp, strlen(temp));
		}
		memset(temp, 0, sizeof(temp));
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_MEDIA]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_MEDIA], i);
		if (col_ptr->is_view) {
			snprintf(temp, sizeof(temp), ", media.%s", col_ptr->name);
			strncat(table_query, temp, strlen(temp));
		}
		memset(temp, 0, sizeof(temp));
	}
	sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_VIEW_PLAYLIST, MEDIA_SVC_DB_VIEW_PLAYLIST, table_query);
	ret = _media_svc_sql_query(sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*create tag_view */
	sflag = false;
	memset(table_query, 0, sizeof(table_query));

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG], i);
		if (col_ptr->is_view) {
			if (sflag == true) {
				snprintf(temp, sizeof(temp), ", tag.%s", col_ptr->name);
				strncat(table_query, temp, strlen(temp));
			} else {
				snprintf(temp, sizeof(temp), "tag.%s", col_ptr->name);
				strncpy(table_query, temp, strlen(temp));
				sflag = true;
			}
		}
		memset(temp, 0, sizeof(temp));
	}
	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG_MAP]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG_MAP], i);
		if (col_ptr->is_view) {
			if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_MAP_ID, strlen(MEDIA_SVC_DB_COLUMN_MAP_ID)) == 0)
				snprintf(temp, sizeof(temp), ", media_count IS NOT NULL AS media_count, tag_map.%s AS tm_id", col_ptr->name);
			else
				snprintf(temp, sizeof(temp), ", tag_map.%s", col_ptr->name);
			strncat(table_query, temp, strlen(temp));
		}
		memset(temp, 0, sizeof(temp));
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_MEDIA]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_MEDIA], i);
		if (col_ptr->is_view) {
			snprintf(temp, sizeof(temp), ", media.%s", col_ptr->name);
			strncat(table_query, temp, strlen(temp));
		}
		memset(temp, 0, sizeof(temp));
	}
	sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_VIEW_TAG, MEDIA_SVC_DB_VIEW_TAG, table_query);
	ret = _media_svc_sql_query(sql, uid);
	sqlite3_free(sql);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_make_table_query(const char *table_name, media_svc_table_slist_e list, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	table_info_s *tb = NULL;
	column_info_s *col_ptr = NULL;
	char *sql = NULL;
	char table_query[4096] = {0, };
	char index_query[4096] = {0, };
	char trigger_query[4096] = {0, };
	char table_query_sub[1024] = {0, };
	char temp[1024] = {0 ,};
	int sflag = false;
	int index_len = 0;
	int trigger_len = 0;
	int table_sub_len = 0;
	int len = 0;
	int i = 0;

	tb = g_hash_table_lookup(table, table_name);
	if (tb == NULL) {
		media_svc_debug("lookup fail.. table name [%s] ", table_name);
		tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_MEDIA);
	}

	len = g_slist_length(column_list[list]);

	if (len == 0) {
		media_svc_error("Invalid column");
		return MS_MEDIA_ERR_INTERNAL;
	}

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[list], i);
		/*create table */
		if (col_ptr->has_option) {
			if (sflag == true) {
				snprintf(temp, sizeof(temp), ", %s %s %s", col_ptr->name, col_ptr->type, col_ptr->option);
				strncat(table_query, temp, strlen(temp));
			} else {
				snprintf(temp, sizeof(temp), "%s %s %s", col_ptr->name, col_ptr->type, col_ptr->option);
				strncpy(table_query, temp, strlen(temp));
				sflag = true;
			}
		} else {
			if (sflag == true) {
				snprintf(temp, sizeof(temp), ", %s %s", col_ptr->name, col_ptr->type);
				strncat(table_query, temp, strlen(temp));
			} else {
				snprintf(temp, sizeof(temp), "%s %s", col_ptr->name, col_ptr->type);
				strncpy(table_query, temp, strlen(temp));
				sflag = true;
			}
		}
		memset(temp, 0, sizeof(temp));

		/*unique */
		if (col_ptr->is_unique) {
			if (table_sub_len > 0) {
				snprintf(temp, sizeof(temp), ", %s", col_ptr->name);
				strncat(table_query_sub, temp, strlen(temp));
				table_sub_len = strlen(table_query_sub);
			} else {
				snprintf(temp, sizeof(temp), "%s", col_ptr->name);
				strncpy(table_query_sub, temp, strlen(temp));
				table_sub_len = strlen(table_query_sub);
			}
		}
		memset(temp, 0, sizeof(temp));

		/*create index */
		if (col_ptr->is_index) {
			if (index_len > 0) {
				snprintf(temp, sizeof(temp), MEDIA_SVC_DB_QUERY_INDEX, col_ptr->index_name, table_name, col_ptr->name);
				strncat(index_query, temp, strlen(temp));
				index_len = strlen(index_query);
			} else {
				snprintf(temp, sizeof(temp), MEDIA_SVC_DB_QUERY_INDEX, col_ptr->index_name, table_name, col_ptr->name);
				strncpy(index_query, temp, strlen(temp));
				index_len = strlen(index_query);
			}
		}
		memset(temp, 0, sizeof(temp));

		/*create trigger */
		if (col_ptr->is_trigger) {
			if (STRING_VALID(tb->trigger_name)) {
				if (strncmp(table_name, MEDIA_SVC_DB_TABLE_ALBUM, strlen(MEDIA_SVC_DB_TABLE_ALBUM)) == 0) {
					snprintf(temp, sizeof(temp), MEDIA_SVC_DB_QUERY_TRIGGER_WITH_COUNT, tb->trigger_name, tb->event_table, tb->action_table, tb->event_table, col_ptr->name, col_ptr->name, col_ptr->name, col_ptr->name);
					strncpy(trigger_query, temp, strlen(temp));
					trigger_len = strlen(trigger_query);
				} else {
					snprintf(temp, sizeof(temp), MEDIA_SVC_DB_QUERY_TRIGGER, tb->trigger_name, tb->event_table, tb->action_table, col_ptr->name, col_ptr->name);
					strncpy(trigger_query, temp, strlen(temp));
					trigger_len = strlen(trigger_query);
				}
			} else {
				media_svc_error("invalid trigger name");
			}
		}
		memset(temp, 0, sizeof(temp));
	}

	/*send queries */
	if (table_sub_len > 0) {
		sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_TABLE_WITH_UNIQUE, table_name, table_query, table_query_sub);
		ret = _media_svc_sql_query(sql, uid);
		sqlite3_free(sql);
		memset(table_query, 0, sizeof(table_query));
		memset(table_query_sub, 0, sizeof(table_query_sub));
		media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	} else {
		sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_TABLE, table_name, table_query);
		ret = _media_svc_sql_query(sql, uid);
		sqlite3_free(sql);
		memset(table_query, 0, sizeof(table_query));
		media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	}

	if (index_len > 0) {
		ret = _media_svc_sql_query(index_query, uid);
		memset(index_query, 0, sizeof(index_query));
		media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	}

	if (trigger_len > 0) {
		ret = _media_svc_sql_query(trigger_query, uid);
		memset(trigger_query, 0, sizeof(trigger_query));
		media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	}

	/*create view */
	sflag = false;
	if (tb != NULL && tb->view_name != NULL) {
		if (strncmp(table_name, MEDIA_SVC_DB_TABLE_MEDIA, strlen(MEDIA_SVC_DB_TABLE_MEDIA)) == 0) {
			sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_VIEW_MEDIA, tb->view_name, table_name);
			ret = _media_svc_sql_query(sql, uid);
			sqlite3_free(sql);
			media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

		} else if (strncmp(table_name, MEDIA_SVC_DB_TABLE_PLAYLIST, strlen(MEDIA_SVC_DB_TABLE_PLAYLIST)) == 0) {
			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST], i);
				if (col_ptr->is_view) {
					if (sflag == true) {
						if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_THUMBNAIL, strlen(MEDIA_SVC_DB_COLUMN_THUMBNAIL)) == 0)
							snprintf(temp, sizeof(temp), ", playlist.%s AS p_thumbnail_path", col_ptr->name);
						else
							snprintf(temp, sizeof(temp), ", playlist.%s", col_ptr->name);
						strncat(table_query, temp, strlen(temp));
					} else {
						snprintf(temp, sizeof(temp), "playlist.%s", col_ptr->name);
						strncpy(table_query, temp, strlen(temp));
						sflag = true;
					}
				}
				memset(temp, 0, sizeof(temp));
			}
			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], i);
				if (col_ptr->is_view) {
					if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_MAP_ID, strlen(MEDIA_SVC_DB_COLUMN_MAP_ID)) == 0)
						snprintf(temp, sizeof(temp), ", media_count IS NOT NULL AS media_count, playlist_map.%s AS pm_id", col_ptr->name);
					else
						snprintf(temp, sizeof(temp), ", playlist_map.%s", col_ptr->name);
					strncat(table_query, temp, strlen(temp));
				}
				memset(temp, 0, sizeof(temp));
			}

			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_MEDIA]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_MEDIA], i);
				if (col_ptr->is_view) {
					snprintf(temp, sizeof(temp), ", media.%s", col_ptr->name);
					strncat(table_query, temp, strlen(temp));
				}
				memset(temp, 0, sizeof(temp));
			}
			sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_VIEW_PLAYLIST, tb->view_name, table_query);
			ret = _media_svc_sql_query(sql, uid);
			sqlite3_free(sql);
			media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

		} else {
			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG], i);
				if (col_ptr->is_view) {
					if (sflag == true) {
						snprintf(temp, sizeof(temp), ", tag.%s", col_ptr->name);
						strncat(table_query, temp, strlen(temp));
					} else {
						snprintf(temp, sizeof(temp), "tag.%s", col_ptr->name);
						strncpy(table_query, temp, strlen(temp));
						sflag = true;
					}
				}
				memset(temp, 0, sizeof(temp));
			}
			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG_MAP]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG_MAP], i);
				if (col_ptr->is_view) {
					if (strncmp(col_ptr->name, MEDIA_SVC_DB_COLUMN_MAP_ID, strlen(MEDIA_SVC_DB_COLUMN_MAP_ID)) == 0)
						snprintf(temp, sizeof(temp), ", media_count IS NOT NULL AS media_count, tag_map.%s AS tm_id", col_ptr->name);
					else
						snprintf(temp, sizeof(temp), ", tag_map.%s", col_ptr->name);
					strncat(table_query, temp, strlen(temp));
				}
				memset(temp, 0, sizeof(temp));
			}

			len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_MEDIA]);
			for (i = 1; i < len; i++) {
				col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_MEDIA], i);
				if (col_ptr->is_view) {
					snprintf(temp, sizeof(temp), ", media.%s", col_ptr->name);
					strncat(table_query, temp, strlen(temp));
				}
				memset(temp, 0, sizeof(temp));
			}
			sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_VIEW_TAG, tb->view_name, table_query);
			ret = _media_svc_sql_query(sql, uid);
			sqlite3_free(sql);
			media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

		}
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_upgrade_table_query(sqlite3 *db_handle, const char *table_name, media_svc_table_slist_e list, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	column_info_s *col_ptr = NULL;
	char *sql = NULL;
	char temp[1024] = {0, };
	int len, i;
	int cur_version = 0;
	sqlite3_stmt *sql_stmt = NULL;

	len = g_slist_length(column_list[list]);

	sql = sqlite3_mprintf("PRAGMA user_version");
	ret = _media_svc_sql_prepare_to_step(db_handle, sql, &sql_stmt);

	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when get user_version. err = [%d]", ret);
		return ret;
	}
	cur_version = sqlite3_column_int(sql_stmt, 0);
	SQLITE3_FINALIZE(sql_stmt);

	len = g_slist_length(column_list[list]);
	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[list], i);
		if (col_ptr->version > cur_version) {
			/*alter table */
			if (col_ptr->has_option)
				snprintf(temp, sizeof(temp), "%s %s %s", col_ptr->name, col_ptr->type, col_ptr->option);
			else
				snprintf(temp, sizeof(temp), "%s %s", col_ptr->name, col_ptr->type);
			sql = sqlite3_mprintf(MEDIA_SVC_DB_QUERY_ALTER_TABLE, table_name, temp);
			ret = _media_svc_sql_query(sql, uid);
			sqlite3_free(sql);
			media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
			/*create index */
			if (col_ptr->is_index) {
				memset(temp, 0, sizeof(temp));
				snprintf(temp, sizeof(temp), MEDIA_SVC_DB_QUERY_INDEX, col_ptr->index_name, table_name, col_ptr->name);
				ret = _media_svc_sql_query(temp, uid);
				media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
			}
		}
		memset(temp, 0, sizeof(temp));
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_init_table_query(const char *event_table_name)
{
	int ret = MS_MEDIA_ERR_NONE;
	int i = 0;

	/*variable initialize.. */
	table = g_hash_table_new(g_str_hash, g_str_equal);
	for (i = 0; i < MEDIA_SVC_DB_LIST_MAX; i++)
		column_list[i] = g_slist_alloc();

	/*table specification.. (table_name, index, unique set, trigger, view, trigger name, event table, action table, view name) */
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_MEDIA, NULL, NULL, NULL, MEDIA_SVC_DB_VIEW_MEDIA);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_TRIGGER_FOLDER, event_table_name, MEDIA_SVC_DB_TABLE_FOLDER, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_PLAYLIST_MAP, MEDIA_SVC_DB_TRIGGER_PLAYLIST_MAP, event_table_name, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_PLAYLIST, MEDIA_SVC_DB_TRIGGER_PLAYLIST_MAP1, MEDIA_SVC_DB_TABLE_PLAYLIST, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP, MEDIA_SVC_DB_VIEW_PLAYLIST);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_ALBUM, MEDIA_SVC_DB_TRIGGER_ALBUM, event_table_name, MEDIA_SVC_DB_TABLE_ALBUM, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_TAG_MAP, MEDIA_SVC_DB_TRIGGER_TAG_MAP, event_table_name, MEDIA_SVC_DB_TABLE_TAG_MAP, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_TAG, MEDIA_SVC_DB_TRIGGER_TAG_MAP1, MEDIA_SVC_DB_TABLE_TAG, MEDIA_SVC_DB_TABLE_TAG_MAP, MEDIA_SVC_DB_VIEW_TAG);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_BOOKMARK, MEDIA_SVC_DB_TRIGGER_BOOKMARK, event_table_name, MEDIA_SVC_DB_TABLE_BOOKMARK, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_STORAGE, NULL, NULL, NULL, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_FACE_SCAN_LIST, MEDIA_SVC_DB_TRIGGER_FACE_SCAN_LIST, MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_TABLE_FACE_SCAN_LIST, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_table_info(MEDIA_SVC_DB_TABLE_FACE, MEDIA_SVC_DB_TRIGGER_FACE, MEDIA_SVC_DB_TABLE_FACE_SCAN_LIST, MEDIA_SVC_DB_TABLE_FACE, NULL);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*insert column info.. */
	/*media*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "PRIMARY KEY", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "path", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL UNIQUE", USER_V2, NULL, true, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "file_name", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, "media_file_name_idx", true, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "media_type", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "mime_type", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "size", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "added_time", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "modified_time", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, "media_modified_time_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "folder_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, "folder_uuid_idx", false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "thumbnail_path", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "title", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_title_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "album_id", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "album", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_album_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "artist", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_artist_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "album_artist", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "genre", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_genre_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "composer", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_composer_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "year", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "recorded_date", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "copyright", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "track_num", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "description", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "bitrate", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "bitpersample", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "samplerate", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "channel", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "duration", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "longitude", MEDIA_SVC_DB_TYPE_DOUBLE, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "latitude", MEDIA_SVC_DB_TYPE_DOUBLE, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "altitude", MEDIA_SVC_DB_TYPE_DOUBLE, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "exposure_time", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V4, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "fnumber", MEDIA_SVC_DB_TYPE_DOUBLE, "DEFAULT 0", USER_V4, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "iso", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V4, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "model", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V4, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "width", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "height", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "datetaken", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "orientation", MEDIA_SVC_DB_TYPE_INT, "DEFAULT -1", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "burst_id", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "played_count", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "last_played_time", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "last_played_position", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "rating", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "favourite", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "author", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_author_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "provider", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_provider_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "content_name", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_content_name_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "category", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "location_tag", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, "media_location_tag_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "age_rating", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "keyword", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "is_drm", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "storage_type", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "timeline", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, "media_timeline_idx", false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "weather", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "sync_status", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "file_name_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "title_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "album_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "artist_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "album_artist_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "genre_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "composer_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "copyright_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "description_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "author_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "provider_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "content_name_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "category_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "location_tag_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "age_rating_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "keyword_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	/* storage_uuid column is added in DB v4. When doing DB upgrade to v4, if storage_uuid is NOT NULL, alter table failed. */
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "storage_uuid", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V4, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "validity", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 1", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	/* color column is added with dcm. (DB v5) */
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "color_r", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V5, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "color_g", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V5, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_MEDIA], "color_b", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V5, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*folder*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "folder_uuid", MEDIA_SVC_DB_TYPE_TEXT, "PRIMARY KEY", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "path", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "name", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "modified_time", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "name_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "storage_type", MEDIA_SVC_DB_TYPE_INT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	/* storage_uuid column is added in DB v4. When doing DB upgrade to v4, if storage_uuid is NOT NULL, alter table failed. */
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "storage_uuid", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V4, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "folder_order", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V4, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "parent_folder_uuid", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V4, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "validity", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 1", USER_V4, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FOLDER], "scan_status", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V4, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*playlist_map*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], "_id", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], "playlist_id", MEDIA_SVC_DB_TYPE_INT, "NOT NULL", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, false, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], "play_order", MEDIA_SVC_DB_TYPE_INT, "NOT NULL", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*playlist*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST], "playlist_id", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, true, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST], "name", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL UNIQUE", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST], "name_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_PLAYLIST], "thumbnail_path", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*album*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_ALBUM], "album_id", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_ALBUM], "name", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_ALBUM], "artist", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_ALBUM], "album_art", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*tag_map*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG_MAP], "_id", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG_MAP], "tag_id", MEDIA_SVC_DB_TYPE_INT, "NOT NULL", USER_V2, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG_MAP], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, true, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*tag*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG], "tag_id ", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, true, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG], "name", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL UNIQUE", USER_V2, NULL, false, false, true);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_TAG], "name_pinyin", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*bookmark*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_BOOKMARK], "bookmark_id", MEDIA_SVC_DB_TYPE_INT, "PRIMARY KEY AUTOINCREMENT", USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_BOOKMARK], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V2, NULL, true, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_BOOKMARK], "marked_time", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V2, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_BOOKMARK], "thumbnail_path", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V2, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*storage*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "storage_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V3, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "storage_name", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V3, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "storage_path", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "storage_account", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "storage_type", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "scan_status", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_STORAGE], "validity", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 1", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*face scan list*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE_SCAN_LIST], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V3, NULL, true, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE_SCAN_LIST], "storage_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	/*face*/
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_uuid", MEDIA_SVC_DB_TYPE_TEXT, "PRIMARY KEY", USER_V3, NULL, true, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "media_uuid", MEDIA_SVC_DB_TYPE_TEXT, "NOT NULL", USER_V3, NULL, false, true, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_rect_x", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_rect_y", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_rect_w", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_rect_h", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "orientation", MEDIA_SVC_DB_TYPE_INT, "DEFAULT 0", USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);
	ret = __media_svc_add_column_info(&column_list[MEDIA_SVC_DB_LIST_FACE], "face_tag", MEDIA_SVC_DB_TYPE_TEXT, NULL, USER_V3, NULL, false, false, false);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	return ret;
}
void __media_svc_table_free(table_info_s *tb)
{
	SAFE_FREE(tb->trigger_name);
	SAFE_FREE(tb->view_name);
	SAFE_FREE(tb->event_table);
	SAFE_FREE(tb->action_table);
	SAFE_FREE(tb);
}

void __media_svc_column_free(column_info_s *col)
{
	SAFE_FREE(col->name);
	SAFE_FREE(col->type);
	SAFE_FREE(col->option);
	SAFE_FREE(col->index_name);
	SAFE_FREE(col);
}

void _media_svc_destroy_table_query()
{
	int i = 0;
	table_info_s *tb = NULL;
	column_info_s *col_ptr = NULL;
	int len = 0;

	/* Table Free */
	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_MEDIA);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_MEDIA);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_FOLDER);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_FOLDER);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_PLAYLIST);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_PLAYLIST);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_ALBUM);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_ALBUM);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_TAG);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_TAG);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_TAG_MAP);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_TAG_MAP);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_BOOKMARK);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_BOOKMARK);

	tb = g_hash_table_lookup(table, MEDIA_SVC_DB_TABLE_STORAGE);
	__media_svc_table_free(tb);
	g_hash_table_remove(table, MEDIA_SVC_DB_TABLE_STORAGE);

	g_hash_table_destroy(table);

	/* Column Free */
	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_MEDIA]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_MEDIA], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_FOLDER]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_FOLDER], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST_MAP], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_PLAYLIST]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_PLAYLIST], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_ALBUM]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_ALBUM], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG_MAP]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG_MAP], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_TAG]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_TAG], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_BOOKMARK]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_BOOKMARK], i);
		__media_svc_column_free(col_ptr);
	}

	len = g_slist_length(column_list[MEDIA_SVC_DB_LIST_STORAGE]);

	for (i = 1; i < len; i++) {
		col_ptr = g_slist_nth_data(column_list[MEDIA_SVC_DB_LIST_STORAGE], i);
		__media_svc_column_free(col_ptr);
	}

	for (i = 0; i < MEDIA_SVC_DB_LIST_MAX; i++)
		g_slist_free(column_list[i]);

}

static int __media_svc_db_upgrade(sqlite3 *db_handle, int cur_version, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;

	ret = _media_svc_init_table_query(MEDIA_SVC_DB_TABLE_MEDIA);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("Query initialization failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_MEDIA, MEDIA_SVC_DB_LIST_MEDIA, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	/* Upgrade issue in folder table */
	if (cur_version < USER_V4) {
		/* Create tmp table */
		sql = sqlite3_mprintf("CREATE TABLE '%q' AS SELECT * FROM '%q';", MEDIA_SVC_DB_TABLE_TMP_TABLE, MEDIA_SVC_DB_TABLE_FOLDER);
		if (sql == NULL) {
			 media_svc_error("_media_svc_upgrade_table_query failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		if (ret != MS_MEDIA_ERR_NONE)
			media_svc_error("Error when create backup folder table");
		SQLITE3_SAFE_FREE(sql);

		/* Drop original table */
		sql = sqlite3_mprintf("DROP TABLE '%q';", MEDIA_SVC_DB_TABLE_FOLDER);
		if (sql == NULL) {
			 media_svc_error("_media_svc_upgrade_table_query failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		if (ret != MS_MEDIA_ERR_NONE)
			media_svc_error("Error when drop table");
		SQLITE3_SAFE_FREE(sql);

		/* Create new table */
		ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_LIST_FOLDER, uid);
		if (ret != MS_MEDIA_ERR_NONE) {
			 media_svc_error("_media_svc_make_table_query failed");
			goto ERROR;
		}

		/* Insert into new table */
		sql = sqlite3_mprintf("INSERT INTO '%q'(folder_uuid, path, name, modified_time, name_pinyin, storage_type) SELECT folder_uuid, path, name, modified_time, name_pinyin, storage_type FROM '%q';", MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_TABLE_TMP_TABLE);
		if (sql == NULL) {
			 media_svc_error("Query creation failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		if (ret != MS_MEDIA_ERR_NONE)
			media_svc_error("Error when backup folder table");
		SQLITE3_SAFE_FREE(sql);

		/* Drop tmp table*/
		sql = sqlite3_mprintf("DROP TABLE '%q';", MEDIA_SVC_DB_TABLE_TMP_TABLE);
		if (sql == NULL) {
			 media_svc_error("Query creation failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		if (ret != MS_MEDIA_ERR_NONE)
			media_svc_error("Error when drop backup folder table");
		SQLITE3_SAFE_FREE(sql);

	} else {
		ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_LIST_FOLDER, uid);
		if (ret != MS_MEDIA_ERR_NONE) {
			 media_svc_error("_media_svc_upgrade_table_query failed");
			goto ERROR;
		}
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_PLAYLIST_MAP, MEDIA_SVC_DB_LIST_PLAYLIST_MAP, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_PLAYLIST, MEDIA_SVC_DB_LIST_PLAYLIST, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_ALBUM, MEDIA_SVC_DB_LIST_ALBUM, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_TAG_MAP, MEDIA_SVC_DB_LIST_TAG_MAP, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_TAG, MEDIA_SVC_DB_LIST_TAG, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_BOOKMARK, MEDIA_SVC_DB_LIST_BOOKMARK, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_upgrade_table_query(db_handle, MEDIA_SVC_DB_TABLE_STORAGE, MEDIA_SVC_DB_LIST_STORAGE, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		 media_svc_error("_media_svc_upgrade_table_query failed");
		goto ERROR;
	}

	if (cur_version < USER_V4) {
		/* Need to default value in storage_uuid */
		sql = sqlite3_mprintf("UPDATE %q SET storage_uuid = '%q';", MEDIA_SVC_DB_TABLE_MEDIA, "media");
		if (sql == NULL) {
			 media_svc_error("Query creation failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		SQLITE3_SAFE_FREE(sql);

		sql = sqlite3_mprintf("UPDATE %q SET storage_uuid = '%q';", MEDIA_SVC_DB_TABLE_FOLDER, "media");
		if (sql == NULL) {
			 media_svc_error("Query creation failed");
			 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
			goto ERROR;
		}

		ret = _media_svc_sql_query(sql, uid);
		SQLITE3_SAFE_FREE(sql);
	}

	ret = __media_svc_rebuild_view_query(db_handle, uid);

	sql = sqlite3_mprintf("PRAGMA user_version=%d;", LATEST_VERSION_NUMBER);
	if (sql == NULL) {
		 media_svc_error("Query creation failed");
		 ret = MS_MEDIA_ERR_OUT_OF_MEMORY;
		goto ERROR;
	}

	ret = _media_svc_sql_query(sql, uid);
	SQLITE3_SAFE_FREE(sql);

	_media_svc_destroy_table_query();

	return ret;

ERROR:
	_media_svc_destroy_table_query();

	return ret;
}

int _media_svc_sql_query(const char *sql_str, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_sec_debug("[SQL query] : %s", sql_str);

	ret = media_db_request_update_db(sql_str, uid);

	return ret;
}

int _media_svc_get_user_version(sqlite3 *db_handle, int *user_version)
{
	int ret = MS_MEDIA_ERR_NONE;
	sqlite3_stmt *sql_stmt = NULL;
	char *sql = sqlite3_mprintf("PRAGMA user_version;");

	ret = _media_svc_sql_prepare_to_step(db_handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when get user_version.");
		return ret;
	}

	*user_version = sqlite3_column_int(sql_stmt, 0);
	SQLITE3_FINALIZE(sql_stmt);

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_sql_prepare_to_step(sqlite3 *handle, const char *sql_str, sqlite3_stmt **stmt)
{
	int err = -1;

	media_svc_sec_debug("[SQL query] : %s", sql_str);

	if (!STRING_VALID(sql_str)) {
		media_svc_error("invalid query");
		return MS_MEDIA_ERR_INVALID_PARAMETER;
	}

	err = sqlite3_prepare_v2(handle, sql_str, -1, stmt, NULL);
	sqlite3_free((char *)sql_str);

	if (err != SQLITE_OK) {
		media_svc_error("prepare error %d[%s]", err, sqlite3_errmsg(handle));
		if (err == SQLITE_CORRUPT)
			return MS_MEDIA_ERR_DB_CORRUPT;
		else if (err == SQLITE_PERM)
			return MS_MEDIA_ERR_DB_PERMISSION;

		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	err = sqlite3_step(*stmt);
	if (err != SQLITE_ROW) {
		media_svc_error("[No-Error] Item not found. end of row [%s]", sqlite3_errmsg(handle));
		SQLITE3_FINALIZE(*stmt);
		return MS_MEDIA_ERR_DB_NO_RECORD;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_sql_prepare_to_step_simple(sqlite3 *handle, const char *sql_str, sqlite3_stmt **stmt)
{
	int err = -1;

	media_svc_sec_debug("[SQL query] : %s", sql_str);

	if (!STRING_VALID(sql_str)) {
		media_svc_error("invalid query");
		return MS_MEDIA_ERR_INVALID_PARAMETER;
	}

	err = sqlite3_prepare_v2(handle, sql_str, -1, stmt, NULL);
	sqlite3_free((char *)sql_str);

	if (err != SQLITE_OK) {
		media_svc_error("prepare error %d[%s]", err, sqlite3_errmsg(handle));
		if (err == SQLITE_CORRUPT)
			return MS_MEDIA_ERR_DB_CORRUPT;
		else if (err == SQLITE_PERM)
			return MS_MEDIA_ERR_DB_PERMISSION;

		return MS_MEDIA_ERR_DB_INTERNAL;
	}

	return MS_MEDIA_ERR_NONE;
}

int _media_svc_sql_begin_trans(uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_error("========_media_svc_sql_begin_trans");

	ret = media_db_request_update_db_batch_start("BEGIN IMMEDIATE;", uid);

	return ret;
}

int _media_svc_sql_end_trans(uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	media_svc_error("========_media_svc_sql_end_trans");

	ret = media_db_request_update_db_batch_end("COMMIT;", uid);

	return ret;
}

int _media_svc_sql_rollback_trans(uid_t uid)
{
	media_svc_error("========_media_svc_sql_rollback_trans");

	return _media_svc_sql_query("ROLLBACK;", uid);
}

int _media_svc_sql_query_list(GList **query_list, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	int idx = 0;
	int length = g_list_length(*query_list);
	char *sql = NULL;
	char query_bundle[8000] = {0, };
	int query_len = 0;
	int total_len = 0;

	media_svc_debug("query list length : [%d]", length);

	for (idx = 0; idx < length; idx++) {
		sql = (char *)g_list_nth_data(*query_list, idx);
		if (sql != NULL) {
			query_len = strlen(sql);
			if ((total_len + query_len) >= (sizeof(query_bundle) - 1)) {
				ret = media_db_request_update_db_batch(query_bundle, uid);
				if (ret != MS_MEDIA_ERR_NONE)
					media_svc_error("media_db_request_update_db_batch failed : %d", ret);
				memset(query_bundle, 0, sizeof(query_bundle));
				total_len = 0;
				strncpy(query_bundle, sql, query_len);
				total_len = query_len;
			} else {
				strncat(query_bundle, sql, query_len);
				total_len += query_len;
			}

			sqlite3_free(sql);
			sql = NULL;
		}
	}

	if (total_len > 0) {
		ret = media_db_request_update_db_batch(query_bundle, uid);
		if (ret != MS_MEDIA_ERR_NONE)
			media_svc_error("media_db_request_update_db_batch failed : %d", ret);

		memset(query_bundle, 0, sizeof(query_bundle));
		total_len = 0;
	}

	_media_svc_sql_query_release(query_list);

	return MS_MEDIA_ERR_NONE;

}

void _media_svc_sql_query_add(GList **query_list, char **query)
{
	*query_list = g_list_append(*query_list, *query);
}

void _media_svc_sql_query_release(GList **query_list)
{
	if (*query_list) {
		media_svc_debug("_svc_sql_query_release");
		g_list_free(*query_list);
		*query_list = NULL;
	}
}

int _media_svc_check_db_upgrade(sqlite3 *db_handle, int user_version, uid_t uid)
{
	if (user_version < LATEST_VERSION_NUMBER) {
		media_svc_error("Current DB is out of date(%d).. So start to upgrade DB(%d)", user_version, LATEST_VERSION_NUMBER);
		return __media_svc_db_upgrade(db_handle, user_version, uid);
	} else {
		return MS_MEDIA_ERR_NONE;
	}
}

int _media_db_check_corrupt(sqlite3 *db_handle)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = sqlite3_mprintf("PRAGMA quick_check(1)");
	sqlite3_stmt *sql_stmt = NULL;
	char *result = NULL;

	ret = _media_svc_sql_prepare_to_step(db_handle, sql, &sql_stmt);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("error when check db. err = [%d]", ret);
		return ret;
	}

	result = (char *)sqlite3_column_text(sql_stmt, 0);
	SQLITE3_FINALIZE(sql_stmt);

	if (result != NULL) {
		media_svc_debug("result %s", result);
		if (strcasecmp(result, "OK"))
			ret = MS_MEDIA_ERR_DB_CORRUPT;
	} else {
		media_svc_error("result is NULL");
		ret = MS_MEDIA_ERR_DB_INTERNAL;
	}

	return ret;
}


int _media_svc_create_media_table_with_id(const char *table_id, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	ret = _media_svc_init_table_query(table_id);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_init_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_make_table_query(table_id, MEDIA_SVC_DB_LIST_MEDIA, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	/* Add for trigger */
	ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_FOLDER, MEDIA_SVC_DB_LIST_FOLDER, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_PLAYLIST_MAP, MEDIA_SVC_DB_LIST_PLAYLIST_MAP, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_ALBUM, MEDIA_SVC_DB_LIST_ALBUM, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_TAG_MAP, MEDIA_SVC_DB_LIST_TAG_MAP, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	ret = _media_svc_make_table_query(MEDIA_SVC_DB_TABLE_BOOKMARK, MEDIA_SVC_DB_LIST_BOOKMARK, uid);
	if (ret != MS_MEDIA_ERR_NONE) {
		media_svc_error("_media_svc_make_table_query failed");
		goto ERROR;
	}

	_media_svc_destroy_table_query();

	return ret;
ERROR:

	_media_svc_destroy_table_query();

	return ret;
}

int _media_svc_drop_media_table(const char *storage_id, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;

	char *sql = sqlite3_mprintf("DROP TABLE IF EXISTS '%q'", storage_id);

	ret = _media_svc_sql_query(sql, uid);
	sqlite3_free(sql);

	return ret;
}

int _media_svc_update_media_view(sqlite3 *db_handle, uid_t uid)
{
	int ret = MS_MEDIA_ERR_NONE;
	char *sql = NULL;
	sqlite3_stmt *sql_stmt = NULL;
	int item_cnt = 0;
	int idx = 0;
	GList *storage_list = NULL;
	char view_query[4096] = {0, };
	memset(view_query, 0x00, sizeof(view_query));

	snprintf(view_query, sizeof(view_query), "DROP VIEW IF EXISTS %s; CREATE VIEW IF NOT EXISTS %s AS SELECT * from %s ", MEDIA_SVC_DB_VIEW_MEDIA, MEDIA_SVC_DB_VIEW_MEDIA, MEDIA_SVC_DB_TABLE_MEDIA);

	/*Select list of storage*/
	sql = sqlite3_mprintf("SELECT storage_uuid FROM '%s' WHERE validity=1 AND storage_uuid != '%s'", MEDIA_SVC_DB_TABLE_STORAGE, MEDIA_SVC_DB_TABLE_MEDIA);
	ret = _media_svc_sql_prepare_to_step_simple(db_handle, sql, &sql_stmt);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	while (sqlite3_step(sql_stmt) == SQLITE_ROW) {
		if (STRING_VALID((const char *)sqlite3_column_text(sql_stmt, 0)))
			storage_list = g_list_append(storage_list, strdup((char *)sqlite3_column_text(sql_stmt, 0)));
	}
	SQLITE3_FINALIZE(sql_stmt);

	if ((storage_list != NULL) && (g_list_length(storage_list) > 0)) {
		item_cnt = g_list_length(storage_list);

		for (idx = 0; idx < item_cnt; idx++) {
			int table_cnt = 0;
			char *storage_id = NULL;
			storage_id = g_list_nth_data(storage_list, idx);

			if (STRING_VALID(storage_id)) {
				/*Select list of storage*/
				sql = sqlite3_mprintf("SELECT COUNT(*) FROM SQLITE_MASTER WHERE type='table' and name='%q'", storage_id);
				ret = _media_svc_sql_prepare_to_step(db_handle, sql, &sql_stmt);
				if (ret != MS_MEDIA_ERR_NONE) {
					SAFE_FREE(storage_id);
					continue;
				}

				table_cnt = sqlite3_column_int(sql_stmt, 0);
				SQLITE3_FINALIZE(sql_stmt);

				if (table_cnt > 0) {
					char append_query[128] = {0, };
					memset(append_query, 0x00, sizeof(append_query));
					snprintf(append_query, sizeof(append_query), " UNION SELECT * from '%s'", storage_id);
					strncat(view_query, append_query, strlen(append_query));
				} else {
					media_svc_error("media table not exist for storage [%s]", storage_id);
				}

				SAFE_FREE(storage_id);
			}
		}
		g_list_free(storage_list);
	}

	ret = _media_svc_sql_query(view_query, uid);
	media_svc_retv_if(ret != MS_MEDIA_ERR_NONE, ret);

	return ret;
}
