#include "assembly.h"

.arm
.pushsection .text.tzvectors
.align	12
_monitor_vecs:
	b	mon_panic		@ System reset
	b	mon_panic		@ Undefined instruction
	b	_vec_swi_tz		@ Software interrupt
	b	mon_panic		@ Prefetch abort
	b	mon_panic		@ Data abort
	b	_vec_smc_tz		@ Reserved vector
	b	mon_panic		@ IRQ
	b	_vec_fiq_tz		@ FIQ
.popsection

VECTOR(swi_tz)
	ex_handler	#4, do_monitor, #CPSR_MODE_MON
VECTOR(smc_tz)
	ex_handler	#4, do_monitor, #CPSR_MODE_MON
VECTOR(fiq_tz)
	ex_handler	#4, do_monitor_fiq, #CPSR_MODE_MON

/*
 * Setup monitor mode.  Must be called from supervisor mode.
 *
 * r0: stack for monitor mode.
 * clobbers: r0
 */
.globl init_monitor
init_monitor:
	cps	#CPSR_MODE_MON
	mov	sp, r0
	cps	#CPSR_MODE_SVC
	ldr	r0, monitor_vec_addr
	mcr	p15, 0, r0, c12, c0, 1
	mov	r0, #4
	mcr	p15, 0, r0, c1, c1, 0 @ FIQ's go to the monitor mode.
	bx	lr

monitor_vec_addr:
	.word	_monitor_vecs
