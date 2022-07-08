target = libbtmg.so

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/bt_manager.h $(CONFIG_PREFIX)/usr/include/bt_manager.h
	@cp ./include/bt_log.h $(CONFIG_PREFIX)/usr/include
	@cp ./include/bt_dev_list.h $(CONFIG_PREFIX)/usr/include

INCLUDES += -I. -I../gdbus/ -I $(STAGING_DIR)/usr/include/glib-2.0 \
	-I $(STAGING_DIR)/usr/include/gio-unix-2.0 -I $(STAGING_DIR)/usr/include/dbus-1.0 \
	-I $(CONFIG_PREFIX)/usr/include -I./include \
	-I $(STAGING_DIR)/usr/include \
	-I $(STAGING_DIR)/usr/lib/dbus-1.0/include

SRC_ROOT=$(PKG_BUILD_DIR)/src

SRC += $(wildcard $(SRC_ROOT)/*.c)
SRC += $(wildcard $(SRC_ROOT)/a2dp/*.c)
SRC += $(wildcard $(SRC_ROOT)/avrcp/*.c)
SRC += $(wildcard $(SRC_ROOT)/gatt/*.c)
SRC += $(wildcard $(SRC_ROOT)/spp/*.c)
SRC += $(wildcard $(SRC_ROOT)/hfp/*.c)
SRC += $(wildcard $(SRC_ROOT)/log/*.c)
SRC += $(wildcard $(SRC_ROOT)/device/*.c)
SRC += $(wildcard $(SRC_ROOT)/platform/*.c)
SRC += $(wildcard $(SRC_ROOT)/common/*.c)
SRC += $(wildcard $(SRC_ROOT)/shared/*.c)

OBJ=$(patsubst %.c,%.o,$(SRC))

####################################################################
all: install $(target)
$(target):$(SRC)
	 $(CC) $(BT_EXT_CFLAG) -fPIC -g -shared -DOS_NET_LINUX_OS $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@
	 cp $@ $(CONFIG_PREFIX)/usr/lib

.PHONY:clean
clean:
	rm -rf $(target)
