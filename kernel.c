#include "io.h"
#include "gpio.h"
#include "uart.h"

/* A very rough approximation of delaying b n clock cycles. */
static inline void delay_n_cycles(unsigned long n)
{
	n /= 2;
	asm volatile("1:	subs	%0, %0, #1\n"
		     "		bne	1b" : "+r"(n) : "r"(n) : "memory");
}

static void pinmux_cfg(void)
{
#define TX_LINE_BIT	(1 << 14)
#define RX_LINE_BIT	(1 << 15)
	unsigned long v;

	v = readl(GPIO_BASE + GPFSEL_N_REG_OFFS(1));
	v &= ~(GPFSEL_F_MASK(4) | GPFSEL_F_MASK(5));
	v |= GPFSEL_F_VAL(4, FN_0) | GPFSEL_F_VAL(5, FN_0);
	writel(GPIO_BASE + GPFSEL_N_REG_OFFS(1), v);

	writel(GPIO_BASE + GPPUD_REG_OFFS, PUD_UP);
	delay_n_cycles(150);
	writel(GPIO_BASE + GPPUDCLK_N_REG_OFFS(0), TX_LINE_BIT | RX_LINE_BIT);
	delay_n_cycles(150);
	writel(GPIO_BASE + GPPUDCLK_N_REG_OFFS(0), 0);
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

static void init_bss(void)
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

void start_kernel(void)
{
	platform_init();

	puts("welcome!\n");

	for (;;) {
		echo();
		asm volatile(".word 0xffffffff" ::: "memory");
	}

	for (;;) {
	}
}
