include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

target = esp32_test

INCLUDES        += -I. -I./include
#LIBS            += -lpthread -ldl -lrt

SRCS =protobuf-c.c platform_wrapper.c esp_hosted_config.pb-c.c \
      transport_pserial.c commands.c test.c

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/*.h $(CONFIG_PREFIX)/usr/include

$(target): $(SRCS)
	$(CC)  $(INCLUDES) $(LDFLAGS) $^ -o $@

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
