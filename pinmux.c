/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#include "io.h"
#include "gpio.h"

/* A very rough approximation of delaying b n clock cycles. */
static inline void delay_n_cycles(unsigned long n)
{
	n /= 2;
	asm volatile("1:	subs	%0, %0, #1\n"
		     "		bne	1b" : "+r"(n) : "r"(n) : "memory");
}

#define GPIO_BASE		(PERIPH_BASE + 0x00200000)
#define GPFSEL_N_REG_OFFS(n)	((n) * 4)

#define GPFSEL_F_SHIFT(f)	((f) * 3)
#define GPFSEL_F_MASK(f)	(0x7 << GPFSEL_F_SHIFT((f)))
#define GPFSEL_F_VAL(f, fn)	(fn << GPFSEL_F_SHIFT((f)))
#define GPPUD_REG_OFFS		0x94
#define GPPUDCLK_N_REG_OFFS(n)	((n) * 4 + 0x98)

int pinmux_cfg_one(const struct pinmux_cfg *cfg);
int pinmux_cfg_many(const struct pinmux_cfg *cfg,
		    unsigned int count);

static void pinmux_cfg_fsel(const struct pinmux_cfg *cfg)
{
	unsigned int fsel_bank = cfg->pin / 10;
	unsigned int fsel_num = cfg->pin % 10;
	unsigned long v;

	v = readl(GPIO_BASE + GPFSEL_N_REG_OFFS(fsel_bank));
	v &= ~GPFSEL_F_MASK(fsel_num);
	v |= GPFSEL_F_VAL(fsel_num, cfg->function);
	writel(GPIO_BASE + GPFSEL_N_REG_OFFS(fsel_bank), v);
}

static void pinmux_cfg_pud(const struct pinmux_cfg *cfg)
{
	unsigned int bit = cfg->pin % 32;
	unsigned int bank = cfg->pin % 32;

	writel(GPIO_BASE + GPPUD_REG_OFFS, cfg->function);
	delay_n_cycles(150);
	writel(GPIO_BASE + GPPUDCLK_N_REG_OFFS(bank), bit);
	delay_n_cycles(150);
	writel(GPIO_BASE + GPPUDCLK_N_REG_OFFS(bank), 0);
}

int pinmux_cfg_one(const struct pinmux_cfg *cfg)
{
	pinmux_cfg_fsel(cfg);
	pinmux_cfg_pud(cfg);

	return 0;
}

int pinmux_cfg_many(const struct pinmux_cfg *cfg,
		    unsigned int count)
{
	unsigned int m;
	int err = 0;

	for (m = 0; m < count; ++m) {
		err = pinmux_cfg_one(&cfg[m]);
		if (err)
			break;
	}

	return err;
}
