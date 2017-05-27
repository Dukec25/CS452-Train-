#include <debug.h>
#include <user_functions.h>

extern void asm_init_kernel();
extern int asm_kernel_create(int priority, void (*code) ());
extern void asm_kernel_pass();
extern void asm_kernel_exit();
extern int asm_kernel_my_tid();
extern int asm_kernel_my_parent_tid();

void init_kernel()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "init_kernel");
	asm_init_kernel();
}

// upper case due to A1 specification 
int Create(int priority, void (*code) ())
{
	debug(DEBUG_SYSCALL, "In user mode Create, priority = %d, code = 0x%x", priority, code);
	vint tid = asm_kernel_create(priority, code);
	debug(DEBUG_SYSCALL, "In user mode Create, tid = %d", tid);
	return tid;
}

void Pass()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "Pass");
	asm_kernel_pass();
}

void Exit()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "Exit");
	asm_kernel_exit();
}

int MyTid()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "MyTid");
	vint tid = asm_kernel_my_tid();
	debug(DEBUG_SYSCALL, "In user mode MyTid tid = %d", tid);
	return tid;
}

int MyParentTid()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "MyParentTid");
	vint tid = asm_kernel_my_parent_tid();
	debug(DEBUG_SYSCALL, "In user mode MyParentTid tid = %d", tid);
	return tid;
}

int Send( int tid, void *msg, int msglen, void *reply, int replylen )
{
    debug(DEBUG_TRACE, "this is in %s", "user Send");
    asm_kernel_send(tid, msg, msglen, reply, replylen);
}

int Receive( int *tid, void *msg, int msglen )
{
    debug(DEBUG_TRACE, "this is in %s", "user Receive");
    asm_kernel_receive(tid, msg, msglen);
}

int Reply( int tid, void *reply, int replylen )
{
    debug(DEBUG_TRACE, "this is in %s", "user Reply");
    /*asm_kernel_reply(tid, msg, msglen, reply, replylen);*/
}
