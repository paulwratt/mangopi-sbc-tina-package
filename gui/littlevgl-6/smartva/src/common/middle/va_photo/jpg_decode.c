#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#include "jpeglib.h"

int jpg_get_scalerdown(int width,int height)
{
	int scalerdown = 0;
	if(width>3000)
    {
        scalerdown=3;
    }
    else if(width>2048)
    {
        scalerdown=2;
    }
    else if(width>1280)
    {
        scalerdown=1;
    }

	return scalerdown;
}

int jpg_is_progressive(char *filename)
{
    int ret = 0;
    unsigned short offset = 0;
    char mark[4] = {0};
    FILE *fp = NULL;

    fp = fopen(filename,"rb");
    if (fp == NULL)
    {
        printf("file open fail, %s", filename);
        return -1;
    }

    fread(mark, 1, 2, fp);
    if (!(mark[0] == 0xFF && mark[1] == 0xD8))
    {
        fclose(fp);
        return -1;
    }

    mark[0] = 0;
    mark[1] = 0;
    fread(mark, 1, 2, fp);
    // printf("mark[0]:%x, mark[1]:%x", mark[0], mark[1]);

    while (mark[0] == 0xFF)
    {
        mark[0] = 0;
        mark[1] = 0;
        fread(mark, 1, 2, fp);
        offset = ((mark[1]) | (mark[0] << 8));
        fseek(fp, offset-2, SEEK_CUR);
        mark[0] = 0;
        mark[1] = 0;
        fread(mark, 1, 2, fp);
        // printf("mark[0]:%x, mark[1]:%x", mark[0], mark[1]);

        if (mark[0] == 0xFF && mark[1] == 0xC2)
        {
            ret = 1;
            break;
        }
        else if (mark[0] == 0xFF && mark[1] == 0xC1)
        {
            ret = 0;
            break;
        }
    }

    fclose(fp);
    return ret;
}

typedef struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
}*my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

int jpg_decode_sw(char *filename,void *output_buf,unsigned int *width,unsigned int *height,int *comp)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE *infile;
	unsigned char *p_buf  = output_buf;
	int ret;

	memset(&cinfo, 0x00, sizeof(cinfo));
    memset(&jerr, 0x00, sizeof(jerr));

	if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("sw_jpeg_decode open file fail, %s", filename);
        return -1;
    }

	cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

	if (setjmp(jerr.setjmp_buffer))
	{
		printf("decode err!\n");
		ret = -1;
		goto exit;
	}

	jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);

	if(cinfo.image_width>3000)
    {
        cinfo.scale_num=1;
        cinfo.scale_denom=8;
    }
    else if(cinfo.image_width>2048)
    {
        cinfo.scale_num=1;
        cinfo.scale_denom=4;
    }
    else if(cinfo.image_width>1280)
    {
        cinfo.scale_num=1;
        cinfo.scale_denom=2;
    }
	cinfo.out_color_space = JCS_RGB;

	(void) jpeg_start_decompress(&cinfo);
	// p_buf = (char *)ionHdle->palloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);
	printf("out w = %d h = %d comp = %d\n",cinfo.output_width , cinfo.output_height , cinfo.output_components);
	printf("in  w = %d h = %d comp = %d\n",cinfo.image_width,cinfo.image_height,cinfo.num_components);


    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void) jpeg_read_scanlines(&cinfo, &p_buf, 1);
		p_buf+=cinfo.output_width * cinfo.output_components;
    }
	(void) jpeg_finish_decompress(&cinfo);
	*width  = cinfo.output_width;
	*height = cinfo.output_height;
    *comp = 3;
	ret = 0;
exit:
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
	return ret;
}
