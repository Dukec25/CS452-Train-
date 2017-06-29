#include <user_functions.h>
#include <debug.h>
#include <string.h>
#include <name_server.h>
#include <rps.h>
#include <clock_server.h>
#include <io_server.h>
#include <train_task.h>
#include <ts7200.h>
#include <uart_irq.h>

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
		Delay(5);
		/*vint *flags = (int *)( UART1_BASE + UART_FLAG_OFFSET  );*/
		/*while( !(*flags & TXFE_MASK) || !( *flags & CTS_MASK  )  ) ;*/
		/*if(*UART1Flag & CTS_MASK){}*/
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
	int tid;

	tid = Create(PRIOR_HIGH, name_server_task);
	debug(DEBUG_K4, "created taskId = %d", tid);

	tid = Create(PRIOR_HIGH, clock_server_task);
	debug(DEBUG_K4, "created taskId = %d", tid);

	tid = Create(PRIOR_MEDIUM, clock_server_notifier);
	debug(DEBUG_K4, "created taskId = %d", tid);

	tid = Create(PRIOR_HIGH, train_task_admin);
	debug(DEBUG_K4, "created taskId = %d", tid);

	Exit();
}
