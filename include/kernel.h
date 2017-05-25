#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <define.h> 
#include <debug.h>

#define TASK_SIZE 			102400 /* 100kb */
#define TASK_START_LOCATION 0x1000000
#define MAX_NUM_TASKS 		64
#define ARGUMENT_LOCATION   0x9000000 // value get with trial and errors, might cause bug in the future
#define INVALID_TID			-1

/* task descriptor */
typedef enum Task_state {
	STATE_ACTIVE, 
	STATE_ZOMBIE,
	STATE_READY,
    STATE_SEND_BLK,
    STATE_RECEIVE_BLK,
    STATE_REPLY_BLK
} Task_state;
typedef enum Task_priority {
	PRIOR_LOWEST,
	PRIOR_LOW,
	PRIOR_MEDIUM,
	PRIOR_HIGH
} Task_priority;
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
	uint32 					tid;
	int 					ptid;
	Task_state 				state;
	Task_priority 			priority;
	struct Task_descriptor *next_ready_task;
} Task_descriptor;

/* Inter-tasks communication */
typedef struct Message {
    // bool             is_match;
    vint                tid; 
    char                content[64];
    // to be continued
} Message;

/* kernel priority queue with fifo ordering */
typedef struct Task_fifo_llist {
	Task_descriptor *head;
	Task_descriptor *tail;
} Task_fifo_llist;
typedef struct Task_priority_queue {
	uint32			mask;
	Task_fifo_llist	fifos[PRIOR_HIGH + 1];
	Task_descriptor tasks[MAX_NUM_TASKS];
} Task_priority_queue;

/* kernel state */
typedef struct Kernel_state {
	Task_priority_queue priority_queue;
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
Task_descriptor *pull_highest_priority_task(Task_priority_queue *priority_queue);
/*
 * Insert the task into the task priority queue
 */
void insert_task(Task_descriptor *td, Task_priority_queue *priority_queue);
/*
 * Removes the task from the task priority queue
 */
void remove_task(Task_descriptor *td, Task_priority_queue *priority_queue);

/*
 * Activate a task
 */
int activate(Task_descriptor *td);

/* Kernel syscall handlers */
void k_create(Task_descriptor *td, Kernel_state *ks, void (*task)(), uint32 tid, Task_priority priority);
void k_my_tid(Task_descriptor *td, Kernel_state *ks);
void k_my_parent_tid(Task_descriptor *td, Kernel_state *ks);
void k_init_kernel();
void k_pass(Task_descriptor *td, Kernel_state *ks);
void k_exit(Task_descriptor *td, Kernel_state *ks);

/* User tasks */
void init_kernel();
void first_task();

#endif // __KERNEL_H__
