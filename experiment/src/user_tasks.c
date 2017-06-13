#include <user_functions.h>
#include <debug.h>
#include <string.h>
#include <name_server.h>
#include <rps.h>
#include <clock_server.h>
#include <io_server.h>
#include <train_task.h>

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

void name_server_task()
{
	debug(DEBUG_TASK, "enter %s", "name_server_task");
    uint32 tid = MyTid();
    debug(DEBUG_TASK, "starting name_server_task tid = %d", tid); 
	name_server_start();
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

void clock_server_task(){
	debug(DEBUG_UART_IRQ, "enter %s", "clock_server_task");
    uint32 tid = MyTid();
    debug(DEBUG_UART_IRQ, "starting clock_server_task tid = %d", tid); 
	clock_server_start();
	debug(DEBUG_UART_IRQ, "tid =%d exiting", tid);
    Exit();
}

void clock_server_notifier(){
	debug(DEBUG_UART_IRQ, "enter %s", "clock_server_notifier");
    Delivery request; 
    Clock_server_message reply_message;
    vint clock_server_tid = WhoIs("CLOCK_SERVER");
    while(1){
        /*debug(DEBUG_UART_IRQ, "before enter %s", "awaitEvent");*/
        request.data = AwaitEvent(TIMER3_RDY, -1); // evtType = here should be clock update event;
        /*debug(DEBUG_UART_IRQ, "after enter awaitEvent, send to clock_server_tid = %d", clock_server_tid);*/
        request.type = CLOCK_NOTIFIER;
        Send(clock_server_tid, &request, sizeof(request), &reply_message, sizeof(reply_message));
        /*debug(SUBMISSION, "after enter %s", "clock notify");*/
    }
}

void idle_task()
{
	debug(DEBUG_UART_IRQ, "enter %s", "idle_task");
//	debug(SUBMISSION, "%s", "idle_task");
    uint32 tid = MyTid();

	int i, j = 0;
    while(1){
        /*Putc(COM2, 'A');*/
        /*irq_printf(COM2, "golden retriever is the best\r\n");*/
        /*debug(SUBMISSION, "%s", "idle_task");*/
	/*for (i = 0; i < 300000; i++) {*/
        /*debug(SUBMISSION, "i = %d", i);*/
        /*Pass();*/
		/*j += 2;*/
	}
	/*debug(DEBUG_TASK, "j = %d, tid =%d exiting", j, tid);*/
    /*Exit();*/
}

void io_test_task(){
	int i = 0;
    /*debug(100, "!!!!!!!!!!!!!!!!!!before printint irq sentence%s", "% ");*/
    
    for( i = 0; i < 10000; i++ ){
        debug(SUBMISSION, "%s", "about to irq_printf");
        irq_printf(COM2, "golden retriever is the best%d\r\n", i);
        /*irq_printf(COM2, "%d\r\n", i);*/
    }
	/*for (i = 0; i < 255; i++) {*/
   		// Putc(0, 'a');
        /*char val = Getc(COM2);*/
		/*debug(DEBUG_UART_IRQ, "return from Getc, receive %d", val);*/
        /*Putc(COM1, i);*/
//		uart1_irq_soft();
	/*}*/
	Exit();
}

void uart1_rcv_server(){
    uint32 tid = MyTid();
    debug(DEBUG_UART_IRQ, "starting io_server_task tid = %d", tid); 
	io_server_receive_start(COM1);
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void uart2_rcv_server(){
    uint32 tid = MyTid();
    debug(DEBUG_UART_IRQ, "starting io_server_task tid = %d", tid); 
	io_server_receive_start(COM2);
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void uart1_xmit_server(){
    uint32 tid = MyTid();
    debug(DEBUG_UART_IRQ, "starting io_server_task tid = %d", tid); 
	io_server_transmit_start(COM1);
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void uart2_xmit_server(){
    uint32 tid = MyTid();
    debug(DEBUG_UART_IRQ, "starting io_server_task tid = %d", tid); 
	io_server_transmit_start(COM2);
	debug(DEBUG_TASK, "tid =%d exiting", tid);
    Exit();
}

void uart1_rcv_notifier(){
    debug(DEBUG_UART_IRQ, "enter = %s", "rcv_notifier");
	int io_server_id = WhoIs("IO_SERVER_UART1_RECEIVE");
    Delivery request;
    request.type = RECEIVE_RDY;
    Delivery reply_msg;
    while(1){
        request.data = AwaitEvent(RCV_UART1_RDY, -1); 
        Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg) );
        debug(DEBUG_UART_IRQ, "receive_notifer get awaked= %s", "");
    }
}

void uart2_rcv_notifier(){
    debug(DEBUG_UART_IRQ, "enter = %s", "rcv_notifier");
	int io_server_id = WhoIs("IO_SERVER_UART2_RECEIVE");
    Delivery request;
    request.type = RECEIVE_RDY;
    Delivery reply_msg;
    while(1){
        request.data = AwaitEvent(RCV_UART2_RDY, -1); 
        Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg) );
        debug(DEBUG_UART_IRQ, "uart2 receive_notifer get awaked", "");
    }
}

void uart1_xmit_notifier(){
    debug(DEBUG_UART_IRQ, "enter = %s", "xmit_notifier");
	int io_server_id = WhoIs("IO_SERVER_UART1_TRANSMIT");
    Delivery request;
    request.type = TRANSMIT_RDY;
    Delivery reply_msg;
    while(1) {
        Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
		debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
        AwaitEvent(XMIT_UART1_RDY, reply_msg.data);
		debug(DEBUG_UART_IRQ, "wake up from %s", "XMIT_RDY");
    }
}

void uart2_xmit_notifier(){
    debug(DEBUG_UART_IRQ, "enter = %s", "xmit_notifier");
	int io_server_id = WhoIs("IO_SERVER_UART2_TRANSMIT");
    Delivery request;
    request.type = TRANSMIT_RDY;
    Delivery reply_msg;
    while(1) {
        Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
		debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
        AwaitEvent(XMIT_UART2_RDY, reply_msg.data);
		debug(DEBUG_UART_IRQ, "wake up from %s", "XMIT_RDY");
    }
}

void uart1_xmit_enable(){
    int tid;
    tid = Create(PRIOR_HIGH, uart1_xmit_server);
    debug(DEBUG_TASK, "created taskId = %d", tid);

    tid = Create(PRIOR_HIGH, uart1_xmit_notifier);
    debug(DEBUG_TASK, "created taskId = %d", tid);
}

void uart1_rcv_enable(){
    int tid;
    tid = Create(PRIOR_HIGH, uart1_rcv_server);
    debug(DEBUG_TASK, "created taskId = %d", tid);

    tid = Create(PRIOR_HIGH, uart1_rcv_notifier);
    debug(DEBUG_TASK, "created taskId = %d", tid);
}

void uart2_xmit_enable(){
    int tid;
    tid = Create(PRIOR_HIGH, uart2_xmit_server);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_HIGH, uart2_xmit_notifier);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);
}

void uart2_rcv_enable(){
    int tid;
    tid = Create(PRIOR_HIGH, uart2_rcv_server);
    debug(DEBUG_TASK, "created taskId = %d", tid);

    tid = Create(PRIOR_HIGH, uart2_rcv_notifier);
    debug(DEBUG_TASK, "created taskId = %d", tid);
}

void irq_io_tasks_cluster(){
    uart2_xmit_enable();
    uart1_xmit_enable();
    uart2_rcv_enable();
    uart1_rcv_enable();
}

void first_task()
{
	// debug(DEBUG_UART_IRQ, "In user task first_task, priority=%d", PRIOR_MEDIUM);
    int tid;

	tid = Create(PRIOR_HIGH, name_server_task);
	debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    irq_io_tasks_cluster();

    /*tid = Create(PRIOR_MEDIUM, io_test_task);*/
    /*debug(DEBUG_TASK, "created taskId = %d", tid);*/

    tid = Create(PRIOR_LOWEST, idle_task);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_HIGH, clock_server_task);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_MEDIUM, clock_server_notifier);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_LOW, clock_task);
    debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_LOW, train_task);
   	debug(DEBUG_UART_IRQ, "created taskId = %d", tid);

    tid = Create(PRIOR_LOW, sensor_task);
    debug(DEBUG_UART_IRQ, "create taskId = %d", tid);

    /*debug(SUBMISSION, "%s", "FirstUserTask: exiting");*/
	Exit();
}
