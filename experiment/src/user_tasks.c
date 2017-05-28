#include <kernel.h>
#include <user_functions.h>
#include <debug.h>
#include <string.h>
#include <name_server.h>

#define TIMER_MAX	0xFFFFFFFF

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
    debug(DEBUG_TRACE, "entering send %s", "yes!!!");
    vint send_result = Send(1, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    debug(DEBUG_TRACE, "unblock from reply_block, result = %d", send_result);
	debug(DEBUG_TRACE, "replied_message=%s", reply_msg.content);

	debug(DEBUG_TRACE, "tid =%d exiting", tid);
    Exit();
}

void receive_task()
{
	debug(DEBUG_TRACE, "enter %s", "receive task");
    int sender_tid; // why, why, why????????? Compare with previous version 
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
    vint reply_result = Reply(sender_tid, &reply_msg, sizeof(reply_msg));
    Exit();
}

void name_server_task()
{
	debug(DEBUG_TASK, "enter %s", "name_server_task");
    uint32 tid = MyTid();
    debug(DEBUG_TASK, "starting name_server_task tid = %d", tid); 
	name_server_start();
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void name_client_task1()
{
	debug(DEBUG_TASK, "enter %s", "name_client_task1");
    uint32 tid = MyTid();

	int result = 0;
	// expected 0
    debug(DEBUG_TASK, "starting RegisterAs Apple1 tid = %d, size = %d", tid, sizeof("Apple1")); 
	result = RegisterAs("Apple1");
	debug(DEBUG_TASK, "RegisterAs Apple1 result = %d", result);

	// expected 16
    debug(DEBUG_TASK, "starting RegisterAs Apple1 tid = %d, size = %d, again", tid, sizeof("Apple1")); 
	result = RegisterAs("Apple1");
	debug(DEBUG_TASK, "RegisterAs Apple1 result = %d", result); 
    
	// expected 0
	debug(DEBUG_TASK, "starting RegisterAs Apple2 tid = %d", tid); 
	result = RegisterAs("Apple2");
	debug(DEBUG_TASK, "RegisterAs Apple2 result = %d", result);	

	// expected 18
    debug(DEBUG_TASK, "starting RegisterAs Apple3 tid = %d", tid); 
	debug(DEBUG_TASK, "starting registeras_task tid = %d", tid); 
	result = RegisterAs("Apple3");
	debug(DEBUG_TASK, "RegisterAs Apple3 result = %d", result);	

	// expected 2
	debug(DEBUG_TASK, "starting WhoIs Apple1 tid = %d", tid); 
	result = WhoIs("Apple1");
	debug(DEBUG_TASK, "WhoIs Apple1 result = %d", result);

	// expected 2
	debug(DEBUG_TASK, "starting WhoIs Apple2 tid = %d", tid); 
	result = WhoIs("Apple2");
	debug(DEBUG_TASK, "WhoIs Apple2 result = %d", result);

	// expected 17
	debug(DEBUG_TASK, "starting WhoIs Apple3 tid = %d", tid); 
	result = WhoIs("Apple3");
	debug(DEBUG_TASK, "WhoIs Apple3 result = %d", result);

	debug(DEBUG_TASK, "tid = %d exiting", tid);
    Exit();
}

void name_client_task2()
{
	debug(DEBUG_TASK, "enter %s", "name_client_task2");
    uint32 tid = MyTid();

	int result = 0;
    debug(DEBUG_TASK, "starting RegisterAs Orange1 tid = %d", tid); 
	result = RegisterAs("Orange1");
	debug(DEBUG_TASK, "RegisterAs Orange1 result = %d", result);

	// expected 16
	 debug(DEBUG_TASK, "starting RegisterAs Orange1 tid = %d again", tid); 
	result = RegisterAs("Orange1");
	debug(DEBUG_TASK, "RegisterAs Orange1 result = %d", result);

	// expected 0
    debug(DEBUG_TASK, "starting RegisterAs Orange2 tid = %d", tid); 
	result = RegisterAs("Orange2");
	debug(DEBUG_TASK, "RegisterAs Orange2 result = %d", result);	

	// expected 18
    debug(DEBUG_TASK, "starting RegisterAs Orange3 tid = %d", tid); 
	debug(DEBUG_TASK, "starting registeras_task tid = %d", tid); 
	result = RegisterAs("Orange3");
	debug(DEBUG_TASK, "RegisterAs Orange3 result = %d", result);	

	// expected 3
	debug(DEBUG_TASK, "starting WhoIs Orange1 tid = %d", tid); 
	result = WhoIs("Orange1");
	debug(DEBUG_TASK, "WhoIs Orange1 result = %d", result);

	// expected 3
	debug(DEBUG_TASK, "starting WhoIs Orange2 tid = %d", tid); 
	result = WhoIs("Orange2");
	debug(DEBUG_TASK, "WhoIs Orange2 result = %d", result);

	// expected 17
	debug(DEBUG_TASK, "starting WhoIs Orange3 tid = %d", tid); 
	result = WhoIs("Orange3");
	debug(DEBUG_TASK, "WhoIs Orange3 result = %d", result);

	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void time_receive(){
    int round;
    /*debug(DEBUG_TIME, "enter %s", "time receive");*/
    int sender_tid;   
    /*vint reply_msg = 5;*/
    /*vint msg;*/
    char reply_msg[64];
    char msg[64];
    for(round=0; round < 1000; round++){
        Receive( &sender_tid, &msg, sizeof(msg) );  
        /*debug(DEBUG_TIME, "sender_tid=%d, received_message=%d", sender_tid, msg);*/
        vint reply_result = Reply(sender_tid, &reply_msg, sizeof(reply_msg));
    }
    Exit();
}

void time_send(){
    int round;
    /*vint msg = 5;*/
    /*vint reply_msg;*/
    char msg[64];
    char reply_msg[64];
    /*debug(DEBUG_TIME, "!!!!!!!!!!!enter %s", "time send");*/
    for(round=0; round < 1000; round++){
        vint send_result = Send(1, &msg, sizeof(msg), &reply_msg, sizeof(reply_msg));
    }    
    /*debug(DEBUG_TIME, "enter %s", "time task2");*/
    /*debug(DEBUG_TIME, "replied=%d", reply_four_bytes);*/
	vint *ptimer = timer();
	uint32 timer_output = TIMER_MAX - *ptimer;
    debug(DEBUG_TIME, "time = %d", timer_output);
    Exit();
}

void first_task()
{
	debug(DEBUG_TASK, "In user task first_task, priority=%d", PRIOR_LOW);
    int tid = Create(PRIOR_HIGH, time_receive); 
    debug(DEBUG_TASK, "created taskId = %d", tid);
    tid = Create(PRIOR_MEDIUM, time_send);
    debug(DEBUG_TASK, "created taskId = %d", tid);
    /*int tid = Create(PRIOR_HIGH, name_server_task);  // comment out for now to test generalized priority queue*/
    /*debug(DEBUG_TASK, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, name_client_task1);*/
    /*debug(DEBUG_TASK, "created taskId = %d", tid);*/
    /*tid = Create(PRIOR_HIGH, name_client_task2);*/
    /*debug(DEBUG_TASK, "created taskId = %d", tid);*/
    /* int tid = Create(PRIOR_HIGH, send_task); */  // comment out for now to test generalized priority queue
    /* debug(DEBUG_TRACE, "created taskId = %d", tid); */
    /* tid = Create(PRIOR_HIGH, receive_task); */
    /* debug(DEBUG_TRACE, "created taskId = %d", tid); */
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
