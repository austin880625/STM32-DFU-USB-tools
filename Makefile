libsrc=$(wildcard lib/*.c)
src=$(wildcard src/*.c)
libobj:=$(patsubst %.c,%.o,$(libsrc))
obj:=$(patsubst %.c,%.o,$(src))

CFLAGS=-Iinc `pkg-config --cflags libusb` -lusb-1.0

all: $(libobj) $(obj)
	gcc $(CFLAGS) $(libobj) src/stm_dfu_get_status.o -o bin/stm_dfu_get_status
	gcc $(CFLAGS) $(libobj) src/stm_dfu_read_memory.o -o bin/stm_dfu_read_memory

src/%.o: src/%.c
	gcc -c $(CFLAGS) -o $@ $<

lib/%.o: lib/%.c
	gcc -c $(CFLAGS) -o $@ $<

clean:
	rm -f bin/*
	rm -f $(libobj) $(obj)
