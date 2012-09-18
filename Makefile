CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

CPPFLAGS := -ggdb -nostdlib -nostdinc -ffreestanding -marm -O2 -march=armv6zk
LDFLAGS := -nostdlib -ffreestanding -T ldscript.X

OBJS := entry.o \
	kernel.o

kernel:	$(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o:	%.c
	$(CC) -c $< -o $@ $(CPPFLAGS)

%.o:	%.S
	$(CC) -c $< -o $@ $(CPPFLAGS)

clean:
	rm -f *.img *.o kernel
