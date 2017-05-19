#include <heap.h>
#include <define.h> 
#include <fifo.h>

typedef enum task_state{
	STATE_ACTIVE, 
	STATE_ZOMBIE,
	STATE_READY
} task_state;

typedef struct task_descriptor
{
	vint *sp; 
	vint *lr;
	int spsr;
	heap_t *priority_queue;
	uint32 id;
	uint32 parent_id;
	task_state state;
	int retval; // how can we be sure the return type is int
    int priority; // not sure whether needed
} task_descriptor;

typedef struct kernel_state
{
	vint *u_sp; 
	vint *u_lr;
	int u_spsr;
	int u_retval;
	heap_t *priority_queue;
	fifo_t *active_tasks;
	vint *rb_lr;
} kernel_state;

int k_create(int priority, void(*task)(), int current_task_id, int parent_task_id, vint **pavailable_memeory_ptr, heap_t *pready_queue);

int k_myTid();

int k_myParentTid();

void k_init_kernel();

void init_kernel();

void first_task();
