#include <kernel.h>
#include <debug.h>
#include <string.h>

static uint8 is_task_created(int tid, Kernel_state *ks)
{
	return (ks->free_list & (0x1 << tid));
}

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
	ks->free_list &= ~(0x1 << td->tid);
	debug(DEBUG_SYSCALL, "In %s", "k_exit");
	td->state = STATE_ZOMBIE;
	remove_task(td, &(ks->ready_queue));
}

void k_send(int tid, void *send_message, int send_length, void *reply, int replylen,
        Task_descriptor *td, Kernel_state *ks)
{
    Message *msg = (Message*)send_message;
    Message *reply_msg = (Message*)(reply);
    
    debug(DEBUG_MESSAGE, "enter kernel_send %s", "this is kernel send");
    
    debug(DEBUG_MESSAGE, "tid = 0x%x, message = %s, length = 0x%x, reply = %s, replylen = 0x%x",
        tid, msg->content, send_length, reply_msg->content, replylen);
    int result = is_task_created(tid, ks);
    debug(DEBUG_MESSAGE, "after is_task_created, tid=%d", td->tid);

    if(result){
        // if the receiver task has already been created
        Task_descriptor *receive_td =  &(ks->tasks[tid]);
        int task_exist = remove_task(receive_td, &(ks->receive_block));
        if(task_exist == -1){
            debug(DEBUG_MESSAGE, "task being send_blocked, tid=%d", td->tid);
            // if the task not existed in the receive_block queue 
            // block the task in the send_block 
            td->state = STATE_SEND_BLK; 
            insert_task(td, &(ks->send_block));
            remove_task(td, &(ks->ready_queue));
        } else{
            debug(DEBUG_MESSAGE, "task being reply_blocked, tid=%d", td->tid);
            // pass tid, message to receive task
            // put the received blocked task into the ready queue
            vint receive_task_sp = receive_td->sp; // get the task sp
            int *receive_tid = *((vint*) (receive_task_sp + 0));
            void *receive_message = *((vint*) (receive_task_sp + 4));
            int receive_length = *((vint*) (receive_task_sp + 8));
            *receive_tid = td->tid;
            // if receive_length and send_length are different, shall we deal
            // with it ?
            memcpy(receive_message, send_message, receive_length);

            receive_td->state = STATE_READY;
            insert_task(receive_td, &(ks->ready_queue)); 
            td->state = STATE_REPLY_BLK;
            insert_task(td, &(ks->reply_block));
            remove_task(td, &(ks->ready_queue));
        }
        /*reschedule(td, ks);*/
        td->retval = 0; // currently didn't consider situation of trunction
    } else {
        // if the receiver task has not been created
        debug(DEBUG_MESSAGE, "task has not been created, tid=%d", td->tid);
        reschedule(td, ks);
        td->retval = -2;
    }
}

void k_receive(vint *receive_tid, void *receive_message, int receive_length, Task_descriptor *td, Kernel_state *ks)
{
    debug(DEBUG_MESSAGE, "enter kernel_receive tid=%d", td->tid);
    Task_descriptor *send_td; 
    int result = find_sender(&(ks->send_block), td->tid, &send_td);
    debug(DEBUG_MESSAGE, "return result=%d", result);

    // check if anyone send any messages to this task by looking at send_block(don't yet know how)
    // if yes, put that task onto reply_block and grab its data 
    if(result != -1){
        remove_task(send_td, &(ks->send_block));
        debug(DEBUG_MESSAGE, "task being reply_blocked, tid=%d", send_td->tid);
        insert_task(send_td, &(ks->reply_block));
        vint send_task_sp = send_td->sp;
        void *send_message = *((vint*) (send_task_sp + 4));
        int send_length = *((vint*) (send_task_sp + 8));
        // be aware the case receive_length and send_length are different 
        *receive_tid = send_td->tid; // ??? *receive_tid = send_tid will crash
        memcpy(receive_message, send_message, receive_length);
        debug(DEBUG_MESSAGE, "get to the end, tid=%d", send_td->tid);
        reschedule(td, ks);
    } else {
        insert_task(td, &(ks->receive_block));
        remove_task(td, &(ks->ready_queue));
    }
    td->retval = 0;
    debug(DEBUG_MESSAGE, "get to the very end, tid=%d", td->tid);
}

void k_reply(int reply_tid, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks){
    Message *reply_msg = (Message*)reply;
    debug(DEBUG_MESSAGE, "enter kernel_reply %s", "this is kernel reply");
    debug(DEBUG_MESSAGE, "want to reply to tid = %d, message is %s", reply_tid, reply_msg->content);
    int task_created = is_task_created(reply_tid, ks);
    if(task_created){
        Task_descriptor *reply_to_td = &ks->tasks[reply_tid];
        debug(DEBUG_MESSAGE, "getting into here %d", reply_to_td->tid);
        int task_exist = remove_task(reply_to_td, &(ks->reply_block));
        debug(DEBUG_MESSAGE, "getting into here task_exist=%d", task_exist);
        if(task_exist == -1){
            debug(DEBUG_MESSAGE, "getting into here %s", "this is kernel reply");
            td->retval = -3;
        } else{
            debug(DEBUG_MESSAGE, "task does exist in reply_block %s", "that's right");
            vint send_task_sp = reply_to_td->sp;
            void *send_reply_message = *((vint*) (send_task_sp + 12));
            /*vint send_task_fp = reply_to_td->fp;*/
            /*vint send_reply_message_length = *((vint*) (send_task_fp + 4));*/
            memcpy(send_reply_message, reply, replylen);
            Message *output = (Message*)send_reply_message;
            debug(DEBUG_MESSAGE, "!!!!!!!!!!! value of send_reply_message%s", output->content);
            reply_to_td->state = STATE_READY;
            insert_task(reply_to_td, &(ks->ready_queue)); // send get inserted before reply
            td->retval = 0; // currently didn't consider truncation
        }
    } else{
        td->retval = -2;
    }
    
    reschedule(td, ks);
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
