#include <kernel.h>
#include <debug.h>
#include <string.h>

void k_init_kernel()
{
	debug(DEBUG_SYSCALL, "In %s", "k_init_kernel");
}
 
void k_create(Task_descriptor *td, Kernel_state *ks, void (*task)(), uint32 tid, Task_priority priority)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_create, tid = %d", tid);
	uint32 ptid = td->tid;
	td_intialize(task, ks, tid, ptid, priority);
	td->retval = tid;
    reschedule(td, ks); 
}

void k_my_tid(Task_descriptor *td, Kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_tid, td->tid = %d", td->tid);
	td->retval = td->tid;
    reschedule(td, ks);
}

void k_my_parent_tid(Task_descriptor *td, Kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In kernel mode k_my_parent_tid, td->ptid = %d", td->ptid);
	td->retval = td->ptid;
    reschedule(td, ks);
}

void k_pass(Task_descriptor *td, Kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_pass");
    reschedule(td, ks);
}

void k_exit(Task_descriptor *td, Kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "In %s", "k_exit");
	td->state = STATE_ZOMBIE;
	remove_task(td, &(ks->ready_queue));
}

static uint8 is_task_created(int tid, Kernel_state *ks)
{
	return (ks->free_list & (0x1 << tid));
}

void k_send(int tid, void *msg, int msglen, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks)
{
    debug(DEBUG_SYSCALL, "tid = 0x%d, msg = %s, msglen = 0x%x, reply = %s, reply_len = %d",
        tid, msg, msglen, replylen);
	// Error checking
	if (msglen > MAX_MSG_LEN || replylen > MAX_MSG_LEN) {
		td->retval = ERR_INVALID_MSG_LEN;
		debug(DEBUG_SYSCALL, "ERROR!!! invalid message length %d and %d", msglen, replylen); 
	}
	if (tid >= MAX_NUM_TASKS) {
		td->retval = ERR_INVALID_TID;
		debug(DEBUG_SYSCALL, "ERROR!!! invalid tid %d", tid); 
	}

    // check the received blocked queue 
    // 
    // if found some tasks, then unblock that task and pass the message
    // if no task found, block this task 
	if (is_task_created(tid, ks)) {
		Task_descriptor *receiver = &(ks->tasks[tid]);
		if (td->state == STATE_ACTIVE && receiver->state == STATE_RECEIVE_BLK) {
			// Sender wants to send message, receiver has been waiting for the sender to send the message
			// unblock the receiver
			receiver->state = STATE_READY;
			remove_task(receiver, &(ks->receive_block));
			insert_task(receiver, &(ks->ready_queue));
			// pass the message to receiver td
			receiver->msg.content_len = msglen;
			memcpy(receiver->msg.content, msg, msglen);
			// reply block the task
			td->state = STATE_REPLY_BLK;
			remove_task(td, &(ks->send_block));
			insert_task(td, &(ks->reply_block));
		}
		else if (td->state == STATE_SEND_BLK) {
			if (receiver->msg.reply_content_len > 0) {
				// Sender has sent the message, receiver has replied
				// unblock the sender
				td->state = STATE_READY;
				remove_task(td, &(ks->reply_block));
				insert_task(td, &(ks->ready_queue));
				// pass the reply message to sender
				memcpy(reply, receiver->msg.reply_content, replylen);
			}
		}
		else {
			// The receiver is not ready to receive yet, send block task
			td->state = STATE_SEND_BLK;
			remove_task(td, &(ks->ready_queue));
			insert_task(td, &(ks->send_block)); 
		}
	}
	else {
		// no task with tid has been created, send block the task
		td->state = STATE_SEND_BLK;
		remove_task(td, &(ks->ready_queue));
		insert_task(td, &(ks->send_block)); 
	}
	
	// set return value to as success
	td->retval = ERR_SUCCESS;
}
