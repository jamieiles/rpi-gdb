#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_BASE		(PERIPH_BASE + 0x00200000)
#define GPFSEL_N_REG_OFFS(n)	((n) * 4)

#define GPFSEL_F_SHIFT(f)	((f) * 3)
#define GPFSEL_F_MASK(f)	(0x7 << GPFSEL_F_SHIFT((f)))
#define GPFSEL_F_VAL(f, fn)	(fn << GPFSEL_F_SHIFT((f)))

/* The oddly ordered function mappings... */
enum gpio_function {
	FN_IN,
	FN_OUT,
	FN_5,
	FN_4,
	FN_0,
	FN_1,
	FN_2,
	FN_3,
	FN_MASK = 0x7,
};

#define GPPUD_REG_OFFS		0x94

enum pud_mode {
	PUD_OFF,
	PUD_DOWN,
	PUD_UP
};

#define GPPUDCLK_N_REG_OFFS(n)	((n) * 4 + 0x98)

#endif /* __GPIO_H__ */
