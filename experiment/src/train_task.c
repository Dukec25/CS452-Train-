#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>

void train_task_startup()
{
	bwputc(COM1, START); // switches won't work without start command
	cli_startup();

	irq_io_tasks_cluster();
	// velocity4 initialization
//	Velocity_data velocity_data;
//	velocity14_initialization(&velocity_data);
//	velocity_lookup(78, 43, &velocity_data);

	initialize_switch();
	sensor_initialization();

	int tid;

	tid = Create(PRIOR_MEDIUM, cli_server);
	dump(SUBMISSION, "created sensor_task taskId = %d", tid);

	tid = Create(PRIOR_MEDIUM, train_server);
	dump(SUBMISSION, "created train_task taskId = %d", tid);
	
	Exit();
}

void train_server()
{
	// train_server initialization 
	Train_server train_server;
	fifo_init(&train_server.cmd_fifo);
	train_server.sensor_lifo_top = -1;
	train_server.is_shutdown = 0;
	train_server.last_stop = -1;
	train_server.num_sensor_polls = 0;
	train_server.is_park = 0;
	train_server.sensor_to_deaccelate_train = -1;
	train_server.park_delay_time = -1;
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server.switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}

	// velocity4 initialization
	Velocity_data velocity_data;
	velocity14_initialization(&velocity_data);

	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	char sensor_group = 0;

	int result = RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	dump(SUBMISSION, "cli_server %d", cli_server_tid);

	Handshake cli_server_handshake = HANDSHAKE_AKG;
	Handshake requester_handshake = HANDSHAKE_AKG;

	train_server.sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
	dump(SUBMISSION, "sensor_reader_tid %d", train_server.sensor_reader_tid);

	int stopping_distance_tid = Create(PRIOR_MEDIUM, stopping_distance_collector_task);
	dump(SUBMISSION, "stopping_distance_tid %d", stopping_distance_tid);
	vint train_server_address = (vint) &train_server;
	dump(SUBMISSION, "train_server sending train_server_address = 0x%x", train_server_address);	 
	Send(stopping_distance_tid, &train_server_address, sizeof(train_server_address), &requester_handshake, sizeof(requester_handshake));

	while(!train_server.is_shutdown) {
		int requester_tid = INVALID_TID;
		Command request;
		Receive(&requester_tid, &request, sizeof(request));
		requester_handshake = train_server.is_shutdown ? HANDSHAKE_SHUTDOWN : HANDSHAKE_AKG;
		Reply(requester_tid, &requester_handshake, sizeof(requester_handshake));
		fifo_put(&train_server.cmd_fifo, &request);
		dump(SUBMISSION, "%s", "ts put cmd");

		if (is_fifo_empty(&train_server.cmd_fifo)) {
			continue;
		}
		
		Command *cmd;
		fifo_get(&train_server.cmd_fifo, &cmd);
		dump(SUBMISSION, "ts get cmd type %d", cmd->type);

		// handle TR, RV, SW, GO, STOP
		Cli_request cli_update_request;
		switch (cmd->type) {
		case TR:
			dump(SUBMISSION, "%s", "handle tr cmd");
			command_handle(cmd, &train_server);
			cli_update_request.type = CLI_UPDATE_TRAIN;
			cli_update_request.train_update.id = cmd->arg0; 
			cli_update_request.train_update.speed = cmd->arg1;
			train_server.train.id = cmd->arg0;
			train_server.train.speed = cmd->arg1;
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &cli_server_handshake, sizeof(cli_server_handshake));
			break;
		case RV:
			dump(SUBMISSION, "%s", "handle rv cmd");
			command_handle(cmd, &train_server);
			break;
		case SW:
			dump(SUBMISSION, "%s", "handle sw cmd");
			command_handle(cmd, &train_server);
			cli_update_request.type = CLI_UPDATE_SWITCH;
			cli_update_request.switch_update.id = cmd->arg0; 
			cli_update_request.switch_update.state = cmd->arg1;
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &cli_server_handshake, sizeof(cli_server_handshake));
			break;
		case GO:
			command_handle(cmd, &train_server);
			break;
		case STOP:
			command_handle(cmd, &train_server);
			break;
		default:
			break;
		}

		// handle DC, PARK, and SENSOR
		if (cmd->type == DC) {
			//debug(SUBMISSION, "train_server handle dc cmd, %d, %d", cmd->arg0, cmd->arg1);
			Send(stopping_distance_tid, cmd, sizeof(*cmd), &requester_handshake, sizeof(requester_handshake));
		}
        else if(cmd->type == PARK) {
            int i;

			// flag is_park
			train_server.is_park = 1;

			// parse destination
			Sensor stop_sensor;
			stop_sensor.group = toupper(cmd->arg0) - SENSOR_LABEL_BASE;
			stop_sensor.id = cmd->arg1;
			int stop = sensor_to_num(stop_sensor);
			debug(SUBMISSION, "train_server handle PARK: stop sensor is %d, %d, stop = %d", stop_sensor.group, stop_sensor.id, stop);

			// flip switches such that the train can arrive at the stop
            int num_switch = choose_destination(track, train_server.last_stop, stop, &train_server, &cli_update_request);
            cli_update_request.type = CLI_UPDATE_SWITCH;
            for(i=0; i<num_switch; i++) {
                cli_update_request.switch_update.id = cli_update_request.br_update[i].id; 
                cli_update_request.switch_update.state = cli_update_request.br_update[i].state;
                Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request),
					 &cli_server_handshake, sizeof(cli_server_handshake));
            }
			debug(SUBMISSION, "train_server handle PARK: %d br done", num_switch);

			// retrieve stopping distance
			int stopping_distance = velocity_data.stopping_distance;
			debug(SUBMISSION, "train_server handle PARK: stopping_distance = %d", stopping_distance);

			Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
			int num_park_stops = find_stops_by_distance(track, train_server.last_stop, stop, stopping_distance, park_stops);

			// retrieve the sensor_to_deaccelate_train
			int sensor_to_deaccelate_train = park_stops[num_park_stops - 1].sensor_id; // need to fill in
			train_server.sensor_to_deaccelate_train = sensor_to_deaccelate_train;
			debug(SUBMISSION, "train_server handle PARK: sensor_to_deaccelate_train = %d", sensor_to_deaccelate_train);

			// calculate the delta = the distance between sensor_to_deaccelate_train
			// calculate average velocity
			int delta = 0;
			int weighted_avg_velocity = 0;
			for (i = 0; i < num_park_stops; i++) {
				int sensor_distance = park_stops[i].distance;
				int sensor_src = (i - 1 < 0) ? stop : park_stops[i - 1].sensor_id;
				int sensor_dest = park_stops[i].sensor_id;
				int sensor_velocity = velocity_lookup(sensor_src, sensor_dest, &velocity_data);
				sensor_velocity = (sensor_velocity == -1) ? 0: sensor_velocity;

				delta += sensor_distance; 
				weighted_avg_velocity += sensor_distance * sensor_velocity;
			}
			weighted_avg_velocity /= delta;
			debug(SUBMISSION, "train_server handle PARK: delta = %d, avg_velocity = %d", delta, weighted_avg_velocity);

			// calculate delay time
			int park_delay_time = (delta - stopping_distance) / weighted_avg_velocity; // in [mm] / ([mm] / [tick]) = [tick]
			train_server.park_delay_time = park_delay_time;
			debug(SUBMISSION, "train_server handle PARK: park_delay_time = %d", park_delay_time);
        } else if (cmd->type == SENSOR) {
			uint16 sensor_data[SENSOR_GROUPS];
			dump(SUBMISSION, "%s", "sensor cmd");
			Putc(COM1, SENSOR_QUERY);
			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				char lower = Getc(COM1);
				char upper = Getc(COM1);
				sensor_data[(int) sensor_group] = upper << 8 | lower;
			}
			train_server.num_sensor_polls++;
			dump(SUBMISSION, "sensor queried, num_sensor_polls = %d", train_server.num_sensor_polls);
			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				if (sensor_data[(int) sensor_group] == 0) {
					continue;
				}
				dump(SUBMISSION, "sensor_data[%d] = %d", sensor_group, sensor_data[(int) sensor_group]);
				char bit = 0;
				for (bit = 0; bit < SENSORS_PER_GROUP; bit++) {
					//sensor_data actually looks like 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
					if (!(sensor_data[(int) sensor_group] & (0x1 << bit))) {
						continue;
					}
					Sensor sensor;
					sensor.group = sensor_group;
					sensor.triggered_time = Time();
					sensor.triggered_poll = train_server.num_sensor_polls;
					if (bit + 1 <= 8) {
						sensor.id = 8 - bit;
					}
					else {
						sensor.id = 8 + 16 - bit;
					}

					// distance	
					int current_location = sensor_to_num(sensor);
					int distance = cal_distance(track, train_server.last_stop, current_location);
                    int next_location = predict_next(track, current_location, &train_server);
    
					// Send sensor update
					if (current_location != train_server.last_stop) {
						Cli_request sensor_update_request;
						sensor_update_request.type = CLI_UPDATE_SENSOR;
						sensor_update_request.sensor_update = sensor;
						sensor_update_request.last_sensor_update = train_server.last_stop;
						sensor_update_request.next_sensor_update = next_location;
						Send(cli_server_tid, &sensor_update_request, sizeof(sensor_update_request),
						 	&cli_server_handshake, sizeof(cli_server_handshake));
					}
				
	                /*test_sensor(next_location);*/

					// velcoity
					int start_time = 0;
					int start_poll = 0; 
					while (train_server.sensor_lifo_top != -1) {
						Sensor last_sensor;
						last_sensor = train_server.sensor_lifo[train_server.sensor_lifo_top];
						train_server.sensor_lifo_top -= 1;
						dump(SUBMISSION, "pop sensor group = %d, id = %d, time = %d",
										 sensor.group, sensor.id, sensor.triggered_time);
						if (sensor_to_num(last_sensor) == train_server.last_stop) {
							start_time = last_sensor.triggered_time;
							start_poll = last_sensor.triggered_poll;
							break;
						}
					}
					// Send calibration update
					if (distance != 0) {
						int end_time = sensor.triggered_time;
						int end_poll = sensor.triggered_poll;
						int time = end_time - start_time;
						int poll = end_poll - start_poll;
						// int velocity = distance / (20 * poll);
						int velocity = velocity_lookup(train_server.last_stop, current_location, &velocity_data);

					//	debug(SUBMISSION, "last_stop = %d, current_location = %d, distance = %d, time = %d, poll = %d velocity = %d",
					//					 train_server.last_stop, current_location, distance, time, poll, velocity);
						Cli_request calibration_update_request;
						calibration_update_request.type = CLI_UPDATE_CALIBRATION;
						calibration_update_request.calibration_update.src = train_server.last_stop;
						calibration_update_request.calibration_update.dest = current_location;
						calibration_update_request.calibration_update.distance = distance;
						calibration_update_request.calibration_update.time = time;
						calibration_update_request.calibration_update.velocity = velocity; 
						Send(cli_server_tid, &calibration_update_request, sizeof(calibration_update_request),
							 &cli_server_handshake, sizeof(cli_server_handshake));
					}

					if (train_server.sensor_lifo_top != SENSOR_LIFO_SIZE - 1) {
						train_server.sensor_lifo_top += 1;
						train_server.sensor_lifo[train_server.sensor_lifo_top] = sensor;
						dump(SUBMISSION, "insert sensor group = %d, id = %d, time = %d",
										 sensor.group, sensor.id, sensor.triggered_time);
					}
					train_server.last_stop = current_location;
				}
			}
		}

		// deaccelerate
		if (train_server.is_park) {
			if (train_server.last_stop == train_server.sensor_to_deaccelate_train) {
				debug(SUBMISSION, "deaccelerate: train just passed %d", train_server.sensor_to_deaccelate_train);
				Delay(train_server.park_delay_time);
				Command stop_cmd;
				stop_cmd.type = TR;
				stop_cmd.arg0 = train_server.train.id;
				stop_cmd.arg1 = MIN_SPEED;
				fifo_put(&train_server.cmd_fifo, &stop_cmd);

				// reset
				train_server.is_park = 0;
				train_server.sensor_to_deaccelate_train = -1;
				train_server.park_delay_time = -1;
			}
		}
	}
	Exit();
}

void sensor_reader_task()
{
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	Handshake handshake = HANDSHAKE_AKG;
 
	while (handshake != HANDSHAKE_SHUTDOWN) {
		Delay(20);	// update every 200ms
		Command train_cmd;
		train_cmd.type = SENSOR;
		Send(train_server_tid, &train_cmd, sizeof(&train_cmd), &handshake, sizeof(handshake));
	}
	Exit();
}

void stopping_distance_collector_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	dump(SUBMISSION, "stopping_distance train_server_address = 0x%x", train_server_address);	 

	while (handshake != HANDSHAKE_SHUTDOWN) {
		Command dc_cmd;
		Receive(&train_server_tid, &dc_cmd, sizeof(dc_cmd));
		handshake = HANDSHAKE_AKG;
		Reply(train_server_tid, &handshake, sizeof(handshake));
//		debug(SUBMISSION, "receive dc cmd %d %d", dc_cmd.arg0, dc_cmd.arg1);

		Sensor stop_sensor;
		stop_sensor.group = toupper(dc_cmd.arg0) - SENSOR_LABEL_BASE;
		stop_sensor.id = dc_cmd.arg1;
		int stop = sensor_to_num(stop_sensor);
		debug(SUBMISSION, "receive dc cmd, start stop at %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

		int last_stop = train_server->last_stop;
		while (last_stop != stop) {
			last_stop = train_server->last_stop;
			Pass();
		}
		debug(SUBMISSION, "stopping_distance current stop = %d\r\n", last_stop);
		Command tr_cmd;
		tr_cmd.type = TR;
		tr_cmd.arg0 = train_server->train.id;
		tr_cmd.arg1 = MIN_SPEED;
		debug(SUBMISSION, "stopping_distance send tr %d", tr_cmd.arg0);
		Send(train_server_tid, &tr_cmd, sizeof(tr_cmd), &handshake, sizeof(handshake));
	}
}

void cli_server()
{
	Cli_server cli_server;
	fifo_init(&cli_server.cmd_fifo);
	fifo_init(&cli_server.status_update_fifo);
	cli_server.is_shutdown = 0;

	int result = RegisterAs("CLI_SERVER");
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	cli_server.cli_clock_tid = Create(PRIOR_MEDIUM, cli_clock_task); 
	cli_server.cli_io_tid = Create(PRIOR_MEDIUM, cli_io_task);

	Handshake train_server_handshake = HANDSHAKE_AKG;
	Handshake requester_handshake = HANDSHAKE_AKG;

	int num_track_updates = 0;
	do {
		int requester_tid = INVALID_TID;
		Cli_request request;
		Receive(&requester_tid, &request, sizeof(request));
		requester_handshake = cli_server.is_shutdown ? HANDSHAKE_SHUTDOWN : HANDSHAKE_AKG;
		Reply(requester_tid, &requester_handshake, sizeof(requester_handshake));

		switch (request.type) {
		case CLI_TRAIN_COMMAND:
			fifo_put(&cli_server.cmd_fifo, &request);
			dump(SUBMISSION, "%s", "cli_server put train cmd cli req");
			break;

		case CLI_UPDATE_TRAIN:
		case CLI_UPDATE_SENSOR:
		case CLI_UPDATE_SWITCH:
		case CLI_UPDATE_CLOCK:
		case CLI_UPDATE_CALIBRATION:
			fifo_put(&cli_server.status_update_fifo, &request);
			if (request.type != CLI_UPDATE_CLOCK) dump(SUBMISSION, "update cli req %d", request.type);
			break;

		case CLI_SHUTDOWN:
			cli_server.is_shutdown = 1;
			dump(SUBMISSION, "%s", "shutdown...");
			break;
		}
		
		if (!is_fifo_empty(&cli_server.cmd_fifo)) {
			Cli_request *cli_cmd_request;
			fifo_get(&cli_server.cmd_fifo, &cli_cmd_request);
			Command train_cmd = cli_cmd_request->cmd;
			dump(SUBMISSION, "cli send train cmd %d", train_cmd.type);
			Send(train_server_tid, &train_cmd, sizeof(train_cmd), &train_server_handshake, sizeof(train_server_handshake));
		}

		if (!is_fifo_empty(&cli_server.status_update_fifo)) {
			Cli_request *update_request;
			fifo_get(&cli_server.status_update_fifo, &update_request);
			switch (update_request->type) {
			case CLI_UPDATE_CLOCK:
				cli_update_clock(update_request->clock_update);
				break;
			case CLI_UPDATE_TRAIN:
				dump(SUBMISSION, "%s", "cli pop train update req");
				cli_update_train(update_request->train_update);
				break;
			case CLI_UPDATE_SWITCH:
				dump(SUBMISSION, "%s", "cli pop switch update req");
				cli_update_switch(update_request->switch_update);
				break;
			case CLI_UPDATE_SENSOR:
				dump(SUBMISSION, "%s", "cli pop sensor group = %d, id = %d, time = %d",
							update_request->sensor_update.group, update_request->sensor_update.id,
							update_request->sensor_update.triggered_time);		
				cli_update_sensor(update_request->sensor_update, update_request->last_sensor_update, update_request->next_sensor_update);
				break;
			case CLI_UPDATE_CALIBRATION:
				dump(SUBMISSION, "%s", "cli pop calibration update req");
				cli_update_track(update_request->calibration_update, num_track_updates++);
				break;
			default:
				break;
			}
		}
	} while (!cli_server.is_shutdown);
	Terminate();
}

void cli_clock_task()
{
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}

	// digital clock
	vint elapsed_tenth_sec = 0;
	Clock clock;
	clock_init(&clock);

	Handshake handshake = HANDSHAKE_AKG;
 
	while (handshake != HANDSHAKE_SHUTDOWN) {
		Delay(10);	// update every 100ms
		elapsed_tenth_sec++;
		clock_update(&clock, elapsed_tenth_sec);

		Cli_request cli_update_request;
		cli_update_request.type = CLI_UPDATE_CLOCK;
		cli_update_request.clock_update = clock;
		Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &handshake, sizeof(handshake));
	}
	Exit();
}

void cli_io_task()
{
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}

	// command
	Command_buffer command_buffer;
	command_buffer.pos = 0;
	Train train;
	int parse_result = 0;

	Handshake handshake = HANDSHAKE_AKG;

	while(handshake != HANDSHAKE_SHUTDOWN) {
		// user I/O
		char c = Getc(COM2);
		if (c == 'q') {
			Cli_request cli_request;
			cli_request.type = CLI_SHUTDOWN;	
			dump(SUBMISSION, "%s", "io entered q, send shutdown");
			Send(cli_server_tid, &cli_request, sizeof(cli_request), &handshake, sizeof(handshake));
		}
		else if (c == '\r') {
			// user hits ENTER
			// parse command
			Command cmd;
			parse_result = command_parse(&command_buffer, &train, &cmd);
			if (parse_result != -1) {
				Cli_request cli_cmd_request;
				cli_cmd_request.type = CLI_TRAIN_COMMAND;
				cli_cmd_request.cmd = cmd;
				dump(SUBMISSION, "%s", "io entered train cmd, send cmd");
				Send(cli_server_tid, &cli_cmd_request, sizeof(cli_cmd_request), &handshake, sizeof(handshake));
			}
			// clears command_buffer
			command_clear(&command_buffer);
		}
		else {
			command_buffer.data[command_buffer.pos++] = c;
			Putc(COM2, c);
		}
	}
	Exit();
}
