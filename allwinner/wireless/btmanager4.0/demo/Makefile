INCLUDES += -I./include -I$(CONFIG_PREFIX)/usr/include -I$(STAGING_DIR)/usr/include
LIB_PATH := $(CONFIG_PREFIX)/usr/lib
LIBS += -L$(LIB_PATH) -lbluetooth -lglib-2.0 -lsbc -ljson-c -lgio-2.0 -lgobject-2.0 -lasound -lbtmg -ldl -lm -lresolv -ldbus-1 -lwirelesscom
LIBS+=-lshared-mainloop -lbluetooth-internal -lreadline -lncurses

target	= bt_test
C_SRCS = bt_test.c bt_cmd.c bt_gatt_server_demo.c bt_gatt_client_demo.c bt_gatt_test_demo.c
C_OBJS := $(C_SRCS:.c=.o)
LIB_OBJS = $(C_OBJS)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

$(C_OBJS): %.o: %.c
	$(CXX) $(CFLAGS) $(INCLUDES) -g -c -o $@ $^

$(target): $(LIB_OBJS)
	$(CC) -o $@ $^ $(INCLUDES) -g $(LIBS)  $(LDFLAGS)
	@cp $(target)  $(CONFIG_PREFIX)/usr/bin/

all: install $(target)
clean:
	rm -rf $(target) $(LIB_OBJS)
