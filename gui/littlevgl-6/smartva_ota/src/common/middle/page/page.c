#include "page.h"
#include "dbList.h"
#include <assert.h>


db_list_t *page_head = NULL;
db_list_t *page_msg_head = NULL;

static inline int compare_by_id(void *dst, void *src)
{
	if(dst == NULL || src == NULL){
		printf("[%s: %d] paragarms error\n", __FILE__, __LINE__);
		return -1;
	}

	page_interface_t *dst_page = (page_interface_t *)dst;
	page_id_t src_id = *((page_id_t *)src);
	if(src_id == dst_page->info.id)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

static void *find_page(page_id_t id)
{
	assert(page_head != NULL);
	return __db_list_search_node(page_head, (void *)&id, compare_by_id);
}

void page_init()
{
	page_head = db_list_create();
	page_msg_head = db_list_create();
}

void page_uninit()
{
	assert(page_head != NULL);
	assert(page_msg_head != NULL);
	__db_list_destory(page_head);
	__db_list_destory(page_msg_head);
}

void reg_page(page_interface_t *page)
{
	assert(page_head != NULL);
	__db_list_put_tail(page_head, page);
}

void unreg_page(page_interface_t *page)
{
	;
}

static page_id_t current_id = PAGE_NONE;

int create_page(page_id_t id)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		if (page->ops.on_create) {
			if(!page->ops.on_create()){
				current_id = id;
				return 0;
			}
		}
	}
	return -1;
}

page_id_t current_page(void)
{
	return current_id;
}

void destory_page(page_id_t id)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		if (page->ops.on_destroy) {
			page->ops.on_destroy();
		}
	}
}

void show_page(page_id_t id)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		if (page->ops.on_show) {
			page->ops.on_show();
		}
	}
}

void hide_page(page_id_t id)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		if (page->ops.on_hide) {
			page->ops.on_hide();
		}
	}
}

void switch_page(page_id_t old_id, page_id_t new_id)
{
	page_param *p;
	p = (page_param *)malloc(sizeof(page_param));
	if(!p){
		com_err("malloc failed!!!");
		return;
	}
	p->cmd = 0;
	p->param[0] = (long int)old_id;
	p->param[1] = (long int)new_id;
	com_info("old_id=%d -> new_id=%d\n", old_id, new_id);
	__db_list_put_tail(page_msg_head, p);

	return;
}

void update_page(void)
{
	int empty;
	page_param *p;

	empty = is_list_empty(page_msg_head);
	if(empty){
		return;
	}

	p = (page_param *)__db_list_pop(page_msg_head);
	destory_page((page_id_t)p->param[0]);
	create_page((page_id_t)p->param[1]);
	free(p);
	return;
}