#include <kernel.h>
#include <math.h>
#include <debug.h>

/* scheduler */
Task_descriptor *schedule(Kernel_state *ks)
{
	debug(DEBUG_TRACE, "In %s", "schedule");
	return pull_highest_priority_task(&(ks->ready_queue));
}

void reschedule(Task_descriptor *td, Kernel_state *ks){
	debug(DEBUG_PRIOR_FIFO, "In %s", "k_reschedule");
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

int find_sender(Priority_fifo *blocked_queue, int tid, Task_descriptor **psender)
{
    debug(DEBUG_PRIOR_FIFO, "within %s", "find_sender");
	uint8 lz = clz(blocked_queue->mask);
	uint8 highest_priority = PRIOR_HIGH - (lz - (32 - PRIOR_HIGH - 1));

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
            debug(DEBUG_PRIOR_FIFO, "receiver value is = %d", receiver);
            debug(DEBUG_PRIOR_FIFO, "tid is = %d", tid);

			if (tid == receiver) {
                debug(DEBUG_PRIOR_FIFO, "there is a match %d", 100);
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
