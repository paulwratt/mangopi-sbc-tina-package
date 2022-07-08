#ifndef __VA_IMAGE_H__
#define __VA_IMAGE_H__
#include "smt_config.h"
#include "list.h"

typedef struct ui_image_tag {
	void *image_buff;
	char *image_path;
} ui_image_t;

typedef struct ui_image_list_tag {
	ui_image_t image;
	int used_count;
} manage_image_t;
void *get_image_buff_form_list(ui_image_t *image_list, int list_len, int index);
void free_image_buff_form_list(ui_image_t *image_list, int list_len);
void *parse_image_from_file(char *path);
int free_image(void *buf);

#endif /*__VA_IMAGE_H__*/
