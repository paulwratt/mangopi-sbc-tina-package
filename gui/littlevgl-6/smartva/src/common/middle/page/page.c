#include "page.h"
#include "dbList.h"
#include <assert.h>

db_list_t *page_head = NULL;
db_list_t *page_msg_head = NULL;
lv_task_t *page_task_id;

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

void page_task(lv_task_t *task)
{
	(void)task;
	int empty;
	int old_id, new_id;
	page_interface_t *old_page;
	page_interface_t *new_page;
	static page_param *p = NULL;
	static int b_animing = 0;
	
	if (b_animing == 0)
	{
		empty = is_list_empty(page_msg_head);
		if (empty) {
			return;
		}
		p = (page_param *)__db_list_pop(page_msg_head);

		if(p->cmd == CMD_SWITCH)
		{
			destory_page((page_id_t)p->param[0]);
			create_page((page_id_t)p->param[1]);
			free(p);
			p = NULL;
		}
		else if(p->cmd == CMD_ANIM_SWITCH)
		{
			// ��ʼ�����л���Ч
			com_info("anim begin\n");
			old_id = (page_id_t)p->param[0];
			new_id = (page_id_t)p->param[1];
			create_page(new_id);
			
			old_page = (page_interface_t *)find_page(old_id);
			if(old_page) 
			{
				if (old_page->ops.on_anim) 
				{
					old_page->ops.on_anim(old_id, new_id);
				}
				else
				{
					com_warn("\n");
				}
			}
			
			new_page = (page_interface_t *)find_page(new_id);
			if(new_page) 
			{
				if (new_page->ops.on_anim) 
				{
					new_page->ops.on_anim(old_id, new_id);
				}
				else
				{
					com_warn("\n");
				}
			}
			b_animing = 1;
		}
		else
		{
			free(p);
			p = NULL;
		}
	}
	else
	{
		// ��ѯ��Ч�Ƿ����
		old_id = (page_id_t)p->param[0];
		new_id = (page_id_t)p->param[1];
		old_page = (page_interface_t *)find_page(old_id);
		new_page = (page_interface_t *)find_page(new_id);
		if(old_page && new_page) 
		{
			if (old_page->ops.is_anim_end && new_page->ops.is_anim_end) 
			{
				if(old_page->ops.is_anim_end() && new_page->ops.is_anim_end()) 
				{
					b_animing = 0;
				}
			}
			else 
			{
				com_warn("\n");
			}
		}
		else 
		{
			com_warn("\n");
		}

		// ��Ч���
		if(b_animing == 0)
		{
			com_info("anim end\n");
			destory_page(old_id);
			free(p);
			p = NULL;
		}
	}
	
	return;
}

void page_init()
{
	page_head = db_list_create("page_head", 0);
	page_msg_head = db_list_create("page_head", 1);
	page_task_id = lv_task_create(page_task, 5, LV_TASK_PRIO_HIGH, NULL);
}

void page_uninit()
{
	lv_task_del(page_task_id);
	
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

void sent_page_msg(MsgDataInfo *msg)
{
	page_interface_t *page = (page_interface_t *)find_page(msg->to);
	if(page) {
		if (page->ops.msg_proc) {
			page->ops.msg_proc(msg);
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
	p->cmd = CMD_SWITCH;
	p->param[0] = (long int)old_id;
	p->param[1] = (long int)new_id;
	__db_list_put_tail(page_msg_head, p);

	return;
}

void anim_switch_page(page_id_t old_id, page_id_t new_id)
{
	page_param *p;
	p = (page_param *)malloc(sizeof(page_param));
	if(!p){
		com_err("malloc failed!!!");
		return;
	}
	
	p->cmd = CMD_ANIM_SWITCH;
	p->param[0] = (long int)old_id;
	p->param[1] = (long int)new_id;
	__db_list_put_tail(page_msg_head, p);

	return;
}

int set_page_user_data(page_id_t id,void *user_data)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		page->info.user_data = user_data;
		return 0;
	}
	return -1;
}

void* get_page_user_data(page_id_t id)
{
	page_interface_t *page = (page_interface_t *)find_page(id);
	if(page) {
		return page->info.user_data;
	}
	return NULL;
}
