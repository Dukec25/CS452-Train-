#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>
#include <calculation.h>

void train_server_init(Train_server *train_server)
{
	train_server->is_shutdown = 0;
	train_server->is_special_cmd = 0;

	train_server->cmd_fifo_head = 0;
	train_server->cmd_fifo_tail = 0;

	train_server->cli_req_fifo_head = 0;
	train_server->cli_req_fifo_tail = 0;

	train_server->last_sensor.group = 0;
    train_server->last_sensor.id = 0;
    train_server->last_stop=-1;

    train_server->cli_map.test = 5;

	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server->switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}

	velocity14_initialization(&train_server->velocity14_data);

	velocity10_initialization(&train_server->velocity10_data);

	velocity8_initialization(&train_server->velocity8_data);

	velocity6_initialization(&train_server->velocity6_data);
}

void train_server()
{
	Handshake kill_all_reply = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
	Handshake *kill_all = kill_all_addr;

	// train_server initialization 
	Train_server train_server;
	train_server_init(&train_server);

    // by default using track A, can be changed by command map
    track_node track[TRACK_MAX];
    init_tracka(train_server.track);

	RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}

	Handshake handshake = HANDSHAKE_AKG;
	vint train_server_address = (vint) &train_server;
	/*irq_debug(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 */
    Send(cli_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int sensor_server_tid = Create(PRIOR_MEDIUM, sensor_server);
	/*irq_debug(SUBMISSION, "sensor_reader_tid %d", sensor_reader_tid);*/
	Send(sensor_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	while (*kill_all != HANDSHAKE_SHUTDOWN) {
		// receive command request
		int requester_tid;
		TS_request ts_request;
		Receive(&requester_tid, &ts_request, sizeof(ts_request));
		if (ts_request.type == TS_COMMAND) {
			// from train_command_courier
			push_cmd_fifo(&train_server, ts_request.cmd);
			handshake = HANDSHAKE_AKG;
			Reply(requester_tid, &handshake, sizeof(handshake));
		}
		else if (ts_request.type == TS_WANT_CLI_REQ) {
			// from cli_request_courier
			Cli_request cli_req;
			if (train_server.cli_req_fifo_head == train_server.cli_req_fifo_tail) {
				cli_req.type = CLI_NULL;
			}
			else {
				pop_cli_req_fifo(&train_server, &cli_req);
				//irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
			}
			Reply(requester_tid, &cli_req, sizeof(cli_req));
		}
        else if (ts_request.type == TS_SENSOR_SERVER) {
            Sensor_result sensor_result= ts_request.sensor;
            int num_sensor = sensor_result.num_sensor;
            int idx = 0;

            for( ; idx < num_sensor; idx++){
                Sensor current_sensor = sensor_result.sensors[idx];
                Sensor last_sensor = train_server.last_sensor;
                int last_stop = sensor_to_num(last_sensor);
                int current_stop = sensor_to_num(current_sensor);

                // update last triggered sensor
                train_server.last_sensor = current_sensor;
                train_server.last_stop = current_stop; // to be deleted 
        
                if ((current_stop == last_stop) ||  (last_stop == -1)) {
                    continue;
                }

                // calculate distance, next stop, time, and new_velocity
                int distance = cal_distance(train_server.track, last_stop, current_stop);
                int next_stop = predict_next(train_server.track, current_stop, &train_server);
                int time = current_sensor.triggered_time - last_sensor.triggered_time;

                // update velocity_data
                velocity_update(last_stop, current_stop, time, train_server.current_velocity_data);

                Cli_request update_sensor_request = get_update_sensor_request(current_sensor, last_stop, next_stop);
                push_cli_req_fifo(&train_server, update_sensor_request);

                int velocity = velocity_lookup(last_stop, current_stop, train_server.current_velocity_data); 
                Cli_request update_calibration_request =
                get_update_calibration_request(last_stop, current_stop, distance, time, velocity);
                push_cli_req_fifo(&train_server, update_calibration_request);
            }
        }
		else {
			Reply(requester_tid, &handshake, sizeof(handshake));
		}

		if (train_server.cmd_fifo_head == train_server.cmd_fifo_tail) {
			// cmd_fifo is empty
			continue;
		}

		Command cmd;
		pop_cmd_fifo(&train_server, &cmd);

		Cli_request cli_update_request;
		int cli_req_fifo_put_next;
		switch (cmd.type) {
        case MAP: 
            if(cmd.arg0 == 'A' || cmd.arg0 == 'a'){
                init_tracka(train_server.track);
                /*bwprintf(COM2, "TRIGGER train_server"); */
                cli_draw_trackA(&(train_server.cli_map));
            } else if(cmd.arg0 == 'B' || cmd.arg0 == 'b'){
                init_trackb(train_server.track);
                cli_draw_trackB(&(train_server.cli_map));
            }
            break;
		case TR:
			/*irq_debug(SUBMISSION, "handle tr cmd %d %d", cmd.arg0, cmd.arg1);*/
			command_handle(&cmd);

			train_server.train.id = cmd.arg0;
			train_server.train.speed = cmd.arg1;

			switch(train_server.train.speed){
				case 14:
					train_server.current_velocity_data = &train_server.velocity14_data;
					break;
				case 10:
					train_server.current_velocity_data = &train_server.velocity10_data;
					break;
				case 8:
					train_server.current_velocity_data = &train_server.velocity8_data;
					break;
				case 6:
					train_server.current_velocity_data = &train_server.velocity6_data;
					break;
				default:
					break;
			}

			cli_update_request = get_update_train_request(cmd.arg0, cmd.arg1);
			push_cli_req_fifo(&train_server, cli_update_request);
	
			break;
		case SW:
			irq_debug(SUBMISSION, "%s", "handle sw cmd");
			command_handle(&cmd);

			train_server.switches_status[cmd.arg0 - 1] = switch_state_to_byte(cmd.arg1);

			cli_update_request = get_update_switch_request(cmd.arg0, cmd.arg1);
			push_cli_req_fifo(&train_server, cli_update_request);
	
			break;
		case RV:
		case GO:
		case STOP:
			command_handle(&cmd);
			break;

		case DC:
			train_server.is_special_cmd = 1;
			train_server.special_cmd = cmd;
			break;

		case BR:
			br_handle(&train_server, cmd);
			break;

		case PARK:
			/*irq_debug(SUBMISSION, "handle park cmd %c%d", cmd.arg0, cmd.arg1);*/
			train_server.is_special_cmd = 1;
			train_server.special_cmd = cmd;
			br_handle(&train_server, cmd);
			break;
				
		default:
			break;
		}

		if (train_server.is_special_cmd) {
			switch (train_server.special_cmd.type) {
			case DC:
				dc_handle(&train_server, train_server.special_cmd);
				break;

			case PARK:
				park_handle(&train_server, train_server.special_cmd);	
				break;

			default:
				break;
			}
		}
	}

	train_server.is_shutdown = 1;

	int expected_num_exit = 1;
	int num_exit = 0;
	int exit_list[1];
	exit_list[0] = sensor_server_tid;
	while(num_exit < expected_num_exit) {
		Handshake exit_handshake;
		Handshake exit_reply = HANDSHAKE_AKG;
		int exit_tid;
		Receive(&exit_tid, &exit_handshake, sizeof(exit_handshake));
		Reply(exit_tid, &exit_reply, sizeof(exit_reply));
		if (exit_handshake == HANDSHAKE_SHUTDOWN) {
			if (exit_tid == exit_list[0]) {
				exit_list[0] = INVALID_TID;
				num_exit++;
			}
		}
	}
	
	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 	
	Exit();
}


void dc_handle(Train_server *train_server, Command dc_cmd)
{
	Sensor stop_sensor = parse_stop_sensor(dc_cmd);
	int stop = sensor_to_num(stop_sensor);

	int last_stop = train_server->last_stop;
	if (last_stop != stop) {
		last_stop = train_server->last_stop;
	}
	else {
		//irq_debug(SUBMISSION, "stopping_distance current stop = %d", last_stop);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}

void br_handle(Train_server *train_server, Command br_cmd)
{
	// parse destination
	Sensor stop_sensor = parse_stop_sensor(br_cmd);
	int stop = sensor_to_num(stop_sensor);
	/*irq_debug(SUBMISSION, "br_task: stop sensor is %d, %d, stop = %d", stop_sensor.group, stop_sensor.id, stop);*/
	
	// flip switches such that the train can arrive at the stop
	int num_switch = choose_destination(train_server->track, train_server->last_stop, stop, train_server);
	//irq_debug(SUBMISSION, "br_task: send flip %d switches start", num_switch);
	/*irq_debug(SUBMISSION, "num_switch = %d", num_switch);*/
	int i;
	for(i = 0; i < num_switch; i++) {
		/*irq_debug(SUBMISSION, "SW ID = %d, STATE = %d", train_server->br_update[i].id, train_server->br_update[i].state);*/
		Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
		push_cmd_fifo(train_server, sw_cmd);
	}
	//irq_debug(SUBMISSION, "br_task: flip %d br done", num_switch);
}

void park_handle(Train_server *train_server, Command park_cmd)
{
	// parse destination
	Sensor stop_sensor = parse_stop_sensor(park_cmd);
	int stop = sensor_to_num(stop_sensor);
	//irq_debug(SUBMISSION, "park_task, stop sensor is %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

	// retrieve stopping distance
	int stopping_distance = train_server->current_velocity_data->stopping_distance; 
	/*irq_debug(SUBMISSION, "park_task: stopping_distance = %d", stopping_distance);*/

	Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
	int num_park_stops = find_stops_by_distance(train_server->track, train_server->last_stop, stop, stopping_distance, park_stops);
	if(num_park_stops == -1){
		return; // there is error, ignore this iteration
	}

	// retrieve the sensor_to_deaccelate_train
	int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
	//irq_debug(SUBMISSION, "park_task: deaccelarate_stop = %c%d",
					  //num_to_sensor(deaccelarate_stop).group + SENSOR_LABEL_BASE, num_to_sensor(deaccelarate_stop).id);

	// calculate the delta = the distance between sensor_to_deaccelate_train
	// calculate average velocity measured in [tick]
	int delta = 0;
	int park_delay_time = 0;
	int first_stop_distance = 0;
	int first_stop_velocity = 0;
	int i;
	for (i = 0; i < num_park_stops; i++) {
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

	/*irq_debug(SUBMISSION, "delta=%d, first_dist=%d,first_velo=%d, stop_dist=%d",delta, first_stop_distance, first_stop_velocity, stopping_distance);*/
	/*irq_debug(SUBMISSION, "first_stop[%d]\r\n", park_stops[num_park_stops-1].sensor_id);*/
	/*irq_debug(SUBMISSION, "first_stop[%d]\r\n", park_stops[num_park_stops-1].sensor_id);*/
	vint delay_distance = delta - stopping_distance;
	vint delay_velocity = first_stop_distance/first_stop_velocity;
	park_delay_time = delay_distance/ delay_velocity;

	//irq_debug(SUBMISSION, "park_task: delta = %d, park_delay_time = %d", delta, park_delay_time);

	int last_stop = train_server->last_stop;
	if (last_stop != deaccelarate_stop) {
		last_stop = train_server->last_stop;
	}
	else {
		//irq_debug(SUBMISSION, "park_task current stop = %d, start delay", last_stop);
		Delay(park_delay_time);
		//irq_debug(SUBMISSION, "park_task send tr %d", tr_cmd.arg0);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}
