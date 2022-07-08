#pragma once
#ifndef HFP_RFCOMM_H_
#define HFP_RFCOMM_H_

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include "at.h"

/* Timeout for the command acknowledgment. */
#define BA_RFCOMM_TIMEOUT_ACK 1000
/* Timeout for the connection idle state. */
#define BA_RFCOMM_TIMEOUT_IDLE 2500
/* Number of retries during the SLC stage. */
#define BA_RFCOMM_SLC_RETRIES 10

/**
 * Data associated with RFCOMM communication. */
struct hfp_rfcomm_t {
    /* RFCOMM socket */
    int fd;
    pthread_t thread;
    /* thread notification PIPE */
    int sig_fd[2];
    /* service level connection state */
    enum hfp_slc_state state;
    enum hfp_slc_state state_prev;
    /* initial connection setup */
    enum hfp_setup setup;
    /* handler used for sync response dispatching */
    const struct ba_rfcomm_handler *handler;
    enum hfp_slc_state handler_resp_ok_new_state;
    bool handler_resp_ok_success;
    /* external RFCOMM handler */
    int handler_fd;
    /* determine whether connection is idle */
    bool idle;
    /* number of failed communication attempts */
    int retries;
    /* AG/HF supported features bitmask */
    uint32_t hfp_features;
    /* requested codec by the AG */
    int codec;
    /* received AG indicator values */
    unsigned char hfp_ind[__HFP_IND_MAX];
    /* indicator activation state */
    bool hfp_ind_state[__HFP_IND_MAX];
    /* 0-based indicators index */
    enum hfp_ind hfp_ind_map[20];
    /* received event reporting setup */
    unsigned int hfp_cmer[5];
    /* variables used for AG<->HF sync */
    uint8_t gain_mic;
    uint8_t gain_spk;
    /* BlueZ does not trigger profile disconnection signal when the Bluetooth
	 * link has been lost (e.g. device power down). However, it is required to
	 * remove all references, otherwise resources will not be freed. If this
	 * quirk workaround is enabled, RFCOMM link lost will trigger SCO transport
	 * destroy rather than a simple unreferencing. */
    bool link_lost_quirk;
};

/**
 * Callback function used for RFCOMM AT message dispatching. */
typedef int ba_rfcomm_callback(struct hfp_rfcomm_t *r, const struct bt_at *at);

/**
 * AT message dispatching handler. */
struct ba_rfcomm_handler {
    enum bt_at_type type;
    const char *command;
    ba_rfcomm_callback *callback;
};

struct hfp_rfcomm_t *hfp_rfcomm_new(int fd);
int rfcomm_write_at(int fd, enum bt_at_type type, const char *command, const char *value);

#endif
