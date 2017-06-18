#include <clock_server.h>

static void initialize(Clock_server *cs)
{
	debug(DEBUG_CLOCK, "Enter %s", "clock_server_initialize");
    cs->ticks = 0;
}

int DelayUntil(int ticks)
{
	debug(DEBUG_CLOCK, "Enter %s", "Primitive DelayUntil");
    if (ticks < 0){
        return -2;
    }
    int clock_server_tid = WhoIs("CLOCK_SERVER");
    Clock_server_message send_msg;
    Clock_server_message reply_msg;
    send_msg.ticks = ticks;
    send_msg.type = DELAY_REQUEST_UNTIL;
    Send(clock_server_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    return 0;
}

int Delay(int ticks)
{
	/*debug(SUBMISSION, "Enter %s", "Primitive Delay");*/
    if (ticks <= 0){
        return -2;
    }
    vint clock_server_tid = WhoIs("CLOCK_SERVER");
    Delivery send_msg;
    Clock_server_message reply_msg;
    send_msg.data = ticks;
    send_msg.type = DELAY_REQUEST;
    /*debug(SUBMISSION,  "before send delay_message, clock server id=%d", clock_server_tid);*/
    Send(clock_server_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    return 0;
}

int Time()
{
	debug(SUBMISSION, "Enter %s", "Primitive TIME");
    vint clock_server_tid = WhoIs("CLOCK_SERVER");
    Clock_server_message send_msg;
    Clock_server_message reply_msg;
    send_msg.type = TIME_REQUEST;
    Send(clock_server_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
	debug(DEBUG_CLOCK, "current number of ticks are = %d", reply_msg.ticks);
    return reply_msg.ticks;
}

void clock_server_start()
{
	debug(DEBUG_CLOCK, "Enter %s", "clock_server_start");
	//debug(SUBMISSION, "Enter %s", "clock_server_start");

	vint register_result = RegisterAs("CLOCK_SERVER");
	Clock_server cs;
	initialize(&cs);

    // initialize heap to store delayed task 
	node_t delayed_tasks[MAX_DELAYED_TASKS];
	heap_t delay_h = heap_init(delayed_tasks, MAX_DELAYED_TASKS);

	while(1) {
		int requester;
		Delivery request;
        Clock_server_message reply_msg;
		Receive(&requester, &request, sizeof(request));

		/*debug(SUBMISSION, "request type= %d", request.type);*/
        vint tid;
		switch(request.type) {
            case CLOCK_NOTIFIER:
                /*debug(SUBMISSION, "%s", "CL");*/
                Reply(requester, &reply_msg, sizeof(reply_msg));
                //debug(DEBUG_UART_IRQ, "Enter %s", "CLOCK_NOTIFIER");
                cs.ticks++;
                //debug(DEBUG_UART_IRQ, "increment ticks = %d", cs.ticks);
                break;
			case TIME_REQUEST:
                reply_msg.ticks = cs.ticks;
                debug(SUBMISSION, "current ticks= %d", cs.ticks);
                Reply(requester, &reply_msg, sizeof(reply_msg));
		 		break;
			case DELAY_REQUEST:
                //debug(DEBUG_UART_IRQ, "Enter DELAY_REQUEST, requester = %d", requester);
                tid = requester;
                vint freedom_tick = cs.ticks + request.data;
                /*Clock_server_message reply_msg;*/
                /*Reply(tid, &reply_msg, sizeof(reply_msg));*/
                /*debug(SUBMISSION, "request_tick=%d", request.data);*/
                heap_insert(&delay_h, freedom_tick, (void*)tid);
		 		break;
            case DELAY_REQUEST_UNTIL:
                /*vint freedom_tick = cs.ticks + request.data;*/
                /*delayed_task.tid = requester;*/
                /*delayed_task.freedom_tick = request.data;*/
                /*heap_insert(&delay_h, delayed_task.freedom_tick, &delayed_task);*/
		 		break;
		}
        if(is_heap_empty(&delay_h)){
            continue; 
        }
        
        node_t root, del;
		root = heap_root(&delay_h);
        while(root.priority <= cs.ticks)
        {
            Clock_server_message reply_msg;
            vint tid = (vint)root.data;
			/*debug(SUBMISSION, "!!!!!!!!!! reply to %d", tid);*/
            Reply(tid, &reply_msg, sizeof(reply_msg));
            heap_delete(&delay_h, &del);
            /*int n;*/
            /*for(n=1; n<=delay_h.len; n++){*/
                /*debug(SUBMISSION, "heap is idx=%d", delay_h.nodes[n].priority);*/
            /*}*/
            if(is_heap_empty(&delay_h)){
                break;
            }
            root = heap_root(&delay_h);
        }
	}
}
