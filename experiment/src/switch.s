	.global	asm_print_sp
	.type	asm_print_sp, %function
asm_print_sp:
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	mov	r0, #2
	mov	r1, sp
	bl	bwputr(PLT)
	mov	r3, sp
	mov	r0, r3
	ldmfd	sp, {fp, sp, pc}
	.size	add, .-add
	.ident	"GCC: (GNU) 4.0.2"
