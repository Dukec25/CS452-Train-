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
    td.priority = priority; // not sure whether priority is needed 
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

void k_my_tid(task_descriptor *td){
    bwprintf(COM2, "%s:%d in the kernel myPid\n", __FILE__, __LINE__);
    bwprintf(COM2, "task ID value =%d ", td->id);
    td->retval = td->id;
}

void k_my_parent_tid(task_descriptor *td){
    td->retval = td->parent_id;
}

void k_pass(task_descriptor *td, heap_t *pready_queue) {
    bwprintf(COM2, "%s:%d in the kernel pass\n", __FILE__, __LINE__);
    td->state = STATE_READY;
    heap_insert(pready_queue, td->priority, td);
}
