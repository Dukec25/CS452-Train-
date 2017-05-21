#include <bwio.h>
#include <kernel.h>
#include <define.h>
#include <math.h>

extern void asm_print_sp();
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
	// assign memory to the first task
	td->sp = (vint *) (TASK_START_LOCATION + (tid + 1) * TASK_SIZE); 
	// assign lr to point to the function pointer
	td->lr = (vint *)task + 0x218000 / 4;
#ifdef DEBUG_SPSR
	// set spsr to user mode
	td->spsr = USR;
#endif // DEBUG_SPSR
	// set next_ready_task
	td->next_ready_task = NULL;
	debug("tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d",
			ks->tasks[tid].tid, ks->tasks[tid].state, ks->tasks[tid].priority,
			ks->tasks[tid].sp, ks->tasks[tid].lr, ks->tasks[tid].next_ready_task);
	// insert td into the ready_queue
	debug("ks->priority_mask = 0x%x", ks->priority_mask);
	if (ks->priority_mask & (0x1 << priority)) {
		// ready_queue is non-empty
		task_descriptor *tail = ks->ready_queues[priority].tail;
		tail->next_ready_task = td;
		debug("old tail->tid = %d, old tail->next_ready_task->tid = %d", tail->tid, tail->next_ready_task->tid);
		ks->ready_queues[priority].tail = td;
		debug("new tail->tid = %d", ks->ready_queues[priority].tail->tid);
	} else {
		debug("old ks->priority_mask = 0x%x", ks->priority_mask);
		// ready_queue is empty
		ks->ready_queues[priority].head = td;
		ks->ready_queues[priority].tail = td;
		debug("new ks->ready_queues[priority].head->tid = %d, ks->ready_queues[priority].tail->tid = %d",
				ks->ready_queues[priority].head->tid, ks->ready_queues[priority].tail->tid);
		// set priority_mask
		ks->priority_mask |= (0x1 << priority);
		debug("new ks->priority_mask = 0x%x", ks->priority_mask);
	}
}

task_descriptor *schedule(kernel_state *ks) {
	debug("In %s", "schedule");
	uint8 lz = clz(ks->priority_mask);
	uint8 priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));
	task_descriptor *head = ks->ready_queues[priority].head;
	debug("lz = %d, priority = %d, head->tid = %d", lz, priority, head->tid);
	return head;
}

int activate(task_descriptor *td, kernel_state *ks) {
	td->state = STATE_ACTIVE;
	debug("In activate tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x\r\n",
					td->tid, td->state, td->priority, td->sp, td->lr);
	return asm_kernel_activate(td);
}

int main()
{
	// set up swi jump table 
	vint *swi_handle_entry = (vint*)0x28;
	debug("swi_handle_entry = 0x%x", swi_handle_entry);
	debug("asm_kernel_swiEntry = 0x%x", asm_kernel_swiEntry);
	*swi_handle_entry = (vint*)(asm_kernel_swiEntry + 0x218000);
	debug("swi_handle_entry = 0x%x", *swi_handle_entry);

	kernel_state ks;
	ks_initialize(&ks);

	uint8 tid = 0;

	td_intialize(first_task, &ks, tid++, INVALID_TID, PRIOR_LOW);

	int loop = 0;	
	for(loop = 0; loop < 4; loop++)
	{
			task_descriptor *td = schedule(&ks);
			debug("tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d",
					td->tid, td->state, td->priority, td->sp, td->lr, td->next_ready_task ? td->next_ready_task->tid : INVALID_TID);
			int req = activate(td, &ks);
			debug("get back into kernel again, req = %d", req);
			// retrieve td lr, sp, spsr
#ifdef	DEBUG_SPSR 
			asm volatile("mrs r0, spsr");	
			register uint32 cur_spsr asm("r0");
#endif // DEBUG_SPSR
			// enter system mode
			asm volatile("msr CPSR, %0" :: "I" (SYS));
			asm volatile("mov ip, sp");
			register uint32 cur_sp asm("ip");
			register uint32 cur_lr asm("lr");
			uint32 arg0 = *((vint*) (cur_sp + 0));
			uint32 arg1 = *((vint*) (cur_sp + 4));
			// get back to svc mode 
			asm volatile("msr CPSR, %0" :: "I" (SVC));
#ifdef DEBUG_SPSR
			debug("get back into kernel again, cur_sp = 0x%x, cur_lr = 0x%x, cur_spsr = 0x%x", cur_sp, cur_lr, cur_spsr);
#else
			debug("get back into kernel again, cur_sp = 0x%x, cur_lr = 0x%x", cur_sp, cur_lr);
#endif // DEBUG_SPSR
			// update td: sp, lr, spsr
			td->sp = cur_sp;
			td->lr = cur_lr;
#ifdef	DEBUG_SPSR 
			td->spsr = cur_spsr;
			debug("updated td, td->tid = %d, td->sp = 0x%x, td->lr = 0x%x, td->spsr = 0x%x", td->tid, td->sp, td->lr, td->spsr);
#else
			debug("updated td, td->tid = %d, td->sp = 0x%x, td->lr = 0x%x", td->tid, td->sp, td->lr);
#endif	// DEBUG_SPSR
			switch(req){
				case 1:
					debug("ready to call k_create, arg0 = 0x%x, arg1 = 0x%x", arg0, arg1);
					k_create(arg1, &ks, tid++, td->tid, arg0);
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
