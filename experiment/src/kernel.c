#include <kernel.h>

void k_init_kernel(){
    bwprintf(COM2, "in the kernel");
}

int k_create( int priority, void(*code) ){

}

int k_myTid(){

}

int k_myParentTid(){

}

int k_pass() {
	bwprintf(COM2, "in the k_pass\n");
}
