#ifndef __REGS_H__
#define __REGS_H__

enum {
	R0, R1, R2, R3, R4, R5, R6,
	R7, R8, R9, R10, R11, R12, SP, LR, PC,
	CPSR
};

struct arm_regs {
	unsigned long	r[17];
};

#endif /* __REGS_H__ */
