#include <track.h>
#include <calculation.h>
#include <debug.h>

static uint32 choice_seed = 0;

void track_init(Train_server *train_server){
    train_server->resouce_available = 1;
}

void track_server()
{
    RegisterAs("TRACK_SERVER");

    Handshake handshake = HANDSHAKE_AKG;
    int train_server_tid = INVALID_TID;
    vint train_server_address;
    Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
    Reply(train_server_tid, &handshake, sizeof(handshake));
    Train_server *train_server = (Train_server *) train_server_address;

    Track_server track_server;
    track_init(&track_server);
    
    while (train_server->is_shutdown == 0) {
        int requester_tid;
        Track_req track_req;
        Receive(&requester_tid, &track_req, sizeof(park_req));
        /*Reply(requester_tid, &handshake, sizeof(handshake)); // pay attention to this line */

        /*// parse destination*/
        /*Sensor stop_sensor = parse_stop_sensor(park_req.park_cmd);*/
        /*int stop = sensor_to_num(stop_sensor);*/

        int stop = choose_rand_destination();

        /*// get extra distance (offset) in mm*/
        /*int offset = park_req.park_cmd.arg2*10; // was cm */
        /*irq_debug(SUBMISSION, "offset value%d", offset);*/

        // retrieve stopping distance
        int stopping_distance = track_req.train->velocity_model.stopping_distance[train->speed];

        int reverse = 0;
        Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
        int num_park_stops = find_stops_by_distance(train_server->track, train_server->last_stop, stop, stopping_distance, park_stops, resource, &reverse);

        if(num_park_stops == -1){
            return; // there is error, ignore this iteration
        }

        // retrieve the sensor_to_deaccelate_train
        int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
        // calculate the delta = the distance between sensor_to_deaccelate_train
        // calculate average velocity measured in [tick]
        int delta = 0;
        int velocity = track_req.train->velocity_model.velocity[train_server->train.speed];

        int i = 0;

        for ( ; i < num_park_stops; i++) {
            delta += park_stops[i].distance;
            track_server.resources[park_stops[i].sensor_id] == 0;
        }

        int park_delay_time = (delta + offset*1000 - stopping_distance * 1000) / velocity;

        TS_request ts_request;
        ts_request.type = TS_PARK_SERVER;
        ts_request.park_result.deaccelarate_stop = deaccelarate_stop;
        ts_request.park_result.park_delay_time = park_delay_time;
        ts_request.park_result.reverse = reverse;
        Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));
    }
}

// choose a random number from 0 ~ 79, which only choose sensors 
int choose_rand_destination(){
    int val = abs(rand(&choice_seed)%100);
    return val;
}

int pair(int idx){
    int ret_val = 0;
    if(idx % 2 == 0){
        ret_val -= 1;
    } else{
        ret_val += 1;
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


