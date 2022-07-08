target := dsp_debug

.PHONY: all

all: $(target)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm $(target)
