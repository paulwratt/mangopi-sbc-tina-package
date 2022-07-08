#include "bt_list.h"
#include "bt_log.h"

/*free each list node, including it's data and the list_node_t pointer itself*/
static list_node_t *btmg_list_free_node_(list_t *list, list_node_t *node)
{
    if (list == NULL || node == NULL)
        return NULL;

    list_node_t *next = node ? node->next : NULL;
    if (list && node) {
        if (list->free_cb) {
            list->free_cb(node->data); //call the registered callback function to do some cleanup
        }

        free(node);
        --list->length;
    }

    return next;
}

bool btmg_list_remove(list_t *list, void *data)
{
    if (list == NULL) {
        BTMG_ERROR("list is NULL!");
        return false;
    }

    if (data == NULL) {
        BTMG_ERROR("data is NULL!");
        return false;
    }

    if (!list || btmg_list_is_empty(list))
        return false;

    if (list->head->data == data) {
        list_node_t *next = btmg_list_free_node_(list, list->head);
        if (list->tail == list->head)
            list->tail = next;
        list->head = next;
        return true;
    }

    for (list_node_t *prev = list->head, *node = list->head->next; node;
         prev = node, node = node->next)
        if (node->data == data) {
            prev->next = btmg_list_free_node_(list, node);
            if (list->tail == node)
                list->tail = prev;
            return true;
        }

    return false;
}

void btmg_list_clear(list_t *list)
{
    if (list == NULL)
        return;

    if (list)
        for (list_node_t *node = list->head; node;)
            node = btmg_list_free_node_(list, node);
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

list_t *btmg_list_new_internal(list_free_cb callback)
{
    list_t *list = (list_t *)calloc(1, sizeof(list_t));
    if (!list)
        return NULL;

    list->free_cb = callback;
    return list;
}

list_t *btmg_list_new(list_free_cb callback)
{
    return btmg_list_new_internal(callback);
}

/*free all the list, including freeing it node firstly,
if haven't provide freeing function for each node's data, you should
free the memory each node's data pointer pointed to before running list_free()*/
void btmg_list_free(list_t *list)
{
    if (!list)
        return;

    btmg_list_clear(list);
    free(list);
}

bool btmg_list_is_empty(const list_t *list)
{
    if (list == NULL)
        return true;

    if (list)
        return (list->length == 0);
    else
        return true;
}

bool btmg_list_contains(const list_t *list, const void *data)
{
    if (list == NULL)
        return false;

    if (data == NULL)
        return false;

    for (const list_node_t *node = btmg_list_begin(list); node != btmg_list_end(list);
         node = btmg_list_next(node)) {
        if (btmg_list_node(node) == data)
            return true;
    }

    return false;
}

size_t btmg_list_length(const list_t *list)
{
    if (list)
        return list->length;
    else
        return 0;
}

void *btmg_list_front(const list_t *list)
{
    if (list && list->head)
        return list->head->data;
    return NULL;
}

void *btmg_list_back(const list_t *list)
{
    if (list && list->tail)
        return list->tail->data;
    return NULL;
}

bool btmg_list_insert_after(list_t *list, list_node_t *prev_node, void *data)
{
    if (list == NULL || prev_node == NULL || data == NULL)
        return false;

    list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));

    if (!node)
        return false;

    node->next = prev_node->next;
    node->data = data;
    prev_node->next = node;
    if (list->tail == prev_node)
        list->tail = node;
    ++list->length;

    return true;
}

bool btmg_list_prepend(list_t *list, void *data)
{
    if (!list || !data)
        return false;

    list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));
    if (!node)
        return false;

    node->next = list->head;
    node->data = data;
    list->head = node;
    if (list->tail == NULL)
        list->tail = list->head;
    ++list->length;
    return true;
}

bool btmg_list_append(list_t *list, void *data)
{
    if (!list || !data)
        return false;

    list_node_t *node = (list_node_t *)calloc(1, sizeof(list_node_t));
    if (!node)
        return false;
    node->next = NULL;
    node->data = data;
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    ++list->length;
    return true;
}

void btmg_list_foreach(const list_t *list, list_iter_cb callback)
{
    if (!list || !callback)
        return;

    if (list)
        for (list_node_t *node = list->head; node;) {
            list_node_t *next = node->next;
            callback(node->data);
            node = next;
        }
}

// Iterates through the entire |list| and calls |callback| for each data element.
// Passes the caller provided data along with node
// If the list is empty, |callback| will never be called. It is safe to mutate the
// list inside the callback. If an element is added before the node being visited,
// there will be no callback for the newly-inserted node. Neither |list| nor
// |callback| may be NULL.
void btmg_list_foreach_ext(const list_t *list, list_iter_cb_ext callback, void *cb_data)
{
    if (!list || !callback)
        return;

    if (list)
        for (list_node_t *node = list->head; node;) {
            list_node_t *next = node->next;
            callback(node->data, cb_data);
            node = next;
        }
}

list_node_t *btmg_list_begin(const list_t *list)
{
    if (list)
        return list->head;
    else
        return NULL;
}

list_node_t *btmg_list_end(const list_t *list)
{
    return NULL;
}

list_node_t *btmg_list_next(const list_node_t *node)
{
    if (node)
        return node->next;
    else
        return NULL;
}

void *btmg_list_node(const list_node_t *node)
{
    if (node)
        return node->data;

    return NULL;
}
