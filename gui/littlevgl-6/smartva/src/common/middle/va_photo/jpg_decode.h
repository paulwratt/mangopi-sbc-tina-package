#ifndef __JPG_DECODE_H__
#define __JPG_DECODE_H__

int jpg_get_scalerdown(int width,int height);
int jpg_is_progressive(char *filename);
int jpg_decode_sw(char *filename,void *output_buf,unsigned int *width,unsigned int *height,int *comp);

#endif
