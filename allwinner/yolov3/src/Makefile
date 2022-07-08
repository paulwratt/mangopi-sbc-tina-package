LOCAL_PATH := $(shell dirname $(lastword $(MAKEFILE_LIST)))
SDK_DIR = $(LOCAL_PATH)/sdk
VIVANTE_SDK_DIR ?= $(SDK_DIR)
CFLAGS += -I$(VIVANTE_SDK_DIR)/include
LIBS += -L$(VIVANTE_SDK_DIR)/drivers -Wl,-rpath-link,$(VIVANTE_SDK_DIR)/drivers

CC = $(TINA_TOP)/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-muslgnueabi-gcc
export STAGING_DIR = $(LICHEE_PLAT_OUT)/staging_dir/target
INCLUDES += -I$(STAGING_DIR)/usr/include
LIB_PATH := $(STAGING_DIR)/usr/lib
LIBS += -L$(LIB_PATH)

LIBS += -ljpeg
SRCS=${wildcard *.c}
BIN=yolov3
OBJS=$(addsuffix .o, $(basename $(SRCS)))

.c.o:
	$(CC) $(CFLAGS) -c $<

.c:
	$(CC) $(CFLAGS) $< -o $@ -lm

all: $(BIN) 

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) $(EXTRALFLAGS) $(OBJS) $(LIBS) -o $@ -l VIPlite

clean:
	rm -rf *.o
	rm -rf $(BIN)
	rm -rf *~
