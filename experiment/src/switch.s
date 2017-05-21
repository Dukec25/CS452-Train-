	.equ	USR_MODE, 					0x10
	.equ	SYS_MODE, 					0xDF
	.equ	SVC_MODE, 					0xD3
	.equ	LOAD_OFFSET,				0x218000
 	.equ	USER_STATE_STORE_OFFSET, 	0x9000000

	.global	asm_print_sp
	.global asm_kernel_exit
	.global asm_kernel_swiEntry
	.global asm_init_kernel
	.global asm_kernel_create
	.global asm_kernel_activate
	.global	asm_kernel_pass
	.global asm_kernel_my_tid

asm_print_sp:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
	mov		r0, #2
	mov		r1, sp
	bl		bwputr(PLT)
	mov		r3, sp
	mov		r0, r3
	ldmfd	sp, {fp, sp, pc}


/*load this function after swi instruction*/
asm_kernel_swiEntry:
	@ get syscall type
	ldr 	r2, [lr, #-4]
	BIC 	r2, r2, #0xff000000
	@ set return value
	mov		r0, r2
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}

asm_kernelExit:
	
asm_kernel_activate:
	@ didn't store fp to sp here, might cause problems in future
	@@r0 = task_descriptor *td
	@ save kernel state
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	@ install active task state
	@@ r10 = r0
	mov 	r10, r0
	@@ r4 = td->sp
	ldr		r4, [r10, #0]
	mov		r0, #2
	mov 	r1, r4
	bl		bwputr(PLT)
	@@ r5 = td->lr
	ldr		r5, [r10, #4]
@	add		r5, r5, #LOAD_OFFSET
	mov		r0, #2
	mov 	r1, r5
	bl		bwputr(PLT)
	@enter system mode 
	msr 	CPSR, #SYS_MODE
	mov 	sp, r4
	@get back to svc mode 
	msr 	CPSR, #SVC_MODE
	@spsr = user mode
	mov 	r0, #USR_MODE
	msr 	SPSR, r0
	mov 	lr, r5
	@ return value = r0 = td->retval
	ldr		r0, [r10, #8]
	@ start the task executing
	movs 	pc, lr

asm_set_usr_state:
	mov 	r5, #0x9000000
	str 	r0, [r5]
	str 	r1, [r5, #4]

asm_init_kernel:
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	0
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	movs 	pc, lr

asm_kernel_create:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	1
	mov		ip, r0
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov		r0, ip
	movs 	pc, lr

asm_kernel_pass:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	2
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	movs 	pc, lr

asm_kernel_my_tid:
	mov		ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	3
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	movs 	pc, lr
