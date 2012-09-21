#include "debug.h"
#include "io.h"
#include "regs.h"
#include "printk.h"
#include "uart.h"

#define IRQC_BASE			0x2000B000
#define INT_PENDING_REG_OFFS		0x200
#define INT_PENDING1_REG_OFFS		0x204
#define INT_PENDING2_REG_OFFS		0x208
#define INT_FIQ_CONTROL_REG_OFFS	0x20c
#define		FIQ_ENABLE		(1 << 7)
#define INT_ENABLE1_REG_OFFS		0x210
#define INT_ENABLE2_REG_OFFS		0x214
#define INT_BASIC_EN_REG_OFFS		0x218
#define INT_DISABLE1_REG_OFFS		0x21c
#define INT_DISABLE2_REG_OFFS		0x220
#define INT_DISABLE_BASIC_REG_OFFS	0x224

void mon_panic(void)
{
	BUG("monitor paniced!");
}

enum abort_t do_monitor(struct arm_regs *regs)
{
	puts("\nin monitor mode: ");
	print_hex(regs->r[PC]);
	puts("\n");

	return ABORT_NEXT_INSN;
}

enum abort_t do_monitor_fiq(struct arm_regs *regs)
{
	uart_putc(uart_getc());

	return ABORT_RESTART;
}

#define UART_INT		57
#define UART_CONTROLLER_BIT	19

static void fiq_init(void)
{
	writel(IRQC_BASE + INT_DISABLE1_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_DISABLE2_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_DISABLE_BASIC_REG_OFFS, ~0LU);
	writel(IRQC_BASE + INT_FIQ_CONTROL_REG_OFFS,
	       FIQ_ENABLE | 57);
}

extern void init_monitor(void *stack_top);

#define MON_STACK_WORDS	1024
unsigned long monitor_stack[MON_STACK_WORDS];

void gdbstub_init(void)
{
	init_monitor(monitor_stack + MON_STACK_WORDS - 4);
	fiq_init();
	uart_enable(UART_INT_ENABLE);
	uart_flush_rx();
}
