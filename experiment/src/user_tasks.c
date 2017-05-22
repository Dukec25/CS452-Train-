#include <kernel.h>
#include <bwio.h>
void general_task(){
	debug("In user task %s", "general_task");
    uint32 tid = MyTid();
    uint32 ptid = MyParentTid();
    debug("Get back to user task general_task before pass, tid = %d, ptid = %d", tid, ptid);
    Pass();
    debug("Get back to user task general_task after pass, tid = %d, ptid = %d", tid, ptid);
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
	debug("%s", "FirstUserTask: exiting");
    Exit();
}



