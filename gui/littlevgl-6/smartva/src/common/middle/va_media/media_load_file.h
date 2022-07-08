#ifndef _READ_FILE_LIST_H_
#define _READ_FILE_LIST_H_

#include "smt_config.h"
#include "list.h"
#include "rat.h"
#include "rat_common.h"

#define MOUNT_PATH_MAX_LENGTH	64
typedef struct media_file_list_tag {
	HRAT				media_hrat;
	rat_media_type_t	media_type;
	int				total_num;
	int				play_index;
	int				used_count;
} media_file_list_t;

#define MEDIA_FILE_LIST_MAX_NUM		500

typedef enum _char_enc
{
    ENCODE_ISO_8859_1 = 0,
    ENCODE_USC_2 = 1,
    ENCODE_UTF_16_BE = 2,
    ENCODE_UTF_8 = 3,
    ENCODE_ASCII = 4,
	ENCODE_UNICODE = 5,
	ENCODE_UTF_32 = 6,
	ENCODE_GB2312 = 7,
    ENCODE_DEFAULT = ENCODE_UTF_8,
}char_enc;


char_enc get_file_lang(char *file);
char *media_get_next_file_path(media_file_list_t *media_file_list, int index);
media_file_list_t *media_load_file(rat_media_type_t media_type, char *path);
int media_unload_file(media_file_list_t *media_file_list);
char *media_get_file_path(media_file_list_t *media_file_list, int index);
char *media_get_path_to_name(char *path);
unsigned int media_get_play_file_index(media_file_list_t *media_file_list);
void media_move_cursor_to_first(void);
int find_music_lrc(char *basePath, char *lrc_name,char *lrc_path);
void media_get_file_size(media_file_list_t *media_file_list, int index, char *size);
void media_get_file_time(media_file_list_t *media_file_list, int index, char *time);
void media_set_play_file_index(media_file_list_t *media_file_list, int index);
void file_size_int_to_string(long int size_int, char *size_str);
#endif
