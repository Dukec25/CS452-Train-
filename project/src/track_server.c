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

            int stop = choose_rand_destination();
            debug(SUBMISSION, "choosed stop is %d", stop);

            // br operation completes here 
            int num_switch = choose_destination(track_req.train->last_stop, stop, train_server, &br_lifo_struct, track_server.resource_available);

            // retrieve stopping distance
            int stopping_distance = track_req.train->velocity_model.stopping_distance[track_req.train->speed];

            int reverse = 0;
            Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
            int num_park_stops = find_stops_by_distance(train_server->track, track_req.train->last_stop, stop, stopping_distance, park_stops, track_server.resource_available, &reverse);

            if(num_park_stops == -1){
                return; // there is error, ignore this iteration
            }

            // retrieve the sensor_to_deaccelate_train
            int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
            // calculate the delta = the distance between sensor_to_deaccelate_train
            // calculate average velocity measured in [tick]
            int delta = 0;
            int velocity = track_req.train->velocity_model.velocity[track_req.train->speed];

            int i = 0;

            for ( ; i < num_park_stops; i++) {
                delta += park_stops[i].distance;
                track_server.resource_available[park_stops[i].sensor_id] == 0;
            }

            int park_delay_time = (delta - stopping_distance * 1000) / velocity;

            TS_request ts_request;
            ts_request.type = TS_TRACK_SERVER;
            ts_request.track_result.deaccelarate_stop = deaccelarate_stop;
            ts_request.track_result.park_delay_time = park_delay_time;
            ts_request.track_result.reverse = reverse;
            ts_request.track_result.train_id = track_req.train->id;
            ts_request.track_result.br_lifo_struct = br_lifo_struct;

            push_ts_req_fifo(&track_server, ts_request);
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

