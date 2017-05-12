	.file	"test.c"
	.section	.rodata
	.align	2
.LC0:
	.ascii	"1\000"
	.align	2
.LC1:
	.ascii	"atoi test1 failed\012\000"
	.align	2
.LC2:
	.ascii	"123456\000"
	.align	2
.LC3:
	.ascii	"atoi test2 failed\012\000"
	.align	2
.LC4:
	.ascii	"7890\000"
	.align	2
.LC5:
	.ascii	"toupper test1 failed\012\000"
	.align	2
.LC6:
	.ascii	"toupper test2 failed\012\000"
	.align	2
.LC7:
	.ascii	"toupper test3 failed\012\000"
	.align	2
.LC8:
	.ascii	"toupper test4 failed\012\000"
	.align	2
.LC9:
	.ascii	"toupper test5 failed\012\000"
	.align	2
.LC10:
	.ascii	"toupper test6 failed\012\000"
	.align	2
.LC11:
	.ascii	"abcABC\000"
	.align	2
.LC12:
	.ascii	"strcmp test1 failed\012\000"
	.align	2
.LC13:
	.ascii	"abcABD\000"
	.align	2
.LC14:
	.ascii	"strcmp test2 failed\012\000"
	.align	2
.LC15:
	.ascii	"abcABB\000"
	.align	2
.LC16:
	.ascii	"strcmp test3 failed\012\000"
	.align	2
.LC17:
	.ascii	"abcABCDDDDD\000"
	.align	2
.LC18:
	.ascii	"abcABCCCCCC\000"
	.align	2
.LC19:
	.ascii	"strcmp test4 failed\012\000"
	.align	2
.LC20:
	.ascii	"memcpy test1 failed\012\000"
	.align	2
.LC21:
	.ascii	"memcpy test2 failed\012\000"
	.align	2
.LC22:
	.ascii	"memcpy test3 failed\012\000"
	.align	2
.LC23:
	.ascii	"memcpy test4 failed\012\000"
	.align	2
.LC24:
	.ascii	"test1\000"
	.align	2
.LC25:
	.ascii	"heap insert test 1 failed\000"
	.align	2
.LC26:
	.ascii	"test8\000"
	.align	2
.LC27:
	.ascii	"heap insert test 8 failed\000"
	.align	2
.LC28:
	.ascii	"test2\000"
	.align	2
.LC29:
	.ascii	"heap insert test 2 failed\000"
	.align	2
.LC30:
	.ascii	"test7\000"
	.align	2
.LC31:
	.ascii	"heap insert test 7 failed\000"
	.align	2
.LC32:
	.ascii	"test4\000"
	.align	2
.LC33:
	.ascii	"heap insert test 4 failed\000"
	.align	2
.LC34:
	.ascii	"test9dup\000"
	.align	2
.LC35:
	.ascii	"heap insert test 9dup failed\000"
	.align	2
.LC36:
	.ascii	"test6\000"
	.align	2
.LC37:
	.ascii	"heap insert test 6 failed\000"
	.align	2
.LC38:
	.ascii	"test3dup\000"
	.align	2
.LC39:
	.ascii	"heap insert test 3dup failed\000"
	.align	2
.LC40:
	.ascii	"test5\000"
	.align	2
.LC41:
	.ascii	"heap insert test 5 failed\000"
	.align	2
.LC42:
	.ascii	"test9\000"
	.align	2
.LC43:
	.ascii	"heap insert test 9 failed\000"
	.align	2
.LC44:
	.ascii	"test3\000"
	.align	2
.LC45:
	.ascii	"heap insert test 3 failed\000"
	.align	2
.LC46:
	.ascii	"heap root duplicate test 9 failed\012\000"
	.align	2
.LC47:
	.ascii	"heap delete duplicate test 9(A) failed\012\000"
	.align	2
.LC48:
	.ascii	"heap delete duplicate test 9(B) failed\012\000"
	.align	2
.LC50:
	.ascii	"heap root test %d failed\012\000"
	.align	2
.LC51:
	.ascii	"heap delete test %d(A) failed\012\000"
	.align	2
.LC52:
	.ascii	"heap delete test %d(B) failed\012\000"
	.align	2
.LC53:
	.ascii	"heap root duplicate test 3 failed\012\000"
	.align	2
.LC54:
	.ascii	"heap delete duplicate test 3(A) failed\012\000"
	.align	2
.LC55:
	.ascii	"heap delete duplicate test 3(B) failed\012\000"
	.align	2
.LC49:
	.ascii	"test\000"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 168
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {r4, sl, fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #168
	ldr	sl, .L13
.L12:
	add	sl, pc, sl
	mov	r3, sp
	str	r3, [fp, #-164]
	ldr	r3, .L13+4
	add	r3, sl, r3
	mov	r0, r3
	bl	atoi(PLT)
	mov	r3, r0
	cmp	r3, #1
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+8
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+12
	add	r3, sl, r3
	mov	r0, r3
	bl	atoi(PLT)
	mov	r2, r0
	ldr	r3, .L13+16
	cmp	r2, r3
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+20
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+24
	add	r3, sl, r3
	mov	r0, r3
	bl	atoi(PLT)
	mov	r2, r0
	ldr	r3, .L13+28
	cmp	r2, r3
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+20
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #97
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #65
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+32
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #115
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #83
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+36
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #122
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #90
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+40
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #65
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #65
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+44
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #83
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #83
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+48
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r0, #90
	bl	toupper(PLT)
	mov	r3, r0
	cmp	r3, #90
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+52
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+56
	add	r3, sl, r3
	mov	r0, r3
	ldr	r3, .L13+56
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #6
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+60
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+56
	add	r3, sl, r3
	mov	r0, r3
	ldr	r3, .L13+64
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #6
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #1
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+68
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+72
	add	r3, sl, r3
	mov	r0, r3
	ldr	r3, .L13+56
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #6
	bl	strcmp(PLT)
	mov	r0, #0
	ldr	r3, .L13+76
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, .L13+80
	add	r3, sl, r3
	mov	r0, r3
	ldr	r3, .L13+84
	add	r3, sl, r3
	mov	r1, r3
	mov	r2, #6
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+88
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #300
	str	r3, [fp, #-52]
	ldr	r3, [fp, #-52]
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-188]
	ldr	r2, [fp, #-188]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-188]
	ldr	r3, [fp, #-188]
	str	r3, [fp, #-76]
	mov	r3, #0
	str	r3, [fp, #-48]
	b	.L2
.L3:
	ldr	r0, [fp, #-48]
	ldr	r2, [fp, #-48]
	mov	r3, r2, asr #31
	mov	r1, r3, lsr #25
	add	r3, r2, r1
	and	r3, r3, #127
	rsb	r3, r1, r3
	and	r3, r3, #255
	ldr	r2, [fp, #-76]
	strb	r3, [r2, r0]
	ldr	r3, [fp, #-48]
	add	r3, r3, #1
	str	r3, [fp, #-48]
.L2:
	ldr	r2, [fp, #-48]
	ldr	r3, [fp, #-52]
	cmp	r2, r3
	bcc	.L3
	mov	r3, #256
	str	r3, [fp, #-44]
	ldr	r3, [fp, #-44]
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-184]
	ldr	r2, [fp, #-184]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-184]
	ldr	r3, [fp, #-184]
	str	r3, [fp, #-72]
	ldr	r4, [fp, #-76]
	ldr	r3, [fp, #-72]
	ldr	r2, [fp, #-76]
	mov	r0, r3
	mov	r1, r2
	ldr	r2, [fp, #-44]
	bl	memcpy(PLT)
	mov	r3, r0
	mov	r0, r4
	mov	r1, r3
	ldr	r2, [fp, #-44]
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+92
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #288
	str	r3, [fp, #-40]
	ldr	r3, [fp, #-40]
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-180]
	ldr	r2, [fp, #-180]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-180]
	ldr	r3, [fp, #-180]
	str	r3, [fp, #-68]
	ldr	r4, [fp, #-76]
	ldr	r3, [fp, #-68]
	ldr	r2, [fp, #-76]
	mov	r0, r3
	mov	r1, r2
	ldr	r2, [fp, #-40]
	bl	memcpy(PLT)
	mov	r3, r0
	mov	r0, r4
	mov	r1, r3
	ldr	r2, [fp, #-40]
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+96
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #304
	str	r3, [fp, #-36]
	ldr	r3, [fp, #-36]
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-176]
	ldr	r2, [fp, #-176]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-176]
	ldr	r3, [fp, #-176]
	str	r3, [fp, #-64]
	ldr	r4, [fp, #-76]
	ldr	r3, [fp, #-64]
	ldr	r2, [fp, #-76]
	mov	r0, r3
	mov	r1, r2
	ldr	r2, [fp, #-36]
	bl	memcpy(PLT)
	mov	r3, r0
	mov	r0, r4
	mov	r1, r3
	ldr	r2, [fp, #-36]
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+100
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #312
	str	r3, [fp, #-32]
	ldr	r3, [fp, #-32]
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-172]
	ldr	r2, [fp, #-172]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-172]
	ldr	r3, [fp, #-172]
	str	r3, [fp, #-60]
	ldr	r4, [fp, #-76]
	ldr	r3, [fp, #-60]
	ldr	r2, [fp, #-76]
	mov	r0, r3
	mov	r1, r2
	ldr	r2, [fp, #-32]
	bl	memcpy(PLT)
	mov	r3, r0
	mov	r0, r4
	mov	r1, r3
	ldr	r2, [fp, #-32]
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+104
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #12
	str	r3, [fp, #-28]
	ldr	r3, [fp, #-28]
	mov	r3, r3, asl #3
	add	r3, r3, #3
	add	r3, r3, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	rsb	sp, r3, sp
	str	sp, [fp, #-168]
	ldr	r2, [fp, #-168]
	add	r3, r2, #3
	mov	r3, r3, lsr #2
	mov	r3, r3, asl #2
	str	r3, [fp, #-168]
	ldr	r3, [fp, #-168]
	str	r3, [fp, #-56]
	ldr	r2, [fp, #-56]
	ldr	ip, [fp, #-28]
	sub	r3, fp, #128
	mov	r0, r3
	mov	r1, r2
	mov	r2, ip
	bl	heap_init(PLT)
	sub	ip, fp, #88
	sub	r3, fp, #128
	ldmia	r3, {r0, r1, r2}
	stmia	ip, {r0, r1, r2}
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #1
	ldr	r3, .L13+108
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+112
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #8
	ldr	r3, .L13+116
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+120
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #2
	ldr	r3, .L13+124
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+128
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #7
	ldr	r3, .L13+132
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+136
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #4
	ldr	r3, .L13+140
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+144
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #9
	ldr	r3, .L13+148
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+152
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #6
	ldr	r3, .L13+156
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+160
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #3
	ldr	r3, .L13+164
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+168
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #5
	ldr	r3, .L13+172
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+176
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #9
	ldr	r3, .L13+180
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+184
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	mov	r0, r3
	mov	r1, #3
	ldr	r3, .L13+188
	add	r3, sl, r3
	mov	r2, r3
	bl	heap_insert(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+192
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r2, fp, #88
	sub	r3, fp, #136
	mov	r0, r3
	mov	r1, r2
	bl	heap_root(PLT)
	sub	r3, fp, #136
	ldmia	r3, {r3-r4}
	str	r3, [fp, #-96]
	str	r4, [fp, #-92]
	ldr	r3, [fp, #-92]
	mov	r2, r3
	ldr	r3, .L13+148
	add	r3, sl, r3
	mov	r0, r3
	mov	r1, r2
	mov	r2, #8
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+196
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	sub	r2, fp, #104
	mov	r0, r3
	mov	r1, r2
	bl	heap_delete(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+200
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, [fp, #-100]
	mov	r2, r3
	ldr	r3, .L13+148
	add	r3, sl, r3
	mov	r0, r3
	mov	r1, r2
	mov	r2, #8
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+204
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #9
	str	r3, [fp, #-24]
	b	.L5
.L6:
	ldr	r3, .L13+208
	add	r3, sl, r3
	sub	r2, fp, #109
	mov	ip, #5
	mov	r0, r2
	mov	r1, r3
	mov	r2, ip
	bl	memcpy(PLT)
	ldr	r3, [fp, #-24]
	and	r3, r3, #255
	mov	r0, r3
	bl	c2x(PLT)
	mov	r3, r0
	strb	r3, [fp, #-104]
	sub	r2, fp, #88
	sub	r3, fp, #144
	mov	r0, r3
	mov	r1, r2
	bl	heap_root(PLT)
	sub	r3, fp, #144
	ldmia	r3, {r3-r4}
	str	r3, [fp, #-96]
	str	r4, [fp, #-92]
	ldr	r3, [fp, #-92]
	mov	r2, r3
	sub	r3, fp, #109
	mov	r0, r3
	mov	r1, r2
	mov	r2, #5
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+212
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	sub	r3, fp, #88
	sub	r2, fp, #104
	mov	r0, r3
	mov	r1, r2
	bl	heap_delete(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+216
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	ldr	r3, [fp, #-100]
	mov	r2, r3
	sub	r3, fp, #109
	mov	r0, r3
	mov	r1, r2
	mov	r2, #5
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+220
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	ldr	r3, [fp, #-24]
	sub	r3, r3, #1
	str	r3, [fp, #-24]
.L5:
	ldr	r3, [fp, #-24]
	cmp	r3, #3
	bgt	.L6
	sub	r2, fp, #88
	sub	r3, fp, #152
	mov	r0, r3
	mov	r1, r2
	bl	heap_root(PLT)
	sub	r3, fp, #152
	ldmia	r3, {r3-r4}
	str	r3, [fp, #-96]
	str	r4, [fp, #-92]
	ldr	r3, [fp, #-92]
	mov	r2, r3
	ldr	r3, .L13+164
	add	r3, sl, r3
	mov	r0, r3
	mov	r1, r2
	mov	r2, #8
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+224
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	sub	r3, fp, #88
	sub	r2, fp, #104
	mov	r0, r3
	mov	r1, r2
	bl	heap_delete(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+228
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	ldr	r3, [fp, #-100]
	mov	r2, r3
	ldr	r3, .L13+164
	add	r3, sl, r3
	mov	r0, r3
	mov	r1, r2
	mov	r2, #8
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+232
	add	r3, sl, r3
	mov	r1, r3
	bl	assert(PLT)
	mov	r3, #3
	str	r3, [fp, #-24]
	b	.L8
.L9:
	ldr	r3, .L13+208
	add	r3, sl, r3
	sub	r2, fp, #114
	mov	ip, #5
	mov	r0, r2
	mov	r1, r3
	mov	r2, ip
	bl	memcpy(PLT)
	ldr	r3, [fp, #-24]
	and	r3, r3, #255
	mov	r0, r3
	bl	c2x(PLT)
	mov	r3, r0
	strb	r3, [fp, #-109]
	sub	r2, fp, #88
	sub	r3, fp, #160
	mov	r0, r3
	mov	r1, r2
	bl	heap_root(PLT)
	sub	r3, fp, #160
	ldmia	r3, {r3-r4}
	str	r3, [fp, #-96]
	str	r4, [fp, #-92]
	ldr	r3, [fp, #-92]
	mov	r2, r3
	sub	r3, fp, #114
	mov	r0, r3
	mov	r1, r2
	mov	r2, #5
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+212
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	sub	r3, fp, #88
	sub	r2, fp, #104
	mov	r0, r3
	mov	r1, r2
	bl	heap_delete(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+216
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	ldr	r3, [fp, #-100]
	mov	r2, r3
	sub	r3, fp, #114
	mov	r0, r3
	mov	r1, r2
	mov	r2, #5
	bl	strcmp(PLT)
	mov	r3, r0
	cmp	r3, #0
	movne	r3, #0
	moveq	r3, #1
	mov	r0, r3
	ldr	r3, .L13+220
	add	r3, sl, r3
	mov	r1, r3
	ldr	r2, [fp, #-24]
	bl	assert(PLT)
	ldr	r3, [fp, #-24]
	sub	r3, r3, #1
	str	r3, [fp, #-24]
.L8:
	ldr	r3, [fp, #-24]
	cmp	r3, #0
	bgt	.L9
	mov	r3, #0
	ldr	sp, [fp, #-164]
	mov	r0, r3
	sub	sp, fp, #20
	ldmfd	sp, {r4, sl, fp, sp, pc}
.L14:
	.align	2
.L13:
	.word	_GLOBAL_OFFSET_TABLE_-(.L12+8)
	.word	.LC0(GOTOFF)
	.word	.LC1(GOTOFF)
	.word	.LC2(GOTOFF)
	.word	123456
	.word	.LC3(GOTOFF)
	.word	.LC4(GOTOFF)
	.word	7890
	.word	.LC5(GOTOFF)
	.word	.LC6(GOTOFF)
	.word	.LC7(GOTOFF)
	.word	.LC8(GOTOFF)
	.word	.LC9(GOTOFF)
	.word	.LC10(GOTOFF)
	.word	.LC11(GOTOFF)
	.word	.LC12(GOTOFF)
	.word	.LC13(GOTOFF)
	.word	.LC14(GOTOFF)
	.word	.LC15(GOTOFF)
	.word	.LC16(GOTOFF)
	.word	.LC17(GOTOFF)
	.word	.LC18(GOTOFF)
	.word	.LC19(GOTOFF)
	.word	.LC20(GOTOFF)
	.word	.LC21(GOTOFF)
	.word	.LC22(GOTOFF)
	.word	.LC23(GOTOFF)
	.word	.LC24(GOTOFF)
	.word	.LC25(GOTOFF)
	.word	.LC26(GOTOFF)
	.word	.LC27(GOTOFF)
	.word	.LC28(GOTOFF)
	.word	.LC29(GOTOFF)
	.word	.LC30(GOTOFF)
	.word	.LC31(GOTOFF)
	.word	.LC32(GOTOFF)
	.word	.LC33(GOTOFF)
	.word	.LC34(GOTOFF)
	.word	.LC35(GOTOFF)
	.word	.LC36(GOTOFF)
	.word	.LC37(GOTOFF)
	.word	.LC38(GOTOFF)
	.word	.LC39(GOTOFF)
	.word	.LC40(GOTOFF)
	.word	.LC41(GOTOFF)
	.word	.LC42(GOTOFF)
	.word	.LC43(GOTOFF)
	.word	.LC44(GOTOFF)
	.word	.LC45(GOTOFF)
	.word	.LC46(GOTOFF)
	.word	.LC47(GOTOFF)
	.word	.LC48(GOTOFF)
	.word	.LC49(GOTOFF)
	.word	.LC50(GOTOFF)
	.word	.LC51(GOTOFF)
	.word	.LC52(GOTOFF)
	.word	.LC53(GOTOFF)
	.word	.LC54(GOTOFF)
	.word	.LC55(GOTOFF)
	.size	main, .-main
	.ident	"GCC: (GNU) 4.0.2"
