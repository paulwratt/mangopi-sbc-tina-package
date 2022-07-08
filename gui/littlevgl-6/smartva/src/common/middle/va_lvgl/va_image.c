/**********************
 *      includes
 **********************/
#include "va_image.h"
#include "lvgl.h"
#include "dbList.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static db_list_t *image_list = NULL;

static int compare_node_path(void *data, void *path)
{
	manage_image_t *manage_image = (manage_image_t *)data;
	if (manage_image->image.image_path == path) {
		return 0;
	}
	return 1;
}

static int compare_node_buff(void *data, void *buf)
{
	manage_image_t *manage_image = (manage_image_t *)data;
	if (manage_image->image.image_buff == buf) {
		manage_image->used_count--;
		if (manage_image->used_count == 0) {
			return 0;
		}
	}
	return 1;
}

static unsigned char *stbi_convert_rb(unsigned char *data, int w, int h, int Bpp)
{
	int i, j;
	unsigned char *buf;
	unsigned char tmp;

	buf = (unsigned char *)data;
	for(i=0; i<h; i++)
	{
		for(j=0; j<w; j++)
		{
			tmp = buf[2];
			buf[2] = buf[0];	// R
			buf[0] = tmp;		// B
			buf += 4;
		}
	}
	return data;
}

void *parse_image_from_file(char *path)
{
	int w, h, n;
	manage_image_t *manage_image = NULL;
	lv_img_dsc_t *dsc = NULL;
	unsigned char *data = NULL;

	if (NULL == path) {
		com_err("param error");
		return NULL;
	}
	if (NULL == image_list) {
		image_list = db_list_create("image_list", 0);
		if (NULL == image_list) {
			com_err("db_list_create fail\n");
			return NULL;
		}
	}

//	printf("%s %d %s path:%s\n", __FILE__, __LINE__, __func__, path);
	manage_image = __db_list_search_node(image_list, path, compare_node_path);
	if (NULL != manage_image) {
		manage_image->used_count++;
//		printf("find %s %d %s path:%s\n", __FILE__, __LINE__, __func__, path);
		return manage_image->image.image_buff;
	}

	manage_image = (manage_image_t *)malloc(sizeof(manage_image_t));
	if (NULL == manage_image) {
		com_err("malloc error");
		goto err;
	}
	memset(manage_image, 0x00, sizeof(manage_image_t));
	manage_image->image.image_path = path;
	manage_image->used_count = 0;

	dsc = (lv_img_dsc_t *)malloc(sizeof(lv_img_dsc_t));
	if(NULL == dsc) {
		com_err("malloc error");
		goto err;
	}
	memset(dsc, 0, sizeof(lv_img_dsc_t));

	data = stbi_load(path, &w, &h, &n, BYTE_PER_PIXEL);
	if(NULL == data) {
		com_err("stbi_load error");
		goto err;
	}
	data = stbi_convert_rb(data, w, h, BYTE_PER_PIXEL);
	dsc->header.w = w;
	dsc->header.h = h;
	dsc->header.always_zero = 0;
	if (BYTE_PER_PIXEL == 4) {
		dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	} else {
		dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
	}
	dsc->data_size = w * h * BYTE_PER_PIXEL;
	dsc->data = data;

	manage_image->image.image_buff = dsc;
	manage_image->used_count++;
	__db_list_put_tail(image_list, manage_image);
//	printf("%s %d %s image_buff:%p, used_count:%d\n", __FILE__, __LINE__, __func__, manage_image->image.image_buff, manage_image->used_count);
	return dsc;

err:
	if (manage_image) {
		free(manage_image);
	}

	if (data) {
		stbi_image_free(data);
	}
	if (dsc) {
		free(dsc);
	}
	return NULL;
}

int free_image(void *buf)
{
	manage_image_t *manage_image = NULL;
	lv_img_dsc_t *dsc = NULL;

	if (NULL == buf) {
		com_info("param error");
		return -1;
	}

	manage_image = __db_list_search_and_pop(image_list, buf, compare_node_buff);
	if (manage_image == NULL) { //maybe manage_image->used_count not zero;this normal;
		return -1;
	}
	dsc = manage_image->image.image_buff;
	if (dsc->data) {
		stbi_image_free((void *)dsc->data);
	}
	if (dsc) {
		free(dsc);
	}
	free(manage_image);
	return 0;
}

void *get_image_buff_form_list(ui_image_t *image_list, int list_len, int index)
{
	if (NULL == image_list || index >= list_len) {
		com_err("param error");
		return NULL;
	}
	if (index >= 0 && index < list_len) {
		if (NULL == image_list[index].image_buff) {
			image_list[index].image_buff = parse_image_from_file(image_list[index].image_path);
			return image_list[index].image_buff;
		} else {
			return image_list[index].image_buff;
		}
	}
	return NULL;
}
void free_image_buff_form_list(ui_image_t *image_list, int list_len)
{
	int index = 0;
	for (index = 0; index < list_len; index++) {
		if (image_list[index].image_buff) {
			free_image(image_list[index].image_buff);
			image_list[index].image_buff = NULL;
		}
	}
}
