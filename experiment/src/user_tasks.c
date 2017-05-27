#include <kernel.h>
#include <user_functions.h>
#include <debug.h>
#include <string.h>

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

void send_task()
{
	debug(DEBUG_TRACE, "enter %s", "send task");
    Message send_msg;
    Message reply_msg;
    uint32 tid = MyTid();
    debug(DEBUG_TRACE, "this send task tid = %d", tid);
    send_msg.tid = tid;
    char message[] = "hello"; 
    memcpy(&send_msg.content, message, sizeof(message));
    debug(DEBUG_TRACE, "is memcpy has any issue %d", tid);
    /*debug(DEBUG_TRACE, "tid=%d, message=%s, msgLength=%d, replyMsg=%s, replyMsgLen=%d", 2, send_msg.content, sizeof(send_msg), reply_msg.content, sizeof(reply_msg));*/
    debug(DEBUG_TRACE, "entering send %s", "yes!!!");
    Send(2, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    debug(DEBUG_TRACE, "unblock from reply_block%s", " yes, this what I want");
	debug(DEBUG_TRACE, "replied_message=%s", reply_msg.content);

	debug(DEBUG_TRACE, "tid =%d exiting", tid);
    Exit();
}

void receive_task()
{
	debug(DEBUG_TRACE, "enter %s", "receive task");
    int sender_tid; // why, why, why????????? Compare with previous version 
    // why can't I pass in *sender_tid 
    Message msg;
	uint32 tid = MyTid();
	debug(DEBUG_TRACE, "this receive_task tid = %d", tid);
    msg.tid = tid;
    Receive( &sender_tid, &msg, sizeof(msg) ); // should return value here later
	debug(DEBUG_TRACE, "sender_tid=%d, received_message=%s", sender_tid, msg.content);
	debug(DEBUG_TRACE, "receive task id= %d, prepare to reply task id= %d", tid, sender_tid);
    char message[] = "I am great, wanna have sashimi together???";
    Message reply_msg;
    memcpy(&reply_msg.content, message, sizeof(message));
    Reply(sender_tid, &reply_msg, sizeof(reply_msg));
    Exit();
}

void first_task()
{
	debug(DEBUG_TASK, "In user task first_task, priority=%d", PRIOR_MEDIUM);
     int tid = Create(PRIOR_HIGH, send_task);  // comment out for now to test generalized priority queue
     debug(DEBUG_TRACE, "created taskId = %d", tid);
     tid = Create(PRIOR_HIGH, receive_task);  
     debug(DEBUG_TRACE, "created taskId = %d", tid);
	/*int tid = Create(PRIOR_LOW, general_task);*/
	/*debug(KERNEL1, "created taskId = %d", tid);*/
	/*tid = Create(PRIOR_LOW, general_task);*/
	/*debug(KERNEL1, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, general_task);*/
    /*debug(KERNEL1, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, general_task);*/
    /*debug(KERNEL1, "created taskId = %d", tid);*/
	debug(KERNEL1, "%s", "FirstUserTask: exiting");
	Exit();
}
