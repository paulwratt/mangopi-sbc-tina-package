target = libmemleak_check.so

SRCS = lib_memleak_check.c
LOCAL_LDFLAGS := -fexceptions -rdynamic -funwind-tables -ffunction-sections -g -O1 -Wno-deprecated-declarations
install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
#libpeq.so
$(target): $(SRCS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $(LOCAL_LDFLAGS) $^ -o $@
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)
