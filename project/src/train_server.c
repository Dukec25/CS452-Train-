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

	int stopping_distance_tid = Create(PRIOR_MEDIUM, stopping_distance_collector_task);
	/*dump(SUBMISSION, "stopping_distance_tid %d", stopping_distance_tid);*/
	Send(stopping_distance_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int br_tid = Create(PRIOR_MEDIUM, br_task);
	/*dump(SUBMISSION, "br_tid %d", br_tid);*/
	Send(br_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int park_tid = Create(PRIOR_MEDIUM, park_task);
	/*dump(SUBMISSION, "park_tid %d", park_tid);*/
	Send(park_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	while (*kill_all != HANDSHAKE_SHUTDOWN) {
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
		/*dump(SUBMISSION, "%s", "ts put cmd");*/

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
		/*dump(SUBMISSION, "ts get cmd type %d", cmd.type);*/

		// handle TR, RV, SW, GO, STOP
		Cli_request cli_update_request;
		switch (cmd.type) {
		case TR:
			/*dump(SUBMISSION, "%s", "handle tr cmd");*/
			command_handle(&cmd);

			train_server.train.id = cmd.arg0;
			train_server.train.speed = cmd.arg1;

			cli_update_request = get_update_train_request(cmd.arg0, cmd.arg1);
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &handshake, sizeof(handshake));
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
			break;
		case SW:
			/*dump(SUBMISSION, "%s", "handle sw cmd");*/
			command_handle(&cmd);

			train_server.switches_status[cmd.arg0 - 1] = switch_state_to_byte(cmd.arg1);

			cli_update_request = get_update_switch_request(cmd.arg0, cmd.arg1);
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &handshake, sizeof(handshake));
			break;
		case RV:
		case GO:
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
			/*dump(SUBMISSION, "%s", "sensor cmd");*/
			Putc(COM1, SENSOR_QUERY);
			uint16 sensor_data[SENSOR_GROUPS];
			int sensor_group = 0;
			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				char lower = Getc(COM1);
				char upper = Getc(COM1);
				sensor_data[(int) sensor_group] = upper << 8 | lower;
			}
			train_server.num_sensor_query++;
			/*dump(SUBMISSION, "num_sensor_query = %d", train_server.num_sensor_query);*/

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
						/*dump(SUBMISSION, "pop sensor group = %d, id = %d, time = %d",*/
										 /*sensor.group, sensor.id, sensor.triggered_time);*/
						if (sensor_to_num(last_sensor) == last_stop) {
							break;
						}
					}

					// push current triggered sensor onto lifo
					if (train_server.sensor_lifo_top != SENSOR_LIFO_SIZE - 1) {
						train_server.sensor_lifo_top += 1;
						train_server.sensor_lifo[train_server.sensor_lifo_top] = sensor;
						/*dump(SUBMISSION, "insert sensor group = %d, id = %d, time = %d",*/
										 /*sensor.group, sensor.id, sensor.triggered_time);*/
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
					velocity_update(last_stop, current_stop, time, train_server.current_velocity_data);

					// Send sensor update
					Cli_request update_sensor_request = get_update_sensor_request(sensor, last_stop, next_stop);
					Send(cli_server_tid, &update_sensor_request, sizeof(update_sensor_request), &handshake, sizeof(handshake));
	
					// Send calibration update
					int velocity = velocity_lookup(last_stop, current_stop, train_server.current_velocity_data); 
					Cli_request update_calibration_request =
						get_update_calibration_request(last_stop, current_stop, distance, time, velocity);
					Send(cli_server_tid, &update_calibration_request, sizeof(update_calibration_request), &handshake, sizeof(handshake));
				}
			}
		}
	}

	train_server.is_shutdown = 1;

	int expected_num_exit = 4;
	int num_exit = 0;
	int exit_list[4];
	exit_list[0] = sensor_reader_tid;
	exit_list[1] = stopping_distance_tid;
	exit_list[2] = br_tid;
	exit_list[3] = park_tid;
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
			else if (exit_tid == exit_list[1]) {
				exit_list[1] = INVALID_TID;
				num_exit++;
			}
			else if (exit_tid == exit_list[2]) {
				exit_list[2] = INVALID_TID;
				num_exit++;
			}
			else if (exit_tid == exit_list[3]) {
				exit_list[3] = INVALID_TID;
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

void stopping_distance_collector_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	debug(SUBMISSION, "stopping_distance train_server_address = 0x%x", train_server_address);	 

	while (train_server->is_shutdown == 0) {
		if (train_server->is_dc == 0) {
			continue;
		}

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

		train_server->is_dc = 0;
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

void br_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	//debug(SUBMISSION, "br_task train_server_address = 0x%x", train_server_address);	 

	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	while (train_server->is_shutdown == 0) {
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
        bwprintf(COM2, "num_switch = %d\r\n", num_switch);
		int i;
		for(i = 0; i < num_switch; i++) {
            bwprintf(COM2, "SW ID = %d, STATE = %d\r\n", train_server->br_update[i].id, train_server->br_update[i].state);
			Command sw_cmd = get_sw_command(train_server->br_update[i].id, train_server->br_update[i].state);
			Send(train_server_tid, &sw_cmd, sizeof(sw_cmd), &handshake, sizeof(handshake));
		}
		//debug(SUBMISSION, "br_task: flip %d br done", num_switch);
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

void park_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
	/*dump(SUBMISSION, "stopping_distance train_server_address = 0x%x", train_server_address);	 */

	// track A initialization
	track_node track[TRACK_MAX];
	init_tracka(track);

	while (train_server->is_shutdown == 0) {
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
		int stopping_distance = train_server->current_velocity_data->stopping_distance; 
        /*debug(SUBMISSION, "park_task: stopping_distance = %d", stopping_distance);*/

		Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];
		int num_park_stops = find_stops_by_distance(track, train_server->last_stop, stop, stopping_distance, park_stops);
        if(num_park_stops == -1){
            continue; // there is error, ignore this iteration
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
		while (last_stop != deaccelarate_stop) {
			last_stop = train_server->last_stop;
			Pass();
		}

		//debug(SUBMISSION, "park_task current stop = %d, start delay", last_stop);
		Delay(park_delay_time);
		//debug(SUBMISSION, "park_task send tr %d", tr_cmd.arg0);
		Command tr_cmd = get_tr_stop_command(train_server->train.id);
		Send(train_server_tid, &tr_cmd, sizeof(tr_cmd), &handshake, sizeof(handshake));
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}
