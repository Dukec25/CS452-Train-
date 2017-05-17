#include <bwio.h>
void init_kernel(){
    bwprintf(COM2, "in the user mode\n");
//    asm_init_kernel();
	pass();
}

void pass() {
	bwprintf(COM2, "%s:%d in the user mode pass\n", __FILE__, __LINE__);
	asm_kernel_pass();
}
