CROSS_COMPILE ?= arm-linux-gnueabi-
ARCH ?= arm
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

CPPFLAGS := -ggdb -nostdlib -nostdinc -ffreestanding -marm -O2 -march=armv6zk \
	    -Wall -Werror
LDFLAGS := -nostdlib -ffreestanding -Wl,--build-id=none

OBJS := entry.o \
	kernel.o \
	uart.o \
	pinmux.o \
	tzvecs.o \
	gdbstub.o \
	printk.o \
	string.o

all:	install

kernel.img:	loader
	$(OBJCOPY) -O binary $< $@

loader:	loader.o
	$(CC) $^ -o $@ $(LDFLAGS) -T ldscript_loader.X

loader.o:	loader.S rsp.img
	$(CC) -c $< -o $@ $(LDFLAGS)

rsp.img:	rsp
	$(OBJCOPY) -O binary $< $@

rsp:	$(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) -T ldscript.X

%.o:	%.c
	$(CC) -c $< -o $@ $(CPPFLAGS)

%.o:	%.S
	$(CC) -c $< -o $@ $(CPPFLAGS)

clean:
	rm -f *.img *.o rsp loader

install:	kernel.img
	cp kernel.img /var/lib/tftpboot
