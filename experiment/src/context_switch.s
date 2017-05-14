	.file	"main.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"sp = %x\012\000"
	.align	2
.LC1:
	.ascii	"td->id = %d, td->state = %d\012\000"
	.text
	.align	2
	.global	print_td
	.type	print_td, %function
print_td:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	str	r0, [fp, #-24]
	bl	asm_print_sp(PLT)
	mov	r3, r0
	str	r3, [fp, #-20]
	mov	r0, #2
	ldr	r3, .L4+4
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-20]
	bl	bwprintf(PLT)
	ldr	r3, [fp, #-24]
	ldr	r2, [r3, #8]
	ldr	r3, [fp, #-24]
	ldr	ip, [r3, #16]
	mov	r0, #2
	ldr	r3, .L4+8
	add	r3, sl, r3
	mov	r1, r3
	mov	r3, ip
	bl	bwprintf(PLT)
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	.LC0(GOTOFF)
	.word	.LC1(GOTOFF)
	.size	print_td, .-print_td
	.align	2
	.global	intialize
	.type	intialize, %function
intialize:
	@ args = 0, pretend = 0, frame = 544
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #544
	mov	r4, r0
	str	r1, [fp, #-544]
	str	r2, [fp, #-548]
	sub	r2, fp, #528
	sub	r3, fp, #560
	mov	r0, r3
	mov	r1, r2
	mov	r2, #64
	bl	heap_init(PLT)
	sub	ip, fp, #540
	sub	r3, fp, #560
	ldmia	r3, {r0, r1, r2}
	stmia	ip, {r0, r1, r2}
	ldr	r2, [fp, #-544]
	mov	r3, #1
	str	r3, [r2, #8]
	ldr	r2, [fp, #-544]
	mov	r3, #0
	str	r3, [r2, #12]
	ldr	r2, [fp, #-544]
	mov	r3, #2
	str	r3, [r2, #16]
	ldr	r2, [fp, #-544]
	ldr	r3, [fp, #-548]
	ldr	r3, [r3, #0]
	str	r3, [r2, #0]
	ldr	r3, [fp, #-548]
	ldr	r3, [r3, #0]
	add	r2, r3, #409600
	ldr	r3, [fp, #-548]
	str	r2, [r3, #0]
	ldr	r2, [fp, #-544]
	sub	r3, fp, #540
	mov	r0, r3
	mov	r1, #1
	bl	heap_insert(PLT)
	sub	r3, fp, #544
	mov	r0, r3
	bl	print_td(PLT)
	mov	ip, r4
	sub	r3, fp, #540
	ldmia	r3, {r0, r1, r2}
	stmia	ip, {r0, r1, r2}
	mov	r0, r4
	sub	sp, fp, #16
	ldmfd	sp, {r4, fp, sp, pc}
	.size	intialize, .-intialize
	.align	2
	.global	kerent
	.type	kerent, %function
kerent:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	r0, #2
	ldr	r3, .L13+12
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
	.size	kerent, .-kerent
	.section	.rodata
	.align	2
.LC2:
	.ascii	"kerxit.c: Hello.\012\015\000"
	.align	2
.LC3:
	.ascii	"kerxit.c: Activating.\012\015\000"
	.align	2
.LC4:
	.ascii	"kerxit.c: Good-bye.\012\015\000"
	.text
	.align	2
	.global	kerxit
	.type	kerxit, %function
kerxit:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	ldr	sl, .L13
.L12:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	@Hello start
	mov	r0, #2
	ldr	r3, .L13+4
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	@Hello end
	@Activating start
	mov	r0, #2
	ldr	r3, .L13+8
	add	r3, sl, r3
	mov	r1, r3
	bl	bwprintf(PLT)
	@Activating end
	@This is the begin of context switch	
	b	kerent(PLT)
	@This is the end of context switch	
.L14:
	.align	2
.L13:
	.word	_GLOBAL_OFFSET_TABLE_-(.L12+8)
	.word	.LC2(GOTOFF)
	.word	.LC3(GOTOFF)
	.word	.LC4(GOTOFF)
	.size	kerxit, .-kerxit
	.align	2
	.global	handle
	.type	handle, %function
handle:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	handle, .-handle
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 64
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #64
	mov	r3, #268435456
	str	r3, [fp, #-24]
	sub	r2, fp, #44
	sub	ip, fp, #24
	sub	r3, fp, #76
	mov	r0, r3
	mov	r1, r2
	mov	r2, ip
	bl	intialize(PLT)
	sub	ip, fp, #56
	sub	r3, fp, #76
	ldmia	r3, {r0, r1, r2}
	stmia	ip, {r0, r1, r2}
	sub	r3, fp, #56
	sub	r2, fp, #64
	mov	r0, r3
	mov	r1, r2
	bl	heap_delete(PLT)
	ldr	r3, [fp, #-60]
	str	r3, [fp, #-20]
	ldr	r0, [fp, #-20]
	ldr	r1, [fp, #-16]
	bl	kerxit(PLT)
	ldr	r0, [fp, #-20]
	ldr	r1, [fp, #-16]
	bl	handle(PLT)
	mov	r3, #0
	mov	r0, r3
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	main, .-main
	.ident	"GCC: (GNU) 4.0.2"
