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

	train_server->last_stop = -1;
	train_server->last_sensor_triggered_time = 0;

	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server->switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}

	velocity69_initialization(&train_server->velocity69_model);
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
	init_trackb(track);

	RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	/*irq_debug(SUBMISSION, "cli_server %d", cli_server_tid);*/

	Handshake handshake = HANDSHAKE_AKG;
	vint train_server_address = (vint) &train_server;
	/*irq_debug(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 */

	int sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
	/*irq_debug(SUBMISSION, "sensor_reader_tid %d", sensor_reader_tid);*/
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
				//irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
			}
			Reply(requester_tid, &cli_req, sizeof(cli_req));
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
		case TR:
			irq_debug(SUBMISSION, "handle tr cmd %d %d", cmd.arg0, cmd.arg1);
			command_handle(&cmd);

			train_server.train.id = cmd.arg0;
			train_server.train.speed = cmd.arg1;

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
			irq_debug(SUBMISSION, "handle park cmd %c%d", cmd.arg0, cmd.arg1);
			train_server.is_special_cmd = 1;
			train_server.special_cmd = cmd;
			br_handle(&train_server, cmd);
			break;

		case MC:
			mc_handle(&train_server, cmd);
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
		Command sensor_cmd = get_sensor_command();
		TS_request ts_request;
		ts_request.type = TS_COMMAND;
		ts_request.cmd = sensor_cmd;
		Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));
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
	init_trackb(track);

	// sensor query
	/*irq_debug(SUBMISSION, "%s", "sensor cmd");*/
	Putc(COM1, SENSOR_QUERY);
	uint16 sensor_data[SENSOR_GROUPS];
	int sensor_group = 0;
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		char lower = Getc(COM1);
		char upper = Getc(COM1);
		sensor_data[(int) sensor_group] = upper << 8 | lower;
	}

	// parse sensor data
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		if (sensor_data[(int) sensor_group] == 0) {
			continue;
		}
		/*irq_debug(SUBMISSION, "sensor_data[%d] = %d", sensor_group, sensor_data[(int) sensor_group]);*/
		char bit = 0;
		for (bit = 0; bit < SENSORS_PER_GROUP; bit++) {
			//sensor_data actually looks like 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
			if (!(sensor_data[(int) sensor_group] & (0x1 << bit))) {
				continue;
			}

			Sensor sensor;
			sensor.group = sensor_group;
			if (bit + 1 <= 8) {
				sensor.id = 8 - bit;
			}
			else {
				sensor.id = 8 + 16 - bit;
			}

			int current_stop = sensor_to_num(sensor);
			int last_stop = train_server->last_stop;
			int current_sensor_triggered_time = Time();
			int last_sensor_triggered_time = train_server->last_sensor_triggered_time;

			//if ((current_stop == last_stop) || (last_stop == -1)) {
			if (current_stop == last_stop) {
				return;
			}

			// update last triggered sensor: if a sensor is triggered consecutively, only record the the earliest time
			train_server->last_stop = current_stop;
			train_server->last_sensor_triggered_time = current_sensor_triggered_time; 

			// calculate distance, next stop, time, and new_velocity
			int distance = cal_distance(track, last_stop, current_stop);
			int next_stop = predict_next(track, current_stop, train_server);
			int time = current_sensor_triggered_time - last_sensor_triggered_time;
			double real_velocity = (double) distance / (double) time;
			///irq_debug(SUBMISSION, "distance = %d, time = %d", distance, time);

			// update velocity_data
			velocity_update(train_server->train.speed, real_velocity, &train_server->velocity69_model);

			Cli_request update_sensor_request = get_update_sensor_request(sensor, last_stop, next_stop);
			push_cli_req_fifo(train_server, update_sensor_request);

			Cli_request update_calibration_request = get_update_calibration_request(last_stop, current_stop, distance,
													(int) real_velocity,
													(int) train_server->velocity69_model.velocity[train_server->train.speed]);
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
		//irq_debug(SUBMISSION, "stopping_distance current stop = %d", last_stop);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}

void br_handle(Train_server *train_server, Command br_cmd)
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_trackb(track);

	// parse destination
	Sensor stop_sensor = parse_stop_sensor(br_cmd);
	int stop = sensor_to_num(stop_sensor);
	irq_debug(SUBMISSION, "br_task: stop sensor is %d, %d, stop = %d", stop_sensor.group, stop_sensor.id, stop);
	
	// flip switches such that the train can arrive at the stop
	int num_switch = choose_destination(track, train_server->last_stop, stop, train_server);
	//irq_debug(SUBMISSION, "br_task: send flip %d switches start", num_switch);
	irq_debug(SUBMISSION, "num_switch = %d", num_switch);
	int i;
	for(i = 0; i < num_switch; i++) {
		irq_debug(SUBMISSION, "SW ID = %d, STATE = %d", train_server->br_update[i].id, train_server->br_update[i].state);
		Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
		push_cmd_fifo(train_server, sw_cmd);
	}
	//irq_debug(SUBMISSION, "br_task: flip %d br done", num_switch);
}

void park_handle(Train_server *train_server, Command park_cmd)
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_trackb(track);

	// parse destination
	Sensor stop_sensor = parse_stop_sensor(park_cmd);
	int stop = sensor_to_num(stop_sensor);
	//irq_debug(SUBMISSION, "park_task, stop sensor is %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

	// retrieve stopping distance
	int stopping_distance = train_server->velocity69_model.stopping_distance[train_server->train.speed]; 

	Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
	int num_park_stops = find_stops_by_distance(track, train_server->last_stop, stop, stopping_distance, park_stops);
	if(num_park_stops == -1){
		return; // there is error, ignore this iteration
	}

	// retrieve the sensor_to_deaccelate_train
	int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in

	// calculate the delta = the distance between sensor_to_deaccelate_train
	// calculate average velocity measured in [tick]
	int delta = 0;
	int velocity = train_server->velocity69_model.velocity[train_server->train.speed];
	int i;
	for (i = 0; i < num_park_stops; i++) {
		delta += park_stops[i].distance;
	}

	int park_delay_time = (delta - stopping_distance * 1000) / velocity;
	irq_debug(SUBMISSION, "deaccelarate_stop=%d, delta=%d, stop_dist=%d, park_delay_time = %d",
							deaccelarate_stop, delta, stopping_distance, park_delay_time);

	int last_stop = train_server->last_stop;
	if (last_stop != deaccelarate_stop) {
		last_stop = train_server->last_stop;
	}
	else {
		//irq_debug(SUBMISSION, "park_task current stop = %d, start delay", last_stop);
		Delay(park_delay_time);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		//irq_debug(SUBMISSION, "park_task send tr %d", tr_cmd.arg0);
		push_cmd_fifo(train_server, tr_cmd);

		train_server->is_special_cmd = 0;
	}
}

void mc_handle(Train_server *train_server, Command mc_cmd)
{
	int speed = mc_cmd.arg0;
	int delay_time = mc_cmd.arg1;
	irq_debug(SUBMISSION, "mc: speed = %d, delay_time = %d 100ms", speed, delay_time);

	Command tr_cmd = get_tr_command(train_server->train.id, speed);
	command_handle(&tr_cmd);

	Delay(delay_time * 10);
	
	Command tr_stop_cmd = get_tr_stop_command(train_server->train.id);
	command_handle(&tr_stop_cmd);
}
