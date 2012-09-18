#define UART_BASE	0x16000000
#define FLAG_REG	0x18
#define BUF_FULL	(1 << 5)

static void putc(int c)
{
	*(volatile unsigned long *)UART_BASE = c;
}

static void puts(const char *str)
{
	const char *p = str;

	while (*p)
		putc(*p++);
}

void print_reg(unsigned long r)
{
	char buf[9] = {};
	char digits[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};
	int i;

	for (i = 0; i < 8; i++) {
		unsigned long t = (r & (0xf << (i * 4))) >> (i * 4);
		buf[7 - i] = digits[t];
	}

	puts(buf);
}

static void init_bss(void)
{
	extern char __bss_start, __bss_end;
	char *p;

	for (p = &__bss_start; p < &__bss_end; ++p)
		*p = 0;
}

void start_kernel(void)
{
	static const char *str = "hello, world!";

	puts(str);

	print_reg(0x1234);

	asm volatile(".word 0xffffffff" ::: "memory");

	puts("excepted once!\n");

	asm volatile(".word 0xffffffff" ::: "memory");

	for (;;) {
	}
}

enum {
	R0, R1, R2, R3, R4, R5, R6,
	R7, R8, R9, R10, R11, R12, SP, LR, PC
};

struct arm_regs {
	unsigned long	r[16];
};

enum abort_cause {
	ABORT_UNDEF,
	ABORT_PREFETCH,
	ABORT_DATA,
};

int do_abort(struct arm_regs *regs, enum abort_cause cause)
{
	int i;

	puts("undef!\n");

	for (i = 0; i < 16; ++i) {
		print_reg(i);
		puts(": ");
		print_reg(regs->r[i]);
		puts("\n");
	}
}

void do_irq(struct arm_regs *regs)
{
}
