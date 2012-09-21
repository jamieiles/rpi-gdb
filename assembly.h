#ifndef __ASSEMBLY_H__
#define __ASSEMBLY_H__

#define CPSR_MODE_SVC	0x13
#define CPSR_MODE_MON	0x16

#define IRQ_DISABLE	0x80
#define FIQ_DISABLE	0x40

.macro push_arm_regs mode
	sub	sp, sp, #64
	stmia	sp, {r0-r12}		/* Push the main registers. */
	ldr	r0, [sp, #68]
	str	r0, [sp, #60]

	ldr	r0, [sp, #72]		/* Switch to old mode, grab sp + lr. */
	orr	r0, r0, #(IRQ_DISABLE | FIQ_DISABLE)
	msr	cpsr_c, r0
	mov	r1, sp
	and	r0, r0, #0x1f		/*
					 * If we came from SVC then we need to
					 * nobble the SP so that it looks
					 * like it was before the exception.
					 */
	cmp	r0, \mode
	addeq	r1, r1, #76
	mov	r2, lr
	cps	\mode
	str	r1, [sp, #52]
	str	r2, [sp, #56]
	mov	r0, sp
.endm

.macro pop_arm_regs
	ldmia	sp, {r0-r12}
	add	sp, #64
.endm

.macro ex_handler lr_fixup, handler, mode
	sub	lr, lr, \lr_fixup
	srsdb	sp!, \mode
	cps	\mode
	cpsid	if
	push	{lr}
	push_arm_regs \mode
	bl	\handler
	mov	lr, r0
	pop_arm_regs

	/*
	 * Fixup the return address.  The handler returns 0 if we want to
	 * carry on from the next instruction, otherwise return 1 to retry the
	 * faulting instruction.
	 */
	cmp	lr, #1
	ldreq	lr, [sp, #4]
	addeq	lr, lr, #4
	streq	lr, [sp, #4]

	pop	{lr}
	cpsie	if
	rfeia	sp!
.endm

#define VECTOR(_type) \
	.globl _vec_##_type; \
	_vec_##_type:

#endif /* __ASSEMBLY_H__ */
