#include <kernel.h>
#include <debug.h>

void k_init_kernel()
{
	debug(DEBUG_SYSCALL, "In %s", "k_init_kernel");
}
 
void k_create(task_descriptor *td, kernel_state *ks, void (*task)(), uint32 tid, task_priority priority)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_create, tid = %d", tid);
	uint32 ptid = td->tid;
	td_intialize(task, ks, tid, ptid, priority);
	td->retval = tid;
    reschedule(td, ks); 
}

void k_my_tid(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_tid, td->tid = %d", td->tid);
	td->retval = td->tid;
    reschedule(td, ks);
}

void k_my_parent_tid(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_parent_tid, td->ptid = %d", td->ptid);
	td->retval = td->ptid;
    reschedule(td, ks);
}

void k_pass(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_pass");
    reschedule(td, ks);
}

void k_exit(task_descriptor *td, kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_exit");
	td->state = STATE_ZOMBIE;
	remove_task(td, &(ks->priority_queue));
}

void k_send(int tid, void *message, int length, void *reply, int replylen,
        task_descriptor *td, kernel_state *ks, task_priority_queue *send_block){
    Message *msg = (Message*)message;
    Message *reply_msg = (Message*)(reply);
    debug(DEBUG_TRACE, "tid = 0x%x, message = %s, length = 0x%x, reply = %s, replylen = 0x%x",
        tid, msg->content, length, reply_msg->content, replylen);
    // check the received blocked queue 
    // 
    // if found some tasks, then unblock that task and pass the message
    // if no task found, block this task 
}
