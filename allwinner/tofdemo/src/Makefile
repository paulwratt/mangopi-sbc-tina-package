TARGET = tofdemo
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner -I./core/inc -I./platform/inc
LIBS +=  -lpthread -ldl -lrt \

SRCS = vl53l0x_ContinuousRanging_Example.c
SRCS += ${wildcard ./core/src/*.c ./platform/src/*.c}
OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *~
