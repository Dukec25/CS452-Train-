#include <kernel.h>
#include <user_functions.h>
#include <debug.h>

void general_task()
{
	debug(DEBUG_TASK, "In user task %s", "general_task");
	uint32 tid = MyTid();
	uint32 ptid = MyParentTid();
	debug(KERNEL1, "Get back to user task general_task, 1st print, tid = %d, ptid = %d", tid, ptid);
	Pass();
	debug(KERNEL1, "Get back to user task general_task, 2nd print, tid = %d, ptid = %d", tid, ptid);
	Exit();
}

void special_task()
{
    Message send_msg;
    send_msg.tid = 100;
    memcpy(&send_msg.content, "hello", 6);
    Message reply_msg;
    memcpy(&reply_msg.content, "hi", 3);
    Send(100, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
}

void first_task()
{
	debug(DEBUG_TASK, "In user task first_task, priority=%d", PRIOR_MEDIUM);
	int tid = Create(PRIOR_LOW, special_task);
	debug(KERNEL1, "created taskId = %d", tid);
	/*int tid = Create(PRIOR_LOW, general_task);*/
	/*debug(KERNEL1, "created taskId = %d", tid);*/
	/*tid = Create(PRIOR_LOW, general_task);*/
	/*debug(KERNEL1, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, general_task);*/
    /*debug(KERNEL1, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, general_task);*/
    /*debug(KERNEL1, "created taskId = %d", tid);*/
	/*debug(KERNEL1, "%s", "FirstUserTask: exiting");*/
	Exit();
}
