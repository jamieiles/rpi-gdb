/*
 * Raspberry PI Remote Serial Protocol.
 * Copyright 2012 Jamie Iles, jamie@jamieiles.com.
 * Licensed under GPLv2.
 */
#ifndef __IO_H__
#define __IO_H__

#define PERIPH_BASE		0x20000000

static inline void writel(unsigned long addr, unsigned long v)
{
	*(volatile unsigned long *)addr = v;
}

static inline unsigned long readl(unsigned long addr)
{
	return *(volatile unsigned long *)addr;
}

#endif /* __IO_H__ */
