TARGET = appdemo

SRCDIRS = $(shell find . -maxdepth 3 -type d)
SRCS = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
OBJS = $(SRCS:.c=.o)
INCLUDES += $(foreach dir,$(SRCDIRS),-I$(dir))
LIB +=

%.o: %.c
	$(CC) $(INCLUDES) $(LIB) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIB)


all:$(TARGET)

clean:
	-rm -rf $(OBJS) $(TARGET)
