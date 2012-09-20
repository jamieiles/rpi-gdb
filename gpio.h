#ifndef __GPIO_H__
#define __GPIO_H__

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

enum pud_mode {
	PUD_OFF,
	PUD_DOWN,
	PUD_UP
};

struct pinmux_cfg {
	int			pin;
	enum pud_mode		pud;
	enum gpio_function	function;
};
#define PINMUX(__pin, __pud, __function) \
	{ .pin = (__pin), .pud = (__pud), .function = (__function) }

int pinmux_cfg_one(const struct pinmux_cfg *cfg);
int pinmux_cfg_many(const struct pinmux_cfg *cfg,
		    unsigned int count);

#endif /* __GPIO_H__ */
