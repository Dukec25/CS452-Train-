;	.type	asm_print_sp, %function
	.global	asm_print_sp
    .global asm_kernel_exit
    .global asm_kernel_swiEntry
    .global asm_init_kernel
    .global asm_kernel_create
	.global asm_kernel_activate
	.global	asm_kernel_pass
    .global asm_kernel_my_tid

@.global activate
@activate:
@	mov ip, sp
@	msr CPSR_c, #0xDF /* System mode */
@	mov sp, ip
@	msr CPSR_c, #0xD3 /* Supervisor mode */
@
@	mov r0, #0x10
@	msr SPSR, r0
@	ldr lr, =first
@	add lr, lr, #0x218000
@	movs pc, lr
@
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
    mov r5, #0x9000000
    str r0, [r5]
    str r1, [r5, #4]
    ldr r8, [lr, #-4]
    BIC r8, r8, #0xff000000
    mov	r0, r8
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}

asm_kernelExit:

asm_kernel_activate:
@   didn't store fp to sp here, might cause problems in future
@	@ r0 = task_descriptor *td
@	@ save kernel state
	mov 	ip, sp 
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
@	@ install active task state
@	@@ r10 = r0
	mov 	r10, r0
@	@@ r4 = td->sp
	ldr		r4, [r10, #0]
	mov		r0, #2
	mov 	r1, r4
	bl		bwputr(PLT)
	@@ r5 = td->lr
	ldr		r5, [r10, #4]
	add		r5, r5, #0x218000
	mov		r0, #2
	mov 	r1, r5
	bl		bwputr(PLT)
@	@@ r6 = td->spsr
@	ldr		r6, [r10, #8]
@	mov		r0, #2
@	mov 	r1, r6
@	bl		bwputr(PLT)
@	@ bl		asm_print_sp(PLT)
@	@@ lr = r5
@	mov		lr, r5
@	mov		r0, #2
@	add		r1, lr, #0
@	bl		bwputr(PLT)
@	@@ spsr = r6
@	msr		spsr, r6
@	mov		r0, #2
@	mrs 		r1, spsr
@	bl		bwputr(PLT)
	@@ sp = r4
	@mov		sp, r4
	@@@
	mov ip, r4
    mov fp, r4
    @enter system mode 
	msr CPSR, #0xDF
	mov sp, ip
    mov fp, ip
    @get back to svc mode 
	msr CPSR, #0xD3

	mov lr, r5
	mov r0, #0x10
	msr SPSR, r0
    
	movs pc, lr
	@@@
	@ start the task executing
	@movs	pc, lr

asm_init_kernel:
    mov 	ip, sp 
    @ store a.t.s
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 	0
    @ load k.s
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    @ save current a.t.p.s.r
    movs 	pc, lr

asm_kernel_create:
    mov ip, sp 
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 	1
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    movs 	pc, lr

asm_kernel_pass:
    mov ip, sp 
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 	2
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    movs 	pc, lr

asm_kernel_my_tid:
    mov ip, sp 
    stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
    SWI 	3
    ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
    movs 	pc, lr


