#include <kernel.h>
#include <debug.h>

void k_init_kernel()
{
	debug(DEBUG_SYSCALL, "In %s", "k_init_kernel");
	bwprintf(COM2, "in the kernel");
}
 
void k_create(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_create, tid = %d", tid);
	td_intialize(task, ks, tid, ptid, priority);
	ks->tasks[ptid].retval = tid;
}

void k_my_tid(task_descriptor *td)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_tid, td->tid = %d", td->tid);
	td->retval = td->tid;
}

void k_my_parent_tid(task_descriptor *td)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_parent_tid, td->ptid = %d", td->ptid);
	td->retval = td->ptid;
}

void k_pass(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_pass");
	remove_task(td, ks);
	td->state = STATE_READY;
	insert_task(td, ks);
}

void k_exit(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_exit");
	td->state = STATE_ZOMBIE;
	remove_task(td, ks);
}
