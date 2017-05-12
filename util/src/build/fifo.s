	.file	"fifo.c"
	.text
	.align	2
	.global	fifo_init
	.type	fifo_init, %function
fifo_init:
	@ args = 0, pretend = 0, frame = 1008
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #1008
	mov	r4, r0
	mov	r3, #0
	str	r3, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-20]
	mov	r3, r4
	sub	r2, fp, #1024
	mov	ip, #1008
	mov	r0, r3
	mov	r1, r2
	mov	r2, ip
	bl	memcpy(PLT)
	mov	r0, r4
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
	.size	fifo_init, .-fifo_init
	.align	2
	.global	is_fifo_empty
	.type	is_fifo_empty, %function
is_fifo_empty:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #0]
	ldr	r2, [r3, #1000]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1004]
	cmp	r2, r3
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldmfd	sp, {r3, fp, sp, pc}
	.size	is_fifo_empty, .-is_fifo_empty
	.align	2
	.global	is_fifo_full
	.type	is_fifo_full, %function
is_fifo_full:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	str	r0, [fp, #-20]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1000]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r2, [r3, #1004]
	ldr	r3, [fp, #-16]
	cmp	r2, r3
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	is_fifo_full, .-is_fifo_full
	.align	2
	.global	fifo_put
	.type	fifo_put, %function
fifo_put:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #16
	str	r0, [fp, #-20]
	mov	r3, r1
	strb	r3, [fp, #-24]
	ldr	r0, [fp, #-20]
	bl	is_fifo_full(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L8
	mvn	r3, #0
	str	r3, [fp, #-28]
	b	.L10
.L8:
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1000]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-16]
	ldr	r3, .L14
	cmp	r2, r3
	ble	.L11
	mov	r3, #0
	str	r3, [fp, #-16]
.L11:
	ldr	r3, [fp, #-20]
	ldr	r1, [r3, #0]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r2, [r3, #1000]
	ldrb	r3, [fp, #-24]
	strb	r3, [r1, r2]
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	str	r3, [r2, #1000]
	mov	r3, #0
	str	r3, [fp, #-28]
.L10:
	ldr	r3, [fp, #-28]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.L15:
	.align	2
.L14:
	.word	999
	.size	fifo_put, .-fifo_put
	.align	2
	.global	fifo_get
	.type	fifo_get, %function
fifo_get:
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #16
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	ldr	r0, [fp, #-20]
	bl	is_fifo_empty(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L17
	mvn	r3, #0
	str	r3, [fp, #-28]
	b	.L19
.L17:
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1004]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-16]
	ldr	r3, .L23
	cmp	r2, r3
	ble	.L20
	mov	r3, #0
	str	r3, [fp, #-16]
.L20:
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1004]
	ldrb	r3, [r2, r3]	@ zero_extendqisi2
	ldr	r2, [fp, #-24]
	strb	r3, [r2, #0]
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	str	r3, [r2, #1004]
	mov	r3, #0
	str	r3, [fp, #-28]
.L19:
	ldr	r3, [fp, #-28]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.L24:
	.align	2
.L23:
	.word	999
	.size	fifo_get, .-fifo_get
	.align	2
	.global	fifo_peek
	.type	fifo_peek, %function
fifo_peek:
	@ args = 0, pretend = 0, frame = 12
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #12
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	ldr	r0, [fp, #-16]
	bl	is_fifo_empty(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L26
	mvn	r3, #0
	str	r3, [fp, #-24]
	b	.L28
.L26:
	ldr	r3, [fp, #-16]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #1004]
	ldrb	r3, [r2, r3]	@ zero_extendqisi2
	ldr	r2, [fp, #-20]
	strb	r3, [r2, #0]
	mov	r3, #0
	str	r3, [fp, #-24]
.L28:
	ldr	r3, [fp, #-24]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	fifo_peek, .-fifo_peek
	.ident	"GCC: (GNU) 4.0.2"
