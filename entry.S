.arm
.pushsection .text.vectors
.align	12
	b	_vec_reset		@ System reset
	b	_vec_undef		@ Undefined instruction
	b	_vec_swi		@ Software interrupt
	b	_vec_prefetch_abt	@ Prefetch abort
	b	_vec_dabort		@ Data abort
	b	_vec_reserved		@ Reserved vector
	b	_vec_irq		@ IRQ
	b	_vec_fiq		@ FIQ
.popsection

#define CPSR_MODE_SVC	0x13

.macro push_arm_regs
	sub	sp, sp, #64
	stmia	sp, {r0-r12}		/* Push the main registers. */
	ldr	r0, [sp, #68]
	str	r0, [sp, #60]

	ldr	r0, [sp, #72]		/* Switch to old mode, grab sp + lr. */
	msr	cpsr_c, r0
	mov	r1, sp
	and	r0, r0, #0x1f		/*
					 * If we came from SVC then we need to
					 * nobble the SP so that it looks
					 * like it was before the exception.
					 */
	cmp	r0, #CPSR_MODE_SVC
	addeq	r1, r1, #76
	mov	r2, lr
	cps	#CPSR_MODE_SVC
	str	r1, [sp, #52]
	str	r2, [sp, #56]
	mov	r0, sp
.endm

.macro pop_arm_regs
	ldmia	sp, {r0-r12}
	add	sp, #64
.endm

#define VECTOR(_type) \
	.globl _vec_##_type; \
	_vec_##_type:

VECTOR(reset)
	ldr	sp, stack_addr
	bl	start_kernel

stack_addr:
	.word	stack_top

VECTOR(undef)
	sub	lr, lr, #4
	srsdb	sp!, #CPSR_MODE_SVC
	cps	#CPSR_MODE_SVC
	cpsid	if
	push	{lr}
	push_arm_regs
	mov	r1, #0
	bl	do_abort
	pop_arm_regs

	/*
	 * Fixup the link register.  do_abort will panic() if we couldn't fix
	 * up the exception so we want to continue from the next instruction.
	 */
	ldr	lr, [sp, #4]
	add	lr, lr, #4
	str	lr, [sp, #4]

	pop	{lr}
	cpsie	if
	rfeia	sp!

VECTOR(swi)
	cpsid	if
3:
	b	3b

VECTOR(prefetch_abt)
	sub	lr, lr, #4
	srsdb	sp!, #CPSR_MODE_SVC
	cps	#CPSR_MODE_SVC
	cpsid	if
	push_arm_regs
	mov	r1, #1
	bl	do_abort
	pop_arm_regs
	cpsie	if
	rfeia	sp!

VECTOR(dabort)
	sub	lr, lr, #8
	srsdb	sp!, #CPSR_MODE_SVC
	cps	#CPSR_MODE_SVC
	cpsid	if
	push_arm_regs
	mov	r1, #2
	bl	do_abort
	pop_arm_regs
	cpsie	if
	rfeia	sp!

VECTOR(reserved)
	cpsid	if
6:
	b	6b

VECTOR(irq)
	sub	lr, lr, #4
	srsdb	sp!, #CPSR_MODE_SVC
	cps	#CPSR_MODE_SVC
	push_arm_regs
	bl	do_irq
	pop_arm_regs
	rfeia	sp!

VECTOR(fiq)
	sub	lr, lr, #4
	srsdb	sp!, #CPSR_MODE_SVC
	cps	#CPSR_MODE_SVC
	push_arm_regs
	bl	do_irq
	pop_arm_regs
	rfeia	sp!

stack:
	.rept 1024
	.word 0
	.endr
stack_top:
