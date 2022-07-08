TARGET = story_bin

SRCDIRS = $(shell find . -maxdepth 5 -type d)

INCLUDES += $(foreach dir,$(SRCDIRS),-I$(dir))  \
						-I$(STAGING_DIR)/usr/include/allwinner/include \
					  -I$(STAGING_DIR)/usr/include/allwinner \
					  -I$(STAGING_DIR)/usr/include/
SRCS = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS = $(SRCS:.c=.o)
LOCAL_LDFLAGS += -Werror -Wall -Wno-unused-but-set-variable -Wno-unused-function -ltrecorder -lpthread -ldl\
						 -ltplayer -lxplayer -lvdecoder -ladecoder -lsubdecoder -lcdc_base -lVE -lMemAdapter \
             -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
             -luapi -lrt -lc  -lasound -lspeexdsp \
             -lawrecorder -lvencoder -laencoder -ljpeg -lpng \
             -lcdx_muxer -ljpegdecode  -ltmetadataretriever -lcdx_common -lwifimg -lm -lfreetype -lcurl\
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
