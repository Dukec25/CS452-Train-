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

	train_server->cmd_fifo_head = 0;
	train_server->cmd_fifo_tail = 0;

	train_server->cli_req_fifo_head = 0;
	train_server->cli_req_fifo_tail = 0;

	train_server->sensor_lifo_top = -1;
	train_server->last_stop = -1;
	train_server->num_sensor_query = 0;

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

	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	/*dump(SUBMISSION, "cli_server %d", cli_server_tid);*/

	Handshake handshake = HANDSHAKE_AKG;
	vint train_server_address = (vint) &train_server;
	/*dump(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 */

	int sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
	/*dump(SUBMISSION, "sensor_reader_tid %d", sensor_reader_tid);*/
	Send(sensor_reader_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

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
			}
			Reply(requester_tid, &cli_req, sizeof(cli_req));
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
		case TR:
			/*dump(SUBMISSION, "%s", "handle tr cmd");*/
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
			/*dump(SUBMISSION, "%s", "handle sw cmd");*/
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

		case SENSOR:
			sensor_handle(&train_server);
			break;

		case DC:
			train_server.is_special_cmd = 1;
			train_server.special_cmd = cmd;
			break;

		case BR:
			br_handle(&train_server, cmd);
			break;

		case PARK:
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
	exit_list[0] = sensor_reader_tid;
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

void sensor_reader_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	
	while (train_server->is_shutdown == 0) {
		Delay(20);	// update every 200ms
		Command sensor_cmd = get_sensor_command();
		Send(train_server_tid, &sensor_cmd, sizeof(sensor_cmd), &handshake, sizeof(handshake));
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

void sensor_handle(Train_server *train_server)
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	// sensor query
	/*dump(SUBMISSION, "%s", "sensor cmd");*/
	Putc(COM1, SENSOR_QUERY);
	uint16 sensor_data[SENSOR_GROUPS];
	int sensor_group = 0;
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		char lower = Getc(COM1);
		char upper = Getc(COM1);
		sensor_data[(int) sensor_group] = upper << 8 | lower;
	}
	train_server->num_sensor_query++;
	/*dump(SUBMISSION, "num_sensor_query = %d", train_server->num_sensor_query);*/

	// parse sensor data
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		if (sensor_data[(int) sensor_group] == 0) {
			continue;
		}
		/*dump(SUBMISSION, "sensor_data[%d] = %d", sensor_group, sensor_data[(int) sensor_group]);*/
		char bit = 0;
		for (bit = 0; bit < SENSORS_PER_GROUP; bit++) {
			//sensor_data actually looks like 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
			if (!(sensor_data[(int) sensor_group] & (0x1 << bit))) {
				continue;
			}
			Sensor sensor;
			sensor.group = sensor_group;
			sensor.triggered_time = Time();
			sensor.triggered_query = train_server->num_sensor_query;
			if (bit + 1 <= 8) {
				sensor.id = 8 - bit;
			}
			else {
				sensor.id = 8 + 16 - bit;
			}

			int current_stop = sensor_to_num(sensor);
			int last_stop = train_server->last_stop;
	
			Sensor last_sensor;
			while (train_server->sensor_lifo_top != -1) {
				pop_sensor_lifo(train_server, &last_sensor);
				if (sensor_to_num(last_sensor) == last_stop) {
					break;
				}
			}

			push_sensor_lifo(train_server, sensor);

			// update last triggered sensor
			train_server->last_stop = current_stop;
	
			if (current_stop == last_stop) {
				continue;
			}

			// calculate distance, next stop, time, and new_velocity
			int distance = cal_distance(track, last_stop, current_stop);
			int next_stop = predict_next(track, current_stop, train_server);
			int time = sensor.triggered_time - last_sensor.triggered_time;
			int query = sensor.triggered_query - last_sensor.triggered_query;
			// int new_velocity = 19 * query;
			//debug(SUBMISSION, "last_stop = %d, current_stop = %d, distance = %d, time = %d, query = %d velocity = %d",
			//				 last_stop, current_stop, distance, time, query, velocity);

			// update velocity_data
			// velocity_update(last_stop, current_stop, new_velocity, &velocity_data);
			velocity_update(last_stop, current_stop, time, train_server->current_velocity_data);

			Cli_request update_sensor_request = get_update_sensor_request(sensor, last_stop, next_stop);
			push_cli_req_fifo(train_server, update_sensor_request);
	
			int velocity = velocity_lookup(last_stop, current_stop, train_server->current_velocity_data); 
			Cli_request update_calibration_request =
				get_update_calibration_request(last_stop, current_stop, distance, time, velocity);
			push_cli_req_fifo(train_server, update_calibration_request);
		}
	}
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
		//debug(SUBMISSION, "stopping_distance current stop = %d", last_stop);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}

void br_handle(Train_server *train_server, Command br_cmd)
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	// parse destination
	Sensor stop_sensor = parse_stop_sensor(br_cmd);
	int stop = sensor_to_num(stop_sensor);
	//debug(SUBMISSION, "br_task: stop sensor is %d, %d, stop = %d", stop_sensor.group, stop_sensor.id, stop);
	
	// flip switches such that the train can arrive at the stop
	int num_switch = choose_destination(track, train_server->last_stop, stop, train_server);
	//debug(SUBMISSION, "br_task: send flip %d switches start", num_switch);
	debug(SUBMISSION, "num_switch = %d", num_switch);
	int i;
	for(i = 0; i < num_switch; i++) {
		debug(SUBMISSION, "SW ID = %d, STATE = %d", train_server->br_update[i].id, train_server->br_update[i].state);
		Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
		push_cmd_fifo(train_server, sw_cmd);
	}
	//debug(SUBMISSION, "br_task: flip %d br done", num_switch);
}

void park_handle(Train_server *train_server, Command park_cmd)
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	// parse destination
	Sensor stop_sensor = parse_stop_sensor(park_cmd);
	int stop = sensor_to_num(stop_sensor);
	//debug(SUBMISSION, "park_task, stop sensor is %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

	// retrieve stopping distance
	int stopping_distance = train_server->current_velocity_data->stopping_distance; 
	/*debug(SUBMISSION, "park_task: stopping_distance = %d", stopping_distance);*/

	Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
	int num_park_stops = find_stops_by_distance(track, train_server->last_stop, stop, stopping_distance, park_stops);
	if(num_park_stops == -1){
		return; // there is error, ignore this iteration
	}

	// retrieve the sensor_to_deaccelate_train
	int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
	//debug(SUBMISSION, "park_task: deaccelarate_stop = %c%d",
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
		/*bwprintf(COM2, "last_stop=%d, current_stop=%d\r\n", sensor_src, sensor_dest);*/
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

	/*bwprintf(COM2, "delta=%d, first_dist=%d,first_velo=%d, stop_dist=%d",delta, first_stop_distance, first_stop_velocity, stopping_distance);*/
	bwprintf(COM2, "first_stop[%d]\r\n", park_stops[num_park_stops-1].sensor_id);
	/*debug(SUBMISSION, "first_stop[%d]\r\n", park_stops[num_park_stops-1].sensor_id);*/
	vint delay_distance = delta - stopping_distance;
	vint delay_velocity = first_stop_distance/first_stop_velocity;
	park_delay_time = delay_distance/ delay_velocity;

	//debug(SUBMISSION, "park_task: delta = %d, park_delay_time = %d", delta, park_delay_time);

	int last_stop = train_server->last_stop;
	if (last_stop != deaccelarate_stop) {
		last_stop = train_server->last_stop;
	}
	else {
		//debug(SUBMISSION, "park_task current stop = %d, start delay", last_stop);
		Delay(park_delay_time);
		//debug(SUBMISSION, "park_task send tr %d", tr_cmd.arg0);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}
