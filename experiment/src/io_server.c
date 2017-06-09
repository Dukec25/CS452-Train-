#include <io_server.h>

static void initialize(Io_server *io){
    fifo_init(&io->get_q);
    fifo_init(&io->transmit_q);
    fifo_init(&io->receive_q);
}


void io_server_start()
{
	debug(DEBUG_UART_IRQ, "Enter %s", "clock_server_start");

	vint register_result = RegisterAs("IO_SERVER_CHANNEL2");
	Io_server ioServer;
    initialize(&ioServer);
    vint transmit_notifier;
    int buffer_empty;
    vint *uart2_ctrl = (vint *) UART2_CTRL;
    vint *pdata = (vint *)(UART2_BASE + UART_DATA_OFFSET);
    
	while(1) {
		vint requester;
		Delivery request;
        Delivery reply_msg;
		Receive(&requester, &request, sizeof(request));
        vint tid;
        vint* character; 
		switch(request.type) {
            case TRANSMIT_RDY:
                buffer_empty = 1;
                transmit_notifier = requester;
                if(!is_fifo_empty(&ioServer.transmit_q)){
                    // if no chars needs to be transmitted
                    Reply(requester, &reply_msg, sizeof(reply_msg));
                    debug(DEBUG_UART_IRQ, "Enter %s", "clock_server_start");
                    fifo_get(&ioServer.transmit_q, &character); // character might cause error
                    debug(DEBUG_UART_IRQ, "UART DATA IS  %d", *character);
                    // insert char into uart 
                    *pdata = *character;
                } else{
                    // turn off the transmit interrupt
                    *uart2_ctrl &= ~(TIEN_MASK);
                }
                break;
            case RECEIVE_RDY:
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
                if(!is_fifo_empty(&ioServer.receive_q)){
                    vint* rcv_data; 
                    vint result = fifo_get(&ioServer.transmit_q, &rcv_data); // character might cause error
                    Delivery reply_client_msg;
                    reply_client_msg.data = *rcv_data;
                    Reply(requester, &reply_client_msg, sizeof(reply_client_msg));
                } else{
                    fifo_put(&ioServer.get_q, requester);
                }
                break;
            case PUTC:
                Reply(requester, &reply_msg, sizeof(reply_msg));
                if(buffer_empty){
                    vint *pdata = (vint *)(UART2_BASE + UART_DATA_OFFSET);
                    *pdata = request.data;
                    // turn on the transmit interrupt
                    *uart2_ctrl |= TIEN_MASK;
                    buffer_empty = 0;
                    Reply(transmit_notifier, &reply_msg, sizeof(reply_msg));
                } else{
                    fifo_put(&ioServer.transmit_q, request.data);
                }
                break;
		}
	}
}

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

