#include <debug.h>
#include <kernel.h>
#include <math.h>
#include <time.h>
#include <irq.h>

extern void asm_print_sp();
extern void asm_kernel_swiEntry();
extern void asm_kernel_hwiEntry();
extern void asm_init_kernel();
extern int asm_kernel_activate(Task_descriptor *td);
extern int asm_get_spsr();
extern int asm_get_sp();
extern int asm_get_fp();

/* Initialze kernel state by setting task priority queue to be empty */
static void ks_initialize(Kernel_state *ks)
{
	ks->free_list = 0;
	ks->ready_queue.mask = 0;
	ks->send_block.mask = 0;
	ks->reply_block.mask = 0;
	ks->receive_block.mask = 0;
	int e = 0;
	for (e = 0; e < NUM_EVENTS; e++) {
		ks->event_blocks[e].mask = 0;
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
		for (e = 0; e < NUM_EVENTS; e++) {
			ks->event_blocks[e].fifos[i].head = NULL;
			ks->event_blocks[e].fifos[i].tail = NULL;
		}
	}
}

/* task descriptor */
void td_intialize(void (*task)(), Kernel_state *ks, uint32 tid, uint32 ptid, Task_priority priority)
{
	Task_descriptor *td = &(ks->tasks[tid]);
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

/* scheduler */
Task_descriptor *schedule(Kernel_state *ks)
{
	debug(DEBUG_SCHEDULER, "In %s", "schedule");
	return pull_highest_priority_task(&(ks->ready_queue));
}

void reschedule(Task_descriptor *td, Kernel_state *ks){
	debug(DEBUG_SYSCALL, "In %s", "k_reschedule");
	remove_task(td, &(ks->ready_queue));
	td->state = STATE_READY;
	insert_task(td, &(ks->ready_queue));
}

/* priority queue operations */
Task_descriptor *pull_highest_priority_task(Priority_fifo *ppriority_queue)
{
	debug(DEBUG_PRIOR_FIFO, "In %s", "pull_highest_priority_task");
	uint8 lz = clz(ppriority_queue->mask);
	uint8 priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));
	Task_descriptor *head = ppriority_queue->fifos[priority].head;
	debug(DEBUG_PRIOR_FIFO, "lz = %d, priority = %d, head->tid = %d", lz, priority, head->tid);
	return head;
}

void insert_task(Task_descriptor *td, Priority_fifo *ppriority_queue)
{
	Task_priority priority = td->priority;
	debug(DEBUG_PRIOR_FIFO, "In insert_task, start inserting td %d into fifo %d", td->tid, priority);
	if (ppriority_queue->mask & (0x1 << priority)) {
		// ready_queue is non-empty
		Task_descriptor *tail = ppriority_queue->fifos[priority].tail;
		tail->next_task = td;
		ppriority_queue->fifos[priority].tail = td;
		debug(DEBUG_PRIOR_FIFO, "inserted into the fifo, tail = %d", ppriority_queue->fifos[priority].tail->tid);
	} else {
		// ready_queue is empty
		ppriority_queue->fifos[priority].head = td;
		ppriority_queue->fifos[priority].tail = td;
		// set mask
		ppriority_queue->mask |= (0x1 << priority);
		debug(DEBUG_PRIOR_FIFO, "inserted into the empty fifo, mask = 0x%x, head = %d, tail = %d",
				ppriority_queue->mask,
				ppriority_queue->fifos[priority].head->tid, ppriority_queue->fifos[priority].tail->tid);
	}
}

int remove_task(Task_descriptor *td, Priority_fifo *ppriority_queue)
{
	Task_priority priority = td->priority;
	debug(DEBUG_PRIOR_FIFO, "In remove_task, start removing td %d from fifo %d, mask = 0x%x",
			td->tid, priority, ppriority_queue->mask);
	if ((ppriority_queue->mask & (0x1 << priority)) == 0) {
		debug(DEBUG_PRIOR_FIFO, "fifo %d is empty", priority);
		return -1;
	}
	uint8 is_found = 0;
    if((ppriority_queue->mask & (0x1 << priority)) == 0){
        debug(DEBUG_PRIOR_FIFO, "!!!!!!!!!!!!!!!!!!! %s", "priority queue is empty");
        return -1;
    }
	debug(DEBUG_PRIOR_FIFO, "!!!!!!!!!!!!!!!!!!!before *head %s", "in remove_task");
	Task_descriptor *head = ppriority_queue->fifos[priority].head;
	debug(DEBUG_PRIOR_FIFO, "!!!!!!!!!!!!!!!!!!!after *head= %d", head);
	if (td->next_task == NULL) {
		if (td == head) {
			// td is the only task on the fifo, empty the fifo
			ppriority_queue->fifos[priority].head = NULL;
			ppriority_queue->fifos[priority].tail = NULL;
			// unset corresponding bit in the mask
			ppriority_queue->mask &= (~(0x1 << priority));
			debug(DEBUG_PRIOR_FIFO, "removed the only td in fifo, head = tail = %d, mask = 0x%x",
					ppriority_queue->fifos[priority].head, ppriority_queue->mask);
			is_found = 1;
		} else {
			// td is the tail, need to find the task whose next_task is td
			Task_descriptor *iter = head;
			for (iter = head; iter->next_task != NULL; iter = iter->next_task) {
				if (iter->next_task == td) {
					is_found = 1;
					break;
				}
			}
			if (is_found) {
				iter->next_task = NULL;
				ppriority_queue->fifos[priority].tail = iter;
				debug(DEBUG_PRIOR_FIFO, "removed td after %d, %d is now tail",
						iter->tid, ppriority_queue->fifos[priority].tail->tid);
			}
			else {
				return -1;
			}
		}
	}
	else {
		if (td == head) {
			// td is the head, ready_queue has more than one tasks
			ppriority_queue->fifos[priority].head = td->next_task;
			td->next_task = NULL;
			debug(DEBUG_PRIOR_FIFO, "%d is old head, head is now %d",
					td->tid, ppriority_queue->fifos[priority].head->tid);
		}
		else {
			// td is in the middle, need to find the task whose next_task is td
			Task_descriptor *iter = head;
			for (iter = head; iter->next_task != NULL; iter = iter->next_task) {
				if (iter->next_task == td) {
					is_found = 1;
					break;
				}
			}
			if (is_found) {
				iter->next_task = td->next_task;
				td->next_task = NULL;
				debug(DEBUG_PRIOR_FIFO, "removed td after %d, next_task of %d is now %d",
						iter->tid, iter->tid, iter->next_task->tid);
			}
			else {
				return -1;
			}
		}
	}
	return 0;
}

int activate(Task_descriptor *td)
{
	td->state = STATE_ACTIVE;
	debug(DEBUG_TRACE, "In activate tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, retval=0x%x",
					td->tid, td->state, td->priority, td->sp, td->lr, td->retval);
	return asm_kernel_activate(td);
}

int find_sender(Priority_fifo *blocked_queue, int tid, Task_descriptor **psender)
{
    debug(DEBUG_MESSAGE, "within %s", "find_sender");
	uint8 lz = clz(blocked_queue->mask);
	uint8 highest_priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));
	/*debug(DEBUG_ITC, "start with highest non-empty fifo %d", highest_priority);*/

	uint8 is_found = 0;
	int priority = 0;
	for (priority = highest_priority; priority >= PRIOR_LOWEST; priority--) {
		if (!(blocked_queue->mask & (0x1 << priority))) {
			// fifo is empty
			continue;
		}

		Task_descriptor *iter = NULL;
		for (iter = blocked_queue->fifos[priority].head; iter != NULL; iter = iter->next_task) {
            vint iter_sp = iter->sp;
            int receiver =  *((vint*) (iter_sp + 0));
            debug(DEBUG_MESSAGE, "receiver value is = %d", receiver);
            debug(DEBUG_MESSAGE, "tid is = %d", tid);

			if (tid == receiver) {
                debug(DEBUG_MESSAGE, "there is a match %d", 100);
				is_found = 1;
				*psender = iter;
				break;
			}
		}
		if (is_found) {
			break;
		}
	}
	return (is_found == 1 ? 0 : -1);
}

/*
 * Update lr, sp, spsr in the td
 */
static void update_td(Task_descriptor *td, vint cur_lr)
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
	debug(DEBUG_TRACE, "cur_sp = 0x%x, cur_lr = 0x%x, cur_fp = 0x%x", cur_sp, cur_lr, cur_fp);
}

int main()
{
    bwsetfifo(COM2, OFF);
/*	timer_start();

    timer_start();
    asm volatile("MRC p15, 0, r2, c1, c0, 0");
    asm volatile("ORR r2, r2, #1<<12");
    asm volatile("ORR r2, r2, #1<<2");
    asm volatile("MCR p15, 0, r2, c1, c0, 0");
*/
	// set up swi jump table 
	vint *swi_handle_entry = (vint*) 0x28;
	debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", swi_handle_entry);
	debug(DEBUG_TRACE, "asm_kernel_swiEntry = 0x%x", asm_kernel_swiEntry);
	*swi_handle_entry = (vint) (asm_kernel_swiEntry + 0x218000);
	debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", *swi_handle_entry);

	// set up hwi jump table
	vint *hwi_handle_entry = (vint*) 0x38;
	debug(DEBUG_IRQ, "hwi_handle_entry = 0x%x", hwi_handle_entry);
	debug(DEBUG_IRQ, "asm_kernel_hwiEntry = 0x%x", asm_kernel_hwiEntry);
	*hwi_handle_entry = (vint) (asm_kernel_hwiEntry + 0x218000);
	debug(DEBUG_IRQ, "hwi_handle_entry = 0x%x", *hwi_handle_entry);

	Kernel_state ks;
	ks_initialize(&ks);

	uint8 tid = 0;
	td_intialize(first_task, &ks, tid++, INVALID_TID, PRIOR_MEDIUM);

	// enable irq
    irq_enable();

	while(ks.ready_queue.mask != 0) {
			debug(DEBUG_SCHEDULER, "mask =%d", ks.ready_queue.mask);
			Task_descriptor *td = schedule(&ks);

			debug(DEBUG_TRACE, "tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_task = %d",
					td->tid, td->state, td->priority, td->sp, td->lr,
					td->next_task ? td->next_task->tid : INVALID_TID);

			// retrieve lr and retrieve syscall request type
			vint cur_lr = activate(td);
			debug(DEBUG_IRQ, "get back into kernel again, cur_lr = 0x%x", cur_lr);
			int is_entry_from_hwi = 0;
			if (cur_lr & HWI_MASK) {
				// hwi entry bit is set, entered from hwi
				cur_lr = cur_lr & ~(HWI_MASK);
				is_entry_from_hwi = 1;
				debug(DEBUG_IRQ, "irq get back into kernel again, cur_lr = 0x%x, is_entry_from_hwi = %d", cur_lr, is_entry_from_hwi);
			}

			update_td(td, cur_lr);
			
			if (is_entry_from_hwi) {
				debug(DEBUG_IRQ, "is_entry_from_hwi = %d, start irq handling", is_entry_from_hwi);
				irq_handle(&ks);
				continue;
			}
 
			uint32 immed_24 = *((vint *)((int) td->lr - 4)) & ~(0xff000000);
			uint32 req = immed_24 & 0xfff;
			uint32 argc = (immed_24 >> 12) & 0xfff;
			debug(DEBUG_TRACE, "swi get back into kernel again, immed_24 = 0x%x, req = %d, argc = %d", immed_24, req, argc);

			vint arg0 = *((vint*) ((int) td->sp + 0));
			uint32 arg1 = *((vint*) ((int) td->sp + 4));
            uint32 arg2 = *((vint*) ((int) td->sp + 8));
            uint32 arg3 = *((vint*) ((int) td->sp + 12 )); 
            uint32 arg4 = *((vint*) ((int) td->sp + 4));
			debug(DEBUG_TRACE, "arg0 = %d, arg1 = %d", arg0, arg1);

			switch(req){
				case 1:		
					k_create(td, &ks, (void (*) ()) arg1, tid++, arg0);
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
                    k_send(arg0, arg1, arg2, arg3, arg4, td, &ks);
                    break;
                case 7:
                    k_receive(arg0, arg1, arg2, td, &ks);
                    break;
                case 8:
                    k_reply(arg0, arg1, arg2, td, &ks);
                    break;
				case 9:
					k_await_event(arg0, td, &ks);
					break;
			}
	}
/*    timer_stop(); */
	return 0;
}
