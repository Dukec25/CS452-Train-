#include <kernel.h>
#include <user_functions.h>
#include <debug.h>
#include <string.h>
#include <name_server.h>
#include <rps.h>
#include <clock_server.h>

#define TIMER_MAX	0xFFFFFFFF

void general_task()
{ 
	debug(DEBUG_TASK, "In user task %s", "general_task");
	uint32 tid = MyTid();
	uint32 ptid = MyParentTid();
	debug(SUBMISSION, "Get back to user task general_task, 1st print, tid = %d, ptid = %d", tid, ptid);
	Pass();
	debug(SUBMISSION, "Get back to user task general_task, 2nd print, tid = %d, ptid = %d", tid, ptid);
	Exit();
}

void send_task()
{
	debug(DEBUG_TASK, "enter %s", "send task");
    Message send_msg;
    Message reply_msg;
    uint32 tid = MyTid();
    debug(DEBUG_TASK, "this send task tid = %d", tid);
    send_msg.tid = tid;
    memcpy(&send_msg.content, "hello", sizeof("hello"));
	debug(DEBUG_TASK, "send_msg.content = %s", send_msg.content);
    debug(DEBUG_TASK, "entering send %s", "yes!!!");
    vint send_result = Send(1, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    debug(DEBUG_TASK, "unblock from reply_block, result = %d", send_result);
	debug(DEBUG_TASK, "replied_message=%s", reply_msg.content);

	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void receive_task()
{
	debug(DEBUG_TASK, "enter %s", "receive task");
    int sender_tid; // why, why, why????????? Compare with previous version 
    Message msg;
	uint32 tid = MyTid();
	debug(DEBUG_TASK, "this receive_task tid = %d", tid);
    msg.tid = tid;
    Receive( &sender_tid, &msg, sizeof(msg) ); // should return value here later
	debug(DEBUG_TASK, "sender_tid=%d, received_message=%s", sender_tid, msg.content);
	debug(DEBUG_TASK, "receive task id= %d, prepare to reply task id= %d", tid, sender_tid);
    char message[] = "I am great, wanna have sashimi together???";
    /*char message[128];*/
    /*char* first_aligned = &message + 8 - (&message % 8);*/
    Message reply_msg;
    memcpy(&reply_msg.content, "I am great, wanna have sashimi together???", sizeof("I am great, wanna have sashimi together???"));
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

void clock_server_task(){
	debug(DEBUG_TASK, "enter %s", "clock_server_task");
    uint32 tid = MyTid();
    debug(DEBUG_TASK, "starting clock_server_task tid = %d", tid); 
	clock_server_start();
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void clock_server_notifier(){
	debug(DEBUG_TASK, "enter %s", "clock_server_notifier");
    Delivery request; 
    Clock_server_message reply_message;
    vint clock_server_tid = WhoIs("CLOCK_SERVER");
    while(1){
        debug(DEBUG_TASK, "before enter %s", "awaitEvent");
        request.data = AwaitEvent(0); // evtType = here should be clock update event;
        debug(DEBUG_TASK, "after enter %s", "awaitEvent");
        request.type = CLOCK_NOTIFIER;
        Send( clock_server_tid, &request, sizeof(request), &reply_message, sizeof(reply_message) );
        debug(SUBMISSION, "after enter %s", "clock notify");
    }
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

void rps_server_task()
{
	debug(DEBUG_TASK, "enter %s", "rps_server_task");
    uint32 tid = MyTid();

    debug(DEBUG_TASK, "starting rps_server_start %d", tid); 
	rps_server_start();

	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void rps_client_task()
{
	debug(DEBUG_TASK, "enter %s", "rps_client_task");
    uint32 tid = MyTid();

    debug(DEBUG_TASK, "starting rps_client_start %d", tid); 
	rps_client_start();

	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void event_task()
{
	debug(DEBUG_TASK, "enter %s", "event_task");
    uint32 tid = MyTid();

    debug(DEBUG_TASK, "starting event_task %d", tid); 
	int value = AwaitEvent(0);

	debug(DEBUG_TASK, "value = %d, tid =%d exiting", value, tid);
    Exit();
}

void idle_task()
{
	debug(DEBUG_TASK, "enter %s", "idle_task");
    uint32 tid = MyTid();

	int i, j = 0;
	for (i = 0; i < 1000; i++) {
		debug(DEBUG_TASK, "***********************************i = %d", i);
		j += 2;
	}
	debug(DEBUG_TASK, "j = %d, tid =%d exiting", j, tid);
    Exit();
}

/*void kernel3_client_task(){*/
    /*vint parent_tid = MyParentTid();*/
    /*vint my_tid = MyTid();*/
    /*Message send_msg;*/
    /*IntIntMessage reply_msg;*/
    /*Send(parent_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));*/
    /*vint delay_time_interval = reply_msg.content1;*/
    /*vint num_delays = reply_msg.content2;*/
    /*vint clock_server_tid = WhoIs("CLOCK_SERVER");*/
    /*int n;*/
    /*for(n=0; n < num_delays; n++){*/
        /*Delay(delay_time_interval); */
        /*debug(SUBMISSION, "my tid = %d", my_tid);*/
        /*debug(SUBMISSION, "delayed_interval = %d", delayed_time_interval);*/
        /*debug(SUBMISSION, "number of delayed has completed = %d", n);*/
    /*}*/
/*}*/

void first_task()
{
	debug(DEBUG_TASK, "In user task first_task, priority=%d", PRIOR_MEDIUM);
	/*debug(DEBUG_TASK, "trigger timer_irq_sort(), priority=%d", PRIOR_MEDIUM);*/
    /*timer_irq_soft();*/
	/*timer_irq_soft_clear();*/
    int tid = Create(PRIOR_MEDIUM, name_server_task); 
    debug(DEBUG_TASK, "created taskId = %d", tid);

	tid = Create(PRIOR_MEDIUM, clock_server_task);
    debug(DEBUG_TASK, "created taskId = %d", tid);

	tid = Create(PRIOR_HIGH, clock_server_notifier);
    debug(DEBUG_TASK, "created taskId = %d", tid);

	tid = Create(PRIOR_LOWEST, idle_task);
    debug(DEBUG_TASK, "created taskId = %d", tid);

	debug(SUBMISSION, "%s", "FirstUserTask: exiting");
	Exit();
}
