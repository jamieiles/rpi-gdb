#include "gdbstub.h"
#include "io.h"
#include "gpio.h"
#include "kernel.h"
#include "uart.h"
#include "debug.h"
#include "regs.h"
#include "printk.h"
#include "types.h"

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
}

static void __used init_bss(void)
{
	extern char __bss_start, __bss_end;
	char *p;

	for (p = &__bss_start; p < &__bss_end; ++p)
		*p = 0;
}

void dump_regs(struct arm_regs *regs)
{
	int i;
	const char * const reg_names[] = {
		"r0 ", "r1 ", "r2 ", "r3 ",
		"r4 ", "r5 ", "r6 ", "r7 ",
		"r8 ", "r9 ", "r10", "r11",
		"r12", "sp ", "lr ", "pc ",
	};

	for (i = 0; i < 16; ++i) {
		puts(reg_names[i]);
		puts(": ");
		print_hex(regs->r[i]);
		puts("\n");
	}
	puts("cpsr: ");
	print_hex(regs->r[CPSR]);
	puts("\n");
}

void panic(void)
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

void do_undef(struct arm_regs *regs)
{
	handle_bugs(regs);
	dump_regs(regs);
	panic();

	regs->r[PC] += 4;
}

void do_prefetch(struct arm_regs *regs)
{
	BUG("prefetch abort\n");

	regs->r[PC] += 4;
}

void do_dabort(struct arm_regs *regs)
{
	BUG("prefetch abort\n");

	regs->r[PC] += 4;
}

void do_reserved(struct arm_regs *regs)
{
	BUG("unhandled exception\n");

	regs->r[PC] += 4;
}

void do_swi(struct arm_regs *regs)
{
	puts("in monitor mode!\n");

	regs->r[PC] += 4;
}

void do_irq(struct arm_regs *regs)
{
	puts("in irq handler\n");
}

void do_fiq(struct arm_regs *regs)
{
	puts("in fiq handler\n");
}

void start_kernel(void)
{
	platform_init();
	gdbstub_init();
	asm volatile("cpsie	f");

	for (;;)
		find_bug(0);
}
