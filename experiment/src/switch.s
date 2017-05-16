;	.type	asm_print_sp, %function
	.global	asm_print_sp
        .global asm_kernel_exit
        .global asm_kernel_swiEntry
        .global asm_init_kernel
        .global asm_create

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


/*load this function after swi instruction*/
asm_kernel_swiEntry:
    ldr r8, [lr, #-4]
@    mov r0, #2
    BIC r8, r8, #0xff000000
    mov r7, #4
    mov r9, #0
    add r6, r8, r9
    mul r8, r6, r7 
    add r1, pc, r8
    b k_init_kernel
    b k_create

asm_kernelExit:

asm_init_kernel:
    mov ip, sp 
    @ store a.t.s
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 0;
    @ load k.s
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    @ save current a.t.p.s.r
    movs pc, lr

asm_create:
    mov ip, sp 
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 1;
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    movs pc, lr

