#include <park_server.h>
#include <calculation.h>

void park_server()
{
    RegisterAs("PARK_SERVER");

    Handshake handshake = HANDSHAKE_AKG;
    int train_server_tid = INVALID_TID;
    vint train_server_address;
    Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
    Reply(train_server_tid, &handshake, sizeof(handshake));
    Train_server *train_server = (Train_server *) train_server_address;
    
    while (train_server->is_shutdown == 0) {
        int requester_tid;
        Park_request park_req;
        Receive(&requester_tid, &park_req, sizeof(park_req));
        Reply(requester_tid, &handshake, sizeof(handshake)); // pay attention to this line 

        // parse destination
        Sensor stop_sensor = parse_stop_sensor(park_req.park_cmd);
        int stop = sensor_to_num(stop_sensor);
        // retrieve stopping distance
        int stopping_distance = train_server->current_velocity_data->stopping_distance; 
        Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
        int num_park_stops = find_stops_by_distance(train_server->track, train_server->last_stop, stop, stopping_distance, park_stops);

        if(num_park_stops == -1){
            return; // there is error, ignore this iteration
        }

        // retrieve the sensor_to_deaccelate_train
        int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
        // calculate the delta = the distance between sensor_to_deaccelate_train
        // calculate average velocity measured in [tick]
        int delta = 0;
        int park_delay_time = 0;
        int first_stop_distance = 0;
        int first_stop_velocity = 0;
        int i = 0;

        for ( ; i < num_park_stops; i++) {
            int sensor_distance = park_stops[i].distance;
            int sensor_src = park_stops[i].sensor_id;
            int sensor_dest = (i - 1 < 0) ? stop : park_stops[i - 1].sensor_id;
            /*irq_debug(SUBMISSION, "last_stop=%d, current_stop=%d\r\n", sensor_src, sensor_dest);*/
            int sensor_velocity = velocity_lookup(sensor_src, sensor_dest, train_server->current_velocity_data);
                
            sensor_velocity = (sensor_velocity == -1) ? 0: sensor_velocity;
            delta += sensor_velocity ? sensor_distance : 0; 
            park_delay_time += sensor_distance * sensor_velocity;
            if( i == num_park_stops - 1 ){
                first_stop_distance = sensor_distance; 
                first_stop_velocity = sensor_velocity;
            }
        }

        park_delay_time /= delta;
        vint delay_distance = delta - stopping_distance;
        vint delay_velocity = first_stop_distance/first_stop_velocity;
        park_delay_time = delay_distance/ delay_velocity;
        TS_request ts_request;
        ts_request.type = TS_PARK_SERVER;
        ts_request.park_result.deaccelarate_stop = deaccelarate_stop;
        ts_request.park_result.park_delay_time = park_delay_time;
        Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));
    }
}

