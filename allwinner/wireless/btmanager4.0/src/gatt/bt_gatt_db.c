#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include "lib/l2cap.h"
#include "lib/uuid.h"
#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"
#include "bt_manager.h"
#include "common.h"
#include "bt_log.h"
#include "bt_mainloop.h"
#include "bt_gatt_server.h"
#include "bt_gatt_inner.h"

#define UUID_GAP 0x1800
#define UUID_GATT 0x1801

static void gatt_service_changed_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                    uint16_t offset, uint8_t opcode, struct bt_att *att,
                                    void *user_data)
{
    BTMG_INFO("Service Changed Read called");

    gatt_db_attribute_read_result(attrib, id, 0, NULL, 0);
}

static void gap_device_name_read_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                    uint16_t offset, uint8_t opcode, struct bt_att *att,
                                    void *user_data)
{
    gatt_server_t *server = user_data;
    uint8_t error = 0;
    size_t len = 0;
    const uint8_t *value = NULL;

    BTMG_INFO("GAP Device Name Read called");

    len = server->name_len;

    if (offset > len) {
        error = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    len -= offset;
    value = len ? &server->device_name[offset] : NULL;

done:
    gatt_db_attribute_read_result(attrib, id, error, value, len);
}
static void gap_device_name_write_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                     uint16_t offset, const uint8_t *value, size_t len,
                                     uint8_t opcode, struct bt_att *att, void *user_data)
{
    gatt_server_t *server = user_data;
    uint8_t error = 0;

    BTMG_INFO("GAP Device Name Write called");

    /* If the value is being completely truncated, clean up and return */
    if (!(offset + len)) {
        free(server->device_name);
        server->device_name = NULL;
        server->name_len = 0;
        goto done;
    }

    /* Implement this as a variable length attribute value. */
    if (offset > server->name_len) {
        error = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    if (offset + len != server->name_len) {
        uint8_t *name;

        name = realloc(server->device_name, offset + len);
        if (!name) {
            error = BT_ATT_ERROR_INSUFFICIENT_RESOURCES;
            goto done;
        }

        server->device_name = name;
        server->name_len = offset + len;
    }

    if (value)
        memcpy(server->device_name + offset, value, len);

done:
    gatt_db_attribute_write_result(attrib, id, error);
}

static void gatt_svc_chngd_ccc_write_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                        uint16_t offset, const uint8_t *value, size_t len,
                                        uint8_t opcode, struct bt_att *att, void *user_data)
{
    gatt_server_t *server = user_data;
    uint8_t ecode = 0;

    BTMG_INFO("Service Changed CCC Write called");

    if (!value || len != 2) {
        ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
        goto done;
    }

    if (offset) {
        ecode = BT_ATT_ERROR_INVALID_OFFSET;
        goto done;
    }

    if (value[0] == 0x00)
        server->svc_chngd_enabled = false;
    else if (value[0] == 0x02)
        server->svc_chngd_enabled = true;
    else
        ecode = 0x80;

    BTMG_DEBUG("Service Changed Enabled: %s", server->svc_chngd_enabled ? "true" : "false");

done:
    gatt_db_attribute_write_result(attrib, id, ecode);
}

static void gap_device_name_ext_prop_read_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                             uint16_t offset, uint8_t opcode, struct bt_att *att,
                                             void *user_data)
{
    uint8_t value[2];

    BTMG_INFO("Device Name Extended Properties Read called");

    value[0] = BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE;
    value[1] = 0;

    gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

static void gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute *attrib, unsigned int id,
                                       uint16_t offset, uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
    gatt_server_t *server = user_data;
    uint8_t value[2];

    BTMG_INFO("Service Changed CCC Read called");

    value[0] = server->svc_chngd_enabled ? 0x02 : 0x00;
    value[1] = 0x00;

    gatt_db_attribute_read_result(attrib, id, 0, value, sizeof(value));
}

static void confirm_write(struct gatt_db_attribute *attr, int err, void *user_data)
{
    if (!err)
        return;

    fprintf(stderr, "Error caching attribute %p - err: %d\n", attr, err);
    exit(1);
}

static void populate_gap_service(gatt_client_t *server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *tmp;
    uint16_t appearance;

    /* Add the GAP service */
    bt_uuid16_create(&uuid, UUID_GAP);
    service = gatt_db_add_service(server->dbs, &uuid, true, 6);

    /*
	* Device Name characteristic. Make the value dynamically read and
	* written via callbacks.
	*/
    bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
    gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
                                       BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_EXT_PROP,
                                       gap_device_name_read_cb, gap_device_name_write_cb, server);

    bt_uuid16_create(&uuid, GATT_CHARAC_EXT_PROPER_UUID);
    gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ,
                                   gap_device_name_ext_prop_read_cb, NULL, server);

    /*
	* Appearance characteristic. Reads and writes should obtain the value
	* from the database.
	*/
    bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
    tmp = gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
                                             BT_GATT_CHRC_PROP_READ, NULL, NULL, server);

    /*
	* Write the appearance value to the database, since we're not using a
	* callback.
	*/
    put_le16(128, &appearance);
    gatt_db_attribute_write(tmp, 0, (void *)&appearance, sizeof(appearance), BT_ATT_OP_WRITE_REQ,
                            NULL, confirm_write, NULL);

    gatt_db_service_set_active(service, true);
}

static void populate_gatt_service(gatt_client_t *server)
{
    bt_uuid_t uuid;
    struct gatt_db_attribute *service, *svc_chngd;

    /* Add the GATT service */
    bt_uuid16_create(&uuid, UUID_GATT);
    service = gatt_db_add_service(server->dbs, &uuid, true, 4);

    bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
    svc_chngd =
            gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
                                               BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_INDICATE,
                                               gatt_service_changed_cb, NULL, server);
    server->gatt_svc_chngd_handle = gatt_db_attribute_get_handle(svc_chngd);

    bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
    gatt_db_service_add_descriptor(service, &uuid, BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
                                   gatt_svc_chngd_ccc_read_cb, gatt_svc_chngd_ccc_write_cb, server);

    gatt_db_service_set_active(service, true);
}

int bt_gatt_db_add_default_services(gatt_client_t *server)
{
    if (!server->dbs) {
        BTMG_ERROR("Failed to create GATT database\n");
        return -1;
    }
    populate_gap_service(server);
    populate_gatt_service(server);
    return 0;
}
