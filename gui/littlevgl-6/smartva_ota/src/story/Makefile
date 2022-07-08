TARGET = story_ota_bin

SRCDIRS = $(shell find . -maxdepth 5 -type d)

INCLUDES += $(foreach dir,$(SRCDIRS),-I$(dir))  \
						-I$(STAGING_DIR)/usr/include/allwinner/include \
					  -I$(STAGING_DIR)/usr/include/allwinner \
					  -I$(STAGING_DIR)/usr/include/
SRCS = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS = $(SRCS:.c=.o)
LOCAL_LDFLAGS += -Werror -Wall -Wno-unused-but-set-variable -Wno-unused-function -lpthread -ldl\
			-lrt -lc\
            -lm -lfreetype \
            -Os

ifneq ($(ARCH), riscv)
LOCAL_LDFLAGS += -mthumb
endif

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS)$(LOCAL_LDFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LOCAL_LDFLAGS) -g

all:install
install:$(TARGET)

clean:
	-rm -rf $(OBJS) $(TARGET)
