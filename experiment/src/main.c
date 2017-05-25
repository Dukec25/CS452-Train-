#include <debug.h>
#include <kernel.h>
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
	// initialize tid, ptid, state, priority, and spsr
	td->tid = tid;
	td->ptid = ptid;
	td->state = STATE_READY;
	td->priority = priority;
	td->spsr = USR; // hardcoded to user mode, not flag bit set
	// assign memory to the first task
	td->sp = (vint *) (TASK_START_LOCATION + (tid + 1) * TASK_SIZE); 
	// assign lr to point to the function pointer
	td->lr = (vint *)task;
	td->lr = (vint *)task + 0x218000 / 4;
	// push lr and sp onto the user task
	*(td->sp - 12) = td->lr;
	*(td->sp - 11) = TASK_START_LOCATION + (tid + 1) * TASK_SIZE;
	// set next_ready_task
	td->next_ready_task = NULL;
	debug(DEBUG_TRACE, "tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d",
			ks->tasks[tid].tid, ks->tasks[tid].state, ks->tasks[tid].priority,
			ks->tasks[tid].sp, ks->tasks[tid].lr, ks->tasks[tid].next_ready_task);
	// insert td into the ready_queue
	debug(DEBUG_SCHEDULER, "ks->priority_mask = 0x%x", ks->priority_mask);
	if (ks->priority_mask & (0x1 << priority)) {
		// ready_queue is non-empty
		task_descriptor *tail = ks->ready_queues[priority].tail;
		tail->next_ready_task = td;
		debug(DEBUG_SCHEDULER, "old tail->tid = %d, old tail->next_ready_task->tid = %d",
			tail->tid, tail->next_ready_task->tid);
		ks->ready_queues[priority].tail = td;
		debug(DEBUG_SCHEDULER, "new tail->tid = %d", ks->ready_queues[priority].tail->tid);
	} else {
		debug(DEBUG_SCHEDULER, "old ks->priority_mask = 0x%x", ks->priority_mask);
		// ready_queue is empty
		ks->ready_queues[priority].head = td;
		ks->ready_queues[priority].tail = td;
		debug(DEBUG_SCHEDULER, "new ks->ready_queues[priority].head->tid = %d, .tail->tid = %d",
				ks->ready_queues[priority].head->tid, ks->ready_queues[priority].tail->tid);
		// set priority_mask
		ks->priority_mask |= (0x1 << priority);
		debug(DEBUG_SCHEDULER, "new ks->priority_mask = 0x%x", ks->priority_mask);
	}
}

task_descriptor *schedule(kernel_state *ks)
{
	debug(DEBUG_SCHEDULER, "In %s", "schedule");
	uint8 lz = clz(ks->priority_mask);
	uint8 priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));
	task_descriptor *head = ks->ready_queues[priority].head;
	debug(DEBUG_SCHEDULER, "lz = %d, priority = %d, head->tid = %d", lz, priority, head->tid);
	return head;
}

void insert_task(task_descriptor *td, kernel_state *ks)
{
	task_priority priority = td->priority;
	debug(DEBUG_SCHEDULER, "In insert_task, start inserting td %d into ready queue %d", td->tid, priority);
	if (ks->priority_mask & (0x1 << priority)) {
		// ready_queue is non-empty
		task_descriptor *tail = ks->ready_queues[priority].tail;
		tail->next_ready_task = td;
		ks->ready_queues[priority].tail = td;
		debug(DEBUG_SCHEDULER, "inserted into the ready queue, tail = %d", ks->ready_queues[priority].tail->tid);
	} else {
		// ready_queue is empty
		ks->ready_queues[priority].head = td;
		ks->ready_queues[priority].tail = td;
		// set priority_mask
		ks->priority_mask |= (0x1 << priority);
		debug(DEBUG_SCHEDULER, "inserted into the empty ready queue, priority_mask = 0x%x, head = %d, tail = %d",
				ks->priority_mask, ks->ready_queues[priority].head->tid, ks->ready_queues[priority].tail->tid);
	}
}

void remove_task(task_descriptor *td, kernel_state *ks)
{
	task_priority priority = td->priority;
	debug(DEBUG_SCHEDULER, "In remove_task, start removing td %d from ready queue %d, priority_mask = 0x%x",
			td->tid, priority, ks->priority_mask);
	task_descriptor *head = ks->ready_queues[priority].head;
	if (td->next_ready_task == NULL) {
		if (td == head) {
			// td is the only task on the ready queue, empty the ready queue
			ks->ready_queues[priority].head = NULL;
			ks->ready_queues[priority].tail = NULL;
			// unset corresponding bit in the priority_mask
			ks->priority_mask &= (~(0x1 << priority));
			debug(DEBUG_SCHEDULER, "removed the only td in ready queue, head = tail = %d, priority_mask = 0x%x",
					ks->ready_queues[priority].head, ks->priority_mask);
		} else {
			// td is the tail, need to find the task whose next_ready_task is td
			task_descriptor *iter = head;
			for (iter = head; iter->next_ready_task != NULL; iter = iter->next_ready_task) {
				if (iter->next_ready_task == td) {
					break;
				}
			}
			iter->next_ready_task = NULL;
			ks->ready_queues[priority].tail = iter;
			debug(DEBUG_SCHEDULER, "removed td after %d, %d is now tail",
					iter->tid, ks->ready_queues[priority].tail->tid);
		}
	}
	else {
		if (td == head) {
			// td is the head, ready_queue has more than one tasks
			ks->ready_queues[priority].head = td->next_ready_task;
			td->next_ready_task = NULL;
			debug(DEBUG_SCHEDULER, "%d is old head, head is now %d", td->tid, ks->ready_queues[priority].head->tid);
		}
		else {
			// td is in the middle, need to find the task whose next_ready_task is td
			task_descriptor *iter = head;
			for (iter = head; iter->next_ready_task != NULL; iter = iter->next_ready_task) {
				if (iter->next_ready_task == td) {
					break;
				}
			}
			iter->next_ready_task = td->next_ready_task;
			td->next_ready_task = NULL;
			debug(DEBUG_SCHEDULER, "removed td after %d, next_ready_task of %d is now %d",
					iter->tid, iter->tid, iter->next_ready_task->tid);
		}
	}
}

int activate(task_descriptor *td, kernel_state *ks)
{
	td->state = STATE_ACTIVE;
	debug(DEBUG_TRACE, "In activate tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, retval=0x%x",
					td->tid, td->state, td->priority, td->sp, td->lr, td->retval);
	return asm_kernel_activate(td);
}

int main()
{
    bwsetfifo(COM2, OFF);
	// set up swi jump table 
	vint *swi_handle_entry = (vint*)0x28;
	debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", swi_handle_entry);
	debug(DEBUG_TRACE, "asm_kernel_swiEntry = 0x%x", asm_kernel_swiEntry);
	*swi_handle_entry = (vint*)(asm_kernel_swiEntry + 0x218000);
	debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", *swi_handle_entry);

	kernel_state ks;
	ks_initialize(&ks);

	uint8 tid = 0;

	td_intialize(first_task, &ks, tid++, INVALID_TID, PRIOR_MEDIUM);

	vint loops;

    Block_queue send_block;

	while(ks.priority_mask != 0) { // this should be the correct one
			debug(DEBUG_SCHEDULER, "priority_mask =%d", ks.priority_mask);
			task_descriptor *td = schedule(&ks);
			debug(DEBUG_TRACE, "tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_ready_task = %d",
					td->tid, td->state, td->priority, td->sp, td->lr,
					td->next_ready_task ? td->next_ready_task->tid : INVALID_TID);
			// retrieve lr and retrieve syscall request type
			vint cur_lr = activate(td, &ks);
			int req = *((vint *)(cur_lr - 4)) & ~(0xff000000);
			debug(DEBUG_TRACE, "get back into kernel again, req = %d", req);
			// retrieve spsr
			asm volatile("mrs ip, spsr"); // assign spsr to ip
			register uint32 temp_spsr asm("ip");
			uint32 cur_spsr = temp_spsr;
			// retrieve sp and arg0 and arg1
			asm volatile("msr CPSR, %0" :: "I" (SYS)); // enter system mode
			asm volatile("mov ip, sp");
			register vint temp_sp asm("ip"); // extremly dangerous!!!, modify its value after the second read, holy cow waste so much time on this
			vint cur_sp = temp_sp;
			register vint temp_fp asm("fp");
			vint cur_fp = temp_fp;
			uint32 arg0 = *((vint*) (cur_sp + 0));
			uint32 arg1 = *((vint*) (cur_sp + 4));
            uint32 arg2 = *((vint*) (cur_sp + 8));
            uint32 arg3 = *((vint*) (cur_fp + 0)); // not too sure, need experiment
            uint32 arg4 = *((vint*) (cur_fp + 4));
			asm volatile("msr CPSR, %0" :: "I" (SVC)); // get back to svc mode 
			debug(DEBUG_TRACE, "cur_sp = 0x%x, cur_lr = 0x%x, cur_arg0 = 0x%x, cur_arg1 = 0x%x",
					cur_sp, cur_lr, arg0, arg1);
			// update td: sp, lr, spsr
			td->sp = cur_sp;
			td->lr = cur_lr;
			td->spsr = cur_spsr;
			switch(req){
				case 1:		
					k_create(arg1, &ks, tid++, td->tid, arg0);
					break;
				case 2:
					k_pass(td, &ks);
					break;
				case 3:
					k_my_tid(td, &ks);
					break;
				case 4:
					k_exit(td, &ks);
					break;
				case 5:
					k_my_parent_tid(td, &ks);
					break;
                case 6:
                    k_send(arg0, arg1, arg2, arg3, arg4, td, &ks, send_block);
                    break;
			}
	}
	return 0;
}
