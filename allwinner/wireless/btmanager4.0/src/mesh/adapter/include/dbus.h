#include <ell/ell.h>

enum dbus_state {
	DBUS_NONE = 0,
	DBUS_INITIALIZED,
	DBUS_STATE_MAX,
};

bool app_dbus_init(void);
struct l_dbus *app_dbus_get_bus(void);
enum dbus_state app_dbus_get_state(void);
bool dbus_append_byte_array(struct l_dbus_message_builder *builder,
						const uint8_t *data, int len);
void dbus_append_dict_entry_basic(struct l_dbus_message_builder *builder,
					const char *key, const char *signature,
					const void *data);
bool dbus_match_interface(struct l_dbus_message_iter *interfaces,
							const char *match);
struct l_dbus_message *dbus_error(struct l_dbus_message *msg, int err,
						const char *description);
int dbus_get_reply_error(struct l_dbus_message *reply);
