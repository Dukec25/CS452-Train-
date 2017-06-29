#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>
#include <kernel.h>
#include <calculation.h>
#include <name_server.h>

void train_task_startup()
{
	cli_startup();
	bwputc(COM1, START); // switches won't work without start command

	irq_io_tasks_cluster();

	initialize_switch();
	sensor_initialization();

	int tid;

	tid = Create(PRIOR_MEDIUM, cli_server);
	dump(SUBMISSION, "created sensor_task taskId = %d", tid);

	tid = Create(PRIOR_MEDIUM, train_server);
	dump(SUBMISSION, "created train_task taskId = %d", tid);
	
	Exit();
}

void train_server_init(Train_server *train_server)
{
	train_server->cmd_fifo_head = 0;
	train_server->cmd_fifo_tail = 0;

	train_server->sensor_lifo_top = -1;
	train_server->last_stop = -1;
	train_server->num_sensor_query = 0;

	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server->switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}
}

void train_server()
{
	Handshake handshake = HANDSHAKE_AKG;

	// train_server initialization 
	Train_server train_server;
	train_server_init(&train_server);

	// velocity4 initialization
	Velocity_data velocity_data;
	velocity14_initialization(&velocity_data);

	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	dump(SUBMISSION, "cli_server %d", cli_server_tid);


	train_server.sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
	dump(SUBMISSION, "sensor_reader_tid %d", train_server.sensor_reader_tid);

	vint train_server_address = (vint) &train_server;
	dump(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 

	int stopping_distance_tid = Create(PRIOR_MEDIUM, stopping_distance_collector_task);
	dump(SUBMISSION, "stopping_distance_tid %d", stopping_distance_tid);
	Send(stopping_distance_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int br_tid = Create(PRIOR_MEDIUM, br_task);
	dump(SUBMISSION, "br_tid %d", br_tid);
	Send(br_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int park_tid = Create(PRIOR_MEDIUM, park_task);
	dump(SUBMISSION, "park_tid %d", park_tid);
	Send(park_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	while(1) {
		// receive command request
		int requester_tid;
		Command request;
		Receive(&requester_tid, &request, sizeof(request));
		handshake = HANDSHAKE_AKG;
		Reply(requester_tid, &handshake, sizeof(handshake));

		// push command request onto the fifo
		int cmd_fifo_put_next = train_server.cmd_fifo_head + 1;
		if (cmd_fifo_put_next != train_server.cmd_fifo_tail) {
			if (cmd_fifo_put_next >= COMMAND_FIFO_SIZE) {
				cmd_fifo_put_next = 0;
			}
		}
		train_server.cmd_fifo[train_server.cmd_fifo_head] = request;
		train_server.cmd_fifo_head = cmd_fifo_put_next;
		dump(SUBMISSION, "%s", "ts put cmd");

		if (train_server.cmd_fifo_head == train_server.cmd_fifo_tail) {
			// cmd_fifo is empty
			continue;
		}

		// pop command off the fifo		
		Command cmd;
		int cmd_fifo_get_next = train_server.cmd_fifo_tail + 1;
		if (cmd_fifo_get_next >= COMMAND_FIFO_SIZE) {
			cmd_fifo_get_next = 0;
		}
		cmd = train_server.cmd_fifo[train_server.cmd_fifo_tail];
		train_server.cmd_fifo_tail = cmd_fifo_get_next;
		dump(SUBMISSION, "ts get cmd type %d", cmd.type);

		// handle TR, RV, SW, GO, STOP
		Cli_request cli_update_request;
		switch (cmd.type) {
		case TR:
			dump(SUBMISSION, "%s", "handle tr cmd");
			command_handle(&cmd);

			train_server.train.id = cmd.arg0;
			train_server.train.speed = cmd.arg1;
	
			cli_update_request = get_update_train_request(cmd.arg0, cmd.arg1);
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &handshake, sizeof(handshake));
			break;
		case RV:
			dump(SUBMISSION, "%s", "handle rv cmd");
			command_handle(&cmd);
			break;
		case SW:
			dump(SUBMISSION, "%s", "handle sw cmd");
			command_handle(&cmd);

			train_server.switches_status[cmd.arg0 - 1] = switch_state_to_byte(cmd.arg1);

			cli_update_request = get_update_switch_request(cmd.arg0, cmd.arg1);
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &handshake, sizeof(handshake));
			break;
		case GO:
			command_handle(&cmd);
			break;
		case STOP:
			command_handle(&cmd);
			break;
		default:
			break;
		}

		// handle DC, PARK, and SENSOR
		if (cmd.type == BR) {
			//debug(SUBMISSION, "train_server handle br cmd %c%d", cmd.arg0, cmd.arg1);
			Send(br_tid, &cmd, sizeof(cmd), &handshake, sizeof(handshake));
		}
		else if (cmd.type == DC) {
			//debug(SUBMISSION, "train_server handle dc cmd, %c%d", cmd.arg0, cmd.arg1);
			Send(stopping_distance_tid, &cmd, sizeof(cmd), &handshake, sizeof(handshake));
		}
		else if(cmd.type == PARK) {
			//debug(SUBMISSION, "train_server handle park cmd, %c%d", cmd.arg0, cmd.arg1);
			Send(park_tid, &cmd, sizeof(cmd), &handshake, sizeof(handshake));
		}
		else if (cmd.type == SENSOR) {
			// sensor query
			dump(SUBMISSION, "%s", "sensor cmd");
			Putc(COM1, SENSOR_QUERY);
			uint16 sensor_data[SENSOR_GROUPS];
			int sensor_group = 0;
			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				char lower = Getc(COM1);
				char upper = Getc(COM1);
				sensor_data[(int) sensor_group] = upper << 8 | lower;
			}
			train_server.num_sensor_query++;
			dump(SUBMISSION, "num_sensor_query = %d", train_server.num_sensor_query);

			// parse sensor data
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
					sensor.triggered_query = train_server.num_sensor_query;
					if (bit + 1 <= 8) {
						sensor.id = 8 - bit;
					}
					else {
						sensor.id = 8 + 16 - bit;
					}

					int current_stop = sensor_to_num(sensor);
					int last_stop = train_server.last_stop;
			
					// pop off last triggered sensor
					Sensor last_sensor;
					while (train_server.sensor_lifo_top != -1) {
						last_sensor = train_server.sensor_lifo[train_server.sensor_lifo_top];
						train_server.sensor_lifo_top -= 1;
						dump(SUBMISSION, "pop sensor group = %d, id = %d, time = %d",
										 sensor.group, sensor.id, sensor.triggered_time);
						if (sensor_to_num(last_sensor) == last_stop) {
							break;
						}
					}

					// push current triggered sensor onto lifo
					if (train_server.sensor_lifo_top != SENSOR_LIFO_SIZE - 1) {
						train_server.sensor_lifo_top += 1;
						train_server.sensor_lifo[train_server.sensor_lifo_top] = sensor;
						dump(SUBMISSION, "insert sensor group = %d, id = %d, time = %d",
										 sensor.group, sensor.id, sensor.triggered_time);
					}

					// update last triggered sensor
					train_server.last_stop = current_stop;
	
					if (current_stop == last_stop) {
						continue;
					}

					// calculate distance, next stop, time, and new_velocity
					int distance = cal_distance(track, last_stop, current_stop);
					int next_stop = predict_next(track, current_stop, &train_server);
					int time = sensor.triggered_time - last_sensor.triggered_time;
					int query = sensor.triggered_query - last_sensor.triggered_query;
					// int new_velocity = 19 * query;
					//debug(SUBMISSION, "last_stop = %d, current_stop = %d, distance = %d, time = %d, query = %d velocity = %d",
					//				 last_stop, current_stop, distance, time, query, velocity);

					// update velocity_data
					// velocity_update(last_stop, current_stop, new_velocity, &velocity_data);
					velocity_update(last_stop, current_stop, time, &velocity_data);

					// Send sensor update
					Cli_request update_sensor_request = get_update_sensor_request(sensor, last_stop, next_stop);
					Send(cli_server_tid, &update_sensor_request, sizeof(update_sensor_request), &handshake, sizeof(handshake));
	
					// Send calibration update
					int velocity = velocity_lookup(last_stop, current_stop, &velocity_data); 
					Cli_request update_calibration_request =
						get_update_calibration_request(last_stop, current_stop, distance, time, velocity);
					Send(cli_server_tid, &update_calibration_request, sizeof(update_calibration_request), &handshake, sizeof(handshake));
				}
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
	while (1) {
		Delay(20);	// update every 200ms
		Command sensor_cmd = get_sensor_command();
		Send(train_server_tid, &sensor_cmd, sizeof(sensor_cmd), &handshake, sizeof(handshake));
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
	debug(SUBMISSION, "stopping_distance train_server_address = 0x%x", train_server_address);	 

	while (1) {
		Command dc_cmd;
		Receive(&train_server_tid, &dc_cmd, sizeof(dc_cmd));
		handshake = HANDSHAKE_AKG;
		Reply(train_server_tid, &handshake, sizeof(handshake));
		//debug(SUBMISSION, "receive dc cmd %c%d", dc_cmd.arg0, dc_cmd.arg1);

		Sensor stop_sensor;
		stop_sensor.group = toupper(dc_cmd.arg0) - SENSOR_LABEL_BASE;
		stop_sensor.id = dc_cmd.arg1;
		int stop = sensor_to_num(stop_sensor);
		//debug(SUBMISSION, "receive dc cmd, stop_sensor at %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

		int last_stop = train_server->last_stop;
		while (last_stop != stop) {
			last_stop = train_server->last_stop;
			Pass();
		}
		//debug(SUBMISSION, "stopping_distance current stop = %d", last_stop);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		//debug(SUBMISSION, "stopping_distance send tr %d", tr_cmd.arg0);
		Send(train_server_tid, &tr_cmd, sizeof(tr_cmd), &handshake, sizeof(handshake));
	}
	Exit();
}

void br_task()
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	//debug(SUBMISSION, "br_task train_server_address = 0x%x", train_server_address);	 

	while (1) {
		Command br_cmd;
		Receive(&train_server_tid, &br_cmd, sizeof(br_cmd));
		handshake = HANDSHAKE_AKG;
		Reply(train_server_tid, &handshake, sizeof(handshake));
		//deubg(SUBMISSION, "receive br cmd %c%d", br_cmd.arg0, br_cmd.arg1);

		// parse destination
		Sensor stop_sensor;
		stop_sensor.group = toupper(br_cmd.arg0) - SENSOR_LABEL_BASE;
		stop_sensor.id = br_cmd.arg1;
		int stop = sensor_to_num(stop_sensor);
		//debug(SUBMISSION, "br_task: stop sensor is %d, %d, stop = %d", stop_sensor.group, stop_sensor.id, stop);
	
		// flip switches such that the train can arrive at the stop
		int num_switch = choose_destination(track, train_server->last_stop, stop, train_server);
		//debug(SUBMISSION, "br_task: send flip %d switches start", num_switch);
		int i;
		for(i = 0; i < num_switch; i++) {
			Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
			Send(train_server_tid, &sw_cmd, sizeof(sw_cmd), &handshake, sizeof(handshake));
		}
		//debug(SUBMISSION, "br_task: flip %d br done", num_switch);
	}
	Exit();
}

void park_task()
{
	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	// velocity4 initialization
	Velocity_data velocity_data;
	velocity14_initialization(&velocity_data);

	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	dump(SUBMISSION, "stopping_distance train_server_address = 0x%x", train_server_address);	 

	while (1) {
		Command park_cmd;
		Receive(&train_server_tid, &park_cmd, sizeof(park_cmd));
		handshake = HANDSHAKE_AKG;
		Reply(train_server_tid, &handshake, sizeof(handshake));
		//debug(SUBMISSION, "receive park cmd %c%d", park_cmd.arg0, park_cmd.arg1);

		// parse destination
		Sensor stop_sensor;
		stop_sensor.group = toupper(park_cmd.arg0) - SENSOR_LABEL_BASE;
		stop_sensor.id = park_cmd.arg1;
		int stop = sensor_to_num(stop_sensor);
		//debug(SUBMISSION, "park_task, stop sensor is %d, %d, stop = %d\r\n", stop_sensor.group, stop_sensor.id, stop);

        if (train_server->last_stop < 0 || stop < 0 || train_server->last_stop > TRACK_MAX || stop > TRACK_MAX) {
            // value out of range, don't do anything
            continue;
        }

		// flip switches such that the train can arrive at the stop
		int num_switch = choose_destination(track, train_server->last_stop, stop, train_server);
		//debug(SUBMISSION, "park_task: send flip %d switches start", num_switch);
		int i;
		for(i = 0; i < num_switch; i++) {
			Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
			Send(train_server_tid, &sw_cmd, sizeof(sw_cmd), &handshake, sizeof(handshake));
		}
		//debug(SUBMISSION, "park_task: flip %d br done", num_switch);

		// retrieve stopping distance
		int stopping_distance = velocity_data.stopping_distance;
		//debug(SUBMISSION, "park_task: stopping_distance = %d", stopping_distance);

		Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
		int num_park_stops = find_stops_by_distance(track, train_server->last_stop, stop, stopping_distance, park_stops);

		// retrieve the sensor_to_deaccelate_train
		int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
		//debug(SUBMISSION, "park_task: deaccelarate_stop = %c%d",
						  //num_to_sensor(deaccelarate_stop).group + SENSOR_LABEL_BASE, num_to_sensor(deaccelarate_stop).id);

		// calculate the delta = the distance between sensor_to_deaccelate_train
		// calculate average velocity measured in [tick]
		int delta = 0;
		int park_delay_time = 0;
		for (i = 0; i < num_park_stops; i++) {
			int sensor_distance = park_stops[i].distance;
			int sensor_src = park_stops[i].sensor_id;
			int sensor_dest = (i - 1 < 0) ? stop : park_stops[i - 1].sensor_id;
			int sensor_velocity = velocity_lookup(sensor_src, sensor_dest, &velocity_data);
			sensor_velocity = (sensor_velocity == -1) ? 0: sensor_velocity;

			delta += sensor_velocity ? sensor_distance : 0;  // total_distance
			park_delay_time += sensor_distance * sensor_velocity; 
		}
        // park_delay_time/delta is weighted velocity
        int final_delay_time = delta/(park_delay_time/delta);
		park_delay_time /= delta;
		//debug(SUBMISSION, "park_task: delta = %d, park_delay_time = %d", delta, park_delay_time);

		int last_stop = train_server->last_stop;
		while (last_stop != deaccelarate_stop) {
			last_stop = train_server->last_stop;
			Pass();
		}

		//debug(SUBMISSION, "park_task current stop = %d, start delay", last_stop);
		Delay(final_delay_time);
		//debug(SUBMISSION, "park_task send tr %d", tr_cmd.arg0);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		Send(train_server_tid, &tr_cmd, sizeof(tr_cmd), &handshake, sizeof(handshake));
	}
	Exit();
}

void cli_server()
{
	Cli_server cli_server;
	fifo_init(&cli_server.cmd_fifo);
	fifo_init(&cli_server.status_update_fifo);

	RegisterAs("CLI_SERVER");
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	cli_server.cli_clock_tid = Create(PRIOR_MEDIUM, cli_clock_task); 
	cli_server.cli_io_tid = Create(PRIOR_MEDIUM, cli_io_task);

	Handshake handshake = HANDSHAKE_AKG;

	int num_track_updates = 0;
	int is_shutdown = 0;
	while(!is_shutdown) {
		int requester_tid = INVALID_TID;
		Cli_request request;
		Receive(&requester_tid, &request, sizeof(request));
		handshake = HANDSHAKE_AKG;
		Reply(requester_tid, &handshake, sizeof(handshake));

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
			is_shutdown = 1;
			bwprintf(COM2, "%s", "shutdown...");
			break;
		}
		
		if (!is_fifo_empty(&cli_server.cmd_fifo)) {
			Cli_request *cli_cmd_request;
			fifo_get(&cli_server.cmd_fifo, &cli_cmd_request);
			Command train_cmd = cli_cmd_request->cmd;
			dump(SUBMISSION, "cli send train cmd %d", train_cmd.type);
			Send(train_server_tid, &train_cmd, sizeof(train_cmd), &handshake, sizeof(handshake));
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
	}
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
 
	while (1) {
		Delay(10);	// update every 100ms
		elapsed_tenth_sec++;
		clock_update(&clock, elapsed_tenth_sec);

		Cli_request update_clock_request = get_update_clock_request(clock);
		Send(cli_server_tid, &update_clock_request, sizeof(update_clock_request), &handshake, sizeof(handshake));
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

	while(1) {
		// user I/O
		char c = Getc(COM2);
		if (c == 'q') {
			Cli_request shutdown_request = get_shutdown_request();
			Send(cli_server_tid, &shutdown_request, sizeof(shutdown_request), &handshake, sizeof(handshake));
		}
		else if (c == '\r') {
			// user hits ENTER
			// parse command
			Command cmd;
			parse_result = command_parse(&command_buffer, &train, &cmd);
			if (parse_result != -1) {
				Cli_request train_cmd_request = get_train_command_request(cmd);
				dump(SUBMISSION, "%s", "io entered train cmd, send cmd");
				Send(cli_server_tid, &train_cmd_request, sizeof(train_cmd_request), &handshake, sizeof(handshake));
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
