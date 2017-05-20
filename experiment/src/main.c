#include <bwio.h>
#include <kernel.h>
#include <define.h>
#include <math.h>

extern int asm_print_sp();
extern void asm_kernel_swiEntry();
extern void asm_init_kernel();
extern int asm_kernel_activate(task_descriptor *td);

static void ks_initialize(kernel_state *ks)
{
	ks->priority_mask = 0;
	int i = 0;
	for (i = PRIOR_LOWEST; i <= PRIOR_HIGH; i++) {
		ks->ready_queues[i].head = NULL;
		ks->ready_queues[i].tail = NULL;
	}
}

void td_intialize(void (*task)(), kernel_state *ks, uint32 tid, uint32 ptid, task_priority priority)
{
	task_descriptor *td = &(ks->tasks[tid]);
	td->tid = tid;
	td->ptid = ptid;
	td->state = STATE_READY;
	td->priority = priority;
	//assign memory to the first task
	td->sp = (vint *) (TASK_START_LOCATION + (tid + 1) * TASK_SIZE); 
	// assign lr to point to the function pointer
	td->lr = (vint *)task;
	// set next_ready_task
	td->next_ready_task = NULL;
	debug("tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d\n",
			ks->tasks[tid].tid, ks->tasks[tid].state, ks->tasks[tid].priority,
			ks->tasks[tid].sp, ks->tasks[tid].lr, ks->tasks[tid].next_ready_task);
	// insert td into the ready_queue
	debug("ks->priority_mask = 0x%x\n", ks->priority_mask);
	if (ks->priority_mask & (0x1 << priority)) {
		// ready_queue is non-empty
		task_descriptor *tail = ks->ready_queues[priority].tail;
		tail->next_ready_task = td;
		debug("old tail->tid = %d, old tail->next_ready_task->tid = %d\n", tail->tid, tail->next_ready_task->tid);
		ks->ready_queues[priority].tail = td;
		debug("new tail->tid = %d\n", ks->ready_queues[priority].tail->tid);
	} else {
		// ready_queue is empty
		ks->ready_queues[priority].head = td;
		ks->ready_queues[priority].tail = td;
		debug("new ks->ready_queues[priority].head->tid = %d, ks->ready_queues[priority].tail->tid = %d\n",
				ks->ready_queues[priority].head->tid, ks->ready_queues[priority].tail->tid);
		// set priority_mask
		ks->priority_mask |= (0x1 << priority);
		debug("ks->priority_mask = 0x%x\n", ks->priority_mask);
	}
}

task_descriptor *schedule(kernel_state *ks) {
	uint8 lz = clz(ks->priority_mask);
	uint8 priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));
	task_descriptor *head = ks->ready_queues[priority].head;
	debug("lz = %d, priority = %d, head->tid = %d\n", lz, priority, head->tid);
	if (head->next_ready_task == NULL) {
		ks->ready_queues[priority].head = NULL;
		ks->ready_queues[priority].tail = NULL;
		ks->priority_mask &= ~(0x1 << head->priority);
		debug("ks->ready_queues[priority].head = %d, ks->ready_queues[priority].tail = %d, ks->priority_mask = 0x%x\n",
				ks->ready_queues[priority].head, ks->ready_queues[priority].tail, ks->priority_mask);
	} else {
		ks->ready_queues[priority].head = head->next_ready_task;
		debug("ks->ready_queues[priority].head->tid = %d\n", ks->ready_queues[priority].head->tid);
	}
	return head;
}

int activate(task_descriptor *td, kernel_state *ks) {
	debug("in %s\n", "activate");
	ks->u_sp = td->sp;
	ks->u_lr = td->lr;
	td->state = STATE_ACTIVE;
	debug("ks->u_sp = 0x%x, ks->u_lr = 0x%x\n", ks->u_sp, ks->u_lr);	
	return asm_kernel_activate(td);
}

int main()
{
	// set up swi jump table 
	vint *swi_handle_entry = (vint*)0x28;
	debug("swi_handle_entry = 0x%x\n", swi_handle_entry);
	debug("asm_kernel_swiEntry = 0x%x\n", asm_kernel_swiEntry);
	*swi_handle_entry = (vint*)(asm_kernel_swiEntry + 0x218000);
	debug("swi_handle_entry = 0x%x\n", *swi_handle_entry);

	kernel_state ks;
	ks_initialize(&ks);

	uint8 tid = 0;

	td_intialize(first_task, &ks, tid++, INVALID_TID, PRIOR_LOW);
	
//	for(;;)
	{
			task_descriptor *td = schedule(&ks);
			debug("tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d\n",
					td->tid, td->state, td->priority, td->sp, td->lr, td->next_ready_task ? td->next_ready_task->tid : INVALID_TID);
			int req = activate(td, &ks);
			vint *first_arg= (vint*)0x9000000;
			vint *second_arg= (vint*)0x9000004;
			debug("get back into kernel again, req = %d\n", req);
			switch(req){
				case 1:
					k_create(*((int*)first_arg), *second_arg, tid++, td->tid, NULL, NULL);
					break;
				case 2:
					k_pass(td, &ks);
					break;
				case 3:
					k_my_tid(td);
					break;
			}
	}
	return 0;
}

