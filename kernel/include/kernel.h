#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <define.h> 
#include <int_fifo.h>

/* task descriptor */
#define TASK_SIZE 			102400 /* 100kb */
#define TASK_START_LOCATION 0x1000000

/* HWI */
#define ENTER_FROM_HWI	0xAA
#define NUM_EVENTS		100

/* inter-tasks communication */
typedef struct Message {
    char                content[MAX_MSG_LEN];
    vint                tid; 
	uint8				content_len;
} Message;

typedef struct IntIntMessage {
    vint                tid; 
	vint 				content1;
    vint                content2;
} IntIntMessage;

typedef enum Await_event{
    TIMER3_RDY,
    RCV_UART1_RDY,
    XMIT_UART1_RDY,
    RCV_UART2_RDY,
    XMIT_UART2_RDY    
} Await_event;

/* task descriptor */
typedef enum Task_state {
	STATE_ACTIVE, 
	STATE_ZOMBIE,
	STATE_READY,
    STATE_SEND_BLK,
	STATE_RECEIVE_BLK,
    STATE_REPLY_BLK,
	STATE_EVENT_BLK
} Task_state;
typedef enum Processor_mode {
	USR = 0x10,
	SYS = 0xDF,
	SVC = 0xD3
} Processor_mode;
typedef struct Task_descriptor {
	vint 					*sp; 
	vint 					*lr;
	uint32					spsr;
	uint32 					retval;
	int						is_entry_from_hwi;
	uint32 					tid;
	int 					ptid;
	Task_state 				state;
	Task_priority 			priority;
	struct Task_descriptor *next_task;
    vint                    *fp;
    char                    ch;
	uint8					is_ch_transmitted;
} Task_descriptor;

/* Priority queue with fifo ordering */
typedef struct Fifo_llist {
	Task_descriptor *head;
	Task_descriptor *tail;
} Fifo_llist;
typedef struct Priority_fifo {
	uint32		mask;
	Fifo_llist	fifos[PRIOR_HIGH + 1];
} Priority_fifo;

/* kernel state */
typedef struct Kernel_state {
	Priority_fifo 		ready_queue;
	Priority_fifo 		send_block;
	Priority_fifo		receive_block;
	Priority_fifo 		reply_block;
	Task_descriptor 	tasks[MAX_NUM_TASKS];
	uint64				free_list;
	/* event blocked, there can only have one task wait on each type of event */
	// indicate whether there is task await on particular type of event;
	uint8				blocked_on_event[NUM_EVENTS];
	// stores the event blocked tasks, one task for each event type
	Task_descriptor		*event_blocks[NUM_EVENTS];
	int_fifo_t			uart1_putc_q;
} Kernel_state;

/* task descriptor */
/*
 * Initialize a task descriptor using given arguments:
 * tid = task id, which will also be to used to calculate initial stack pointer
 * ptid = parent task id
 * priority = task priority
 * task = points to the start of the set of instructions. will be the initial link register
 * ks = task will be inserted into ks's scheduler priority queue after intialzed
 * The state of the task will be intialzed as "STATE_READY" and spsr will be intialzed as user mode
 * without flag bits set.
 */
void td_intialize(void (*task)(), Kernel_state *ks, uint32 tid, uint32 ptid, Task_priority priority);
void ks_initialize(Kernel_state *ks);
void update_td(Task_descriptor *td, vint cur_lr);

/* scheduler */
/*
 * Returns the highest priority task that arrived the earliest
 */
Task_descriptor *schedule(Kernel_state *ks);
/*
 * Re-schedule the task. Change its state from "STATE_ACTIVE" -> "STATE_READY" and insert it
 * to the end of the ready queue associated with its priority.
 */
void reschedule(Task_descriptor *td, Kernel_state *ks);

/* priority queue operations */
/*
 * Pulls the highest priority task that arrived the earliest from the task priority queue
 */
Task_descriptor *pull_highest_priority_task(Priority_fifo *priority_queue);
/*
 * Insert the task into the task priority queue
 */
void insert_task(Task_descriptor *td, Priority_fifo *priority_queue);
/*
 * Removes the task from the task priority queue.
 * Return -1 if the task doesn't exist in the priority queue, 0 otherwise.
 */
int remove_task(Task_descriptor *td, Priority_fifo *priority_queue);

/*
 * Activate a task
 */
int activate(Task_descriptor *td);

/* Inter-tasks communication */
/*
 * Given a receiver, finds the matching sender that has the highest priority.
 * Return -1 if not found, 0 otherwise.
 */
int find_sender(Priority_fifo *blocked_queue, int tid, Task_descriptor **psender);

/* Kernel syscall handlers */
void k_create(Task_descriptor *td, Kernel_state *ks, void (*task)(), uint32 tid, Task_priority priority);
void k_my_tid(Task_descriptor *td, Kernel_state *ks);
void k_my_parent_tid(Task_descriptor *td, Kernel_state *ks);
void k_init_kernel();
void k_pass(Task_descriptor *td, Kernel_state *ks);
void k_exit(Task_descriptor *td, Kernel_state *ks);
void k_send(int tid, void *msg, int msglen, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks);
void k_receive(vint *receive_tid, void *receive_message, int receive_length, Task_descriptor *td, Kernel_state *ks);
void k_reply(int reply_tid, void *reply, int replylen, Task_descriptor *td, Kernel_state *ks);
// void k_await_event(int event_type, Task_descriptor *td, Kernel_state *ks); 
void k_await_event(int event_type, char ch, Task_descriptor *td, Kernel_state *ks);
/* User tasks */
void init_kernel();
void first_task();

#endif // __KERNEL_H__
