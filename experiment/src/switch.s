	.equ	USR_MODE, 					0x10
	.equ	SYS_MODE, 					0xDF
	.equ	SVC_MODE, 					0xD3
	.equ	IRQ_MASK,					0x80
	.equ	IRQ_MODE,					0xD2

	.equ	HWI_MASK,					0x80000000
	.equ	ENTER_FROM_HWI,				0xAA

	.equ	INIT_KEREL,					0x0
	.equ	CREATE,						0x2001
	.equ	PASS,						0x2
	.equ	MY_TID,						0x3
	.equ	EXIT,						0x4
	.equ	MY_PARENT_TID,				0x5
	.equ	SEND,						0x5006
	.equ	RECEIVE,					0x3007
	.equ	REPLY,						0x3008
	.equ	AWAIT_EVENT,				0x1009

	.global	asm_print_sp
	.global asm_kernel_swiEntry
	.global asm_kernel_hwiEntry
	.global asm_init_kernel
	.global asm_kernel_create
	.global asm_kernel_activate
	.global asm_kernel_my_tid
	.global asm_kernel_my_parent_tid
	.global	asm_kernel_pass
	.global asm_kernel_exit
    .global asm_kernel_send
    .global asm_kernel_receive
    .global asm_kernel_reply
	.global asm_kernel_await_event
    .global asm_get_spsr
    .global asm_get_sp
    .global asm_get_fp

asm_print_sp:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
	mov		r0, sp
	bl		debug_asm(PLT)
	mov		r3, sp
	mov		r0, r3
	ldmfd	sp, {fp, sp, pc}

asm_get_spsr:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
    mrs     ip, spsr
    mov     r0, ip
	ldmfd	sp, {fp, sp, pc}

asm_get_sp:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
    msr     CPSR, #SYS_MODE
    mov     ip, sp
    mov     r0, ip
    msr     CPSR, #SVC_MODE 
	ldmfd	sp, {fp, sp, pc}

asm_get_fp:
	mov		ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub		fp, ip, #4
    msr     CPSR, #SYS_MODE
    mov     ip, fp
    mov     r0, ip
    msr     CPSR, #SVC_MODE 
	ldmfd	sp, {fp, sp, pc}

/*load this function after swi instruction*/
asm_kernel_hwiEntry:
	@ store user registers on user stack
	msr		CPSR, #SYS_MODE
	mov		ip, sp
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	msr		CPSR, #IRQ_MODE
	@ set the most significant bit of lr to 1 and mov to r1
	sub		lr, lr, #4
	mov		r1, lr
	@ step back one instruction to compensate the instruction abandoned
	ORR		r1, r1, #HWI_MASK
	@ save user spsr
	mrs     r3, spsr
	@ enter svc
	mrs		r0, CPSR
	mov		r0, #SVC_MODE
 	msr 	CPSR, r0
	@ flag to indicate entry from hwi
	mov		r2, #ENTER_FROM_HWI
asm_kernel_swiEntry:
	@ check entry from hwi
	CMP		r2,	#ENTER_FROM_HWI
	BEQ		is_entry_from_hwi
	mov		r0, lr
	ldmia   sp, {r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, pc}
is_entry_from_hwi:
	msr		spsr, r3
	mov		r0, r1
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
	@r5 = td->lr
	ldr		r5, [r8, #4]
	@r6 = td->spsr
	ldr		r6, [r8, #8]
	@r7 = td->is_entry_from_hwi
	ldr		r7, [r8, #16]
	mov		r1, #0
	str		r1, [r8, #16]
	
	@enter system mode 
	msr 	CPSR, #SYS_MODE
	mov 	sp, r4

	@get back to svc mode 
	msr 	CPSR, #SVC_MODE
	@spsr = user mode
	msr		SPSR, r6

	@ return value = r0 = td->retval
	ldr		r0, [r8, #12]


	@ check whether ENTER_FROM_HWI, if not, branch to not_entry_from_hwi
	CMP		r7, #ENTER_FROM_HWI
	BNE		entry_from_swi
	BEQ		entry_from_hwi
entry_from_hwi:
	@ install user task state and start the task executing, then branch to reinstall registers
	@@lr = r5	
	mov		lr, r5
	b		asm_hwi_reinstall
entry_from_swi:
	@ install user task state and start the task executing
	@@lr = r5
	mov		lr, r5
	movs 	pc, lr

asm_hwi_reinstall:
	msr 	CPSR, #SYS_MODE
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	msr 	CPSR, #SVC_MODE
	movs 	pc, lr
	mov		pc, lr

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
	mov		ip, r0
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov		r0, ip
	mov 	pc, lr

asm_kernel_receive:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#RECEIVE
	mov		ip, r0
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov		r0, ip
	mov 	pc, lr

asm_kernel_reply:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#REPLY
	mov		ip, r0
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
    mov     r0, ip
	mov 	pc, lr

asm_kernel_await_event:
	mov 	ip, sp 
	stmdb   sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, ip, lr}
	SWI 	#AWAIT_EVENT
	mov		ip, r0
	ldmia   sp,  {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, fp, sp, lr}
	mov		r0, ip
	mov 	pc, lr
