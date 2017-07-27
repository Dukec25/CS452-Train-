#include <workers.h>

void delay_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	
	while (train_server->is_shutdown == 0) {
        int requester_tid;
        Delay_request delay_req;

        Receive(&requester_tid, &delay_req, sizeof(delay_req));
        /*debug(SUBMISSION, "Receive delay_request from train, tid = %d", */
                /*delay_req.train_id);*/
        Reply(requester_tid, &handshake, sizeof(handshake));

        Delay(delay_req.delay_time);

		TS_request ts_request;
		ts_request.type = TS_DELAY_TIME_UP;
        ts_request.delay_result.train_id = delay_req.train_id;

		Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

