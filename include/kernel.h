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
typedef enum task_state {
	STATE_ACTIVE, 
	STATE_ZOMBIE,
	STATE_READY,
    STATE_SEND_BLK,
    STATE_RECEIVE_BLK,
    STATE_REPLY_BLK
} task_state;
typedef enum task_priority {
	PRIOR_LOWEST,
	PRIOR_LOW,
	PRIOR_MEDIUM,
	PRIOR_HIGH
} task_priority;
typedef enum processor_mode {
	USR = 0x10,
	SYS = 0xDF,
	SVC = 0xD3
} processor_mode;
typedef struct task_descriptor {
	vint 					*sp; 
	vint 					*lr;
	uint32					spsr;
	uint32 					retval;
	uint32 					tid;
	int 					ptid;
	task_state 				state;
	task_priority 			priority;
	struct task_descriptor *next_ready_task;
} task_descriptor;

/* Inter-tasks communication */
typedef struct Message {
    // bool             is_match;
    vint                tid; 
    char                content[64];
    // to be continued
} Message;

/* kernel priority queue with fifo ordering */
typedef struct task_fifo_llist {
	task_descriptor *head;
	task_descriptor *tail;
} task_fifo_llist;
typedef struct task_priority_queue {
	uint32			priority_mask;
	task_fifo_llist	task_fifos[PRIOR_HIGH + 1];
	task_descriptor tasks[MAX_NUM_TASKS];
} task_priority_queue;

/* kernel state */
typedef struct kernel_state {
	task_priority_queue priority_queue;
} kernel_state;

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
void td_intialize(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority);

/* scheduler */
/*
 * Returns the highest priority task that arrived the earliest
 */
task_descriptor *schedule(kernel_state *ks);
/*
 * Re-schedule the task. Change its state from "STATE_ACTIVE" -> "STATE_READY" and insert it
 * to the end of the ready queue associated with its priority.
 */
void reschedule(task_descriptor *td, kernel_state *ks);

/* priority queue operations */
/*
 * Pulls the highest priority task that arrived the earliest from the task priority queue
 */
task_descriptor *pull_highest_priority_task(task_priority_queue *priority_queue);
/*
 * Insert the task into the task priority queue
 */
void insert_task(task_descriptor *td, task_priority_queue *priority_queue);
/*
 * Removes the task from the task priority queue
 */
void remove_task(task_descriptor *td, task_priority_queue *priority_queue);

/*
 * Activate a task
 */
int activate(task_descriptor *td);

/* Kernel syscall handlers */
void k_create(task_descriptor *td, kernel_state *ks, void (*task)(), uint32 tid, task_priority priority);
void k_my_tid(task_descriptor *td, kernel_state *ks);
void k_my_parent_tid(task_descriptor *td, kernel_state *ks);
void k_init_kernel();
void k_pass(task_descriptor *td, kernel_state *ks);
void k_exit(task_descriptor *td, kernel_state *ks);

/* User tasks */
void init_kernel();
void first_task();

#endif // __KERNEL_H__
