#include <bwio.h>
#include <user_functions.h>

void init_kernel()
{
    debug("In user mode %s", "init_kernel");
    asm_init_kernel();
}

// upper case due to A1 specification 
int Create(int priority, void (*code) ())
{
    debug("In user mode Create, priority = %d, code = 0x%x\n", priority, code);
    asm_kernel_create(priority, code);
}

void Pass()
{
	debug("In user mode %s", "Pass");
    asm_kernel_pass();
}

void Exit()
{
	debug("In user mode %s", "Exit");
    asm_kernel_exit();
}

int MyTid()
{
	debug("In user mode %s", "MyTid");
    vint tid = asm_kernel_my_tid();
	debug("In user mode MyTid tid = %d", tid);
	return tid;
}
