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
    task_descriptor *td = &(ks->tasks[tid]);
    k_reschedule(td, ks);   
}

void k_my_tid(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_tid, td->tid = %d", td->tid);
	td->retval = td->tid;
    k_reschedule(td, ks);
}

void k_my_parent_tid(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_parent_tid, td->ptid = %d", td->ptid);
	td->retval = td->ptid;
    k_reschedule(td, ks);
}

void k_pass(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_pass");
    k_reschedule(td, ks);
}

void k_reschedule(task_descriptor *td, kernel_state *ks){
	debug(DEBUG_SYSCALL, "In %s", "k_reschedule");
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

void k_send(int tid, void *message, int length, void *reply, int replylen,
        task_descriptor *td, kernel_state *ks){
    Message *msg = (Message*)(message);
    Message *reply_msg = (Message*)(reply);
    debug(DEBUG_TRACE, "tid = 0x%x, message = 0x%x, length = 0x%x, reply = 0x%x, replylen = 0x%x",
        tid, msg->content, length, reply_msg->content, replylen);
    send_block();

    // check the blocked queue 
    // if found some tasks, then unblock that task and pass the message
    // if no task found, block this task 
}
