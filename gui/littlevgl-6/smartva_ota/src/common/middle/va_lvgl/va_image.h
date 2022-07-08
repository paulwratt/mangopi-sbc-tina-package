#ifndef __VA_IMAGE_H__
#define __VA_IMAGE_H__
#include "smt_config.h"

void *parse_image_from_file(char *path);
int free_image(void *buf);

#endif /*__VA_IMAGE_H__*/
