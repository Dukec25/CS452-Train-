	.file	"fifo.c"
	.text
	.align	2
	.global	fifo_init
	.type	fifo_init, %function
fifo_init:
	@ args = 0, pretend = 0, frame = 4008
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4000
	sub	sp, sp, #8
	mov	r4, r0
	mov	r3, #0
	str	r3, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-20]
	mov	r1, r4
	sub	r3, fp, #4016
	sub	r3, r3, #8
	ldr	r2, .L3
	mov	r0, r1
	mov	r1, r3
	bl	memcpy(PLT)
	mov	r0, r4
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
.L4:
	.align	2
.L3:
	.word	4008
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
	ldr	r2, [r3, #4000]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #4004]
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
	ldr	r3, [r3, #4000]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r2, [r3, #4004]
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
	str	r1, [fp, #-24]
	ldr	r0, [fp, #-20]
	bl	is_fifo_full(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L10
	mvn	r3, #0
	str	r3, [fp, #-28]
	b	.L12
.L10:
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #4000]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-16]
	ldr	r3, .L16
	cmp	r2, r3
	ble	.L13
	mov	r3, #0
	str	r3, [fp, #-16]
.L13:
	ldr	r3, [fp, #-20]
	ldr	r1, [r3, #0]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r2, [r3, #4000]
	ldr	r3, [fp, #-24]
	str	r3, [r1, r2, asl #2]
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	str	r3, [r2, #4000]
	mov	r3, #0
	str	r3, [fp, #-28]
.L12:
	ldr	r3, [fp, #-28]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.L17:
	.align	2
.L16:
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
	beq	.L19
	mvn	r3, #0
	str	r3, [fp, #-28]
	b	.L21
.L19:
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #4004]
	add	r3, r3, #1
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-16]
	ldr	r3, .L25
	cmp	r2, r3
	ble	.L22
	mov	r3, #0
	str	r3, [fp, #-16]
.L22:
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #4004]
	ldr	r2, [r2, r3, asl #2]
	ldr	r3, [fp, #-24]
	str	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	str	r3, [r2, #4004]
	mov	r3, #0
	str	r3, [fp, #-28]
.L21:
	ldr	r3, [fp, #-28]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.L26:
	.align	2
.L25:
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
	beq	.L28
	mvn	r3, #0
	str	r3, [fp, #-24]
	b	.L30
.L28:
	ldr	r3, [fp, #-16]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #0]
	ldr	r3, [r3, #4004]
	ldr	r2, [r2, r3, asl #2]
	ldr	r3, [fp, #-20]
	str	r2, [r3, #0]
	mov	r3, #0
	str	r3, [fp, #-24]
.L30:
	ldr	r3, [fp, #-24]
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	fifo_peek, .-fifo_peek
	.ident	"GCC: (GNU) 4.0.2"
