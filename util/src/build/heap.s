	.file	"heap.c"
	.text
	.align	2
	.global	is_heap_full
	.type	is_heap_full, %function
is_heap_full:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #4]
	add	r2, r3, #1
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #8]
	cmp	r2, r3
	movcc	r3, #0
	movcs	r3, #1
	mov	r0, r3
	ldmfd	sp, {r3, fp, sp, pc}
	.size	is_heap_full, .-is_heap_full
	.align	2
	.global	is_heap_empty
	.type	is_heap_empty, %function
is_heap_empty:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	str	r0, [fp, #-16]
	ldr	r3, [fp, #-16]
	ldr	r3, [r3, #4]
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldmfd	sp, {r3, fp, sp, pc}
	.size	is_heap_empty, .-is_heap_empty
	.align	2
	.global	heap_init
	.type	heap_init, %function
heap_init:
	@ args = 0, pretend = 0, frame = 20
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #20
	mov	lr, r0
	str	r1, [fp, #-28]
	str	r2, [fp, #-32]
	ldr	r3, [fp, #-28]
	str	r3, [fp, #-24]
	ldr	r3, [fp, #-32]
	str	r3, [fp, #-16]
	mov	r3, #0
	str	r3, [fp, #-20]
	mov	ip, lr
	sub	r3, fp, #24
	ldmia	r3, {r0, r1, r2}
	stmia	ip, {r0, r1, r2}
	mov	r0, lr
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	heap_init, .-heap_init
	.align	2
	.global	heap_root
	.type	heap_root, %function
heap_root:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	mov	r2, r0
	str	r1, [fp, #-20]
	ldr	r3, [fp, #-20]
	ldr	r3, [r3, #0]
	add	r3, r3, #8
	ldmia	r3, {r3-r4}
	stmia	r2, {r3-r4}
	mov	r0, r2
	ldmfd	sp, {r3, r4, fp, sp, pc}
	.size	heap_root, .-heap_root
	.align	2
	.global	heap_insert
	.type	heap_insert, %function
heap_insert:
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #32
	str	r0, [fp, #-36]
	str	r1, [fp, #-40]
	str	r2, [fp, #-44]
	ldr	r0, [fp, #-36]
	bl	is_heap_full(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L10
	mvn	r1, #0
	str	r1, [fp, #-48]
	b	.L12
.L10:
	ldr	r3, [fp, #-40]
	str	r3, [fp, #-32]
	ldr	r3, [fp, #-44]
	str	r3, [fp, #-28]
	ldr	r3, [fp, #-36]
	ldr	r3, [r3, #4]
	add	r3, r3, #1
	str	r3, [fp, #-24]
	ldr	r2, [fp, #-24]
	mov	r3, r2, lsr #31
	add	r3, r3, r2
	mov	r3, r3, asr #1
	str	r3, [fp, #-20]
	b	.L13
.L14:
	ldr	r3, [fp, #-36]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	mov	r3, r3, asl #3
	add	r1, r2, r3
	ldr	r3, [fp, #-36]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldmia	r3, {r3-r4}
	stmia	r1, {r3-r4}
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-24]
	ldr	r2, [fp, #-20]
	mov	r3, r2, lsr #31
	add	r3, r3, r2
	mov	r3, r3, asr #1
	str	r3, [fp, #-20]
.L13:
	ldr	r3, [fp, #-24]
	cmp	r3, #1
	ble	.L15
	ldr	r3, [fp, #-36]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-40]
	cmp	r2, r3
	blt	.L14
.L15:
	ldr	r3, [fp, #-36]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	sub	r1, fp, #32
	ldmia	r1, {r1-r2}
	stmia	r3, {r1-r2}
	ldr	r3, [fp, #-36]
	ldr	r3, [r3, #4]
	add	r2, r3, #1
	ldr	r3, [fp, #-36]
	str	r2, [r3, #4]
	mov	r2, #0
	str	r2, [fp, #-48]
.L12:
	ldr	r3, [fp, #-48]
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
	.size	heap_insert, .-heap_insert
	.align	2
	.global	heap_delete
	.type	heap_delete, %function
heap_delete:
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #24
	str	r0, [fp, #-32]
	str	r1, [fp, #-36]
	ldr	r0, [fp, #-32]
	bl	is_heap_empty(PLT)
	mov	r3, r0
	cmp	r3, #0
	beq	.L19
	mvn	r3, #0
	str	r3, [fp, #-40]
	b	.L21
.L19:
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #0]
	add	r3, r3, #8
	ldmia	r3, {r3-r4}
	ldr	r2, [fp, #-36]
	stmia	r2, {r3-r4}
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #0]
	add	r1, r3, #8
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #4]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldmia	r3, {r3-r4}
	stmia	r1, {r3-r4}
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #4]
	sub	r2, r3, #1
	ldr	r3, [fp, #-32]
	str	r2, [r3, #4]
	mov	r3, #1
	str	r3, [fp, #-28]
.L22:
	ldr	r3, [fp, #-28]
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-28]
	mov	r3, r3, asl #1
	str	r3, [fp, #-24]
	ldr	r2, [fp, #-24]
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #4]
	cmp	r2, r3
	bhi	.L23
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldr	r1, [r3, #0]
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldr	r3, [r3, #0]
	cmp	r1, r3
	ble	.L23
	ldr	r3, [fp, #-24]
	str	r3, [fp, #-20]
.L23:
	ldr	r3, [fp, #-24]
	add	r3, r3, #1
	mov	r2, r3
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #4]
	cmp	r2, r3
	bhi	.L26
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-24]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	add	r3, r3, #8
	ldr	r1, [r3, #0]
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldr	r3, [r3, #0]
	cmp	r1, r3
	ble	.L26
	ldr	r3, [fp, #-24]
	add	r3, r3, #1
	str	r3, [fp, #-20]
.L26:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #-28]
	cmp	r2, r3
	beq	.L29
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-28]
	mov	r3, r3, asl #3
	add	r1, r2, r3
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-20]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	ldmia	r3, {r3-r4}
	stmia	r1, {r3-r4}
	ldr	r3, [fp, #-20]
	str	r3, [fp, #-28]
	b	.L22
.L29:
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-28]
	mov	r3, r3, asl #3
	add	r1, r2, r3
	ldr	r3, [fp, #-32]
	ldr	r2, [r3, #0]
	ldr	r3, [fp, #-32]
	ldr	r3, [r3, #4]
	mov	r3, r3, asl #3
	add	r3, r2, r3
	add	r3, r3, #8
	ldmia	r3, {r3-r4}
	stmia	r1, {r3-r4}
	mov	r3, #0
	str	r3, [fp, #-40]
.L21:
	ldr	r3, [fp, #-40]
	mov	r0, r3
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
	.size	heap_delete, .-heap_delete
	.ident	"GCC: (GNU) 4.0.2"
