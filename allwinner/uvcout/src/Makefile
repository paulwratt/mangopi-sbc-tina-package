INCLUDES += -I$(STAGING_DIR)/usr/include/ -I$(STAGING_DIR)/usr/ -I$(STAGING_DIR)/usr/include/allwinner -I$(STAGING_DIR)/usr/include/allwinner/include
LIBS += -L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/rootfs/lib -lm -lpthread -lsunxicamera -lstdc++ -lawrecorder -ldl -lrt

TARGET = uvcout
C_SRCS = sample_uvcout.c confparser.c dictionary.c iniparser.c camerav4l2.c

OBJS = $(C_SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -D__USE_VIN_ISP__ $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) -lm -lisp -lisp_ini -lAWIspApi


all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
