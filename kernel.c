#include "io.h"
#include "gpio.h"
#include "uart.h"
#include "debug.h"

#define __used	__attribute__((used))
#define NULL	((void *)0)

#define ARRAY_SIZE(__arr) (sizeof((__arr)) / sizeof((__arr)[0]))

static void pinmux_cfg(void)
{
	struct pinmux_cfg cfg[] = {
		PINMUX(14, PUD_UP, FN_0),
		PINMUX(15, PUD_UP, FN_0),
	};
	int err = pinmux_cfg_many(cfg, ARRAY_SIZE(cfg));
	BUG_ON(err, "failed to configure pinmux");
}

static void platform_init(void)
{
	uart_disable();
	pinmux_cfg();
	uart_enable();
	uart_flush_rx();
}

static void echo(void)
{
	int i;

	for (i = 0; i < 10; ++i)
		uart_putc(uart_getc());
}

static void __used init_bss(void)
{
	extern char __bss_start, __bss_end;
	char *p;

	for (p = &__bss_start; p < &__bss_end; ++p)
		*p = 0;
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

static void puts(const char *str)
{
	const char *p = str;

	while (*p)
		uart_putc(*p++);
}

void print_hex(unsigned long r)
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

static void dump_regs(struct arm_regs *regs)
{
	int i;
	const char * const reg_names[] = {
		"r0 ", "r1 ", "r2 ", "r3 ",
		"r4 ", "r5 ", "r6 ", "r7 ",
		"r8 ", "r9 ", "r10", "r11",
		"r12", "sp ", "lr ", "pc "
	};

	for (i = 0; i < 16; ++i) {
		puts(reg_names[i]);
		puts(": ");
		print_hex(regs->r[i]);
		puts("\n");
	}
}

static void panic(void)
{
	BUG("panic!, entering infinite loop...");
}

static const struct bug_entry *find_bug(unsigned long addr)
{
	extern const struct bug_entry __bug_start, __bug_end;
	const struct bug_entry *b;

	for (b = &__bug_start; b < &__bug_end; ++b)
		if (b->addr == addr)
			return b;

	return NULL;
}

static void show_bug(const struct bug_entry *b,
		     struct arm_regs *regs)
{
	puts("\nBUG: ");
	puts(b->filename);
	puts(":");
	print_hex(b->line);
	puts(" ");
	puts(b->msg);
	puts("\n");
	dump_regs(regs);
}

static void handle_bugs(struct arm_regs *regs)
{
	const struct bug_entry *b = find_bug(regs->r[PC]);

	if (!b)
		return;

	show_bug(b, regs);

	for (;;)
		continue;
}

void do_abort(struct arm_regs *regs, enum abort_cause cause)
{
	handle_bugs(regs);
	dump_regs(regs);
	panic();
}

void do_irq(struct arm_regs *regs)
{
}

void start_kernel(void)
{
	platform_init();

	puts("welcome!\n");

	for (;;) {
		echo();
		BUG("test");
	}

	for (;;) {
	}
}
