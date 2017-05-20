#include <bwio.h>
#include <user_functions.h>

void init_kernel()
{
    debug("In user mode %s\n", "init_kernel");
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
	debug("In user mode %s\n", "Pass");
    asm_kernel_pass();
}

int MyTid()
{
	debug("In user mode %s\n", "MyTid");
    int tid = asm_kernel_my_tid();
	debug("In user mode MyTid tid = %d\n", 999);
	return tid;
}
