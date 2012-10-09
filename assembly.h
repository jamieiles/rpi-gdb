#ifndef __ASSEMBLY_H__
#define __ASSEMBLY_H__

#define CPSR_MODE_SVC	0x13
#define CPSR_MODE_MON	0x16

#define IRQ_DISABLE	0x80
#define FIQ_DISABLE	0x40

.macro push_arm_regs mode
	sub	sp, sp, #68
	stmia	sp, {r0-r12}		/* Push the main registers. */
	ldr	r0, [sp, #72]
	str	r0, [sp, #60]

	ldr	r0, [sp, #76]		/* Switch to old mode, grab sp + lr. */
	mov	r3, r0
	orr	r0, r0, #(IRQ_DISABLE | FIQ_DISABLE)
	msr	cpsr_c, r0
	mov	r1, sp
	mov	r2, lr
	cps	\mode
	str	r1, [sp, #52]
	str	r2, [sp, #56]
	str	r3, [sp, #64]
	mov	r0, sp
.endm

.macro pop_arm_regs	mode
	ldr	r1, [sp, #52]
	ldr	r2, [sp, #56]
	ldr	r0, [sp, #64]		/* Switch to old mode, grab sp + lr. */
	orr	r0, r0, #(IRQ_DISABLE | FIQ_DISABLE)
	msr	cpsr_c, r0
	mov	sp, r1			/* Restore sp. */
	mov	lr, r2			/* Restore lr. */
	cps	\mode
	ldr	r0, [sp, #60]
	str	r0, [sp, #72]		/* Fixup return PC. */
	ldmia	sp, {r0-r12}
	add	sp, #68
.endm

.macro ex_handler lr_fixup, handler, mode
	sub	lr, lr, \lr_fixup
	srsdb	sp!, \mode
	cps	\mode
	cpsid	if
	push	{lr}

	push_arm_regs \mode
	bl	\handler
	pop_arm_regs \mode

	pop	{lr}
	cpsie	if
	rfeia	sp!
.endm

#define VECTOR(_type) \
	.globl _vec_##_type; \
	_vec_##_type:

#endif /* __ASSEMBLY_H__ */
