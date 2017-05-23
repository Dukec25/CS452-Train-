#include <define.h> 
#include <debug.h>

#define TASK_SIZE 			102400 /* 100kb */
#define TASK_START_LOCATION 0x1000000
#define MAX_NUM_TASKS 		64
#define ARGUMENT_LOCATION   0x9000000 // value get with trial and errors, might cause bug in the future
#define INVALID_TID			-1

typedef enum task_state {
	STATE_ACTIVE, 
	STATE_ZOMBIE,
	STATE_READY
} task_state;

typedef enum task_priority {
	PRIOR_LOWEST,
	PRIOR_LOW,
	PRIOR_MEDIUM,
	PRIOR_HIGH
} task_priority;

typedef struct task_descriptor {
	vint 					*sp; 
	vint 					*lr;
	uint32 					retval;
	uint32 					tid;
	int 					ptid;
	task_state 				state;
	task_priority 			priority;
	struct task_descriptor *next_ready_task;
} task_descriptor;

typedef struct ready_queue {
	task_descriptor *head;
	task_descriptor *tail;
} ready_queue;

typedef struct kernel_state {
	vint			*u_sp; 
	vint 			*u_lr;
	uint32 			u_spsr;
	uint32 			u_retval;
	vint 			*rb_lr;
	uint32			priority_mask;
	ready_queue		ready_queues[PRIOR_HIGH + 1];
	task_descriptor tasks[MAX_NUM_TASKS];
} kernel_state;

typedef enum processor_mode {
	USR = 0x10,
	SYS = 0xDF,
	SVC = 0xD3
} processor_mode;

void td_intialize(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority);

task_descriptor *schedule(kernel_state *ks);

void insert_task(task_descriptor *td, kernel_state *ks);

void remove_task(task_descriptor *td, kernel_state *ks);

int activate(task_descriptor *td, kernel_state *ks);

void k_create(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority);

void k_my_tid(task_descriptor *td, kernel_state *ks);

void k_my_parent_tid(task_descriptor *td, kernel_state *ks);

void k_init_kernel();

void k_pass(task_descriptor *td, kernel_state *ks);

void k_exit(task_descriptor *td, kernel_state *ks);

void init_kernel();

void first_task();
