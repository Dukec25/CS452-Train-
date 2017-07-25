#include <calculation.h>
#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>

typedef struct Track_server {
    // 1 represent resource available, 0 otherwise 
    int resource_available[143]; 

#define ROUTE_RESULT_FIFO_SIZE	50
    TS_request route_result_fifo[ROUTE_RESULT_FIFO_SIZE];
	int route_result_fifo_head;
	int route_result_fifo_tail;
    int train_courier_on_wait;
} Track_server;

static uint32 choice_seed = 0;

void track_init(Track_server *track_server){
    int i = 0;
    for( ;i < 143; i++){
        track_server->resource_available[i] = 1;
    }
    track_server->train_courier_on_wait = 0;
}

void track_server()
{
    RegisterAs("TRACK_SERVER");
    irq_debug(SUBMISSION, "%s", "track server");
    Handshake handshake = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &handshake, sizeof(handshake));
	Handshake *kill_all = kill_all_addr;

    int train_server_tid = INVALID_TID;
    vint train_server_address;
    Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
    Reply(train_server_tid, &handshake, sizeof(handshake));
    Train_server *train_server = (Train_server *) train_server_address;

    Track_server track_server;
    track_init(&track_server);
    
	while (*kill_all != HANDSHAKE_SHUTDOWN) {
        int requester_tid;
        Track_request track_req;
        Receive(&requester_tid, &track_req, sizeof(track_req));
        /*Reply(requester_tid, &handshake, sizeof(handshake)); // pay attention to this line */
        
        if(track_req.type == TRAIN_WANT_GUIDANCE){
            debug(SUBMISSION, "%s", "track receive park command");
            Br_lifo br_lifo_struct;
            br_lifo_struct.br_lifo_top = -1;

            /*int stop = choose_rand_destination();*/
            /*debug(SUBMISSION, "choosed stop is %d", stop);*/
            int stop = 5; // hardcode for testing purpose

            int src = track_req.train->last_stop;
            track_node *node;
            node = find_path_with_blocks(train_server->track, src, stop, track_server.resource_available);

            if(node == NULL){
                // TODO, no path available  
            } else{
                // br operation completes here 
                int num_switch = switches_need_changes(src, node, train_server, &br_lifo_struct);

                TS_request ts_request;
                ts_request.type = TS_TRACK_SERVER;
                ts_request.track_result.train_id = track_req.train->id;
                ts_request.track_result.br_lifo_struct = br_lifo_struct;
                put_cmd_fifo(train_server->track, stop, track_server.resource_available, node, track_req.train, &ts_request);
                /*ts_request.track_result.num_br_switch  = num_switch;*/
                push_ts_req_fifo(&track_server, ts_request);
            }
        } else if (track_req.type == TRAIN_WANT_RESULT){
			// from track_to_train_courier
			TS_request ts_req;
            irq_debug(SUBMISSION, "%s", "track_server receive courier");
			if (track_server.route_result_fifo_head == track_server.route_result_fifo_tail) {
                track_server.train_courier_on_wait = requester_tid;
			}
			else {
				pop_ts_req_fifo(&track_server, &ts_req);
				//irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
                Reply(requester_tid, &ts_req, sizeof(ts_req));
			}
        }
    }
}

// choose a random number from 0 ~ 79, which only choose sensors 
int choose_rand_destination(){
    int val = abs(rand(&choice_seed)%80);
    return val;
}

int pair(int idx){
    int ret_val = idx;
    if(idx % 2 == 0){
        ret_val += 1;
    } else{
        ret_val -= 1;
    }
    return ret_val;
}

int convert_sw_track_data(int num, int type){
    int result = 80 + (num-1)*2;
    if(type){
        result += 1;
    }
    return result;
}

void push_ts_req_fifo(Track_server *track_server, TS_request ts_req)
{
    int ts_fifo_put_next = track_server->route_result_fifo_head + 1;
    if (ts_fifo_put_next != track_server->route_result_fifo_tail) {
        if (ts_fifo_put_next >= ROUTE_RESULT_FIFO_SIZE) {
            ts_fifo_put_next = 0;
        }
    }
    track_server->route_result_fifo[track_server->route_result_fifo_head] = ts_req;
    track_server->route_result_fifo_head = ts_fifo_put_next;  
}

void pop_ts_req_fifo(Track_server *track_server, TS_request *ts_req)
{
    int ts_fifo_get_next = track_server->route_result_fifo_tail + 1;
    if (ts_fifo_get_next >= ROUTE_RESULT_FIFO_SIZE) {
        ts_fifo_get_next = 0;
    }
    *ts_req = track_server->route_result_fifo[track_server->route_result_fifo_tail];
    track_server->route_result_fifo_tail = ts_fifo_get_next;  
}

void clear_track_var(track_node *track){
    int i = 0;
    for(i = 0; i < 144; i++){
        track[i].buf = 0;
        track[i].previous = NULL;
    }
} 
