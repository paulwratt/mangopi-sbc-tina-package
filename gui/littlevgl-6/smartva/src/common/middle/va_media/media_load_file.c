#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include "media_load_file.h"
#include "dbList.h"

typedef struct media_list_compare_param {
	rat_media_type_t media_type;
	char *path;
} media_list_compare_param_t;
static db_list_t *media_list = NULL;


static int compare_media_list_param(void *data, void *param)
{
	media_list_compare_param_t *compare_param = (media_list_compare_param_t *)param;
	media_file_list_t *media_file_list = (media_file_list_t *)data;
	if (media_file_list->media_type == compare_param->media_type && strcmp(((__rat_list_t *)media_file_list->media_hrat)->str_path, compare_param->path) == 0 &&
				strlen(((__rat_list_t *)media_file_list->media_hrat)->str_path) == strlen(compare_param->path)) {
		return 0;
	}
	return 1;
}

static int compare_media_list(void *data, void *param)
{
	media_file_list_t *media_file_list = (media_file_list_t *)data;
	media_file_list_t *list = (media_file_list_t *)param;

	if (media_file_list == list) {
		media_file_list->used_count--;
		if (media_file_list->used_count == 0) {
			return 0;
		}
	}
	return 1;
}

media_file_list_t *media_load_file(rat_media_type_t media_type, char *path)
{
	media_file_list_t *media_file_list = NULL;
	media_list_compare_param_t  media_list_compare_param;

	if (media_type == RAT_MEDIA_TYPE_ALL) {
	    com_warn("media_type == RAT_MEDIA_TYPE_ALL");
		goto error;
	}

	if (path == NULL) {
	    com_warn("path == NULL");
		goto error;
	}
	if (media_list == NULL) {
		media_list = db_list_create("media_file_list", 0);
		if (media_list == NULL) {
			com_err("db_list_create fail\n");
			return NULL;
		}
	}

	memset(&media_list_compare_param, 0x00, sizeof(media_list_compare_param_t));
	media_list_compare_param.media_type = media_type;
	media_list_compare_param.path = path;
	media_file_list = __db_list_search_node(media_list, &media_list_compare_param, compare_media_list_param);
	if (NULL != media_file_list) {
		media_file_list->used_count++;
		return media_file_list;
	}

	com_info("media is loading file(type=%d, path=%s)...", media_type, path);
	media_file_list = (media_file_list_t *)malloc(sizeof(media_file_list_t));
	if (NULL == media_file_list) {
	    com_err("malloc fail\n");
		goto error;
	}
	memset(media_file_list, 0x00, sizeof(media_file_list_t));

	media_file_list->media_hrat = rat_open(path, media_type, 0); //È«²¿ËÑË÷
	if (!media_file_list->media_hrat) {
		com_err("media:rat open failed!!!");
		goto error;
	}
	media_file_list->total_num = rat_get_cur_scan_cnt(media_file_list->media_hrat);
	if (media_file_list->total_num > MEDIA_FILE_LIST_MAX_NUM) {
		com_warn("media file too many");
		media_file_list->total_num = MEDIA_FILE_LIST_MAX_NUM;
	}
	else if(media_file_list->total_num == 0)
	{
		rat_close(media_file_list->media_hrat);
		goto error;
	}
	com_info("media file load finish, total_num = %d", media_file_list->total_num);
	media_file_list->media_type = media_type;
	media_file_list->used_count++;

	__db_list_put_tail(media_list, media_file_list);
	return media_file_list;
error:
	if (media_file_list) {
		free(media_file_list);
		media_file_list = NULL;
	}
	return NULL;
}

int media_unload_file(media_file_list_t *media_file_list)
{
	int ret = 0;

	if (NULL == media_file_list) {
		com_info("param error");
		return 0;
	}
	if (NULL != __db_list_search_and_pop(media_list, media_file_list, compare_media_list)) {
		if (media_file_list->media_hrat) {
			ret = rat_close(media_file_list->media_hrat);
		}
		free(media_file_list);
	}
	return ret;
}

char *media_get_path_to_name(char *path)
{
extern char * SLIB_strchrlast(char * pstr, char srch_char);
	return (char *)(SLIB_strchrlast(path, '/') + 1);
}

char *media_get_file_path(media_file_list_t *media_file_list, int index)
{
	int ret = 0;
	rat_entry_t ItemInfo;

	if (media_file_list == NULL) {
		com_err("param error");
	}
	memset(&ItemInfo, 0, sizeof(rat_entry_t));
	ret = rat_get_item_info_by_index(media_file_list->media_hrat, index, &ItemInfo);
	if (ret < 0) {
		return NULL;
	}
	return ItemInfo.Path;
}

char *media_get_next_file_path(media_file_list_t *media_file_list, int index)
{
	int ret;
	rat_entry_t ItemInfo;

	if (index == 0) {
		ret = rat_move_cursor_to_first(media_file_list->media_hrat);
		if(ret < 0){
			return NULL;
		}
		ret = rat_get_cur_item_info(media_file_list->media_hrat, &ItemInfo);
		if(ret < 0){
			return NULL;
		}
		return ItemInfo.Path;
	}

	memset(&ItemInfo, 0, sizeof(rat_entry_t));
	ret = rat_move_cursor_forward(media_file_list->media_hrat, index);
	if (!ret) {
		return NULL;
	}
	ret = rat_get_cur_item_info(media_file_list->media_hrat, &ItemInfo);
	if (ret < 0) {
		return NULL;
	}

	return ItemInfo.Path;
}

void media_get_file_size(media_file_list_t *media_file_list, int index, char *size)
{
	char *path = NULL;;
	struct stat file_info;

	path = media_get_file_path(media_file_list, index);
    stat(path, &file_info);

	file_size_int_to_string(file_info.st_size, size);
}

void media_get_file_time(media_file_list_t *media_file_list, int index, char *time)
{
	char *path;
	struct stat file_info;
	time_t tTimeTmp = 0;
	struct tm stuTimeTmp;

	path = media_get_file_path(media_file_list, index);
    stat(path, &file_info);
	memset(&stuTimeTmp, 0, sizeof(struct tm));
	tTimeTmp = file_info.st_mtime;

	(void *)gmtime_r(&tTimeTmp, &stuTimeTmp);

	sprintf(time, "%d-%d-%d",
		stuTimeTmp.tm_year + 1900,
		stuTimeTmp.tm_mon + 1,
		stuTimeTmp.tm_mday);
}

void media_set_play_file_index(media_file_list_t *media_file_list, int index)
{
	if (NULL == media_file_list) {
		com_err("param error");
	}

	if (index >= media_file_list->total_num) {
		media_file_list->play_index = 0;
	} else if (index < 0) {
		media_file_list->play_index = media_file_list->total_num;
	} else {
		media_file_list->play_index = index;
	}
}

unsigned int media_get_play_file_index(media_file_list_t *media_file_list)
{
	if (NULL == media_file_list) {
		com_err("param error");
	}
	return media_file_list->play_index;
}

int find_music_lrc(char *basePath, char *lrc_name,char *lrc_path)
{
    DIR *dir = NULL;
    struct dirent *ptr;
    char base[512];
	static int ret = -1;

	if(!basePath){
		goto END;
	}

	if(!lrc_path){
		goto END;
	}

	if(!lrc_name){
		goto END;
	}

    if ((dir=opendir(basePath)) == NULL){
        perror("Open dir error!!!");
		goto END;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){    ///file ptr->d_name
			if(strcmp(ptr->d_name, lrc_name) == 0){
				sprintf(lrc_path, "%s/%s", basePath, ptr->d_name);
				ret = 0;
				break;
			}
		} else if(ptr->d_type == 10){    ///link file
            //printf("d_name:%s/%s\n",basePath,ptr->d_name);
        }else if(ptr->d_type == 4){	    ///dir
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            find_music_lrc(base, lrc_name, lrc_path);
        }
    }
END:
	if(dir)
		closedir(dir);
    return ret;
}
