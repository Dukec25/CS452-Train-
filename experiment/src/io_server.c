#include <io_server.h>
#include <user_functions.h>
#include <debug.h>
#include <name_server.h>
#include <clock_server.h>

static void initialize(Io_server *io)
{
    fifo_init(&io->get_q);
    fifo_init(&io->transmit_q);
    fifo_init(&io->receive_q);
}

void io_server_receive_start(int channel)
{
	debug(DEBUG_UART_IRQ, "Enter %s", "io_server_receive_start");

    vint register_result;
    if(channel == COM1){
        register_result = RegisterAs("IO_SERVER_UART1_RECEIVE");
    } else{
        register_result = RegisterAs("IO_SERVER_UART2_RECEIVE");
    }
	Io_server ioServer;
    initialize(&ioServer);
    
	int i;
	while(1) {
		vint requester;
		Delivery request;
        Delivery reply_msg;
		Receive(&requester, &request, sizeof(request));
        debug(DEBUG_UART_IRQ, "io server receive request, type=%d", request.type);
        vint* character; 
		
		i++;
		switch(request.type) {
            case RECEIVE_RDY:
                debug(DEBUG_UART_IRQ, "io server %s", "RCV RDY request");
                Reply(requester, &reply_msg, sizeof(reply_msg));
				// if (channel == COM1) irq_printf(COM2, "train\r\n");
                if(!is_fifo_empty(&ioServer.get_q)){
                    vint client; 
                    vint result = fifo_get(&ioServer.get_q, &client); // character might cause error
                    vint rcv_data = request.data; 
                    debug(DEBUG_UART_IRQ, "Received Data is %d", rcv_data);
                    debug(DEBUG_UART_IRQ, "Client is  %d", client);
                    Delivery reply_client_msg;
                    reply_client_msg.data = rcv_data;
					// if (channel == COM1) irq_printf(COM2, "rdy%d\r\n", i);
                    Reply(client, &reply_client_msg, sizeof(reply_client_msg));
                }
                else{
                    fifo_put(&ioServer.receive_q, request.data);
                }
                break;
            case GETC:
                debug(DEBUG_UART_IRQ, "enter %s", "IO SERVER, REQUEST GETC");
                if(!is_fifo_empty(&ioServer.receive_q)){
                    debug(DEBUG_UART_IRQ, "Fifo %s", "receive_q is not empty");
                    vint rcv_data; 
                    vint result = fifo_get(&ioServer.receive_q, &rcv_data); // character might cause error
                    Delivery reply_client_msg;
                    reply_client_msg.data = rcv_data;
					// if (channel == COM1) irq_printf(COM2, "getc rel\r\n");
                    Reply(requester, &reply_client_msg, sizeof(reply_client_msg));
                } else{
                    debug(DEBUG_UART_IRQ, "Fifo %s", "receive_q is empty");
                    fifo_put(&ioServer.get_q, requester);
					// if (channel == COM1) irq_printf(COM2, "getc blk\r\n");
                }
                break;
		}
	}
}

void io_server_transmit_start(int channel)
{
	debug(DEBUG_UART_IRQ, "Enter %s", "io_server_transmit_start");

    vint register_result;
    if(channel == COM1){
        register_result = RegisterAs("IO_SERVER_UART1_TRANSMIT");
    } else{
        register_result = RegisterAs("IO_SERVER_UART2_TRANSMIT");
    }

	Io_server ioServer;
    initialize(&ioServer);
    vint transmit_notifier;
	vint xmit_not_waiting = 0;
    
	while(1) {
		vint requester;
		Delivery request;
        Delivery reply_msg;
		Receive(&requester, &request, sizeof(request));
        debug(DEBUG_UART_IRQ, "io server receive request, type=%d", request.type);
        vint character; 
        char *printf_buf;

		switch(request.type) {
            case TRANSMIT_RDY:
                debug(DEBUG_UART_IRQ, "io server %s", "XMIT RDY request");
				xmit_not_waiting = 1;
				debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d", xmit_not_waiting);
                transmit_notifier = requester;
                if(!is_fifo_empty(&ioServer.transmit_q)){
                    debug(DEBUG_UART_IRQ, "transmit queue is not empty %s", "XMIT RDY request");
                    fifo_get(&ioServer.transmit_q, &character); // character might cause error
					reply_msg.data = character;
                    debug(DEBUG_UART_IRQ, "reply data is %d", &character);
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
					xmit_not_waiting = 0;
					debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d", xmit_not_waiting);
                }
                break;
            case PUTC:
                debug(DEBUG_UART_IRQ, "enter IO SERVER, REQUEST PUTC, put data = %d", request.data);
				fifo_put(&ioServer.transmit_q, request.data);	// transmit_q is not empty
                // if (channel == COM1) irq_printf(COM2, "putc put %d\r\n", request.data);
				debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d, transmit_q is_empty = %d", xmit_not_waiting, is_fifo_empty(&ioServer.transmit_q));
				reply_msg.data = 0;
                Reply(requester, &reply_msg, sizeof(reply_msg));
				debug(DEBUG_UART_IRQ, "replied to %d", requester);
				if (xmit_not_waiting) {
					debug(DEBUG_UART_IRQ, "inside if xmit_not_waiting = %d", xmit_not_waiting);
	                vint result = fifo_get(&ioServer.transmit_q, &character); // character might cause error
                    debug(DEBUG_UART_IRQ, "result = %d, reply data is %d", result, character);
                	// if (channel == COM1) irq_printf(COM2, "putc get %d\r\n", character);
					reply_msg.data = character;
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
					xmit_not_waiting = 0;
					debug(DEBUG_UART_IRQ, "NOw xmit_not_waiting = %d", xmit_not_waiting);
				}
                break;
            case PRINTF:
                printf_buf = request.data_arr;
				if (channel == COM1) {
                	while(*printf_buf != 127){
						// if (channel == COM1) irq_printf(COM2, "printf put %d\r\n", *printf_buf);
                    	fifo_put(&ioServer.transmit_q, *printf_buf++);
                	}
				} else {
                	while(*printf_buf != '\0'){
                    	fifo_put(&ioServer.transmit_q, *printf_buf++);
                	}
				}
				reply_msg.data = 0;
                Reply(requester, &reply_msg, sizeof(reply_msg));
				if (xmit_not_waiting) {
	                vint result = fifo_get(&ioServer.transmit_q, &character); // character might cause error
					// if (channel == COM1) irq_printf(COM2, "printf get %d\r\n", character);
                    debug(DEBUG_UART_IRQ, "result = %d, reply data is %d", result, character);
					reply_msg.data = character;
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
					xmit_not_waiting = 0;
					debug(DEBUG_UART_IRQ, "NOw xmit_not_waiting = %d", xmit_not_waiting);
				}
                break;
		}
	}
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
	// if (channel == COM1) irq_printf(COM2, "rcv %d\r\n", reply_msg.data);
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
    debug(DEBUG_UART_IRQ, "enter Putc, channel = %d, server is %d, type = %d", channel, io_server_id, PUTC);
    Delivery request;
    request.type = PUTC;
    request.data = ch;
    Delivery reply_msg;
	debug(DEBUG_UART_IRQ, "send %d to io_server_id %d", ch, io_server_id);
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
	debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
    return 1;
}
