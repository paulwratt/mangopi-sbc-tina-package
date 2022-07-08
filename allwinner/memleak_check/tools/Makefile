target = memleak_check

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

$(target) : tools_memleak_check.c
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDES)  $(LDFLAGS)
	@cp -f $(target) $(CONFIG_PREFIX)/usr/bin

all: install $(target)
clean:
	rm -rf $(target)
