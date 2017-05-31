#include <name_server.h>
#include <clock_server.h>

static void initialize(Clock_server *cs){
	debug(DEBUG_CLOCK, "Enter %s", "clock_server_initialize");
    cs->ticks = 0;
	fifo_init(&(cs->delayed_task_queue)); // not sure about this
}

int DelayUntil( int ticks ){
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

int Delay( int ticks ){
	debug(DEBUG_CLOCK, "Enter %s", "Primitive Delay");
    if (ticks <= 0){
        return -2;
    }
    int clock_server_tid = WhoIs("CLOCK_SERVER");
    Clock_server_message send_msg;
    Clock_server_message reply_msg;
    send_msg.ticks = ticks;
    send_msg.type = DELAY_REQUEST;
    Send(clock_server_tid, &send_msg, sizeof(send_msg), &reply_msg, sizeof(reply_msg));
    return 0;
}

int Time( ){
	debug(DEBUG_CLOCK, "Enter %s", "Primitive TIME");
    int clock_server_tid = WhoIs("CLOCK_SERVER");
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

	int register_result = RegisterAs("CLOCK_SERVER");
	debug(DEBUG_CLOCK, "RegisterAs result = %d", register_result);

	Clock_server cs;
	initialize(&cs);

	while(1) {
		int requester;
		Delivery request;
        Clock_server_message reply_msg;
		Receive(&requester, &request, sizeof(request));

		debug(DEBUG_CLOCK, "request type= %d", request.type);

		Server_err result = SERVER_ERR_FAILURE;
		Name_server_message reply;
		switch(request.type) {
            case CLOCK_NOTIFIER:
                debug(DEBUG_CLOCK, "Enter %s", "CLOCK_NOTIFIER");
                cs.ticks++;
                Reply(requester, &reply_msg, sizeof(reply_msg));
                break;
			case TIME_REQUEST:
                debug(DEBUG_CLOCK, "Enter %s", "TIME_REQUESTER");
                reply_msg.ticks = cs.ticks;
                Reply(requester, &reply_msg, sizeof(reply_msg));
		 		break;
			case DELAY_REQUEST:
                debug(DEBUG_CLOCK, "Enter %s", "DELAY_REQUESTER");
                Delayed_task delayed_task;
                delayed_task.tid = requester;
                delayed_task.ticks = cs.ticks + request.ticks;
                // add the delay task to the structure
		 		break;
            case DELAY_REQUEST_UNTIL:
                debug(DEBUG_CLOCK, "Enter %s", "DELAY_REQUESTER_UNTIL");
                Delayed_task delayed_task;
                delayed_task.tid = requester;
                delayed_task.ticks = request.ticks;
                // add the delay task to the structure
		 		break;

		}
        // Reply to any timed-out tasks
        // confirm whether to use heap prior queue here
	}
}

void clock_server_notifier(){
    Delivery request; 
    Clock_server_message reply_message;
    int clock_server_tid = WhoIs("CLOCK_SERVER");
    while(1){
        request.data = AwaitEvent( evtType  ); // evtType = here should be clock update event;
        request.type = NOTIFIER;
        Send( clock_server_tid, &request, sizeof(request), &reply_message, sizeof(reply_message) );
    }
}
