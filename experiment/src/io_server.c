#include <io_server.h>

void io_server_start()
{
	debug(DEBUG_UART_IRQ, "Enter %s", "clock_server_start");

	vint register_result = RegisterAs("IO_SERVER");
	Io_server ioServer;
    
	/*initialize(&cs);*/
    fifo_t receive_q;
    fifo_t transmit_q;
    vint    char_data; // probably need an array here 
    fifo_init(receive_q);
    fifo_init(transmit_q);

	while(1) {
		vint requester;
		Delivery request;
        Delivery reply_msg;
        bool buffer_empty;
		Receive(&requester, &request, sizeof(request));
        vint tid;
		switch(request.type) {
            case TRANSMIT_RDY:
                buffer_empty = true;
                if(!is_fifo_empty(&ioServer.transmit_q)){
                    void* character; 
                    Reply(requester, &reply_msg, sizeof(reply_msg));
                    vint result = fifo.get(&ioServer.transmit_q, &character) // character might cause error
                    uartData = (vint)(*character);
                    // insert char into uart 
                } else{
                    // turn off the transmit interrupt
                }
            case GETC:
                fifo_put(&ioServer.receive_q, requester);
            case PUTC:
                Reply(requester, &reply_msg, sizeof(reply_msg));

		}
	}
}

void io_notifier(){
	int io_server_id = WhoIs("IO_SERVER");
    Delivery request;
    request.type = RECEIVE_RDY;
    Delivery reply_msg;
    while(1){
        request.data = AwaitEvent(RCV_RDY); // evtType = here should be clock update event;
        send(io_server_id, request, sizeof(request), reply_msg, sizeof(reply_msg) );
    }
}

