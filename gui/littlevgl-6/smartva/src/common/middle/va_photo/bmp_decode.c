#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "libnsbmp.h"

#define BMP_BYTES_PER_PIXEL 4 //默认为ABGR输出
static void *g_output_buf;

static void *bitmap_create(int width, int height, unsigned int state)
{
    (void)state;
    // return calloc(width * height, BYTES_PER_PIXEL);
    if(g_output_buf == NULL)
    {
        printf("bmp output buf err!\n");
        return NULL;
    }
    return g_output_buf;//内存手动管理，此处使用外部申请的内存
}

static unsigned char *bitmap_get_buffer(void *bitmap)
{
    assert(bitmap);
    return bitmap;
}

static size_t bitmap_get_bpp(void *bitmap)
{
    (void)bitmap;
    return BMP_BYTES_PER_PIXEL;//默认输出ABGR数据
}

static void bitmap_destroy(void *bitmap)
{
    assert(bitmap);
    //free(bitmap);//内存由手动管理，此处不释放
}

static unsigned char *bmp_load_file(const char *path, size_t *data_size)
{
    FILE *fp = NULL;
    int ret = 0;
    unsigned char *data = NULL;

    fp = fopen(path, "rb");
    if(fp == NULL)
    {
        printf("read bmp file error, errno(%d)\n", errno);
        return NULL;
    }

    fseek(fp,0,SEEK_END);
    *data_size = ftell(fp);
    rewind(fp);
    data = (unsigned char *)malloc(sizeof(char)*(*data_size));

    if(data == NULL)
    {
        printf("malloc memory fail\n");
        fclose(fp);
        return NULL;
    }

    ret = fread(data,1,*data_size,fp);
    if (ret != *data_size)
    {
        printf("read src file fail\n");
        fclose(fp);
        free(data);
        return NULL;
    }

    if(fp != NULL)
    {
        fclose(fp);
    }
    return data;
}


int bmp_decode_sw(char *filename,void *output_buf,int *width,int *height,int *comp)
{
    bmp_bitmap_callback_vt bitmap_callbacks = {
        bitmap_create,
        bitmap_destroy,
        bitmap_get_buffer,
        bitmap_get_bpp
    };

    bmp_result code;
    bmp_image bmp;
    size_t size;
    unsigned short ret = 0;
    if(!filename || !output_buf || !width || !height ||!comp)
    {
        printf("bmp para err!\n");
        return -1;
    }
    unsigned char *data = bmp_load_file(filename, &size);
    if(data == NULL)
    {
        printf("bmp file err!\n");
        return -1;
    }

    g_output_buf = output_buf;
    bmp_create(&bmp, &bitmap_callbacks);
    code = bmp_analyse(&bmp, size, data);
    if (code != BMP_OK)
    {
        printf("bmp analyse err! ret = %d\n",code);
        ret = -1;
        goto cleanup;
    }
    code = bmp_decode(&bmp);
    if (code != BMP_OK)
    {
        ret = -1;
        printf("bmp decode err! ret = %d\n",code);
    }

cleanup:
    *width  = bmp.width;
    *height = bmp.height;
    *comp   = BMP_BYTES_PER_PIXEL;
    bmp_finalise(&bmp);
    free(data);

    return ret;
}
