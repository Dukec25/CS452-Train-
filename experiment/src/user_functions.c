#include <bwio.h>
void init_kernel(){
    bwprintf(COM2, "in the user mode\n");
    asm_init_kernel();
}

