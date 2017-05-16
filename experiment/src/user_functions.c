#include <bwio.h>
void init_kernel(){
    bwprintf(COM2, "get into init\n");
    asm_init_kernel();
}

