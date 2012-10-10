_entry:
.globl _entry
.arm
	ldr	r0, img_addr
	ldr	r1, end_img_addr
	mov	r2, #0x01000000
1:
	ldrb	r3, [r0]
	strb	r3, [r2]
	cmp	r0, r1
	beq	done
	add	r0, r0, #1
	add	r2, r2, #1
	b	1b
done:
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 12)
	mcr	p15, 0, r0, c1, c0, 0
	mcr	p15, 0, r0, c7, c10, 4
	mcr	p15, 0, r0, c7, c5, 4
	mcr	p15, 0, r0, c7, c5, 6
	mov	r0, #0
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	mov	r5, #0
	mov	r6, #0
	mov	r7, #0
	mov	r8, #0
	mov	r9, #0
	mov	r10, #0
	mov	r11, #0
	mov	r12, #0
	mov	r13, #0
	mov	r14, #0
	cpsid	if
	mov	pc, #0x01000000

img_addr:
	.word _img
end_img_addr:
	.word _endimg

_img:
	.incbin "rsp.img"
_endimg:
