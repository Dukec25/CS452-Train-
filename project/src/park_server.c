#include <park_server.h>
#include <calculation.h>
#include <debug.h>

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
        /*int requester_tid;*/
        /*Park_request park_req;*/
        /*Receive(&requester_tid, &park_req, sizeof(park_req));*/
        /*Reply(requester_tid, &handshake, sizeof(handshake)); // pay attention to this line */

        /*// parse destination*/
        /*Sensor stop_sensor = parse_stop_sensor(park_req.park_cmd);*/
        /*int stop = sensor_to_num(stop_sensor);*/

        /*// get extra distance (offset) in mm*/
        /*int offset = park_req.park_cmd.arg2*10; // was cm */
        /*irq_debug(SUBMISSION, "offset value%d", offset);*/

        /*// retrieve stopping distance*/
        /*int stopping_distance = train_server->velocity69_model.stopping_distance[train_server->train.speed]; */

        /*Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];*/
        /*int num_park_stops = find_stops_by_distance(train_server->track, train_server->last_stop, stop, stopping_distance, park_stops);*/

        /*if(num_park_stops == -1){*/
            /*return; // there is error, ignore this iteration*/
        /*}*/

        /*// retrieve the sensor_to_deaccelate_train*/
        /*int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in*/
        /*// calculate the delta = the distance between sensor_to_deaccelate_train*/
        /*// calculate average velocity measured in [tick]*/
        /*int delta = 0;*/
        /*int velocity = train_server->velocity69_model.velocity[train_server->train.speed];*/

        /*int i = 0;*/

        /*for ( ; i < num_park_stops; i++) {*/
            /*delta += park_stops[i].distance;*/
        /*}*/

        /*int park_delay_time = (delta + offset*1000 - stopping_distance * 1000) / velocity;*/

        /*TS_request ts_request;*/
        /*ts_request.type = TS_PARK_SERVER;*/
        /*ts_request.park_result.deaccelarate_stop = deaccelarate_stop;*/
        /*ts_request.park_result.park_delay_time = park_delay_time;*/
        /*Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));*/
    }
}

