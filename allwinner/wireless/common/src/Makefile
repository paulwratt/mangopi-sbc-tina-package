target = libwirelesscom.so

INCLUDES += \
	-I $(CONFIG_PREFIX)/usr/include \
	-I $(PKG_BUILD_DIR)/src/api_action -I $(PKG_BUILD_DIR)/src/os \
	-I $(PKG_BUILD_DIR)/src/os/linux -I $(PKG_BUILD_DIR)/src/tracing \

SRC_ROOT=$(PKG_BUILD_DIR)

SRC += $(wildcard $(SRC_ROOT)/api_action/*.c)
SRC += $(wildcard $(SRC_ROOT)/tracing/*.c)
SRC += $(wildcard $(SRC_ROOT)/os/linux/*.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

####################################################################
install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./tracing/log_core.h  $(CONFIG_PREFIX)/usr/include
	@cp ./api_action/api_action.h $(CONFIG_PREFIX)/usr/include
	@cp ./os/*.h $(CONFIG_PREFIX)/usr/include
	@cp ./os/linux/*.h $(CONFIG_PREFIX)/usr/include

all: install $(target)

$(target):$(SRC)
	 $(CC) -fPIC -g -shared -DOS_NET_LINUX_OS $(CFLAGS) $(INCLUDES) $(CPPFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@
	 cp $@ $(CONFIG_PREFIX)/usr/lib

.PHONY:clean
clean:
	rm -rf $(target)
