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
