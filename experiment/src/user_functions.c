#include <bwio.h>
#include <user_functions.h>


void init_kernel(){
    bwprintf(COM2, "in the user mode\n");
//    asm_init_kernel();
}

// upper case due to A1 specification 
int Create( int priority, void (*code) (  )  ){
    bwprintf(COM2, "in user_create");
    asm_kernel_create(priority, code);
}

void Pass() {
    bwprintf(COM2, "%s:%d in the user mode pass\n", __FILE__, __LINE__);
    asm_kernel_pass();
}

int MyTid(){
    bwprintf(COM2, "%s:%d in the user mode myTid\n", __FILE__, __LINE__);
    asm_kernel_my_tid();
}
