#include "assembly.h"

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

VECTOR(undef)
	ex_handler	#4, do_undef, #CPSR_MODE_SVC
VECTOR(swi)
	ex_handler	#4, do_swi, #CPSR_MODE_SVC
VECTOR(prefetch_abt)
	ex_handler	#4, do_prefetch, #CPSR_MODE_SVC
VECTOR(dabort)
	ex_handler	#8, do_dabort, #CPSR_MODE_SVC
VECTOR(reserved)
	ex_handler	#8, do_reserved, #CPSR_MODE_SVC
VECTOR(irq)
	ex_handler	#4, do_irq, #CPSR_MODE_SVC
VECTOR(fiq)
	ex_handler	#4, do_fiq, #CPSR_MODE_SVC
VECTOR(reset)
	ldr	sp, stack_addr
	bl	start_kernel
stack_addr:
	.word	stack_top

stack:
	.rept 1024
	.word 0
	.endr
stack_top:
