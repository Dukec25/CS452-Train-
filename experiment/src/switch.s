	.equ	USR_MODE, 					0x10
	.equ	SYS_MODE, 					0xDF
	.equ	SVC_MODE, 					0xD3
	.equ	LOAD_OFFSET,				0x218000
 	.equ	USER_STATE_STORE_OFFSET, 	0x9000000

	.equ	INIT_KEREL,					0x0
	.equ	CREATE,						0x2001
	.equ	PASS,						0x2
	.equ	MY_TID,						0x3
	.equ	EXIT,						0x4
	.equ	MY_PARENT_TID,				0x5
	.equ	SEND,						0x5006
	.equ	RECEIVE,					0x3007
	.equ	REPLY,						0x3008

	.global	asm_print_sp
	.global asm_kernel_swiEntry
	.global asm_init_kernel
	.global asm_kernel_create
	.global asm_kernel_activate
	.global asm_kernel_my_tid
	.global asm_kernel_my_parent_tid
	.global	asm_kernel_pass
	.global asm_kernel_exit
    .global asm_kernel_send

asm_print_sp:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
	mov		r0, sp
	bl		debug_asm(PLT)
	mov		r3, sp
	mov		r0, r3
	ldmfd	sp, {fp, sp, pc}


/*load this function after swi instruction*/
asm_kernel_swiEntry:
	mov		r0, lr
	ldmia   sp, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}

asm_kernel_activate:
	@ didn't store fp to sp here, might cause problems in future
	@@r0 = task_descriptor *td
	@ save kernel state
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	@ install active task state
	@@r8 = r0
	mov 	r8, r0
	@@r4 = td->sp
	ldr		r4, [r8, #0]
	mov		r0, r4
	bl		debug_asm(PLT)
	@r5 = td->lr
	ldr		r5, [r8, #4]
	@@@@
	@r6 = td->spsr
	ldr		r6, [r8, #8]
	mov		r0, r6
	bl		debug_asm(PLT)
	@@@@

	@enter system mode 
	msr 	CPSR, #SYS_MODE
	mov 	sp, r4

	@get back to svc mode 
	msr 	CPSR, #SVC_MODE
	@spsr = user mode
	@@@mov 	r0, #USR_MODE
	@@@msr 	SPSR, r0
	@@@@
	msr		SPSR, r6
	@@@@

	@ return value = r0 = td->retval
	@@@ldr		r0, [r8, #8]
	ldr		r0, [r8, #12]

	mov		lr, r5
	@ install user task state and start the task executing
	movs 	pc, lr

asm_set_usr_state:
	mov 	r5, #0x9000000
	str 	r0, [r5]
	str 	r1, [r5, #4]

asm_init_kernel:
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#INIT_KEREL
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr

asm_kernel_create:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#CREATE

	mov		ip, r0

	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}

	mov		r0, ip
	mov 	pc, lr

asm_kernel_pass:
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	#PASS
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr

asm_kernel_my_tid:
	mov		ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	#MY_TID
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr

asm_kernel_exit:
	mov 	ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	#EXIT
	@no restore as the task end after SWI

asm_kernel_my_parent_tid:
	mov		ip, sp 
	stmdb   sp!, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	mov		r0, ip
	SWI 	#MY_PARENT_TID
	ldmia   sp,  {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr

asm_kernel_send:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#SEND
	;mov		ip, r0
	;ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	;mov		r0, ip
	;mov 	pc, lr

asm_kernel_receive:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#RECEIVE
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr

asm_kernel_reply:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#REPLY
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov 	pc, lr
