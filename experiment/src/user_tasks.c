#include <kernel.h>
#include <bwio.h>
void general_task(){
	debug("In user task %s", "general_task");
	//uint32 tid = MyTid();
	debug("Get back to user task general_task, tid = %d", 666);
    Exit();
}

void first_task(){
	debug("In user task first_task, priority=%d", PRIOR_MEDIUM);
    int tid = Create(PRIOR_LOW, general_task);
	debug("created taskId = %d", tid);
    tid = Create(PRIOR_LOW, general_task);
	debug("created taskId = %d", tid);
    /*tid = Create(PRIOR_HIGH, general_task);*/
	/*debug("created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, general_task);*/
	/*debug("created taskId = %d", tid);*/
	/*uint32 tid = MyTid();*/
	debug("In user task first_task, tid = %d", tid);
    Exit();
}



