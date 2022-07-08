#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>
#include <bt_manager.h>
#include "bt_log.h"
#include "bt_dev_list.h"
#include "bt_test.h"

#include <bluetooth/bluetooth.h>
// #include <bluetooth/hci.h>
// #include <bluetooth/hci_lib.h>
// #include <src/shared/bt.h>
// #include <src/shared/hci.h>

#define NORDIC_UART_SERVICE_UUID ((char *)"6e400001-b5a3-f393-e0a9-e50e24dcca9e")
#define NORDIC_UART_CHAR_RX_UUID ((char *)"6e400002-b5a3-f393-e0a9-e50e24dcca9e")
#define NORDIC_UART_CHAR_TX_UUID ((char *)"6e400003-b5a3-f393-e0a9-e50e24dcca9e")

/******************************************************************
 * global variant for test
******************************************************************/

static int test_id = 0;

static uint16_t read_handle;
static uint16_t write_handle;
static uint16_t write_cmd_handle;
static uint16_t register_notify_indicate_handle;

static int test_init_times = 0;
static int test_deinit_times = 0;
static int test_scanon_times = 0;
static int test_scanoff_times = 0;
static int test_scaned_times = 0;

static int test_connect_times = 0;
static int test_connect_failed_times = 0;
static int test_disconnect_times = 0;

static int test_write_times = 0;
static int test_write_failed_times = 0;
;

static int test_read_times = 0;
static int test_read_failed_times = 0;
static int test_notify_indicate_times = 0;
static int test_indicate_times = 0;

static int gattc_cn_handle = 0;
static char gattc_select_addr[18];
static char gattc_connect_addr[18];

#define CMD_NOTIFY_INDICATE_ID_MAX 20
static int notify_indicate_id_save[CMD_NOTIFY_INDICATE_ID_MAX];

// #define PRINT_TEST_INFO printf("----test----: %d %d %d %d %d", test_init_times,test_deinit_times,test_scan_times,test_connect_times);

/******************************************************************
 * for test macro
******************************************************************/

#define TEST_ERR_RETURN(fun)                                                                       \
    {                                                                                              \
        int ret = fun;                                                                             \
        if (ret == 0) {                                                                            \
        } else {                                                                                   \
            printf("test" #fun "faild=%d\n", ret);                                                 \
            return ret;                                                                            \
        }                                                                                          \
    }

/******************************************************************
 * very simple thread pool
******************************************************************/
#include <stdlib.h>
#include <pthread.h>

struct log_queue_node {
    struct log_queue_node *next;
    struct log_queue_node *prev;
    void *data;
};

typedef struct log_queue {
    struct log_queue_node *front;
    struct log_queue_node *back;
    unsigned long size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} log_queue_t;
int log_queue_init(log_queue_t *b)
{
    int ret = 0;
    b->front = 0;
    b->back = 0;
    b->size = 0;
    if ((ret = pthread_mutex_init(&b->mutex, 0)) != 0) {
        return ret;
    }
    if ((ret = pthread_cond_init(&b->cond, 0)) != 0) {
        return ret;
    }
    return 0;
}

int log_queue_push(log_queue_t *b, void *data)
{
    int ret = 0;
    if ((ret = pthread_mutex_lock(&b->mutex)) != 0) {
        return ret;
    }
    struct log_queue_node *n = (struct log_queue_node *)malloc(sizeof(*n));
    n->data = data;
    n->next = 0;
    n->prev = b->back;
    if (b->back != 0) {
        b->back->next = n;
        b->back = n;
    } else {
        b->front = b->back = n;
    }
    b->size = b->size + 1;
    if ((ret = pthread_cond_signal(&b->cond)) != 0) {
        return ret;
    }
    if ((ret = pthread_mutex_unlock(&b->mutex)) != 0) {
        return ret;
    }
    return 0;
}

int log_queue_pop(log_queue_t *b, void **data)
{
    int ret = 0;
    if ((ret = pthread_mutex_lock(&b->mutex)) != 0) {
        return ret;
    }
    while (b->front == 0) {
        if ((ret = pthread_cond_wait(&b->cond, &b->mutex)) != 0) {
            // POSIX guarantees EINTR will not be returned
            return ret;
        }
    }
    struct log_queue_node *front = b->front;
    b->front = front->next;
    if (b->front != 0) {
        b->front->prev = 0;
    } else {
        b->back = 0;
    }
    *data = front->data;
    free(front);
    b->size = b->size - 1;
    if ((ret = pthread_mutex_unlock(&b->mutex)) != 0) {
        return ret;
    }
    return 0;
}

int log_queue_destroy(log_queue_t *b)
{
    int ret = 0;
    if ((ret = pthread_mutex_lock(&b->mutex)) != 0) {
        return ret;
    }
    while (b->front != 0) {
        struct log_queue_node *next = b->front->next;
        free(b->front);
        b->front = next;
    }
    if ((ret = pthread_mutex_unlock(&b->mutex)) != 0) {
        return ret;
    }
    if ((ret = pthread_mutex_destroy(&b->mutex)) != 0) {
        return ret;
    }
    if ((ret = pthread_cond_destroy(&b->cond)) != 0) {
        return ret;
    }
    return 0;
}

int log_queue_size(log_queue_t *b, unsigned long *size)
{
    int ret = 0;
    if ((ret = pthread_mutex_lock(&b->mutex)) != 0) {
        return ret;
    }
    *size = b->size;
    if ((ret = pthread_mutex_unlock(&b->mutex)) != 0) {
        return ret;
    }
    return 0;
}

typedef int (*act_function)(void *);

#define THREAD_POOL_NUM (30)

struct thread_pool {
    log_queue_t worker_queue;
    pthread_t worker_handle[THREAD_POOL_NUM];
};

static struct thread_pool thread_pools;

static void *gatt_stress_test_worker_loop(void *arg)
{
    act_function fun;
    act_function *p_fun;

    while (1) {
        log_queue_pop(&(thread_pools.worker_queue), (void **)&fun);
        if (fun) {
            fun(NULL);
        }
    }
    return NULL;
}

static int gatt_stress_test_worker_schedule(int (*function)(void *), void *arg, void **ret)
{
    if (!function) {
        return ENOEXEC;
    }
    if (log_queue_push(&(thread_pools.worker_queue), (void *)function) != 0) {
        return ENOMEM;
    }
    return 0;
}

static int gatt_stress_test_worker_init()
{
    int i = 0;
    if (log_queue_init(&(thread_pools.worker_queue)) != 0) {
        return -1;
    }
    if (thread_pools.worker_handle == NULL) {
        return -1;
    }
    for (i = 0; i < THREAD_POOL_NUM; i++) {
        if (pthread_create(&(thread_pools.worker_handle[i]), NULL, gatt_stress_test_worker_loop,
                           NULL)) {
            return -1;
        }
    }
    return 0;
}

static int gatt_stress_test_worker_deinit()
{
    int i = 0;
    log_queue_destroy(&(thread_pools.worker_queue));
    for (i = 0; i < THREAD_POOL_NUM; i++) {
        pthread_cancel(thread_pools.worker_handle[i]);
    }
    return 0;
}

/******************************************************************
 * for gatt client
******************************************************************/

#define GATT_TINA_TEST 1

static int fun_init(void *arg)
{
    test_init_times++;
    return bt_manager_gatt_client_init();
}
static int fun_deinit(void *arg)
{
    test_deinit_times++;
    return bt_manager_gatt_client_deinit();
}

static int fun_scanon(void *arg)
{
    static btmg_le_scan_param_t scan_param = {
        .scan_type = LE_SCAN_TYPE_ACTIVE,
        .scan_interval = 0x0010,
        .scan_window = 0x0010,
        .filter_duplicate = LE_SCAN_DUPLICATE_DISABLE,
        .own_addr_type = BTMG_LE_PUBLIC_ADDRESS,
        .filter_policy = LE_SCAN_FILTER_POLICY_ALLOW_ALL,
    };
    TEST_ERR_RETURN(
            bt_manager_le_set_scan_parameters(&scan_param));
    TEST_ERR_RETURN(bt_manager_le_scan_start(&scan_param));
    test_scanon_times++;
    return 0;
}

static int fun_scanoff(void *arg)
{
    TEST_ERR_RETURN(bt_manager_le_scan_stop());
    test_scanoff_times++;
    return 0;
}

static int fun_connect(void *addr)
{
    // if (addr == NULL) {
    // 	BTMG_ERROR("ERROR_TEST\n");
    // 	return 0;
    // }
    BTMG_DEBUG("fun_connect");
    if (strlen(gattc_connect_addr) < 17) {
        return 0;
    }
    btmg_le_addr_type_t addr_type = BTMG_LE_RANDOM_ADDRESS; //use random addr for test
    TEST_ERR_RETURN(bt_manager_gatt_client_connect((uint8_t *)gattc_connect_addr, addr_type, 517,
                                                   BTMG_SECURITY_LOW));
    return 0;
}

static int fun_disconnect(uint8_t *addr)
{
    // if (addr == NULL) {
    // 	BTMG_ERROR("ERROR_TEST\n");
    // 	return 0;
    // }
    TEST_ERR_RETURN(bt_manager_gatt_client_disconnect((uint8_t *)addr));
    return 0;
}

static int fun_update_conn_params(void *arg)
{
    btmg_le_conn_param_t conn_params;
    conn_params.min_conn_interval = 0x0020;
    conn_params.max_conn_interval = 0x0100;
    conn_params.slave_latency = 0;
    conn_params.conn_sup_timeout = 0x0400;
    TEST_ERR_RETURN(bt_manager_le_update_conn_params(&conn_params));
    return 0;
}

static int task_read(void *p_handle)
{
    uint16_t handle;
    while (1) {
        handle = read_handle;
        TEST_ERR_RETURN(bt_manager_gatt_client_read_request(gattc_cn_handle, handle));
        sched_yield();
    }
    return 0;
}

static int task_write(void *p_handle)
{
    // if (p_handle == NULL) {
    // 	BTMG_ERROR("ERROR_TEST\n");
    // 	return 0;
    // }
    uint16_t handle;
    static uint8_t data[60] = { 0 };
    int len = sizeof(data);
    while (1) {
        handle = write_handle;
        TEST_ERR_RETURN(bt_manager_gatt_client_write_request(gattc_cn_handle, handle, data, len));
        data[0]++;
        sched_yield();
    }
    return 0;
}

static int task_write_cmd(void *p_handle)
{
    // if (p_handle == NULL) {
    // 	BTMG_ERROR("ERROR_TEST\n");
    // 	return 0;
    // }
    uint16_t handle;
    static uint8_t data[60] = { 0 };
    int len = sizeof(data);
    while (1) {
        handle = write_cmd_handle;
        TEST_ERR_RETURN(bt_manager_gatt_client_write_command(gattc_cn_handle, handle, false, data, len));
        data[0]++;
        usleep(1000);
    }
    return 0;
}

static int fun_discover_svc(void *arg)
{
    bt_manager_gatt_client_discover_all_services(gattc_cn_handle, 0x0001, 0xffff);
    return 0;
}

static int fun_register_notify_indicate(void *p_handle)
{
    // if (p_handle == NULL) {
    // 	BTMG_ERROR("ERROR_TEST\n");
    // 	return 0;
    // }
    uint16_t handle = register_notify_indicate_handle;
    int i = 0;
    int notify_indicate_id;

    notify_indicate_id = bt_manager_gatt_client_register_notify_indicate(gattc_cn_handle, handle);
    if (notify_indicate_id == 0) {
        printf("register err\n");
    } else {
        for (int i = 0; i < CMD_NOTIFY_INDICATE_ID_MAX && notify_indicate_id_save[i]; i++) {
            // do nothing
        }
        notify_indicate_id_save[i] = notify_indicate_id;
        printf("register with id = %d\n", notify_indicate_id);
    }
    return 0;
}

// static void bt_test_gattc_get_conn_list_cb(gattc_connected_list_cb_para_t *data)
// {
// 	if (strcmp((const char *)(data->addr),gattc_select_addr)==0) {
// 		gattc_cn_handle = data->cn_handle;
// 		printf("----------------[select]----------------\n");
// 	}
// 	printf("[device]: %p %s (%s)\n",data->cn_handle,
// 		data->addr,data->addr_type==BTMG_LE_PUBLIC_ADDRESS?"public"
// 		:(data->addr_type==BTMG_LE_RANDOM_ADDRESS?"random":"error"));
// 	return;
// }

static void bt_test_gattc_scan_report_cb(btmg_le_scan_report_t *data)
{
    int j = 0;
    int type;
    char name[31] = { 0 };
    char addrstr[64] = { 0 };

    for (;;) {
        type = data->report.data[j + 1];
        //complete local name.
        if (type == 0x09) {
            memcpy(name, &data->report.data[j + 2], data->report.data[j] - 1);
            name[data->report.data[j] - 1] = '\0';
        }
        j = j + data->report.data[j] + 1;
        if (j >= data->report.data_len)
            break;
    }
    if (name[0] == 'A') {
        char adv_name[20];
        snprintf(adv_name, sizeof(adv_name) - 1, "AW_BLE_4.0_%04d", test_id);
        if (strcmp(adv_name, name) == 0) {
            printf("cmp adv name\n");
        }
        char addr_str[18];
        snprintf(addr_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", data->peer_addr[5],
                 data->peer_addr[4], data->peer_addr[3], data->peer_addr[2], data->peer_addr[1],
                 data->peer_addr[0]);
        memcpy(gattc_connect_addr, (uint8_t *)addr_str, 18);
        gatt_stress_test_worker_schedule(fun_scanoff, NULL, NULL);
        gatt_stress_test_worker_schedule(fun_connect, NULL, NULL);
    }
    test_scaned_times++;
    // snprintf(addrstr, 64, "%02X:%02X:%02X:%02X:%02X:%02X (%s)",
    // 		data->peer_addr[5], data->peer_addr[4], data->peer_addr[3],
    // 		data->peer_addr[2], data->peer_addr[1], data->peer_addr[0],
    // 		(data->addr_type==BTMG_LE_PUBLIC_ADDRESS)?("public"):
    // 		((data->addr_type==BTMG_LE_RANDOM_ADDRESS)?("random"):
    // 		("error")));

    // printf("[devices]: %s, adv type:%d, rssi:%d %s\n",addrstr, data->adv_type, data->rssi, name);
}

void bt_test_gattc_notify_indicate_cb(gattc_notify_indicate_cb_para_t *data)
{
    int i;

    printf("\n\tHandle Value Notify/Indicate: 0x%04x - ", data->value_handle);

    if (data->length == 0) {
        printf("(0 bytes)\n");
        return;
    }

    printf("(%u bytes): ", data->length);

    for (i = 0; i < data->length; i++)
        printf("%02x ", data->value[i]);

    printf("\n");
}

void bt_test_gattc_write_long_cb(gattc_write_long_cb_para_t *data)
{
    if (data->success) {
        printf("Write successful\n");
    } else if (data->reliable_error) {
        printf("Reliable write not verified\n");
    } else {
        printf("\nWrite failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
    }
}

void bt_test_gattc_write_cb(gattc_write_cb_para_t *data)
{
    if (data->success) {
        test_write_times++;
        printf("\nWrite successful\n");
    } else {
        test_write_failed_times++;
        printf("\nWrite failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
    }
}

void bt_test_gattc_read_cb(gattc_read_cb_para_t *data)
{
    int i;
    if (data->success) {
        test_read_times++;
        printf("\nRead successful\n");
    } else {
        test_read_failed_times++;
        printf("\nRead failed: %s (0x%02x)\n",
               bt_manager_gatt_client_ecode_to_string(data->att_ecode), data->att_ecode);
    }
    return;
    // printf("\nRead value");
    // if (data->length == 0) {
    // 	printf(": 0 bytes\n");
    // 	return;
    // }
    // printf(" (%u bytes): ", data->length);
    // for (i = 0; i < data->length; i++)
    // 	printf("%02x ", data->value[i]);
    // printf("\n");
}

void bt_test_gattc_conn_cb(gattc_conn_cb_para_t *data)
{
    if (!data->success) {
        printf("gattc connect failed,error code: 0x%02x\n", data->att_ecode);
        test_connect_failed_times++;
        return;
    }
    test_connect_times++;
    gattc_cn_handle = data->conn_id;
    printf("gattc connect completed, conn_id = 0x%x\n", gattc_cn_handle);
    gatt_stress_test_worker_schedule(fun_discover_svc, NULL, NULL);
}

void bt_test_gattc_disconn_cb(gattc_disconn_cb_para_t *data)
{
    gattc_cn_handle = 0;
    // for multi connections, who disconnected?
    BTMG_INFO("Device disconnected");

    switch (data->reason) {
    case LOCAL_HOST_TERMINATED:
        BTMG_INFO("reason: LOCAL_HOST_TERMINATED");
        break;
    case CONNECTION_TIMEOUT:
        BTMG_INFO("reason: CONNECTION_TIMEOUT");
        break;
    case REMOTE_USER_TERMINATED:
        BTMG_INFO("reason: REMOTE_USER_TERMINATED");
        break;
    default:
        BTMG_INFO("reason: UNKNOWN_OTHER_ERROR");
    }
    gatt_stress_test_worker_schedule(fun_scanon, NULL, NULL);
}

void bt_test_gattc_service_changed_cb(gattc_service_changed_cb_para_t *data)
{
    printf("\nService Changed handled - start: 0x%04x end: 0x%04x\n", data->start_handle,
           data->end_handle);
}

static void print_uuid(const btmg_uuid_t *uuid)
{
    char uuid_str[37];
    btmg_uuid_t uuid128;

    bt_manager_uuid_to_uuid128(uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, uuid_str, sizeof(uuid_str));

    printf("%s\n", uuid_str);
}

void bt_test_gattc_dis_service_cb(gattc_dis_service_cb_para_t *data)
{
    char buf[37];
    btmg_uuid_t uuid128;

    bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));

    // printf("-------|--------------------------------------------\n");
    // printf("0x%04x | %s:\n",data->start_handle,
    // 	!strcmp(buf, "00001800-0000-1000-8000-00805f9b34fb")?"Generic Access":
    // 	(!strcmp(buf, "00001801-0000-1000-8000-00805f9b34fb")?"Generic Attribute":"Unknow Service"));
    // printf("       | UUID: %s\n",buf);
    // printf("       | %s:\n",data->primary ? "PRIMARY SERVICE" : "SECONDARY SERVICE");
    // printf("       |\n");
    bt_manager_gatt_client_discover_service_all_char(data->conn_id, data->attr);
}

static void bt_test_gattc_dis_char_cb(gattc_dis_char_cb_para_t *data)
{
    char buf[128];
    btmg_uuid_t uuid128;

    bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));

    if (strcmp(buf, "00002223-0000-1000-8000-00805f9b34fb") == 0) {
        write_handle = data->value_handle;
        gatt_stress_test_worker_schedule(task_write, NULL, NULL);
        gatt_stress_test_worker_schedule(task_write, NULL, NULL);
    }
    if (strcmp(buf, "00003334-0000-1000-8000-00805f9b34fb") == 0) {
        read_handle = data->value_handle;
        gatt_stress_test_worker_schedule(task_read, NULL, NULL);
        gatt_stress_test_worker_schedule(task_read, NULL, NULL);
    }
    if (strcmp(buf, NORDIC_UART_CHAR_RX_UUID) == 0) {
        write_cmd_handle = data->value_handle;
        gatt_stress_test_worker_schedule(task_write_cmd, NULL, NULL);
    }
    if (strcmp(buf, NORDIC_UART_CHAR_TX_UUID) == 0) {
        register_notify_indicate_handle = data->value_handle;
        gatt_stress_test_worker_schedule(fun_register_notify_indicate, NULL, NULL);
    }

    // snprintf(buf, sizeof(buf), "0x%04x |     Unknow Characteristic          :%s %s %s %s",
    // 	data->start_handle,
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_READ ? "R" : ""),
    // 	(((data->properties & BT_GATT_CHAR_PROPERTY_WRITE) ||
    // 	 (data->properties & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE) ||
    // 	 (data->properties & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE)) ? "W" : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_NOTIFY ? "N" : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_INDICATE ? "I" : ""));
    // printf("%s\n", buf);
    // bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    // bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));
    // printf("       |     UUID:%s\n", buf);
    // snprintf(buf, sizeof(buf), "       |     Properties:%s%s%s%s%s%s%s",
    // 	(data->ext_prop?"EXTENDED PROPS,":""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_READ ? "READ," : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_WRITE ? "WRITE," : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE ? "WRITE NO RESPONSE," : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_NOTIFY ? "NOTIFY," : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_INDICATE ? "INDICATE," : ""),
    // 	(data->properties & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE ? "SIGNED WRITE," : ""));
    // printf("%s\n",buf);
    // printf("0x%04x |     Value:\n", data->value_handle);

    bt_manager_gatt_client_discover_char_all_descriptor(data->conn_id, data->attr);
}

static void bt_test_gattc_dis_desc_cb(gattc_dis_desc_cb_para_t *data)
{
    char buf[128];
    btmg_uuid_t uuid128;
    // printf("       |     %s\n",
    // (data->uuid.value.u16==0x2902)?"Client Characteristic Configuration":"Unknow Descripter");
    // bt_manager_uuid_to_uuid128(&data->uuid, &uuid128);
    // bt_manager_uuid_to_string(&uuid128, buf, sizeof(buf));
    // printf("       |     UUID: %s\n",buf);
    // printf("0x%04x |     Value:(0x)**-**\n",data->handle);
}

static int conn_list_for_connect = 0;
static int conn_list_for_compare = 0;

static void bt_test_gattc_get_conn_list_cb(gattc_connected_list_cb_para_t *data)
{
    if (conn_list_for_connect) {
        if (strcmp((const char *)(data->addr), gattc_select_addr) == 0) {
            gattc_cn_handle = data->conn_id;
            printf("----------------[select]----------------\n");
        }
    }
    if (conn_list_for_compare) {
        // if (strcmp((const char *)(data->addr),gattc_select_addr)==0) {
        // 	gattc_cn_handle = data->conn_id;
        // 	printf("----------------[select]----------------\n");
        // }
    }
    printf("[device]: %d %s (%s)\n", data->conn_id, data->addr,
           data->addr_type == BTMG_LE_PUBLIC_ADDRESS ?
                   "public" :
                   (data->addr_type == BTMG_LE_RANDOM_ADDRESS ? "random" : "error"));
    return;
}

btmg_callback_t *global_cb = NULL;

static void __bt_gatt_client_test_demo_register_callback(btmg_callback_t *cb)
{
    if (!global_cb) {
        BTMG_ERROR("global cb is NULL!!!");
        return;
    }
    global_cb->btmg_gatt_client_cb.gattc_conn_cb = bt_test_gattc_conn_cb;
    global_cb->btmg_gatt_client_cb.gattc_disconn_cb = bt_test_gattc_disconn_cb;
    global_cb->btmg_gatt_client_cb.gattc_read_cb = bt_test_gattc_read_cb;
    global_cb->btmg_gatt_client_cb.gattc_write_cb = bt_test_gattc_write_cb;
    global_cb->btmg_gatt_client_cb.gattc_write_long_cb = bt_test_gattc_write_long_cb;
    global_cb->btmg_gatt_client_cb.gattc_service_changed_cb = bt_test_gattc_service_changed_cb;
    global_cb->btmg_gatt_client_cb.gattc_notify_indicate_cb = bt_test_gattc_notify_indicate_cb;
    global_cb->btmg_gatt_client_cb.gattc_dis_service_cb = bt_test_gattc_dis_service_cb;
    global_cb->btmg_gatt_client_cb.gattc_dis_char_cb = bt_test_gattc_dis_char_cb;
    global_cb->btmg_gatt_client_cb.gattc_dis_desc_cb = bt_test_gattc_dis_desc_cb;
    global_cb->btmg_gatt_client_cb.gattc_connected_list_cb = bt_test_gattc_get_conn_list_cb;

    global_cb->btmg_gap_cb.gap_le_scan_report_cb = bt_test_gattc_scan_report_cb;
    return;
}

void bt_gatt_client_test_demo_register_callback(btmg_callback_t *cb)
{
    global_cb = cb;
}

static void cmd_gatt_client_connections(int argc, char *args[])
{
    memset(gattc_select_addr, 0, 18);
    bt_manager_gatt_client_get_conn_list();
    return;
}

static void cmd_gatt_client_select(int argc, char *args[])
{
    if (argc < 1) {
        goto end;
    }
    if (strlen(args[0]) < 17) {
        goto end;
    }
    memcpy(gattc_select_addr, (uint8_t *)args[0], 18);
    bt_manager_gatt_client_get_conn_list();
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
    return;
}

static void cmd_gatt_client_dis_all_svcs(int argc, char *args[])
{
    btmg_log_level_t debug_level = bt_manager_get_loglevel();
    bt_manager_set_loglevel(BTMG_LOG_LEVEL_INFO);
    printf("----------------------------------------------------\nhandle | \n");
    bt_manager_gatt_client_discover_all_services(gattc_cn_handle, 0x0001, 0xffff);
    printf("---------------------------------------------------\n");
    bt_manager_set_loglevel(debug_level);

    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
}

static void cmd_gatt_client_get_mtu(int argc, char *args[])
{
    int mtu;
    mtu = bt_manager_gatt_client_get_mtu(gattc_cn_handle);

    if (mtu > 0) {
        printf("[MTU]:%d\n", mtu);
    } else {
        printf("get mtu failed.\n");
    }
    return;
}

static void cmd_gatt_client_register_notify_indicate(int argc, char *args[])
{
    uint16_t handle;
    int notify_indicate_id;
    int i;
    char *endptr = NULL;
    if (argc != 1) {
        goto end;
    }
    handle = strtol(args[0], &endptr, 0);
    notify_indicate_id = bt_manager_gatt_client_register_notify_indicate(gattc_cn_handle, handle);
    if (notify_indicate_id == 0) {
        printf("register err\n");
    } else {
        for (i = 0; i < (CMD_NOTIFY_INDICATE_ID_MAX - 1) && notify_indicate_id_save[i]; i++) {
            // do nothing
        }
        notify_indicate_id_save[i] = notify_indicate_id;
        printf("register with id = %d\n", notify_indicate_id);
    }
    return;
end:
    BTMG_ERROR("Unexpected argc: %d, see help", argc);
    return;
}

static void cmd_gatt_client_unregister_notify_indicate(int argc, char *args[])
{
    int i;
    for (i = 0; i < CMD_NOTIFY_INDICATE_ID_MAX && notify_indicate_id_save[i]; i++) {
        bt_manager_gatt_client_unregister_notify_indicate(gattc_cn_handle, notify_indicate_id_save[i]);
        notify_indicate_id_save[i] = 0;
    }
    return;
}

static void cmd_ble_set_scan_parameters(int argc, char *args[])
{
    static btmg_le_scan_param_t scan_param = {
        .scan_type = LE_SCAN_TYPE_PASSIVE,
        .scan_interval = 0x0010,
        .scan_window = 0x0005,
        .filter_duplicate = LE_SCAN_DUPLICATE_DISABLE,
        .own_addr_type = BTMG_LE_PUBLIC_ADDRESS,
        .filter_policy = LE_SCAN_FILTER_POLICY_ALLOW_ALL,
    };

    bt_manager_le_set_scan_parameters(&scan_param);

    return;
}

// for outer use

void cmd_gatt_testcase_client(int argc, char *args[])
{
    if (argc <= 0) {
        BTMG_ERROR("error see help");
        return;
    }
    test_id = atoi(args[0]);
    gatt_stress_test_worker_init();
    bt_gatt_client_deinit();
    __bt_gatt_client_test_demo_register_callback(global_cb);
    bt_gatt_client_init();
    fun_scanon(NULL);

    return;
}

void cmd_gatt_stress_test_client(int argc, char *args[])
{
    return;
}
