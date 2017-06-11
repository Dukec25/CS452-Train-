#include <debug.h>
#include <user_functions.h>
#include <io_server.h>

extern void asm_init_kernel();
extern int asm_kernel_create(int priority, void (*code) ());
extern void asm_kernel_pass();
extern void asm_kernel_exit();
extern int asm_kernel_my_tid();
extern int asm_kernel_my_parent_tid();
extern int asm_kernel_await_event(int eventType, char ch);

void init_kernel()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "init_kernel");
	asm_init_kernel();
}

int Create(int priority, void (*code) ())
{
	debug(DEBUG_SYSCALL, "In user mode Create, priority = %d, code = 0x%x", priority, code);
	vint tid = asm_kernel_create(priority, code);
	debug(DEBUG_SYSCALL, "In user mode Create, tid = %d", tid);
	return tid;
}

void Pass()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "Pass");
	asm_kernel_pass();
}

void Exit()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "Exit");
	asm_kernel_exit();
}

int MyTid()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "MyTid");
	vint tid = asm_kernel_my_tid();
	debug(DEBUG_SYSCALL, "In user mode MyTid tid = %d", tid);
	return tid;
}

int MyParentTid()
{
	debug(DEBUG_SYSCALL, "In user mode %s", "MyParentTid");
	vint tid = asm_kernel_my_parent_tid();
	debug(DEBUG_SYSCALL, "In user mode MyParentTid tid = %d", tid);
	return tid;
}

int Send( int tid, void *msg, int msglen, void *reply, int replylen )
{
    debug(DEBUG_SYSCALL, "this is in %s", "user Send");
    vint errorCode = asm_kernel_send(tid, msg, msglen, reply, replylen);
    return errorCode;
}

int Receive( int *tid, void *msg, int msglen )
{
    debug(DEBUG_SYSCALL, "this is in %s", "user Receive");
    int result = asm_kernel_receive(tid, msg, msglen);
    return result;
}

int Reply( int tid, void *reply, int replylen )
{
    debug(DEBUG_SYSCALL, "this is in %s", "user Reply");
    int result = asm_kernel_reply(tid, reply, replylen);
    return result;
}

int AwaitEvent(int eventType, char ch){
    debug(DEBUG_SYSCALL, "this is in %s", "user AwaitEvent");
    int data = asm_kernel_await_event(eventType, ch);
    return data;
}

int Getc(int channel)
{
	int io_server_id;
	switch (channel) {
	case COM1:
		io_server_id = WhoIs("IO_SERVER_UART1_RECEIVE");
		break;
	case COM2:
		io_server_id = WhoIs("IO_SERVER_UART2_RECEIVE");
		break;
	}
    debug(DEBUG_UART_IRQ, "enter Getc, server is %d, type = %d", io_server_id, GETC);
    Delivery request;
    request.type = GETC;
    Delivery reply_msg;
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg) );
    return reply_msg.data;
}

int Putc(int channel, char ch)
{
	int io_server_id;
	switch (channel) {
	case COM1:
		io_server_id = WhoIs("IO_SERVER_UART1_TRANSMIT");
		break;
	case COM2:
		io_server_id = WhoIs("IO_SERVER_UART2_TRANSMIT");
		break;
	}
    debug(DEBUG_UART_IRQ, "enter Putc, server is %d, type = %d", io_server_id, PUTC);
    Delivery request;
    request.type = PUTC;
    request.data = ch;
    Delivery reply_msg;
	debug(DEBUG_UART_IRQ, "send %d to io_server_id %d", ch, io_server_id);
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
	debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
    return 1;
}
