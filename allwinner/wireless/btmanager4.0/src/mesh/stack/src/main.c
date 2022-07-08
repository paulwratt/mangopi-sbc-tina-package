/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017-2019  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
//fix warning
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/stat.h>
#include <ell/ell.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "mesh/mesh.h"
#include "mesh/crypto.h"
#include "mesh/dbus.h"
#include "mesh/mesh-io.h"
#include "xr829-patch.h"

static const char *config_dir;
static const char *mesh_conf_fname;
static enum mesh_io_type io_type;
static void *io_opts;
/*
static const struct option main_options[] = {
	{ "io",		required_argument,	NULL, 'i' },
	{ "config",	optional_argument,	NULL, 'c' },
	{ "nodetach",	no_argument,		NULL, 'n' },
	{ "debug",	no_argument,		NULL, 'd' },
	{ "dbus-debug",	no_argument,		NULL, 'b' },
	{ "help",	no_argument,		NULL, 'h' },
	{ }
};

static void usage(void)
{
	fprintf(stderr,
		"Usage:\n"
	       "\tbluetooth-meshd [options]\n");
	fprintf(stderr,
		"Options:\n"
	       "\t--io <io>         Use specified io (default: generic)\n"
	       "\t--config          Configuration directory\n"
	       "\t--nodetach        Run in foreground\n"
	       "\t--debug           Enable debug output\n"
	       "\t--dbus-debug      Enable D-Bus debugging\n"
	       "\t--help            Show %s information\n", __func__);
	fprintf(stderr,
	       "io:\n"
	       "\t([hci]<index> | generic[:[hci]<index>])\n"
	       "\t\tUse generic HCI io on interface hci<index>, or the first\n"
	       "\t\tavailable one\n");
}
*/

static void do_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	l_info("%s%s", prefix, str);
}

static void mesh_ready_callback(void *user_data, bool success)
{
	struct l_dbus *dbus = user_data;

	if (!success) {
		l_error("Failed to start mesh");
		l_main_quit();
		return;
	}
	if (!dbus_init(dbus)) {
		l_error("Failed to initialize mesh D-Bus resources");
		l_main_quit();
        return ;
	}
}

static void request_name_callback(struct l_dbus *dbus, bool success,
					bool queued, void *user_data)
{
	l_info("Request name %s",
		success ? "success": "failed");

	if (!success) {
		l_main_quit();
		return;
	}
    int index = 0;
    //io_opts
	if (!mesh_init(config_dir, mesh_conf_fname, io_type,&index ,
					mesh_ready_callback, dbus)) {
		l_error("Failed to initialize mesh");
		l_main_quit();
	}
}

static void ready_callback(void *user_data)
{
	struct l_dbus *dbus = user_data;

	l_info("D-Bus ready");
	l_dbus_name_acquire(dbus, BLUEZ_MESH_NAME, false, false, false,
						request_name_callback, NULL);
}

static void disconnect_callback(void *user_data)
{
    xr_warn(TRC_COMMON,"dbus disconnect\n");
	l_main_quit();
}
/*

static void signal_handler(uint32_t signo, void *user_data)
{
	static bool terminated;

	if (terminated)
		return;

	l_info("Terminating");
	l_main_quit();
	terminated = true;
}

static bool parse_io(const char *optarg, enum mesh_io_type *type, void **opts)
{
	if (strstr(optarg, "generic") == optarg) {
		int *index = l_new(int, 1);

		*type = MESH_IO_TYPE_GENERIC;
		*opts = index;

		optarg += strlen("generic");
		if (!*optarg) {
			*index = MGMT_INDEX_NONE;
			return true;
		}

		if (*optarg != ':')
			return false;

		optarg++;

		if (sscanf(optarg, "hci%d", index) == 1)
			return true;

		if (sscanf(optarg, "%d", index) == 1)
			return true;

		return false;
	}

	return false;
}
*/
int32_t meshd_init()
{
	int status;
	bool detached = true;
	bool dbus_debug = false;
	struct l_dbus *dbus = NULL;
	char *io = NULL;
	//int hci_index;
    //int argc = 0;
    //char *argv[] = {NULL};

	l_log_set_stderr();

	if (!mesh_crypto_check_avail()) {
		l_error("Mesh Crypto functions unavailable");
		goto done;
	}

    //hci_index = 0;
    io_type = MESH_IO_TYPE_GENERIC;
    io_opts = NULL;
    detached = false;
    l_debug_enable("*");

    if (!detached)
        umask(0077);

    dbus = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
    if (!dbus) {
        l_error("unable to connect to D-Bus");
        status = EXIT_FAILURE;
        goto done;
    }

    if (dbus_debug)
        l_dbus_set_debug(dbus, do_debug, "[DBUS] ", NULL);
    l_dbus_set_ready_handler(dbus, ready_callback, dbus, NULL);
    l_dbus_set_disconnect_handler(dbus, disconnect_callback, NULL, NULL);

    if (!XR_DBUS_MGR_ENABLE(dbus)) {
        l_error("Failed to enable Object Manager");
        status = EXIT_FAILURE;
        goto done;
    }
    return 0;

done:
    if (io)
        l_free(io);

    if (io_opts)
        l_free(io_opts);

    mesh_cleanup();
    l_dbus_destroy(dbus);
    l_main_exit();
    return status;
}
