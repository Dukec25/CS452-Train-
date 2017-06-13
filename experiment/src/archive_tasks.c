void time_receive(){
    int round;
    debug(DEBUG_TIME, "!!!!!!!!!enter %s", "time receive");
    int sender_tid;   
    /*vint reply_msg = 5;*/
    /*vint msg;*/
    char reply_msg[64];
    char msg[64];
    vint *ptimer = timer();
    int receive_time = 0;
    int reply_time = 0;
    for(round=0; round < 1000000; round++){
        /* debug(DEBUG_TIME, "!!!!!!!!!enter %s", "about to receive"); */
        receive_time += *ptimer;
        Receive( &sender_tid, &msg, sizeof(msg) );  
        receive_time -= *ptimer;
        /*debug(DEBUG_TIME, "sender_tid=%d, received_message=%d", sender_tid, msg);*/
        reply_time += *ptimer;
        vint reply_result = Reply(sender_tid, &reply_msg, sizeof(reply_msg));
        reply_time -= *ptimer;
    }
    debug(DEBUG_TIME, "receive time = %d, reply time = %d", receive_time, reply_time);
    Exit();
}

void time_send(){
    int round;
    /*vint msg = 5;*/
    /*vint reply_msg;*/
    char msg[64];
    char reply_msg[64];
    debug(DEBUG_TIME, "!!!!!!!!!!!enter %s", "time send");
    for(round=0; round < 1000000; round++){
        /*debug(DEBUG_TIME, "!!!!!!!!!!!enter %s", "about to send");*/
        vint send_result = Send(1, &msg, sizeof(msg), &reply_msg, sizeof(reply_msg));
    }    
    /*debug(DEBUG_TIME, "enter %s", "time task2");*/
    /*debug(DEBUG_TIME, "replied=%d", reply_four_bytes);*/
    vint *ptimer = timer();
    uint32 timer_output = TIMER_MAX - *ptimer;
    debug(DEBUG_TIME, "send time = %d", timer_output);
    Exit();
}

void kernel3_client_task(){
    vint parent_tid = MyParentTid();
    vint my_tid = MyTid();
    debug(SUBMISSION, "enter client task %d", my_tid);
    Message send_msg;
    IntIntMessage reply_msg;
    Send(parent_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    vint delayed_time_interval = reply_msg.content1;
    vint num_delays = reply_msg.content2;
    vint clock_server_tid = WhoIs("CLOCK_SERVER");
    /*debug(SUBMISSION, "clock_servertid =%d, delayed_time_interval=%d", clock_server_tid, delayed_time_interval);*/
    int n;
    for(n=0; n < num_delays; n++){
        debug(SUBMISSION, "enter %s", "user task delay");
        Delay(delayed_time_interval); 
        debug(SUBMISSION, "my tid = %d", my_tid);
        debug(SUBMISSION, "delayed_interval = %d", delayed_time_interval);
        debug(SUBMISSION, "number of delayed has completed = %d", n);
    }
    debug(SUBMISSION, "completed task ****= %d", MyTid());
    Exit();
}

void kernel3_task()
{
	int tid;
 	tid = Create(PRIOR_MEDIUM, kernel3_client_task); 
    debug(SUBMISSION, "created taskId = %d", tid);
    int sender_tid;
    Message receive_msg;
    Receive( &sender_tid, &receive_msg, sizeof(receive_msg) ); // should return value here later
    IntIntMessage reply_msg;
    reply_msg.content1 = 10;
    reply_msg.content2 = 20;
    debug(SUBMISSION, "reply to taskId = %d", sender_tid);
    Reply(sender_tid, &reply_msg, sizeof(reply_msg));

    tid = Create(PRIOR_MEDIUM, kernel3_client_task);
    debug(SUBMISSION, "created taskId = %d", tid);
    Receive( &sender_tid, &receive_msg, sizeof(receive_msg) ); // should return value here later
    reply_msg.content1 = 23;
    reply_msg.content2 = 9;
    Reply(sender_tid, &reply_msg, sizeof(reply_msg));

    tid = Create(PRIOR_MEDIUM, kernel3_client_task);
    debug(SUBMISSION, "created taskId = %d", tid);
    Receive( &sender_tid, &receive_msg, sizeof(receive_msg) ); // should return value here later
    reply_msg.content1 = 33;
    reply_msg.content2 = 6;
    Reply(sender_tid, &reply_msg, sizeof(reply_msg));

    tid = Create(PRIOR_MEDIUM, kernel3_client_task);
    debug(SUBMISSION, "created taskId = %d", tid);
    Receive( &sender_tid, &receive_msg, sizeof(receive_msg) ); // should return value here later
    reply_msg.content1 = 71;
    reply_msg.content2 = 3;
    Reply(sender_tid, &reply_msg, sizeof(reply_msg));
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

void send_task()
{
	debug(DEBUG_TASK, "enter %s", "send task");
    Message send_msg;
    Message reply_msg;
    uint32 tid = MyTid();
    debug(DEBUG_TASK, "this send task tid = %d", tid);
    send_msg.tid = tid;
    Memcpy(&send_msg.content, "hello", sizeof("hello"));
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
    Message reply_msg;
    Memcpy(&reply_msg.content, "I am great, wanna have sashimi together???", sizeof("I am great, wanna have sashimi together???"));
    vint reply_result = Reply(sender_tid, &reply_msg, sizeof(reply_msg));
    Exit();
}

