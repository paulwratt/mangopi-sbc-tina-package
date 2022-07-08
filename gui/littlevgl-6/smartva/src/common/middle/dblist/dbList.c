#include <string.h>

#include "dbList.h"
#include "smt_config.h"

/*block if list empty*/
/*notes is task is littlevgl ui task, can not set block*/
db_list_t *db_list_create(char *name, int block)
{
	int ret = 0;
	db_list_t *db_list = NULL;

	if (NULL == name) {
		com_err("name param null");
	}

	db_list = (db_list_t *)malloc(sizeof(db_list_t));
	if (db_list == NULL) {
		errno = ENOMEM;
		com_err("malloc error");
		goto end;
	}
	memset(db_list, 0x00, sizeof(db_list_t));

	INIT_LIST_HEAD(&db_list->head);
	db_list->member_num = 0;
	db_list->block = block;
	strncpy(db_list->name, name, strlen(name) > sizeof(db_list->name)?sizeof(db_list->name):strlen(name));

	ret = pthread_mutex_init(&db_list->list_mutex, NULL);
	if (ret != 0) {
		com_err("pthread_mutex_init error");
		goto error0;
	}
	if (block) {
		ret = pthread_mutex_init(&db_list->list_cond_mutex, NULL);
		if (ret != 0) {
			com_err("pthread_mutex_init error");
			goto error1;
		}
		ret = pthread_cond_init(&db_list->list_cond, NULL);
		if (ret != 0) {
			com_err("pthread_cond_init error");
			goto error2;
		}
	}
	return db_list;

error2:
	pthread_mutex_destroy(&db_list->list_cond_mutex);
error1:
	pthread_mutex_destroy(&db_list->list_mutex);
error0:
	free(db_list);
end:
	return NULL;
}

int is_list_empty(db_list_t *db_list)
{
	int ret = 0;

	if (NULL == db_list) {
		com_err("param error");
		return 0;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	ret = list_empty(&db_list->head);
	pthread_mutex_unlock(&db_list->list_mutex);
	return ret;
}

void __db_list_clear(db_list_t *db_list)
{
	db_lnode_t *db_node = NULL, *db_node_tmp = NULL;

	if (db_list == NULL) {
		com_err("param error\n");
		return;
	}

	if (is_list_empty(db_list)) {
		com_info("%s empty!\n", db_list->name);
		return ;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry_safe(db_node, db_node_tmp, &db_list->head, list) {
		list_del(&db_node->list);
		db_list->member_num--;
		free(db_node);
	}
	pthread_mutex_unlock(&db_list->list_mutex);
}

int __db_list_put_tail(db_list_t *db_list, void *new_node_data)
{
	db_lnode_t *new_node = NULL;

	if (NULL == db_list || NULL == new_node_data) {
		errno=EINVAL;
		com_err("param error\n");
		return -1;
	}

	new_node = (db_lnode_t *)malloc(sizeof(db_lnode_t));
	if (new_node == NULL) {
		com_err("malloc error\n");
		return -1;
	}
	memset(new_node, 0x00, sizeof(db_lnode_t));

	INIT_LIST_HEAD(&new_node->list);
	new_node->data = new_node_data;

	pthread_mutex_lock(&db_list->list_mutex);
	db_list->member_num++;
	list_add_tail(&new_node->list, &db_list->head);
	pthread_mutex_unlock(&db_list->list_mutex);

	if (db_list->block) {
//		com_info("%s insert!\n", db_list->name);
		pthread_mutex_lock(&db_list->list_cond_mutex);
		pthread_cond_signal(&db_list->list_cond);
		pthread_mutex_unlock(&db_list->list_cond_mutex);
	}
	return 0;
}

//if list is not block must check return value
void *__db_list_pop(db_list_t *db_list)
{
	void *node_data = NULL;
	db_lnode_t *db_node = NULL, *db_node_tmp = NULL;

	if (db_list == NULL) {
		com_err("param error\n");
		return NULL;
	}

	if (is_list_empty(db_list)) {
		/*block if list empty*/
		if (db_list->block) {
//			com_info("%s empty will block!\n", db_list->name);
			pthread_mutex_lock(&db_list->list_cond_mutex);
			pthread_cond_wait(&db_list->list_cond, &db_list->list_cond_mutex);
			pthread_mutex_unlock(&db_list->list_cond_mutex);
		} else {
//			com_info("%s empty!\n", db_list->name);
			return NULL;
		}
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry_safe(db_node, db_node_tmp, &db_list->head, list) {
		node_data = db_node->data;
		list_del(&db_node->list);
		db_list->member_num--;
		free(db_node);
		pthread_mutex_unlock(&db_list->list_mutex);
		return node_data;
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	return NULL;
}
void *__db_list_search_and_pop(db_list_t* db_list,void* find_data ,int(*compare)(void* ,void* ))
{
	void *node_data = NULL;
	db_lnode_t *db_node = NULL, *db_node_tmp = NULL;

	if (db_list == NULL || find_data == NULL || compare == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return NULL;
	}

	if (is_list_empty(db_list)) {
//		com_err("%s empty!\n", db_list->name);
		return NULL;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry_safe(db_node, db_node_tmp, &db_list->head, list) {
		if (compare(db_node->data, find_data) == 0) {
			node_data = db_node->data;
			db_list->member_num--;
			list_del(&db_node->list);
			free(db_node);
			pthread_mutex_unlock(&db_list->list_mutex);
			return node_data;
		}
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	return NULL;
}

void *__db_list_search_node(db_list_t *db_list, void *find_data, int(*compare)(void* ,void* ))
{
	void *node_data = NULL;
	db_lnode_t *db_node = NULL;

	if (db_list == NULL || compare == NULL || find_data == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return NULL;
	}

	if (is_list_empty(db_list)) {
//		com_err("%s empty!\n", db_list->name);
		return NULL;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry(db_node, &db_list->head, list) {
		if (compare(db_node->data, find_data) == 0) {
			node_data = db_node->data;
			pthread_mutex_unlock(&db_list->list_mutex);
			return node_data;
		}
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	return NULL;
}

void *__db_list_search_node_byindex(db_list_t *db_list, int index)
{
	int i = 0;
	void *node_data = NULL;
	db_lnode_t *db_node = NULL;

	if (db_list == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return NULL;
	}

	if (is_list_empty(db_list)) {
		com_err("%s empty!\n", db_list->name);
		return NULL;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry(db_node, &db_list->head, list) {
		if (index == i) {
			node_data = db_node->data;
			pthread_mutex_unlock(&db_list->list_mutex);
			return node_data;
		}
		i++;
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	return NULL;
}

void __db_list_for_each_entry_and_pop(db_list_t *db_list, void *param, int(*scan_list)(void *, void*))
{
	db_lnode_t *db_node = NULL, *db_node_tmp = NULL;

	if (db_list == NULL || scan_list == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return ;
	}

	if (is_list_empty(db_list)) {
//		com_err("%s empty!\n", db_list->name);
		return ;
	}

	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry_safe(db_node, db_node_tmp, &db_list->head, list) {
		if (scan_list(db_node->data, param) == 1) {
			db_node->data = NULL;
			list_del(&db_node->list);
			free(db_node);
		}
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	return ;
}

int __db_list_get_num(db_list_t *db_list)
{
	int num = 0;

	if (db_list == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return 0;
	}
	pthread_mutex_lock(&db_list->list_mutex);
	num = db_list->member_num;
	pthread_mutex_unlock(&db_list->list_mutex);
	return num;
}

void __db_list_destory(db_list_t *db_list)
{
	db_lnode_t *db_node = NULL, *db_node_tmp = NULL;
	if (db_list == NULL) {
		errno = EINVAL;
		com_err("param error\n");
		return;
	}
	pthread_mutex_lock(&db_list->list_mutex);
	list_for_each_entry_safe(db_node, db_node_tmp, &db_list->head, list) {
		/*maybe need free db_node->data*/
		free(db_node->data);
		db_node->data = NULL;
		list_del(&db_node->list);
		free(db_node);
	}
	pthread_mutex_unlock(&db_list->list_mutex);
	if (db_list->block) {
		pthread_mutex_destroy(&db_list->list_cond_mutex);
		pthread_cond_destroy(&db_list->list_cond);
	}
	pthread_mutex_destroy(&db_list->list_mutex);
	free(db_list);
}
