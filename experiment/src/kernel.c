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

/*void k_send(int tid, void *send_message, int send_length, void *reply, int replylen,*/
        /*Task_descriptor *td, Kernel_state *ks, Fifo_llist *send_block, Fifo_llist *receive_block)*/
/*{*/
    /*Message *msg = (Message*)send_message;*/
    /*Message *reply_msg = (Message*)(reply);*/
    /*debug(DEBUG_TRACE, "tid = 0x%x, message = %s, length = 0x%x, reply = %s, replylen = 0x%x",*/
        /*tid, msg->content, send_length, reply_msg->content, replylen);*/
    /*Task_descriptor *receive_td =  &(ks->tasks[tid]);*/
    /*int task_exist = remove_task(receive_td, receive_block);*/
    /*if(task_exist == -1){*/
        /*// if the task not existed in the receive_block queue */
        /*// block the task in the send_block */
        /*td->state = STATE_SEND_BLK; */
        /*insert_task(td, send_block);*/
    /*} else{*/
        /*// pass tid, message to receive task*/
        /*// put the received blocked task into the ready queue*/
        /*vint receive_task_sp = receive_td->sp; // get the task sp*/
        /*int *receive_tid = *((vint*) (receive_task_sp + 0));*/
        /*void *receive_message = *((vint*) (receive_task_sp + 4));*/
        /*int receive_length = *((vint*) (receive_task_sp + 8));*/
        /**receive_tid = tid;*/
        /*// if receive_length and send_length are different, shall we deal*/
        /*// with it */
        /*memcpy(receive_message, send_message, receive_length);*/

        /*receive_td->state = STATE_READY;*/
        /*insert_task(receive_td, &(ks->priority_queue));*/
    /*}*/
    /*reschedule(td, ks);*/
/*}*/

void k_receive(int *receive_tid, void *receive_message, int receive_length, Task_descriptor *td, Fifo_llist *send_block, Fifo_llist *receive_block,
        Fifo_llist *reply_block)
{
    /*// check if anyone send message to this task by looking at send_block(don't yet know how)*/
    /*// if yes, put that task onto reply_block and grab its data */
    /*remove_task(send_td, send_block);*/
    /*insert_task(send_td, reply_block);*/
    /*vint send_task_sp = send_td->sp;*/
    /*int send_tid = *((vint*) (receive_task_sp + 0));*/
    /*void *send_message = *((vint*) (receive_task_sp + 4));*/
    /*int send_length = *((vint*) (receive_task_sp + 8));*/
    /*// be aware the case receive_length and send_length are different */
    /**receive_tid = send_tid; */
    /*memcpy(receive_message, send_message, receive_length);*/

    /*// if no */
    /*insert_task(td, receive_block);*/
    
    /*reschedule(td, ks);*/
}

/*void k_reply(int reply_tid, void *reply, int replylen, Fifo_llist *reply_block){*/
    /*// is there a situation that someone reply before send ???*/
    /*Task_descriptor *send_td = &ks->priority_queue.tasks[tid];*/
    /*int task_exist = remove_task(send_td, reply_block);*/
    /*if(task_exist == -1){*/
        /*// should return -3 in this case  */
    /*}    */
    /*else{*/
        /*vint send_task_sp = send_td->sp;*/
        /*void *send_reply_message = *((vint*) (send_task_sp + 12));*/
        /*vint send_task_fp = send_td->fp;*/
        /*vint send_reply_message_length = *((vint*) (cur_fp + 4));*/
        /*// return error code if length doesn't match*/
        /*memcpy(send_reply_message, reply, replylen);*/
        /*send_td.state = ready;*/
    /*}*/
    /*reschedule(td, ks);*/
/*}*/

static uint8 is_task_created(int tid, Kernel_state *ks)
{
	return (ks->free_list & (0x1 << tid));
}

/*void k_send(int tid, void *msg, int msglen, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks)*/
/*{*/
    /*debug(DEBUG_SYSCALL, "tid = 0x%d, msg = %s, msglen = 0x%x, reply = %s, reply_len = %d",*/
        /*tid, msg, msglen, replylen);*/
	/*// Error checking*/
	/*if (msglen > MAX_MSG_LEN || replylen > MAX_MSG_LEN) {*/
		/*td->retval = ERR_INVALID_MSG_LEN;*/
		/*debug(DEBUG_SYSCALL, "ERROR!!! invalid message length %d and %d", msglen, replylen); */
	/*}*/
	/*if (tid >= MAX_NUM_TASKS) {*/
		/*td->retval = ERR_INVALID_TID;*/
		/*debug(DEBUG_SYSCALL, "ERROR!!! invalid tid %d", tid); */
	/*}*/

    /*// check the received blocked queue */
    /*// */
    /*// if found some tasks, then unblock that task and pass the message*/
    /*// if no task found, block this task */
	/*if (is_task_created(tid, ks)) {*/
		/*Task_descriptor *receiver = &(ks->tasks[tid]);*/
		/*if (td->state == STATE_ACTIVE && receiver->state == STATE_RECEIVE_BLK) {*/
			/*// Sender wants to send message, receiver has been waiting for the sender to send the message*/
			/*// unblock the receiver*/
			/*receiver->state = STATE_READY;*/
			/*remove_task(receiver, &(ks->receive_block));*/
			/*insert_task(receiver, &(ks->ready_queue));*/
			/*// pass the message to receiver td*/
			/*receiver->msg.content_len = msglen;*/
			/*memcpy(receiver->msg.content, msg, msglen);*/
			/*// reply block the task*/
			/*td->state = STATE_REPLY_BLK;*/
			/*remove_task(td, &(ks->send_block));*/
			/*insert_task(td, &(ks->reply_block));*/
		/*}*/
		/*else if (td->state == STATE_SEND_BLK) {*/
			/*if (receiver->msg.reply_content_len > 0) {*/
				/*// Sender has sent the message, receiver has replied*/
				/*// unblock the sender*/
				/*td->state = STATE_READY;*/
				/*remove_task(td, &(ks->reply_block));*/
				/*insert_task(td, &(ks->ready_queue));*/
				/*// pass the reply message to sender*/
				/*memcpy(reply, receiver->msg.reply_content, replylen);*/
			/*}*/
		/*}*/
		/*else {*/
			/*// The receiver is not ready to receive yet, send block task*/
			/*td->state = STATE_SEND_BLK;*/
			/*remove_task(td, &(ks->ready_queue));*/
			/*insert_task(td, &(ks->send_block)); */
		/*}*/
	/*}*/
	/*else {*/
		/*// no task with tid has been created, send block the task*/
		/*td->state = STATE_SEND_BLK;*/
		/*remove_task(td, &(ks->ready_queue));*/
		/*insert_task(td, &(ks->send_block)); */
	/*}*/
	
	/*// set return value to as success*/
	/*td->retval = ERR_SUCCESS;*/
/*}*/
