#ifndef __BT_LE_HCI_H
#define __BT_LE_HCI_H

int bt_le_set_advertising_params(btmg_le_advertising_parameters_t *adv_param);
int bt_le_set_advertising_data(btmg_adv_data_t *adv_data);
int bt_le_advertising_enable(bool enable);
int bt_le_set_scan_rsp_data(btmg_scan_rsp_data_t *rsp_data);
int bt_le_set_random_address(void);
int find_ledev_conn_handle(int dev_id, uint8_t *remote_addr);

typedef enum {
    LE_SCAN_START = 0,
    LE_SCAN_STOP,
    LE_SET_SCAN_PARA,
    LE_UPDATE_CN_PARA,
} le_api_t;

typedef struct {
    btmg_le_scan_param_t *scan_param;
} bt_le_scan_start_args_t;

typedef struct {
    btmg_le_scan_param_t *scan_param;
} bt_le_set_scan_para_args_t;

typedef struct {
    btmg_le_conn_param_t *conn_params;
} bt_le_update_cn_para_args_t;

int find_ledev_conn_handle(int dev_id, uint8_t *remote_addr);

int bt_le_scan_init(int dev_id);
int bt_le_scan_deinit(int dev_id);

typedef struct {
    bool state;
    btmg_le_scan_param_t scan_param;
} le_scan_status_t;

le_scan_status_t *bt_le_get_scan_status(void);
int bt_le_scan_status_clean(le_scan_status_t *status);

int bt_le_disconnect(int dev_id, uint16_t handle, uint8_t reason);

#include <api_action.h>
extern act_func_t le_gap_action_table[];
#endif
