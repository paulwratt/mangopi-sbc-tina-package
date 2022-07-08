include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

target			= libwifimg.so

INCLUDES        += -I. -I./include
LIBS            += -lpthread -ldl -lrt

SRCS =wifimanager.c wifi_event.c scan.c \
      wifi.c wpa_supplicant_conf.c wifi_udhcpc.c tool.c wmg_debug.c status_info.c
install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/*.h $(CONFIG_PREFIX)/usr/include

ifeq ($(CONFIG_IPV6), y)
EXT_LDFLAGS=-DCONFIG_IPV6
endif

#libwifimg.so
ifeq ($(CONFIG_ARCH), "aarch64")
    SRCS += libwpa_client_64.a
else
    ifeq ($(CONFIG_SOFT_FLOAT), y)
         SRCS += libwpa_client_softft.a
    else
         ifeq ($(TARGET_ARCH), riscv)
              SRCS += libwpa_client_riscv.a
         else
              SRCS += libwpa_client.a
         endif
    endif
endif

#SRCS += wpa_ctrl.o \
#	os_unix.o \
#	common.o \
#	wpa_debug.o

$(target): $(SRCS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $(EXT_LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
