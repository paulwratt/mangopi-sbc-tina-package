#ifndef __DBLIST_H__
#define __DBLIST_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"

typedef struct db_lnode{
	void* data;
	struct list_head list;
} db_lnode_t;

typedef struct db_list {
	pthread_mutex_t list_mutex;

	int block;
	pthread_cond_t	list_cond;
	pthread_mutex_t list_cond_mutex;

	unsigned int member_num;
	char name[32];
	struct list_head head;
} db_list_t;

/*block is list empty*/
/*notes is task is littlevgl ui task, can not set block*/
#define db_list_
db_list_t *db_list_create(char *name, int block);
int is_list_empty(db_list_t *list_head);
int __db_list_put_tail(db_list_t *list_head, void *new_node_data);
int __db_list_get_num(db_list_t *db_list);
void __db_list_clear(db_list_t *list_head);
void __db_list_destory(db_list_t *list_head);
void *__db_list_search_and_pop(db_list_t *db_list, void *find_data, int(*compare)(void *, void*));
void *__db_list_pop(db_list_t *db_list);
void *__db_list_search_node(db_list_t *db_list, void *find_data, int(*compare)(void* , void*));
void *__db_list_search_node_byindex(db_list_t *db_list, int index);
void __db_list_for_each_entry_and_pop(db_list_t *db_list, void *param, int(*scan_list)(void *, void*));
#endif
