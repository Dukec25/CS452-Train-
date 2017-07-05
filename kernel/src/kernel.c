#include <kernel.h>
#include <debug.h>
#include <string.h>
#include <uart_irq.h>

static uint32 is_task_created(int tid, Kernel_state *ks)
{
	debug(DEBUG_SYSCALL, "ks->free_list = 0x%x", ks->free_list);
	return (ks->free_list & (0x1 << tid)) != 0;
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
    
    debug(DEBUG_SYSCALL, "enter kernel_send %s", "this is kernel send");
    
    debug(DEBUG_SYSCALL, "tid = 0x%x, message = %s, length = 0x%x, reply = %s, replylen = 0x%x",
        tid, msg->content, send_length, reply_msg->content, replylen);
    int result = is_task_created(tid, ks);
    debug(DEBUG_SYSCALL, "after is_task_created, tid=%d", td->tid);

	debug(DEBUG_SYSCALL, "!!!!tid = %d is_task_created result = %d", tid, result);
    if(result){
        // if the receiver task has already been created
        Task_descriptor *receive_td =  &(ks->tasks[tid]);
        int task_exist = remove_task(receive_td, &(ks->receive_block));
        if(task_exist == -1){
            debug(DEBUG_SYSCALL, "task being send_blocked, tid=%d", td->tid);
            // if the task not existed in the receive_block queue 
            // block the task in the send_block 
            td->state = STATE_SEND_BLK; 
            insert_task(td, &(ks->send_block));
            remove_task(td, &(ks->ready_queue));
        } else{
            debug(DEBUG_SYSCALL, "task being reply_blocked, tid=%d", td->tid);
            // pass tid, message to receive task
            // put the received blocked task into the ready queue
            vint receive_task_sp = receive_td->sp; // get the task sp
            int *receive_tid = *((vint*) (receive_task_sp + 0));
            void *receive_message = *((vint*) (receive_task_sp + 4));
            int receive_length = *((vint*) (receive_task_sp + 8));
            *receive_tid = td->tid;
            memcpy(receive_message, send_message, receive_length);

            receive_td->state = STATE_READY;
            insert_task(receive_td, &(ks->ready_queue)); 
            td->state = STATE_REPLY_BLK;
            insert_task(td, &(ks->reply_block));
            remove_task(td, &(ks->ready_queue));
        }
        td->retval = 0; // currently didn't consider situation of trunction
    } else {
        // if the receiver task has not been created
        debug(DEBUG_SYSCALL, "task has not been created, tid=%d", td->tid);
        td->retval = -2;
        reschedule(td, ks);
    }
}

void k_receive(vint *receive_tid, void *receive_message, int receive_length, Task_descriptor *td, Kernel_state *ks)
{
    debug(DEBUG_SYSCALL, "enter kernel_receive tid=%d", td->tid);
    Task_descriptor *send_td; 
    int result = find_sender(&(ks->send_block), td->tid, &send_td);

    // check if anyone send any messages to this task by looking at send_block(don't yet know how)
    // if yes, put that task onto reply_block and grab its data 
    if(result != -1){
        remove_task(send_td, &(ks->send_block));
        debug(DEBUG_SYSCALL, "task being reply_blocked, tid=%d", send_td->tid);
        insert_task(send_td, &(ks->reply_block));
        vint send_task_sp = send_td->sp;
        void *send_message = *((vint*) (send_task_sp + 4));
        int send_length = *((vint*) (send_task_sp + 8));
        // be aware the case receive_length and send_length are different 
        *receive_tid = send_td->tid; // ??? *receive_tid = send_tid will crash
        memcpy(receive_message, send_message, receive_length);
        reschedule(td, ks);
    } else {
        insert_task(td, &(ks->receive_block));
        remove_task(td, &(ks->ready_queue));
    }
    td->retval = 0;
    debug(DEBUG_SYSCALL, "get to the very end, tid=%d", td->tid);
}

void k_reply(int reply_tid, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks){
    Message *reply_msg = (Message*)reply;
    debug(DEBUG_SYSCALL, "enter kernel_reply %s", "this is kernel reply");
    debug(DEBUG_SYSCALL, "want to reply to tid = %d, message is %s", reply_tid, reply_msg->content, is_task_created(reply_tid, ks));
    int task_created = is_task_created(reply_tid, ks);
    if(task_created){
        Task_descriptor *reply_to_td = &ks->tasks[reply_tid];
        debug(DEBUG_SYSCALL, "getting into here %d", reply_to_td->tid);
        int task_exist = remove_task(reply_to_td, &(ks->reply_block));
        debug(DEBUG_SYSCALL, "getting into here task_exist=%d", task_exist);
        if(task_exist == -1){
            debug(DEBUG_SYSCALL, "getting into here %s", "this is kernel reply");
            td->retval = -3;
        } else{
            debug(DEBUG_SYSCALL, "task does exist in reply_block %s", "that's right");
            vint send_task_sp = reply_to_td->sp;
            void *send_reply_message = *((vint*) (send_task_sp + 12));
            /*vint send_task_fp = reply_to_td->fp;*/
            /*vint send_reply_message_length = *((vint*) (send_task_fp + 4));*/
            memcpy(send_reply_message, reply, replylen);
            Message *output = (Message*)send_reply_message;
            debug(DEBUG_SYSCALL, "!!!!!!!!!!! value of send_reply_message%s", output->content);
            reply_to_td->state = STATE_READY;
            insert_task(reply_to_td, &(ks->ready_queue)); // send get inserted before reply
            td->retval = 0; // currently didn't consider truncation
        }
    } else{
        td->retval = -2;
    }
    
    reschedule(td, ks);
}

/*
void k_await_event(int event_type, Task_descriptor *td, Kernel_state *ks)
{
	debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>In kernel mode k_await_event, event_type = %d", event_type);
	td->state = STATE_EVENT_BLK;
	ks->blocked_on_event[event_type] = 1;
    remove_task(td, &(ks->ready_queue));
	ks->event_blocks[event_type] = td;
}
*/

// void k_await_event_v2(int event_type, char ch, Task_descriptor *td, Kernel_state *ks)
void k_await_event(int event_type, char ch, Task_descriptor *td, Kernel_state *ks)
{
	if (event_type == RCV_UART1_RDY) 
		debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>In kernel mode k_await_event, event_type = %d", event_type);
	td->state = STATE_EVENT_BLK;
	ks->blocked_on_event[event_type] = 1;
    remove_task(td, &(ks->ready_queue));
    /*debug(DEBUG_SYSCALL, "%s", "d");*/
	ks->event_blocks[event_type] = td;
    if(event_type == XMIT_UART1_RDY) {
		uart_device_enable(COM1, XMIT);
        td->ch = ch;
		td->is_ch_transmitted = 0;
		//bwprintf(COM2, "td %d, wait %d, is_ch_transmitted = %d\r\n", td->tid, ch, td->is_ch_transmitted);
    } else if(event_type == XMIT_UART2_RDY) {
		uart_device_enable(COM2, XMIT);
        td->ch = ch;
		debug(DEBUG_UART_IRQ, ">>>enable COM2, XMIT, ch = %d", ch);
    }
}
