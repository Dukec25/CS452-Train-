#include <kernel.h>
#include <bwio.h>
void general_task(){
	debug("In user task %s", "general_task");
	uint32 tid = MyTid();
	debug("Get back to user task general_task, tid = %d", tid);
}

void first_task(){
	debug("In user task first_task, priority = %d, code = 0x%x", PRIOR_HIGH, general_task);
    /*int tid = Create(PRIOR_HIGH, general_task);*/
	uint32 tid = MyTid();
	debug("In user task first_task, tid = %d", tid);
}



