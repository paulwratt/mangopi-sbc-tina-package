#ifndef __BTMG_LIST_H
#define __BTMG_LIST_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*list_free_cb)(void *data);
typedef bool (*list_iter_cb)(void *data);
typedef bool (*list_iter_cb_ext)(void *data, void *cb_data);

typedef struct list_node_t {
    struct list_node_t *next;
    void *data;
} list_node_t;

typedef struct list_t {
    list_node_t *head;
    list_node_t *tail;
    size_t length;
    list_free_cb free_cb;
} list_t;

bool btmg_list_remove(list_t *list, void *data);
void btmg_list_clear(list_t *list);
list_t *btmg_list_new_internal(list_free_cb callback);
list_t *btmg_list_new(list_free_cb callback);
void btmg_list_free(list_t *list);
bool btmg_list_is_empty(const list_t *list);
bool btmg_list_contains(const list_t *list, const void *data);
size_t btmg_list_length(const list_t *list);
void *btmg_list_front(const list_t *list);
void *btmg_list_back(const list_t *list);
bool btmg_list_insert_after(list_t *list, list_node_t *prev_node, void *data);
bool btmg_list_prepend(list_t *list, void *data);
bool btmg_list_append(list_t *list, void *data);
void btmg_list_foreach(const list_t *list, list_iter_cb callback);
void btmg_list_foreach_ext(const list_t *list, list_iter_cb_ext callback, void *cb_data);
list_node_t *btmg_list_begin(const list_t *list);
list_node_t *btmg_list_end(const list_t *list);
list_node_t *btmg_list_next(const list_node_t *node);
void *btmg_list_node(const list_node_t *node);

#ifdef __cplusplus
};
#endif

#endif
