CC = gcc
CFLAGS = -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS = -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

.PHONY: all clean test

all: libnand.so

libnand.so: nand.o memory_tests.o
	gcc -shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup -o $@ $^

memory_tests.o: memory_tests.c memory_tests.h
	gcc -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -c -o $@ $<

nand.o: nand.c nand.h
	gcc -Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2 -c -o $@ $<


clean:
	rm -rf *.o *.so *.e