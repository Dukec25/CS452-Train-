#include <kernel.h>
#include <bwio.h>

void k_init_kernel(){
    bwprintf(COM2, "in the kernel");
}

void k_create(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority)
{
	debug("In kernel mode k_create, tid = %d", tid);
	td_intialize(task, ks, tid, ptid, priority);
	debug("In kernel mode k_create before add into retval, tid = %d", tid);
	ks->tasks[ptid].retval = tid;
}

void k_my_tid(task_descriptor *td){
	debug("In kernel mode k_my_tid, td->tid = %d", td->tid);
    td->retval = td->tid;
}

void k_my_parent_tid(task_descriptor *td){
//    td->retval = td->parent_id;
}

void k_pass(task_descriptor *td, heap_t *pready_queue) {
    /*bwprintf(COM2, "%s:%d in the kernel pass", __FILE__, __LINE__);*/
    /*td->state = STATE_READY;*/
    /*heap_insert(pready_queue, td->priority, td);*/
}

void k_exit(task_descriptor *td, kernel_state *ks) {
    remove_task(td, ks);
}
