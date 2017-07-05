#include <debug.h>
#include <kernel.h>
#include <fifo.h>

extern int asm_get_spsr();
extern int asm_get_sp();
extern int asm_get_fp();

/* Initialze kernel state by setting task priority queue to be empty */
void ks_initialize(Kernel_state *ks)
{
	ks->free_list = 0;
	ks->ready_queue.mask = 0;
	ks->send_block.mask = 0;
	ks->reply_block.mask = 0;
	ks->receive_block.mask = 0;
	int_fifo_init(&ks->uart1_putc_q);
	int e = 0;
	for (e = 0; e < NUM_EVENTS; e++) {
		ks->event_blocks[e] = NULL;
		ks->blocked_on_event[e] = 0;
	}
	int i = 0;
	for (i = PRIOR_LOWEST; i <= PRIOR_HIGH; i++) {
		ks->ready_queue.fifos[i].head = NULL;
		ks->ready_queue.fifos[i].tail = NULL;
		ks->send_block.fifos[i].head = NULL;	
		ks->send_block.fifos[i].tail = NULL;
		ks->reply_block.fifos[i].head = NULL;	
		ks->reply_block.fifos[i].tail = NULL;
		ks->receive_block.fifos[i].head = NULL;
		ks->receive_block.fifos[i].tail = NULL;
	}
}

/* task descriptor */
void td_intialize(void (*task)(), Kernel_state *ks, uint32 tid, uint32 ptid, Task_priority priority)
{
	Task_descriptor *td = &(ks->tasks[tid]);
	// initialize tid, ptid, state, priority, spsr, and is_entry_from_hwi
	td->tid = tid;
	td->ptid = ptid;
	td->state = STATE_READY;
	td->priority = priority;
	td->spsr = USR; // hardcoded to user mode, not flag bit set
	td->is_entry_from_hwi = 0;
	td->ch = -1;
	td->is_ch_transmitted = 0;
	// assign memory to the first task
	td->sp = (vint *) (TASK_START_LOCATION + (tid + 1) * TASK_SIZE); 
	// assign lr to point to the function pointer
	td->lr = (vint *)task;
	td->lr = (vint *)task + 0x218000 / 4;
	// push lr and sp onto the user task
	*(td->sp - 12) = (vint) td->lr;
	*(td->sp - 11) = TASK_START_LOCATION + (tid + 1) * TASK_SIZE;
	// set next_task
	td->next_task = NULL;
	insert_task(td, &(ks->ready_queue));
	// update free_list
	ks->free_list |= (0x1 << tid);
	debug(DEBUG_TRACE, "tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_task = %d, free_list = 0x%x",
			ks->tasks[tid].tid, ks->tasks[tid].state, ks->tasks[tid].priority,
			ks->tasks[tid].sp, ks->tasks[tid].lr, ks->tasks[tid].next_task, ks->free_list);
}

/*
 * Update lr, sp, spsr in the td
 */
void update_td(Task_descriptor *td, vint cur_lr)
{
	// retrieve spsr
	vint cur_spsr = asm_get_spsr();

	// retrieve sp and arg0 and arg1
	vint cur_sp = asm_get_sp();
	vint cur_fp = asm_get_fp();

	// update td: sp, lr, spsr
	td->sp = (vint *)cur_sp;
	td->lr = (vint *)cur_lr;
    td->fp = (vint *)cur_fp; // pay attention, confirm with Alicia  
	td->spsr = cur_spsr;
	debug(DEBUG_IRQ, "cur_sp = 0x%x, cur_lr = 0x%x, cur_fp = 0x%x, spsr = 0x%x", cur_sp, cur_lr, cur_fp, cur_spsr);
}
