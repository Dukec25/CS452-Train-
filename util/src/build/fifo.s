	.file	"fifo.c"
	.text
	.align	2
	.global	fifo_init
	.type	fifo_init, %function
fifo_init:
	@ args = 0, pretend = 0, frame = 4008
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, lr}
	sub	sp, sp, #4000
	sub	sp, sp, #8
	add	r1, sp, #8
	mov	r3, #0
	sub	r1, r1, #8
	ldr	r2, .L3
	mov	r4, r0
	str	r3, [sp, #4000]
	str	r3, [sp, #4004]
	bl	memcpy(PLT)
	mov	r0, r4
	add	sp, sp, #936
	add	sp, sp, #3072
	ldmfd	sp!, {r4, pc}
.L4:
	.align	2
.L3:
	.word	4008
	.size	fifo_init, .-fifo_init
	.align	2
	.global	is_fifo_empty
	.type	is_fifo_empty, %function
is_fifo_empty:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, [r0, #0]
	@ lr needed for prologue
	ldr	r2, [r3, #4004]
	ldr	r0, [r3, #4000]
	cmp	r0, r2
	movne	r0, #0
	moveq	r0, #1
	bx	lr
	.size	is_fifo_empty, .-is_fifo_empty
	.align	2
	.global	is_fifo_full
	.type	is_fifo_full, %function
is_fifo_full:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, [r0, #0]
	@ lr needed for prologue
	ldr	r0, [r3, #4000]
	ldr	r2, [r3, #4004]
	add	r0, r0, #1
	cmp	r0, r2
	movne	r0, #0
	moveq	r0, #1
	bx	lr
	.size	is_fifo_full, .-is_fifo_full
	.align	2
	.global	fifo_put
	.type	fifo_put, %function
fifo_put:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, lr}
	mov	r5, r1
	mov	r4, r0
	bl	is_fifo_full(PLT)
	cmp	r0, #0
	mvn	ip, #0
	bne	.L12
	ldr	r3, [r4, #0]
	mov	ip, r0
	ldr	r1, [r3, #4000]
	add	r2, r1, #1
	cmp	r2, #1000
	movge	r2, #0
	str	r5, [r3, r1, asl #2]
	str	r2, [r3, #4000]
.L12:
	mov	r0, ip
	ldmfd	sp!, {r4, r5, pc}
	.size	fifo_put, .-fifo_put
	.align	2
	.global	fifo_get
	.type	fifo_get, %function
fifo_get:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, lr}
	mov	r5, r1
	mov	r4, r0
	bl	is_fifo_empty(PLT)
	cmp	r0, #0
	mvn	ip, #0
	bne	.L20
	ldr	r3, [r4, #0]
	mov	ip, r0
	ldr	r2, [r3, #4004]
	ldr	r1, [r3, r2, asl #2]
	add	r2, r2, #1
	cmp	r2, #1000
	movge	r2, #0
	str	r1, [r5, #0]
	str	r2, [r3, #4004]
.L20:
	mov	r0, ip
	ldmfd	sp!, {r4, r5, pc}
	.size	fifo_get, .-fifo_get
	.align	2
	.global	fifo_peek
	.type	fifo_peek, %function
fifo_peek:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	stmfd	sp!, {r4, r5, lr}
	mov	r5, r1
	mov	r4, r0
	bl	is_fifo_empty(PLT)
	cmp	r0, #0
	ldreq	r3, [r4, #0]
	mvn	ip, #0
	ldreq	r2, [r3, #4004]
	moveq	ip, r0
	ldreq	r1, [r3, r2, asl #2]
	mov	r0, ip
	streq	r1, [r5, #0]
	ldmfd	sp!, {r4, r5, pc}
	.size	fifo_peek, .-fifo_peek
	.ident	"GCC: (GNU) 4.0.2"
