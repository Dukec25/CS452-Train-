#include <kernel.h>

void k_init_kernel(){
    bwprintf(COM2, "in the kernel");
}

int k_create(int priority, void(*task)(), int current_task_id, int parent_task_id, vint **pavailable_memeory_ptr, heap_t *pready_queue){
    bwprintf(COM2, "enter k_create");
    task_descriptor td;
    td.id = current_task_id;
    td.parent_id = parent_task_id;
    td.state = STATE_READY;
    td.spsr = 16; // used to get back to user mode 
    //assign memory to the first task
    td.sp = *pavailable_memeory_ptr; 
    *pavailable_memeory_ptr += TASK_SIZE;
    // assign lr to point to the function pointer
    td.lr = (vint *)task;
    bwprintf(COM2, "%s:%d td->id = %d, td->state = %d\n", __FILE__, __LINE__, td.id, td.state);
    bwprintf(COM2, "%s:%d td->sp = 0x%x, td->lr = 0x%x\n", __FILE__, __LINE__, td.sp, td.lr);
    heap_insert(pready_queue, PRIOR_MEDIUM, &td);
    td.priority_queue = pready_queue;
}

int k_myTid(){

}

int k_myParentTid(){

}

int k_pass() {
	bwprintf(COM2, "in the k_pass\n");
}
