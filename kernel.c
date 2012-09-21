#include "gdbstub.h"
#include "io.h"
#include "gpio.h"
#include "uart.h"
#include "debug.h"
#include "regs.h"
#include "printk.h"

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
}

static void __used init_bss(void)
{
	extern char __bss_start, __bss_end;
	char *p;

	for (p = &__bss_start; p < &__bss_end; ++p)
		*p = 0;
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

enum abort_t do_undef(struct arm_regs *regs)
{
	handle_bugs(regs);
	dump_regs(regs);
	panic();

	return ABORT_NEXT_INSN;
}

enum abort_t do_prefetch(struct arm_regs *regs)
{
	BUG("prefetch abort\n");

	return ABORT_NEXT_INSN;
}

enum abort_t do_dabort(struct arm_regs *regs)
{
	BUG("prefetch abort\n");

	return ABORT_NEXT_INSN;
}

enum abort_t do_reserved(struct arm_regs *regs)
{
	BUG("unhandled exception\n");

	return ABORT_NEXT_INSN;
}

enum abort_t do_swi(struct arm_regs *regs)
{
	puts("in monitor mode!\n");

	return ABORT_NEXT_INSN;
}

enum abort_t do_irq(struct arm_regs *regs)
{
	puts("in irq handler\n");
	return ABORT_RESTART;
}

enum abort_t do_fiq(struct arm_regs *regs)
{
	puts("in fiq handler\n");
	return ABORT_RESTART;
}

void start_kernel(void)
{
	platform_init();
	gdbstub_init();
	asm volatile("cpsie	f");

	for (;;)
		find_bug(0);
}
