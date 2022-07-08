#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <glib.h>
#include "defs.h"
#include "bt_log.h"
#include "hfp_rfcomm.h"
#include "bt_hfp.h"
#include "bt_hfp_alsa.h"
/**
 * Structure used for buffered reading from the RFCOMM. */
struct at_reader {
    struct bt_at at;
    char buffer[256];
    /* pointer to the next message within the buffer */
    char *next;
};

/**
 * Read AT message.
 *
 * Upon error it is required to set the next pointer of the reader structure
 * to NULL. Otherwise, this function might fail indefinitely.
 *
 * @param fd RFCOMM socket file descriptor.
 * @param reader Pointer to initialized reader structure.
 * @return On success this function returns 0. Otherwise, -1 is returned and
 *   errno is set to indicate the error. */
static int rfcomm_read_at(int fd, struct at_reader *reader)
{
    char *buffer = reader->buffer;
    char *msg = reader->next;
    char *tmp;

    /* In case of reading more than one message from the RFCOMM, we have to
	 * parse all of them before we can read from the socket once more. */
    if (msg == NULL) {
        ssize_t len;

    retry:
        if ((len = read(fd, buffer, sizeof(reader->buffer))) == -1) {
            if (errno == EINTR)
                goto retry;
            return BT_ERROR;
        }

        if (len == 0) {
            errno = ECONNRESET;
            return BT_ERROR;
        }

        buffer[len] = '\0';
        msg = buffer;
    }

    /* parse AT message received from the RFCOMM */
    if ((tmp = at_parse(msg, &reader->at)) == NULL) {
        reader->next = msg;
        errno = EBADMSG;
        return BT_ERROR;
    }
    reader->next = tmp[0] != '\0' ? tmp : NULL;

    return BT_OK;
}

/**
 * Write AT message.
 *
 * @param fd RFCOMM socket file descriptor.
 * @param type Type of the AT message.
 * @param command AT command or response code.
 * @param value AT value or NULL if not applicable.
 * @return On success this function returns 0. Otherwise, -1 is returned and
 *   errno is set to indicate the error. */
int rfcomm_write_at(int fd, enum bt_at_type type, const char *command, const char *value)
{
    char msg[256];
    size_t len;

    BTMG_INFO("Sending AT message: %s: command:%s, value:%s", at_type2str(type), command, value);

    at_build(msg, sizeof(msg), type, command, value);
    len = strlen(msg);

retry:
    if (write(fd, msg, len) == -1) {
        if (errno == EINTR)
            goto retry;
        return BT_ERROR;
    }

    return BT_OK;
}

/**
 * HFP set state wrapper for debugging purposes. */
static void rfcomm_set_hfp_state(struct hfp_rfcomm_t *r, enum hfp_slc_state state)
{
    BTMG_INFO("HFP RFCOMM state transition: %d -> %d", r->state, state);

    r->state = state;
}

/**
 * Handle AT command response code. */
static int rfcomm_handler_resp_ok_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_DEBUG("enter");

    r->handler_resp_ok_success = strcmp(at->value, "OK") == 0;

    /* advance service level connection state */
    if (r->handler_resp_ok_success && r->state != HFP_SLC_CONNECTED)
        rfcomm_set_hfp_state(r, r->handler_resp_ok_new_state);

    if (!r->handler_resp_ok_success)
        r->handler = NULL;

    return BT_OK;
}

/**
 * TEST: Standard indicator update AT command */
static int rfcomm_handler_cind_test_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;

    BTMG_DEBUG("enter");
    /* NOTE: The order of indicators in the CIND response message
	 *       has to be consistent with the hfp_ind enumeration. */
    if (rfcomm_write_at(fd, AT_TYPE_RESP, "+CIND",
                        "(\"service\",(0-1))"
                        ",(\"call\",(0,1))"
                        ",(\"callsetup\",(0-3))"
                        ",(\"callheld\",(0-2))"
                        ",(\"signal\",(0-5))"
                        ",(\"roam\",(0-1))"
                        ",(\"battchg\",(0-5))") == -1)
        return BT_ERROR;

    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
        return BT_ERROR;

    if (r->state < HFP_SLC_CIND_TEST_OK)
        rfcomm_set_hfp_state(r, HFP_SLC_CIND_TEST_OK);

    return BT_OK;
}

/**
 * GET: Standard indicator update AT command */
static int rfcomm_handler_cind_get_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;

    BTMG_DEBUG("enter");

    if (rfcomm_write_at(fd, AT_TYPE_RESP, "+CIND", "0,0,0,0,0,0,0") == -1)
        return BT_ERROR;
    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
        return BT_ERROR;

    if (r->state < HFP_SLC_CIND_GET_OK)
        rfcomm_set_hfp_state(r, HFP_SLC_CIND_GET_OK);

    return BT_OK;
}

/**
 * RESP: Standard indicator update AT command */
static int rfcomm_handler_cind_resp_test_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_DEBUG("enter");

    /* parse response for the +CIND TEST command */
    if (at_parse_cind(at->value, r->hfp_ind_map) == -1)
        BTMG_ERROR("Couldn't parse AG indicators: %s", at->value);

    if (r->state < HFP_SLC_CIND_TEST)
        rfcomm_set_hfp_state(r, HFP_SLC_CIND_TEST);

    return BT_OK;
}

/**
 * RESP: Standard indicator update AT command */
static int rfcomm_handler_cind_resp_get_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    char *tmp = at->value;
    size_t i;

    BTMG_DEBUG("enter");

    /* parse response for the +CIND GET command */
    for (i = 0; i < ARRAYSIZE(r->hfp_ind_map); i++) {
        r->hfp_ind[r->hfp_ind_map[i]] = atoi(tmp);
#if 0
		if (r->hfp_ind_map[i] == HFP_IND_BATTCHG) {
		//Can parse cind response events
		}
#endif
        if ((tmp = strchr(tmp, ',')) == NULL)
            break;
        tmp += 1;
    }

    if (r->state < HFP_SLC_CIND_GET)
        rfcomm_set_hfp_state(r, HFP_SLC_CIND_GET);

    return BT_OK;
}

/**
 * SET: Standard event reporting activation/deactivation AT command */
static int rfcomm_handler_cmer_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;
    const char *resp = "OK";

    BTMG_DEBUG("enter");

    if (at_parse_cmer(at->value, r->hfp_cmer) == -1) {
        BTMG_ERROR("Couldn't parse CMER setup: %s", at->value);
        resp = "ERROR";
    }

    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, resp) == -1)
        return BT_ERROR;

    if (r->state < HFP_SLC_CMER_SET_OK)
        rfcomm_set_hfp_state(r, HFP_SLC_CMER_SET_OK);

    return BT_OK;
}

/**
 * RESP: Standard indicator events reporting unsolicited result code */
static int rfcomm_handler_ciev_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    unsigned int index;
    unsigned int value;

    BTMG_DEBUG("enter");

    if (sscanf(at->value, "%u,%u", &index, &value) == 2 && --index < ARRAYSIZE(r->hfp_ind_map)) {
        r->hfp_ind[r->hfp_ind_map[index]] = value;
        switch (r->hfp_ind_map[index]) {
        case HFP_IND_CALLSETUP:
            if (value == 0) {
                BTMG_INFO("Not currently in call set up");
            } else if (value == 1) {
                BTMG_INFO("An incoming call process is ongoing");
                bt_hfp_hf_pcm_start();
            } else if (value == 2) {
                BTMG_INFO("An outgoing call set up is ongoing");
                bt_hfp_hf_pcm_start();
            } else if (value == 3) {
                BTMG_INFO("The remote party being alerted in an outgoing call");
            }
            break;
        case HFP_IND_CALLHELD:
            BTMG_INFO("HFP_IND_CALLHELD");
            break;
        case HFP_IND_CALL:
            if (value == 0) {
                BTMG_INFO("No call is active");
                bt_hfp_hf_pcm_stop();
            } else if (value == 1) {
                BTMG_INFO("A call is active");
                bt_hfp_hf_pcm_start();
            }
            break;
        default:
            break;
        }
    }
    return BT_OK;
}

/**
 * SET: Bluetooth Indicators Activation */
static int rfcomm_handler_bia_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    const int fd = r->fd;
    const char *resp = "OK";

    BTMG_DEBUG("enter");

    if (at_parse_bia(at->value, r->hfp_ind_state) == -1) {
        BTMG_ERROR("Couldn't parse BIA indicators activation: %s", at->value);
        resp = "ERROR";
    }

    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, resp) == -1)
        return BT_ERROR;

    return BT_OK;
}

/**
 * SET: Bluetooth Retrieve Supported Features */
static int rfcomm_handler_brsf_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_DEBUG("enter");
#if 0
// look as ag
	const int fd = r->fd;
	char tmp[16];

	r->hfp_features = atoi(at->value);

	/* If codec negotiation is not supported in the HF, the AT+BAC
	 * command will not be sent. So, we can assume default codec. */
	if (!(r->hfp_features & HFP_HF_FEAT_CODEC))
		ba_transport_set_codec(t_sco, HFP_CODEC_CVSD);

	sprintf(tmp, "%u", ba_adapter_get_hfp_features_ag(t_sco->d->a));
	if (rfcomm_write_at(fd, AT_TYPE_RESP, "+BRSF", tmp) == -1)
		return BT_ERROR;
	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;

	if (r->state < HFP_SLC_BRSF_SET_OK)
		rfcomm_set_hfp_state(r, HFP_SLC_BRSF_SET_OK);
#endif
    return BT_OK;
}

/**
 * RESP: Bluetooth Retrieve Supported Features */
static int rfcomm_handler_brsf_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    r->hfp_features = atoi(at->value);
    BTMG_DEBUG("enter");

    /* codec negotiation is not supported in the AG */
    /* HFP over PCM does not need to be managed */
    if (!(r->hfp_features & HFP_AG_FEAT_CODEC)) {
        BTMG_INFO("codec negotiation is not supported in the AG");
    }

    if (r->state < HFP_SLC_BRSF_SET)
        rfcomm_set_hfp_state(r, HFP_SLC_BRSF_SET);

    return BT_OK;
}

/**
 * SET: Noise Reduction and Echo Canceling */
static int rfcomm_handler_nrec_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;

    /* Currently, we are not supporting Noise Reduction & Echo Canceling,
	 * so just acknowledge this SET request with "ERROR" response code. */
    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
        return BT_ERROR;

    return BT_OK;
}

/**
 * SET: Gain of Microphone */
static int rfcomm_handler_vgm_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	const int fd = r->fd;

	/* skip update in case of software volume */
	if (pcm->soft_volume)
		return rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK");

	r->gain_mic = atoi(at->value);
	int level = ba_transport_pcm_volume_bt_to_level(pcm, r->gain_mic);
	ba_transport_pcm_volume_set(&pcm->volume[0], &level, NULL, NULL);
	bluealsa_dbus_pcm_update(pcm, BA_DBUS_PCM_UPDATE_VOLUME);

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * RESP: Gain of Microphone */
static int rfcomm_handler_vgm_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	struct ba_transport * const t_sco = r->sco;
	struct ba_transport_pcm *pcm = &t_sco->sco.mic_pcm;

	r->gain_mic = atoi(at->value);
	int level = ba_transport_pcm_volume_bt_to_level(pcm, r->gain_mic);
	ba_transport_pcm_volume_set(&pcm->volume[0], &level, NULL, NULL);
	bluealsa_dbus_pcm_update(pcm, BA_DBUS_PCM_UPDATE_VOLUME);
#endif
    return BT_OK;
}

/**
 * SET: Gain of Speaker */
static int rfcomm_handler_vgs_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	struct ba_transport * const t_sco = r->sco;
	struct ba_transport_pcm *pcm = &t_sco->sco.spk_pcm;
	const int fd = r->fd;

	/* skip update in case of software volume */
	if (pcm->soft_volume)
		return rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK");

	r->gain_spk = atoi(at->value);
	int level = ba_transport_pcm_volume_bt_to_level(pcm, r->gain_spk);
	ba_transport_pcm_volume_set(&pcm->volume[0], &level, NULL, NULL);
	bluealsa_dbus_pcm_update(pcm, BA_DBUS_PCM_UPDATE_VOLUME);

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * RESP: Gain of Speaker */
static int rfcomm_handler_vgs_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	struct ba_transport * const t_sco = r->sco;
	struct ba_transport_pcm *pcm = &t_sco->sco.spk_pcm;

	r->gain_spk = atoi(at->value);
	int level = ba_transport_pcm_volume_bt_to_level(pcm, r->gain_spk);
	ba_transport_pcm_volume_set(&pcm->volume[0], &level, NULL, NULL);
	bluealsa_dbus_pcm_update(pcm, BA_DBUS_PCM_UPDATE_VOLUME);
#endif

    return BT_OK;
}

/**
 * SET: Bluetooth Response and Hold Feature */
static int rfcomm_handler_btrh_get_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;

    BTMG_DEBUG("enter");
    /* Currently, we are not supporting Respond & Hold feature, so just
	 * acknowledge this GET request without reporting +BTRH status. */
    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
        return BT_ERROR;

    return BT_OK;
}

/**
 * SET: Bluetooth Codec Connection */
static int rfcomm_handler_bcc_cmd_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    (void)at;
    const int fd = r->fd;

    BTMG_DEBUG("enter");
    /* TODO: Start Codec Connection procedure because HF wants to send audio. */
    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
        return BT_ERROR;

    return BT_OK;
}

/**
 * SET: Bluetooth Codec Selection */
static int rfcomm_handler_bcs_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    const int fd = r->fd;
    int codec;

    BTMG_INFO("enter");
#if 0
	if ((codec = atoi(at->value)) != r->codec) {
		BTMG_ERROR("Codec not acknowledged: %s != %d", at->value, r->codec);
		if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
			return BT_ERROR;
		goto final;
	}

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;

	/* Codec negotiation process is complete. Update transport and
	 * notify connected clients, that transport has been changed. */
	ba_transport_set_codec(t_sco, codec);
	bluealsa_dbus_pcm_update(&t_sco->sco.spk_pcm,
			BA_DBUS_PCM_UPDATE_SAMPLING | BA_DBUS_PCM_UPDATE_CODEC);
	bluealsa_dbus_pcm_update(&t_sco->sco.mic_pcm,
			BA_DBUS_PCM_UPDATE_SAMPLING | BA_DBUS_PCM_UPDATE_CODEC);

final:
	pthread_cond_signal(&r->codec_selection_completed);
#endif

    return BT_OK;
}

static int rfcomm_handler_resp_bcs_ok_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	if (rfcomm_handler_resp_ok_cb(r, at) == -1)
		return BT_ERROR;

	if (!r->handler_resp_ok_success) {
		BTMG_ERROR("Codec selection not finalized: %d", r->codec);
		goto final;
	}

	/* Finalize codec selection process and notify connected clients, that
	 * transport has been changed. Note, that this event might be emitted
	 * for an active transport - switching initiated by Audio Gateway. */
	ba_transport_set_codec(t_sco, r->codec);
	bluealsa_dbus_pcm_update(&t_sco->sco.spk_pcm,
			BA_DBUS_PCM_UPDATE_SAMPLING | BA_DBUS_PCM_UPDATE_CODEC);
	bluealsa_dbus_pcm_update(&t_sco->sco.mic_pcm,
			BA_DBUS_PCM_UPDATE_SAMPLING | BA_DBUS_PCM_UPDATE_CODEC);

final:
	pthread_cond_signal(&r->codec_selection_completed);
#endif
    return BT_OK;
}

/**
 * RESP: Bluetooth Codec Selection */
static int rfcomm_handler_bcs_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_DEBUG("enter");

    static const struct ba_rfcomm_handler handler = { AT_TYPE_RESP, "",
                                                      rfcomm_handler_resp_bcs_ok_cb };
    const int fd = r->fd;
    r->codec = atoi(at->value);
    if (rfcomm_write_at(fd, AT_TYPE_CMD_SET, "+BCS", at->value) == -1)
        return BT_ERROR;
    r->handler = &handler;

    return BT_OK;
}

/**
 * SET: Bluetooth Available Codecs */
static int rfcomm_handler_bac_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    const int fd = r->fd;
    char *tmp = at->value - 1;

    BTMG_DEBUG("enter");

    do {
        tmp += 1;
    } while ((tmp = strchr(tmp, ',')) != NULL);

    if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
        return BT_ERROR;

    if (r->state < HFP_SLC_BAC_SET_OK)
        rfcomm_set_hfp_state(r, HFP_SLC_BAC_SET_OK);

    return BT_OK;
}

/**
 * SET: Android Ext: XHSMICMUTE: Zebra HS3100 microphone mute */
static int rfcomm_handler_android_set_xhsmicmute(struct hfp_rfcomm_t *r, char *value)
{
    BTMG_INFO("enter");
#if 0
	if (value == NULL)
		return errno = EINVAL, -1;

	struct ba_transport * const t_sco = r->sco;
	struct ba_transport_pcm *pcm = &t_sco->sco.mic_pcm;
	const int fd = r->fd;

	bool muted = value[0] == '0' ? false : true;
	ba_transport_pcm_volume_set(&pcm->volume[0], NULL, NULL, &muted);
	bluealsa_dbus_pcm_update(pcm, BA_DBUS_PCM_UPDATE_VOLUME);

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * SET: Android Ext: XHSTBATSOC: Zebra HS3100 battery state of charge */
static int rfcomm_handler_android_set_xhstbatsoc(struct hfp_rfcomm_t *r, char *value)
{
    BTMG_INFO("enter");
#if 0
	if (value == NULL)
		return errno = EINVAL, -1;

	struct ba_device * const d = r->sco->d;
	const int fd = r->fd;

	char *ptr = value;
	d->battery.charge = atoi(strsep(&ptr, ","));
	bluealsa_dbus_rfcomm_update(r, BA_DBUS_RFCOMM_UPDATE_BATTERY);
	bluez_battery_provider_update(d);

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif

    return BT_OK;
}

/**
 * SET: Android Ext: XHSTBATSOH: Zebra HS3100 battery state of health */
static int rfcomm_handler_android_set_xhstbatsoh(struct hfp_rfcomm_t *r, char *value)
{
    BTMG_INFO("enter");
#if 0
	if (value == NULL)
		return errno = EINVAL, -1;

	struct ba_device * const d = r->sco->d;
	const int fd = r->fd;

	char *ptr = value;
	d->battery.health = atoi(strsep(&ptr, ","));
	bluealsa_dbus_rfcomm_update(r, BA_DBUS_RFCOMM_UPDATE_BATTERY);
	bluez_battery_provider_update(d);

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * SET: Android Ext: Report various state changes */
static int rfcomm_handler_android_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	static const struct {
		const char *name;
		int (*cb)(struct hfp_rfcomm_t *, char *);
	} handlers[] = {
		{ "XHSMICMUTE", rfcomm_handler_android_set_xhsmicmute },
		{ "XHSTBATSOC", rfcomm_handler_android_set_xhstbatsoc },
		{ "XHSTBATSOH", rfcomm_handler_android_set_xhstbatsoh },
	};

	char *sep = ",";
	char *value = at->value;
	char *name = strsep(&value, sep);

	size_t i;
	for (i = 0; i < ARRAYSIZE(handlers); i++)
		if (strcmp(name, handlers[i].name) == 0) {
			int rv = handlers[i].cb(r, value);
			if (rv == -1 && errno == EINVAL)
				break;
			return rv;
		}

	if (value == NULL)
		sep = value = "";
	BTMG_ERROR("Unsupported +ANDROID value: %s%s%s", name, sep, value);
	if (rfcomm_write_at(r->fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * SET: Apple Ext: Report a headset state change */
static int rfcomm_handler_iphoneaccev_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	struct ba_device * const d = r->sco->d;
	const int fd = r->fd;

	char *ptr = at->value;
	size_t count = atoi(strsep(&ptr, ","));
	char tmp;

	while (count-- && ptr != NULL)
		switch (tmp = *strsep(&ptr, ",")) {
		case '1':
			if (ptr != NULL) {
				d->battery.charge = atoi(strsep(&ptr, ",")) * 100 / 9;
				bluealsa_dbus_rfcomm_update(r, BA_DBUS_RFCOMM_UPDATE_BATTERY);
				bluez_battery_provider_update(d);
			}
			break;
		case '2':
			if (ptr != NULL)
				d->xapl.accev_docked = atoi(strsep(&ptr, ","));
			break;
		default:
			BTMG_ERROR("Unsupported +IPHONEACCEV key: %c", tmp);
			strsep(&ptr, ",");
		}

	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif
    return BT_OK;
}

/**
 * SET: Apple Ext: Enable custom AT commands from an accessory */
static int rfcomm_handler_xapl_set_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    BTMG_INFO("enter");
#if 0
	struct ba_device * const d = r->sco->d;
	const int fd = r->fd;

	unsigned int vendor, product;
	char version[sizeof(d->xapl.software_version)];
	char resp[32];
	char *tmp;

	if ((tmp = strrchr(at->value, ',')) == NULL) {
		BTMG_ERROR("Invalid +XAPL value: %s", at->value);
		if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
			return BT_ERROR;
		return BT_OK;
	}

	d->xapl.features = atoi(tmp + 1);
	*tmp = '\0';

	if (sscanf(at->value, "%x-%x-%7s", &vendor, &product, version) != 3)
		BTMG_ERROR("Couldn't parse +XAPL vendor and product: %s", at->value);

	d->xapl.vendor_id = vendor;
	d->xapl.product_id = product;
	strcpy(d->xapl.software_version, version);

	snprintf(resp, sizeof(resp), "+XAPL=%s,%u",
			config.hfp.xapl_product_name, config.hfp.xapl_features);
	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, resp) == -1)
		return BT_ERROR;
	if (rfcomm_write_at(fd, AT_TYPE_RESP, NULL, "OK") == -1)
		return BT_ERROR;
#endif

    return BT_OK;
}

/**
 * RESP: Apple Ext: Enable custom AT commands from an accessory */
static int rfcomm_handler_xapl_resp_cb(struct hfp_rfcomm_t *r, const struct bt_at *at)
{
    char *tmp;

    BTMG_DEBUG("enter");

    static const struct ba_rfcomm_handler handler = { AT_TYPE_RESP, "", rfcomm_handler_resp_ok_cb };

    if ((tmp = strrchr(at->value, ',')) == NULL)
        return BT_ERROR;

    xapl_features = atoi(tmp + 1);
    r->handler = &handler;

    return BT_OK;
}

static const struct ba_rfcomm_handler rfcomm_handler_resp_ok = { AT_TYPE_RESP, "",
                                                                 rfcomm_handler_resp_ok_cb };
static const struct ba_rfcomm_handler rfcomm_handler_cind_test = { AT_TYPE_CMD_TEST, "+CIND",
                                                                   rfcomm_handler_cind_test_cb };
static const struct ba_rfcomm_handler rfcomm_handler_cind_get = { AT_TYPE_CMD_GET, "+CIND",
                                                                  rfcomm_handler_cind_get_cb };
static const struct ba_rfcomm_handler rfcomm_handler_cind_resp_test = {
    AT_TYPE_RESP, "+CIND", rfcomm_handler_cind_resp_test_cb
};
static const struct ba_rfcomm_handler rfcomm_handler_cind_resp_get = {
    AT_TYPE_RESP, "+CIND", rfcomm_handler_cind_resp_get_cb
};
static const struct ba_rfcomm_handler rfcomm_handler_cmer_set = { AT_TYPE_CMD_SET, "+CMER",
                                                                  rfcomm_handler_cmer_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_ciev_resp = { AT_TYPE_RESP, "+CIEV",
                                                                   rfcomm_handler_ciev_resp_cb };
static const struct ba_rfcomm_handler rfcomm_handler_bia_set = { AT_TYPE_CMD_SET, "+BIA",
                                                                 rfcomm_handler_bia_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_brsf_set = { AT_TYPE_CMD_SET, "+BRSF",
                                                                  rfcomm_handler_brsf_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_brsf_resp = { AT_TYPE_RESP, "+BRSF",
                                                                   rfcomm_handler_brsf_resp_cb };
static const struct ba_rfcomm_handler rfcomm_handler_nrec_set = { AT_TYPE_CMD_SET, "+NREC",
                                                                  rfcomm_handler_nrec_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_vgm_set = { AT_TYPE_CMD_SET, "+VGM",
                                                                 rfcomm_handler_vgm_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_vgm_resp = { AT_TYPE_RESP, "+VGM",
                                                                  rfcomm_handler_vgm_resp_cb };
static const struct ba_rfcomm_handler rfcomm_handler_vgs_set = { AT_TYPE_CMD_SET, "+VGS",
                                                                 rfcomm_handler_vgs_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_vgs_resp = { AT_TYPE_RESP, "+VGS",
                                                                  rfcomm_handler_vgs_resp_cb };
static const struct ba_rfcomm_handler rfcomm_handler_btrh_get = { AT_TYPE_CMD_GET, "+BTRH",
                                                                  rfcomm_handler_btrh_get_cb };
static const struct ba_rfcomm_handler rfcomm_handler_bcc_cmd = { AT_TYPE_CMD, "+BCC",
                                                                 rfcomm_handler_bcc_cmd_cb };
static const struct ba_rfcomm_handler rfcomm_handler_bcs_set = { AT_TYPE_CMD_SET, "+BCS",
                                                                 rfcomm_handler_bcs_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_bcs_resp = { AT_TYPE_RESP, "+BCS",
                                                                  rfcomm_handler_bcs_resp_cb };
static const struct ba_rfcomm_handler rfcomm_handler_bac_set = { AT_TYPE_CMD_SET, "+BAC",
                                                                 rfcomm_handler_bac_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_android_set = {
    AT_TYPE_CMD_SET, "+ANDROID", rfcomm_handler_android_set_cb
};
static const struct ba_rfcomm_handler rfcomm_handler_iphoneaccev_set = {
    AT_TYPE_CMD_SET, "+IPHONEACCEV", rfcomm_handler_iphoneaccev_set_cb
};
static const struct ba_rfcomm_handler rfcomm_handler_xapl_set = { AT_TYPE_CMD_SET, "+XAPL",
                                                                  rfcomm_handler_xapl_set_cb };
static const struct ba_rfcomm_handler rfcomm_handler_xapl_resp = { AT_TYPE_RESP, "+XAPL",
                                                                   rfcomm_handler_xapl_resp_cb };

/**
 * Get callback (if available) for given AT message. */
static ba_rfcomm_callback *rfcomm_get_callback(const struct bt_at *at)
{
    static const struct ba_rfcomm_handler *handlers[] = {
        &rfcomm_handler_resp_ok,         &rfcomm_handler_cind_test, &rfcomm_handler_cind_get,
        &rfcomm_handler_cmer_set,        &rfcomm_handler_ciev_resp, &rfcomm_handler_bia_set,
        &rfcomm_handler_brsf_set,        &rfcomm_handler_nrec_set,  &rfcomm_handler_vgm_set,
        &rfcomm_handler_vgm_resp,        &rfcomm_handler_vgs_set,   &rfcomm_handler_vgs_resp,
        &rfcomm_handler_btrh_get,        &rfcomm_handler_bcc_cmd,   &rfcomm_handler_bcs_set,
        &rfcomm_handler_bcs_resp,        &rfcomm_handler_bac_set,   &rfcomm_handler_android_set,
        &rfcomm_handler_iphoneaccev_set, &rfcomm_handler_xapl_set,  &rfcomm_handler_xapl_resp,
    };

    size_t i;

    for (i = 0; i < ARRAYSIZE(handlers); i++) {
        if (handlers[i]->type != at->type)
            continue;
        if (strcmp(handlers[i]->command, at->command) != 0)
            continue;

        return handlers[i]->callback;
    }

    return NULL;
}

void ba_rfcomm_destroy(struct hfp_rfcomm_t *r)
{
    int err;

    /* Disable link lost quirk, because we don't want
	 * any interference during the destroy procedure. */
    r->link_lost_quirk = false;
    if (!pthread_equal(r->thread, pthread_self())) {
        if ((err = pthread_cancel(r->thread)) != 0 && err != ESRCH)
            BTMG_ERROR("Couldn't cancel RFCOMM thread: %s", strerror(err));
        if ((err = pthread_join(r->thread, NULL)) != 0)
            BTMG_ERROR("Couldn't join RFCOMM thread: %s", strerror(err));
    }

    if (r->handler_fd != -1)
        close(r->handler_fd);

    if (r->sig_fd[0] != -1)
        close(r->sig_fd[0]);
    if (r->sig_fd[1] != -1)
        close(r->sig_fd[1]);

    free(r);
}

static void rfcomm_thread_cleanup(struct hfp_rfcomm_t *r)
{
    if (r->fd == -1)
        return;

    BTMG_DEBUG("Closing RFCOMM: %d", r->fd);

    shutdown(r->fd, SHUT_RDWR);
    close(r->fd);
    r->fd = -1;

    ba_rfcomm_destroy(r);
}

static void *rfcomm_thread(struct hfp_rfcomm_t *r)
{
    char tmp[24];
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_push(PTHREAD_CLEANUP(rfcomm_thread_cleanup), r);

    struct at_reader reader = { .next = NULL };

    struct pollfd pfds[] = {
        { r->sig_fd[0], POLLIN, 0 },
        { r->fd, POLLIN, 0 },
        { -1, POLLIN, 0 },
    };

    BTMG_DEBUG("Starting hfp rfcomm thread");

    for (;;) {
        /* During normal operation, RFCOMM should block indefinitely. However,
		 * in the HFP-HF mode, service level connection has to be initialized
		 * by ourself. In order to do this reliably, we have to assume, that
		 * AG might not receive our message and will not send proper response.
		 * Hence, we will incorporate timeout, after which we will send our
		 * AT command once more. */
        int timeout = BA_RFCOMM_TIMEOUT_IDLE;
        ba_rfcomm_callback *callback;
        char tmp[256];

        if (r->handler != NULL)
            goto process;

        if (r->state != HFP_SLC_CONNECTED) {
            /* If some progress has been made in the SLC procedure, reset the
			 * retries counter. */
            if (r->state != r->state_prev) {
                r->state_prev = r->state;
                r->retries = 0;
            }

            /* If the maximal number of retries has been reached, terminate the
			 * connection. Trying indefinitely will only use up our resources. */
            if (r->retries > BA_RFCOMM_SLC_RETRIES) {
                BTMG_ERROR("Couldn't establish connection: Too many retries");
                errno = ETIMEDOUT;
                goto ioerror;
            }

            switch (r->state) {
            case HFP_DISCONNECTED:
                sprintf(tmp, "%u", get_hfp_features_hf());
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_SET, "+BRSF", tmp) == -1)
                    goto ioerror;
                r->handler = &rfcomm_handler_brsf_resp;
                break;
            case HFP_SLC_BRSF_SET:
                r->handler = &rfcomm_handler_resp_ok;
                r->handler_resp_ok_new_state = HFP_SLC_BRSF_SET_OK;
                break;
            case HFP_SLC_BRSF_SET_OK:
                if (r->hfp_features & HFP_AG_FEAT_CODEC) {
                    if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_SET, "+BAC", "1") == -1)
                        goto ioerror;
                    r->handler = &rfcomm_handler_resp_ok;
                    r->handler_resp_ok_new_state = HFP_SLC_BAC_SET_OK;
                    break;
                }
                /* fall-through */
            case HFP_SLC_BAC_SET_OK:
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_TEST, "+CIND", NULL) == -1)
                    goto ioerror;
                r->handler = &rfcomm_handler_cind_resp_test;
                break;
            case HFP_SLC_CIND_TEST:
                r->handler = &rfcomm_handler_resp_ok;
                r->handler_resp_ok_new_state = HFP_SLC_CIND_TEST_OK;
                break;
            case HFP_SLC_CIND_TEST_OK:
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_GET, "+CIND", NULL) == -1)
                    goto ioerror;
                r->handler = &rfcomm_handler_cind_resp_get;
                break;
            case HFP_SLC_CIND_GET:
                r->handler = &rfcomm_handler_resp_ok;
                r->handler_resp_ok_new_state = HFP_SLC_CIND_GET_OK;
                break;
            case HFP_SLC_CIND_GET_OK:
                /* Activate indicator events reporting. The +CMER specification is
					 * as follows: AT+CMER=[<mode>[,<keyp>[,<disp>[,<ind>[,<bfr>]]]]] */
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_SET, "+CMER", "3,0,0,1,0") == -1)
                    goto ioerror;
                r->handler = &rfcomm_handler_resp_ok;
                r->handler_resp_ok_new_state = HFP_SLC_CMER_SET_OK;
                break;
            case HFP_SLC_CMER_SET_OK:
                rfcomm_set_hfp_state(r, HFP_SLC_CONNECTED);
                /* fall-through */
            case HFP_SLC_CONNECTED:
                BTMG_INFO("HFP_SLC_CONNECTED");
            }
        } else if (r->setup != HFP_SETUP_COMPLETE) {
            /* Notify audio gateway about our initial setup. This setup
			 * is dedicated for HSP and HFP, because both profiles have
			 * volume gain control and Apple accessory extension. */
            switch (r->setup) {
            case HFP_SETUP_GAIN_MIC:
                r->gain_mic = 8;
                BTMG_INFO("Updating microphone gain: %d", r->gain_mic);
                sprintf(tmp, "%d", r->gain_mic);
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_SET, "+VGM", tmp) == -1) {
                    goto ioerror;
                }
                r->handler = &rfcomm_handler_resp_ok;
                r->setup++;
                break;
            case HFP_SETUP_GAIN_SPK:
                r->gain_spk = 10;
                sprintf(tmp, "%d", r->gain_spk);
                if (rfcomm_write_at(pfds[1].fd, AT_TYPE_CMD_SET, "+VGS", tmp) == -1) {
                    goto ioerror;
                }
                r->handler = &rfcomm_handler_resp_ok;
                r->setup++;
                break;
            case HFP_SETUP_ACCESSORY_XAPL:
                sprintf(tmp, "%04X-%04X-%s,%u", xapl_vendor_id, xapl_product_id,
                        xapl_software_version, xapl_features);
                if (rfcomm_write_at(r->fd, AT_TYPE_CMD_SET, "+XAPL", tmp) == -1)
                    goto ioerror;
                r->handler = &rfcomm_handler_xapl_resp;
                r->setup++;
                break;
            case HFP_SETUP_ACCESSORY_BATT:
                //TODO
                BTMG_DEBUG("HFP_SETUP_ACCESSORY_BATT");
                r->setup++;
                break;
            case HFP_SETUP_COMPLETE:
                break;
            }
        } else {
            /* setup is complete, block infinitely */
            BTMG_DEBUG("Initial connection setup completed");
            timeout = -1;
        }

    process:
        if (r->handler != NULL) {
            timeout = BA_RFCOMM_TIMEOUT_ACK;
            r->retries++;
        }

        /* skip poll() since we've got unprocessed data */
        if (reader.next != NULL)
            goto read;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

        r->idle = false;
        pfds[2].fd = r->handler_fd;
        switch (poll(pfds, ARRAYSIZE(pfds), timeout)) {
        case 0:
            BTMG_DUMP("RFCOMM poll timeout");
            r->idle = true;
            continue;
        case -1:
            if (errno == EINTR)
                continue;
            BTMG_ERROR("RFCOMM poll error: %s", strerror(errno));
            goto fail;
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if (pfds[1].revents & POLLIN) {
            /* read data from the RFCOMM */
        read:
            if (rfcomm_read_at(pfds[1].fd, &reader) == -1)
                switch (errno) {
                case EBADMSG:
                    BTMG_ERROR("Invalid AT message: %s", reader.next);
                    reader.next = NULL;
                    continue;
                default:
                    goto ioerror;
                }

            /* use predefined callback, otherwise get generic one */
            bool predefined_callback = false;
            if (r->handler != NULL && r->handler->type == reader.at.type &&
                strcmp(r->handler->command, reader.at.command) == 0) {
                callback = r->handler->callback;
                predefined_callback = true;
                r->handler = NULL;
            } else
                callback = rfcomm_get_callback(&reader.at);

            if (pfds[2].fd != -1 && !predefined_callback) {
                at_build(tmp, sizeof(tmp), reader.at.type, reader.at.command, reader.at.value);
                if (write(pfds[2].fd, tmp, strlen(tmp)) == -1)
                    BTMG_ERROR("Couldn't forward AT: %s", strerror(errno));
            }

            if (callback != NULL) {
                if (callback(r, &reader.at) == -1)
                    goto ioerror;
            } else if (pfds[2].fd == -1) {
                BTMG_ERROR("Unsupported AT message: %s: command:%s, value:%s",
                           at_type2str(reader.at.type), reader.at.command, reader.at.value);
                if (reader.at.type != AT_TYPE_RESP)
                    if (rfcomm_write_at(pfds[1].fd, AT_TYPE_RESP, NULL, "ERROR") == -1)
                        goto ioerror;
            }

        } else if (pfds[1].revents & (POLLERR | POLLHUP)) {
            errno = ECONNRESET;
            goto ioerror;
        }

        if (pfds[2].revents & POLLIN) {
            /* read data from the external handler */
            ssize_t ret;
            while ((ret = read(pfds[2].fd, tmp, sizeof(tmp) - 1)) == -1 && errno == EINTR)
                continue;

            if (ret <= 0)
                goto ioerror_exthandler;

            tmp[ret] = '\0';
            if (rfcomm_write_at(pfds[1].fd, AT_TYPE_RAW, tmp, NULL) == -1)
                goto ioerror;

        } else if (pfds[2].revents & (POLLERR | POLLHUP)) {
            errno = ECONNRESET;
            goto ioerror_exthandler;
        }

        continue;

    ioerror_exthandler:
        if (errno != 0)
            BTMG_ERROR("AT handler IO error: %s", strerror(errno));
        close(r->handler_fd);
        r->handler_fd = -1;
        continue;

    ioerror:
        switch (errno) {
        case ECONNABORTED:
        case ECONNRESET:
        case ENOTCONN:
        case ETIMEDOUT:
        case EPIPE:
            /* exit the thread upon socket disconnection */
            BTMG_ERROR("RFCOMM disconnected: %s", strerror(errno));
            bt_hfp_hf_pcm_stop();
            goto fail;
        default:
            BTMG_ERROR("RFCOMM IO error: %s", strerror(errno));
        }
    }

fail:
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(1);

    return NULL;
}

struct hfp_rfcomm_t *hfp_rfcomm_new(int fd)
{
    int err;
    struct hfp_rfcomm_t *r;
    const char *name = "hfp-rfcomm";

    if ((r = calloc(1, sizeof(*r))) == NULL)
        return NULL;

    r->fd = fd;
    r->sig_fd[0] = -1;
    r->sig_fd[1] = -1;
    r->handler_fd = -1;
    r->thread = pthread_self();
    r->state = HFP_DISCONNECTED;
    r->state_prev = HFP_DISCONNECTED;
    r->codec = HFP_CODEC_UNDEFINED;
    r->link_lost_quirk = true;

    if (pipe(r->sig_fd) == -1)
        goto fail;

    if ((err = pthread_create(&r->thread, NULL, PTHREAD_ROUTINE(rfcomm_thread), r)) != 0) {
        BTMG_ERROR("Couldn't create rfcomm thread: %s", strerror(err));
        goto fail;
    }
#ifdef _GNU_SOURCE
    pthread_setname_np(r->thread, name);
#endif
    BTMG_INFO("Created new hfp rfcomm thread: %s", name);

    return r;

fail:

    err = errno;
    ba_rfcomm_destroy(r);
    errno = err;

    return NULL;
}
