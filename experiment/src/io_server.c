#include <io_server.h>

static void initialize(Io_server *io){
    fifo_init(&io->get_q);
    fifo_init(&io->transmit_q);
    fifo_init(&io->receive_q);
}


void io_server_start()
{
	debug(DEBUG_UART_IRQ, "Enter %s", "io_server_start");

	vint register_result = RegisterAs("IO_SERVER_CHANNEL2");
	Io_server ioServer;
    initialize(&ioServer);
    vint transmit_notifier;
    vint buffer_empty=0;
    vint *uart1_ctrl = (vint *) UART1_CTRL;
    vint *pdata = (vint *) UART1_DATA;
	vint xmit_not_waiting = 0;
    
	while(1) {
		vint requester;
		Delivery request;
        Delivery reply_msg;
		Receive(&requester, &request, sizeof(request));
        debug(DEBUG_UART_IRQ, "io server receive request, type=%d", request.type);
        vint tid;
        vint* character; 

		switch(request.type) {
            case TRANSMIT_RDY:
                debug(DEBUG_UART_IRQ, "io server %s", "XMIT RDY request");
				xmit_not_waiting = 1;
				debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d", xmit_not_waiting);
                transmit_notifier = requester;
                if(!is_fifo_empty(&ioServer.transmit_q)){
                    debug(DEBUG_UART_IRQ, "transmit queue is not empty %s", "XMIT RDY request");
                    fifo_get(&ioServer.transmit_q, &character); // character might cause error
					reply_msg.data = *character;
                    debug(DEBUG_UART_IRQ, "reply data is %d", *character);
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
					xmit_not_waiting = 0;
					debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d", xmit_not_waiting);
                }
                break;
            case RECEIVE_RDY:
                debug(DEBUG_UART_IRQ, "io server %s", "RCV RDY request");
                Reply(requester, &reply_msg, sizeof(reply_msg));
                if(!is_fifo_empty(&ioServer.get_q)){
                    vint* client; 
                    vint result = fifo_get(&ioServer.get_q, &client); // character might cause error
                    vint rcv_data = request.data; 
                    debug(DEBUG_UART_IRQ, "Received Data is %d", rcv_data);
                    debug(DEBUG_UART_IRQ, "Client is  %d", client);
                    Delivery reply_client_msg;
                    reply_client_msg.data = rcv_data;
                    Reply(*client, &reply_client_msg, sizeof(reply_client_msg));
                }
                else{
                    fifo_put(&ioServer.receive_q, request.data);
                }
                break;
            case GETC:
                debug(DEBUG_UART_IRQ, "enter %s", "IO SERVER, REQUEST GETC");
                if(!is_fifo_empty(&ioServer.receive_q)){
                    debug(DEBUG_UART_IRQ, "Fifo %s", "receive_q is not empty");
                    vint* rcv_data; 
                    vint result = fifo_get(&ioServer.transmit_q, &rcv_data); // character might cause error
                    Delivery reply_client_msg;
                    reply_client_msg.data = *rcv_data;
                    Reply(requester, &reply_client_msg, sizeof(reply_client_msg));
                } else{
                    debug(DEBUG_UART_IRQ, "Fifo %s", "receive_q is empty");
                    fifo_put(&ioServer.get_q, requester);
                }
                break;
            case PUTC:
                debug(DEBUG_UART_IRQ, "enter %s", "IO SERVER, REQUEST PUTC");
				fifo_put(&ioServer.transmit_q, &request.data);	// transmit_q is not empty
				debug(DEBUG_UART_IRQ, "xmit_not_waiting = %d, transmit_q is_empty = %d", xmit_not_waiting, is_fifo_empty(&ioServer.transmit_q));
				reply_msg.data = 0;
                Reply(requester, &reply_msg, sizeof(reply_msg));
				debug(DEBUG_UART_IRQ, "replied to %d", requester);
				if (xmit_not_waiting) {
					debug(DEBUG_UART_IRQ, "inside if xmit_not_waiting = %d", xmit_not_waiting);
	                vint result = fifo_get(&ioServer.transmit_q, &character); // character might cause error
                    debug(DEBUG_UART_IRQ, "result = %d, reply data is %d", result, *character);
					reply_msg.data = *character;
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
					xmit_not_waiting = 0;
				}
                break;
		}
	}
}

/*void io_server_uart1_transmit_start();*/
/*void io_server_uart2_transmit_start();*/
/*void io_server_uart1_receive_start();*/
/*void io_server_uart2_receive_start();*/

/*void receive_notifier(){*/
	/*int io_server_id = WhoIs("IO_SERVER_CHANNEL2");*/
    /*Delivery request;*/
    /*request.type = RECEIVE_RDY;*/
    /*Delivery reply_msg;*/
    /*while(1){*/
        /*request.data = AwaitEvent(RCV_RDY); */
        /*send(io_server_id, request, sizeof(request), reply_msg, sizeof(reply_msg) );*/
    /*}*/
/*}*/

/*void transmit_notifier(){*/
	/*int io_server_id = WhoIs("IO_SERVER_CHANNEL2");*/
    /*Delivery request;*/
    /*request.type = TRANSMIT_RDY;*/
    /*Delivery reply_msg;*/
    /*while(1){*/
        /*send(io_server_id, request, sizeof(request), reply_msg, sizeof(reply_msg) );*/
        /*request.data = AwaitEvent(XMIT_RDY);*/
    /*}*/
/*}*/

