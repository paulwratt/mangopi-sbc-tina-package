#ifndef __AT_H_
#define __AT_H_

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "bt_hfp.h"

enum bt_at_type {
    AT_TYPE_RAW,
    AT_TYPE_CMD,
    AT_TYPE_CMD_GET,
    AT_TYPE_CMD_SET,
    AT_TYPE_CMD_TEST,
    AT_TYPE_RESP,
    __AT_TYPE_MAX
};

struct bt_at {
    enum bt_at_type type;
    char command[256];
    char *value;
};

char *at_build(char *buffer, size_t size, enum bt_at_type type, const char *command,
               const char *value);
char *at_parse(const char *str, struct bt_at *at);
int at_parse_bia(const char *str, bool state[__HFP_IND_MAX]);
int at_parse_cind(const char *str, enum hfp_ind map[20]);
int at_parse_cmer(const char *str, unsigned int map[5]);
const char *at_type2str(enum bt_at_type type);

#endif
