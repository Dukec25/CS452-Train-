#include <bwio.h>

void init_kernel(){
    bwprintf(COM2, "in the user mode\n");
//    asm_init_kernel();
	pass();
}

int create( int priority, void (*code) (  )  ){
    bwprintf(COM2, "in user_create");
    asm_kernel_create();
}

void pass() {
	bwprintf(COM2, "%s:%d in the user mode pass\n", __FILE__, __LINE__);
	asm_kernel_pass();
}
