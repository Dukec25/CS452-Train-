	.file	"cursor.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"%c[2J\000"
	.text
	.align	2
	.global	bw_cls
	.type	bw_cls, %function
bw_cls:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L4
.L3:
	add	sl, pc, sl
	mov	r0, #2
	ldr	r3, .L4+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	bwprintf(PLT)
	ldmfd	sp, {sl, fp, sp, pc}
.L5:
	.align	2
.L4:
	.word	_GLOBAL_OFFSET_TABLE_-(.L3+8)
	.word	.LC0(GOTOFF)
	.size	bw_cls, .-bw_cls
	.section	.rodata
	.align	2
.LC1:
	.ascii	"%c[%d;%dH\000"
	.text
	.align	2
	.global	bw_pos
	.type	bw_pos, %function
bw_pos:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #12
	ldr	sl, .L9
.L8:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	ldr	r3, [fp, #-24]
	str	r3, [sp, #0]
	mov	r0, #2
	ldr	r3, .L9+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	ldr	r3, [fp, #-20]
	bl	bwprintf(PLT)
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L10:
	.align	2
.L9:
	.word	_GLOBAL_OFFSET_TABLE_-(.L8+8)
	.word	.LC1(GOTOFF)
	.size	bw_pos, .-bw_pos
	.section	.rodata
	.align	2
.LC2:
	.ascii	"%c7\000"
	.text
	.align	2
	.global	bw_save
	.type	bw_save, %function
bw_save:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L14
.L13:
	add	sl, pc, sl
	mov	r0, #2
	ldr	r3, .L14+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	bwprintf(PLT)
	ldmfd	sp, {sl, fp, sp, pc}
.L15:
	.align	2
.L14:
	.word	_GLOBAL_OFFSET_TABLE_-(.L13+8)
	.word	.LC2(GOTOFF)
	.size	bw_save, .-bw_save
	.section	.rodata
	.align	2
.LC3:
	.ascii	"%c8\000"
	.text
	.align	2
	.global	bw_restore
	.type	bw_restore, %function
bw_restore:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	ldr	sl, .L19
.L18:
	add	sl, pc, sl
	mov	r0, #2
	ldr	r3, .L19+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	bwprintf(PLT)
	ldmfd	sp, {sl, fp, sp, pc}
.L20:
	.align	2
.L19:
	.word	_GLOBAL_OFFSET_TABLE_-(.L18+8)
	.word	.LC3(GOTOFF)
	.size	bw_restore, .-bw_restore
	.align	2
	.global	buffer_cls
	.type	buffer_cls, %function
buffer_cls:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L24
.L23:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	ldr	r0, [fp, #-20]
	ldr	r3, .L24+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	buffer_printf(PLT)
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L25:
	.align	2
.L24:
	.word	_GLOBAL_OFFSET_TABLE_-(.L23+8)
	.word	.LC0(GOTOFF)
	.size	buffer_cls, .-buffer_cls
	.align	2
	.global	buffer_pos
	.type	buffer_pos, %function
buffer_pos:
	@ args = 0, pretend = 0, frame = 12
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #16
	ldr	sl, .L29
.L28:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	str	r2, [fp, #-28]
	ldr	r3, [fp, #-28]
	str	r3, [sp, #0]
	ldr	r0, [fp, #-20]
	ldr	r3, .L29+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	ldr	r3, [fp, #-24]
	bl	buffer_printf(PLT)
	sub	sp, fp, #16
	ldmfd	sp, {sl, fp, sp, pc}
.L30:
	.align	2
.L29:
	.word	_GLOBAL_OFFSET_TABLE_-(.L28+8)
	.word	.LC1(GOTOFF)
	.size	buffer_pos, .-buffer_pos
	.align	2
	.global	buffer_save
	.type	buffer_save, %function
buffer_save:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L34
.L33:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	ldr	r0, [fp, #-20]
	ldr	r3, .L34+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	buffer_printf(PLT)
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L35:
	.align	2
.L34:
	.word	_GLOBAL_OFFSET_TABLE_-(.L33+8)
	.word	.LC2(GOTOFF)
	.size	buffer_save, .-buffer_save
	.align	2
	.global	buffer_restore
	.type	buffer_restore, %function
buffer_restore:
	@ args = 0, pretend = 0, frame = 4
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #4
	ldr	sl, .L39
.L38:
	add	sl, pc, sl
	str	r0, [fp, #-20]
	ldr	r0, [fp, #-20]
	ldr	r3, .L39+4
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #27
	bl	buffer_printf(PLT)
	ldmfd	sp, {r3, sl, fp, sp, pc}
.L40:
	.align	2
.L39:
	.word	_GLOBAL_OFFSET_TABLE_-(.L38+8)
	.word	.LC3(GOTOFF)
	.size	buffer_restore, .-buffer_restore
	.align	2
	.global	buffer_nextline
	.type	buffer_nextline, %function
buffer_nextline:
	@ args = 0, pretend = 0, frame = 12
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #12
	str	r0, [fp, #-20]
	str	r1, [fp, #-24]
	mov	r3, #0
	str	r3, [fp, #-16]
	mov	r3, #32
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-16]
	ldr	r3, [fp, #-24]
	add	r3, r2, r3
	ldr	r0, [fp, #-20]
	mov	r1, r3
	mov	r2, #0
	bl	buffer_pos(PLT)
	ldr	r0, [fp, #-20]
	mov	r1, #62
	bl	buffer_putc(PLT)
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	buffer_nextline, .-buffer_nextline
	.ident	"GCC: (GNU) 4.0.2"
