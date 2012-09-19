CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy

CPPFLAGS := -ggdb -nostdlib -nostdinc -ffreestanding -marm -O2 -march=armv6zk \
	    -Wall -Werror
LDFLAGS := -nostdlib -ffreestanding -T ldscript.X

OBJS := entry.o \
	kernel.o \
	uart.o

all:	install

kernel.img:	kernel
	$(OBJCOPY) -O binary $< $@

kernel:	$(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o:	%.c
	$(CC) -c $< -o $@ $(CPPFLAGS)

%.o:	%.S
	$(CC) -c $< -o $@ $(CPPFLAGS)

clean:
	rm -f *.img *.o kernel

install:	kernel.img
	cp kernel.img /var/lib/tftpboot
