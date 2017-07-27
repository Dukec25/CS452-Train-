#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>
#include <calculation.h>

void trains_init(Train_server *train_server){
    int i = 0;
    for( ; i < MAX_NUM_TRAINS; i++ ) {
        train_server->trains[i].id = 0;
        train_server->trains[i].speed = 0;
		train_server->trains[i].last_speed = 0;
        train_server->trains[i].last_stop = -1;
		train_server->trains[i].predict_stop = -1;
		train_server->trains[i].predict_time_lo = -1;
		train_server->trains[i].predict_time_hi = -1;
        train_server->trains[i].last_sensor_triggered_time = 0;
		train_server->trains[i].velocity_state = IDLE;
		train_server->trains[i].current_speed_num_query = 0;
        br_lifo_init(&train_server->trains[i].br_lifo_struct);
    }
}

void br_lifo_init(Br_lifo *br_lifo_struct){
    br_lifo_struct->br_lifo_top = -1;
}

void train_server_init(Train_server *train_server)
{
	train_server->is_shutdown = 0;

	train_server->cmd_fifo_head = 0;
	train_server->cmd_fifo_tail = 0;

	train_server->cli_req_fifo_head = 0;
	train_server->cli_req_fifo_tail = 0;

    train_server->track_req_fifo_head = 0;
	train_server->track_req_fifo_tail = 0;
    
    train_server->cli_map.test = 5;

    train_server->cli_courier_on_wait = 0;
    train_server->track_courier_on_wait = 0;

    train_server->go_cmd_state = -1;
    train_server->train_idx = 0;

	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server->switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
		train_server->switches_status[sw-1] = -1;
	}

	walk_table_initialization(&train_server->walk_table);
    trains_init(train_server);
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

    int track_server_tid = INVALID_TID;
	while(!(track_server_tid > 0 && track_server_tid < MAX_NUM_TASKS)) {
		track_server_tid = WhoIs("TRACK_SERVER");
	}
	/*irq_debug(SUBMISSION, "cli_server %d", cli_server_tid);*/

	Handshake handshake = HANDSHAKE_AKG;
	vint train_server_address = (vint) &train_server;
	/*irq_debug(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 */
    Send(cli_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));
    Send(track_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
    /*irq_debug(SUBMISSION, "sensor_reader_tid %d", sensor_reader_tid);*/
	Send(sensor_reader_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));
  
    int delay_task_tid = Create(PRIOR_MEDIUM, delay_task);
    Send(delay_task_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	//int project_tid = Create(PRIOR_MEDIUM, project);
    //Send(project_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

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
        else if (ts_request.type == TS_TRAIN_TO_TRACK_REQ){
            // courier send request to track 
            Track_request track_req;
            if (train_server.track_req_fifo_head == train_server.track_req_fifo_tail) {
                train_server.track_courier_on_wait = requester_tid; 
            }
            else {
                pop_track_req_fifo(&train_server, &track_req);
                //irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
                Reply(requester_tid, &track_req, sizeof(track_req));
            }
        }
        else if (ts_request.type == TS_TRACK_SERVER) {
            // result from track server
            int i = 0;
            Train *train;
            for( ; i < MAX_NUM_TRAINS; i++){
                if(ts_request.track_result.train_id == train_server.trains[i].id){
                    train = &(train_server.trains[i]);
                    break;
                }
            }
            int no_more_command = track_cmd_handle(&train_server, &ts_request, train, delay_task_tid);
            if(no_more_command == -1){
                //TODO no_more_command, ask track_server_for_more
            }
			/*Reply(requester_tid, &handshake, sizeof(handshake));*/
            /*irq_printf(COM1, "%c%c", GO_CMD_FINAL_SPEED+16, train->id); // speed up the current train */
        }
		else if (ts_request.type == TS_WANT_CLI_REQ) {
			// from cli_request_courier
			Cli_request cli_req;
			if (train_server.cli_req_fifo_head == train_server.cli_req_fifo_tail) {
				cli_req.type = CLI_NULL;
                train_server.cli_courier_on_wait = requester_tid;
			}
			else {
				pop_cli_req_fifo(&train_server, &cli_req);
				//irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
                Reply(requester_tid, &cli_req, sizeof(cli_req));
			}
		}
        else if (ts_request.type == TS_DELAY_TIME_UP){
            Train *train;
            int i = 0;
            for( ; i < MAX_NUM_TRAINS; i++){
                if(ts_request.delay_result.train_id == train_server.trains[i].id){
                    train = &(train_server.trains[i]);
                    break;
                }
            }
            Command tr_cmd = get_tr_stop_command(train->id);
            push_cmd_fifo(&train_server, tr_cmd);
			if (train->velocity_state == DEACCELERATE) {
            	train->deaccel_stop = -1; // TODO, case slow walk and park
			}
            int no_more_command = track_cmd_handle(&train_server, &ts_request, train, delay_task_tid);
            if(no_more_command == -1){
                //TODO no_more_command, ask track_server_for_more
            }
			Reply(requester_tid, &handshake, sizeof(handshake));
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

			train_server.train_idx = (cmd.arg0 == 58 ? 0 : 1);

			train_server.trains[train_server.train_idx].id = cmd.arg0;
			if (train_server.trains[train_server.train_idx].speed != cmd.arg1) {
				train_server.trains[train_server.train_idx].last_speed = train_server.trains[train_server.train_idx].speed;
				train_server.trains[train_server.train_idx].current_speed_num_query = 0;
			}
			train_server.trains[train_server.train_idx].speed = cmd.arg1;

            if(cmd.arg0 == 58){
                velocity58_initialization(&train_server.trains[train_server.train_idx].velocity_model);
            } else if(cmd.arg0 == 71){
                velocity71_initialization(&train_server.trains[train_server.train_idx].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 58 and %d", 71);
            }

			cli_update_request = get_update_train_request(&(train_server.trains[train_server.train_idx]));
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
		case STOP:
			command_handle(&cmd);
			break;

		case SENSOR:
			sensor_handle(&train_server, delay_task_tid);
			break;

		case DC:
			break;

		case GO:
			irq_debug(SUBMISSION, "%s", "go command");
            go_handle(&train_server, cmd);
            break;

		default:
			break;
		}

        if (train_server.cli_courier_on_wait && 
                train_server.cli_req_fifo_head != train_server.cli_req_fifo_tail)
        {
            Cli_request cli_req;
            pop_cli_req_fifo(&train_server, &cli_req);
            int courier_id =  train_server.cli_courier_on_wait;
            Reply(courier_id, &cli_req, sizeof(cli_req));
            train_server.cli_courier_on_wait = 0;
        }

        if (train_server.track_courier_on_wait && 
                train_server.track_req_fifo_head != train_server.track_req_fifo_tail)
        {
            Track_request track_req;
            pop_track_req_fifo(&train_server, &track_req);
            int courier_id =  train_server.track_courier_on_wait;
            Reply(courier_id, &track_req, sizeof(track_req));
            train_server.track_courier_on_wait = 0;
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

void sensor_handle(Train_server *train_server, int delay_task_tid)
{
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
	/*irq_debug(SUBMISSION, "num_sensor_query = %d", train_server->num_sensor_query);*/

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
            int current_sensor_triggered_time = Time();

            // code for initializing two trains 
            if( train_server->go_cmd_state == 0 ) {
                Train *train = &(train_server->trains[0]);
                train->last_stop = current_stop;
                train->last_sensor_triggered_time = current_sensor_triggered_time;

                irq_debug(SUBMISSION, "stop the train %d", train->id);
                irq_printf(COM1, "%c%c", MIN_SPEED+16, train->id); // stop the train 

                continue;
            } else if (train_server->go_cmd_state == 1) {
                Train *train = &(train_server->trains[1]);
                train->last_stop = current_stop;
                train->last_sensor_triggered_time = current_sensor_triggered_time;
                train_server->go_cmd_state = 2;

                Track_request track_req_first_train; 
                track_req_first_train.type = TRAIN_WANT_GUIDANCE; 
                track_req_first_train.train = &(train_server->trains[0]);
                push_track_req_fifo(train_server, track_req_first_train);

                Track_request track_req_second_train; 
                track_req_second_train.type = TRAIN_WANT_GUIDANCE; 
                track_req_second_train.train = train;
                push_track_req_fifo(train_server, track_req_second_train);
                continue;
            }

            // sensor attribution, detect which train hits the sensor
			int attributed = 0;
            Train *train;
            int i = 0;
            for( ; i < MAX_NUM_TRAINS; i++) {
                if (current_stop == train_server->trains[i].predict_stop) {
					if ((train_server->trains[i].predict_time_lo == -1) && (train_server->trains[i].predict_time_hi == -1)) {
						// time unpredictable, use sensor prediction only
						train = &(train_server->trains[i]);
						attributed = 1;
                    	break;
					}
					else if ((train_server->trains[i].predict_time_lo <= current_sensor_triggered_time) &&
							 (train_server->trains[i].predict_time_hi >= current_sensor_triggered_time)) {
						// time predictable, use sensor as well as time bound
						train = &(train_server->trains[i]);
						attributed = 1;
                    	break;
					}
                }
            }

			int last_stop = train->last_stop;
            int last_sensor_triggered_time = train->last_sensor_triggered_time;
			// update the current number of sensor queries on the current train
			train->current_speed_num_query += 1;

			if (attributed == 0) {
				// try to recover sensor attribution failure
				int recoverable = 0;
				int sensor_error = 0;
				int switch_error = 0;
	            for( ; i < MAX_NUM_TRAINS; i++) {
					int predict_two_stop = predict_next(train_server->track, train_server->trains[i].predict_stop, train_server);
					if (predict_two_stop == current_stop) {
						recoverable = 1;
						sensor_error = 1;
						break;
					}
					if (train_server->track[train_server->trains[i].last_stop].edge[DIR_AHEAD].dest->type == NODE_BRANCH) {
						track_node *prev_sw = train_server->track[train_server->trains[i].last_stop].edge[DIR_AHEAD].dest;
						int sw_id = prev_sw->num;
						if (train_server->switches_defects[sw_id] != -1) {
							recoverable = 1;
							switch_error = sw_id;
							break;
						}
						int sw_state = train_server->switches_status[sw_id];
						if ((sw_state == STRAIGHT) && (prev_sw->edge[DIR_CURVED].dest->num == current_stop)) {
							train_server->switches_status[sw_id] = CURVE;
							recoverable = 1;
							switch_error = sw_id;
							train_server->switches_defects[sw_id] = CURVE;
							break;
						}
						else if ((sw_state == CURVE) && (prev_sw->edge[DIR_STRAIGHT].dest->num == current_stop)) {
							train_server->switches_status[sw_id] = STRAIGHT;
							recoverable = 1;
							switch_error = sw_id;
							train_server->switches_defects[sw_id] = STRAIGHT;
							break;
						}
					}
                }
				if (recoverable == 0) {
					irq_printf(COM2, "!!! SENSOR %c%d FAILURE\r\n", sensor.group, sensor.id);
					Cli_request update_sensor_request = get_update_sensor_request(sensor, 0, -1, 0, 0, 0, 0);
					push_cli_req_fifo(train_server, update_sensor_request);
					continue;
				}
				else if (sensor_error) {
					irq_printf(COM2, "!!! SENSOR %c%d ERROR, recoverable\r\n", sensor.group, sensor.id);
				}
				else if (switch_error) {
					irq_printf(COM2, "!!! SWICTH ERROR at %d, recoverable\r\n", switch_error);
				}
            }

			// update the velocity_state based on current_speed_num_query and previous velocity_state
			if (train->velocity_state == IDLE) {
				// IDLE->ACCELERATE
				if ((train->last_speed < train->speed) && (current_stop != last_stop)) {
					train->velocity_state = ACCELERATE;
				}
				// else IDLE->IDLE
			}
			else if (train->velocity_state == ACCELERATE) {
				// ACCELERATE->CONSTANT
				if ((current_stop != last_stop) && (train->speed != 0) && (train->current_speed_num_query >= 4)) {
					train->velocity_state = CONSTANT;
				}
				// else ACCELERATE->ACCELERATE
			}
			else if (train->velocity_state == CONSTANT) {
				// CONSTANT->ACCELERATE
				if (train->last_speed > train->speed) {
					train->velocity_state = ACCELERATE;
				}
				// CONSTANT->DEACCELERATE
				else if (train->last_speed < train->speed) {
					train->velocity_state = DEACCELERATE;
				}
				// else CONSTANT->CONSTANT
			}
			else if (train->velocity_state == DEACCELERATE) {
				// DEACCELERATE->IDLE 
				if ((train->speed == 0) && (current_stop == last_stop) && (train->current_speed_num_query >= 4)) {
					train->velocity_state = IDLE;
				}
			}
			else if (train->velocity_state == SLOW_WALK) {
				// SLOW_WALK->IDLE
				if ((train->speed == 0) && (current_stop == last_stop) && (train->current_speed_num_query >= 4)) {
					train->velocity_state = IDLE;
				}
			} 

            if ((current_stop == last_stop)){
				continue;
			}

			// calculate distance, next stop, time, and new_velocity
			int distance = cal_distance(train_server->track, last_stop, current_stop);

			// update last_stop and last_sensor_triggered_time 
			train->last_stop = current_stop;
            train->last_sensor_triggered_time = current_sensor_triggered_time;

			// update velocity
            int time = current_sensor_triggered_time - last_sensor_triggered_time;
            double real_velocity = (double) distance / (double) time;
			double last_expected_velocity = train->velocity_model.velocity[train->speed];
            velocity_update(train->speed, real_velocity, &train->velocity_model);
			double expected_velocity = train->velocity_model.velocity[train->speed];

			// update prediction
			int next_stop = predict_next(train_server->track, current_stop, train_server);
            train->predict_stop = next_stop;
			if (train->velocity_state == CONSTANT) {
				int predict_distance = cal_distance(train_server->track, current_stop, next_stop);
				train->predict_time_lo = current_sensor_triggered_time + (int) ((predict_distance - PREDICTON_ERROR) / expected_velocity);
				train->predict_time_hi = current_sensor_triggered_time + (int) ((predict_distance + PREDICTON_ERROR) / expected_velocity);
			}
			else if ((train->velocity_state == DEACCELERATE) && (train->speed == 0) && (train->deaccel_stop != -1)) {
				// park cmd prediction
				int predict_distance = cal_distance(train_server->track, current_stop, next_stop);
				// d = (v + v0) / 2 + t
				double expected_velocity_median = (expected_velocity + last_expected_velocity) / 2;
				train->predict_time_lo = current_sensor_triggered_time + predict_distance - PREDICTON_ERROR - expected_velocity_median;
				train->predict_time_hi = current_sensor_triggered_time + predict_distance + PREDICTON_ERROR - expected_velocity_median;
			}
			else {
				// unpredictable
				train->predict_time_lo = -1;
				train->predict_time_hi = -1;
			}
			
			Cli_request update_sensor_request = get_update_sensor_request(sensor, current_sensor_triggered_time,
												last_stop, attributed, train, real_velocity, expected_velocity);
			push_cli_req_fifo(train_server, update_sensor_request);

            //Cli_request update_calibration_request = get_update_calibration_request(last_stop, current_stop, distance,
            //    (int) real_velocity, (int) expected_velocity); 
            //push_cli_req_fifo(train_server, update_calibration_request);

            if (current_stop == train->deaccel_stop){
                irq_debug(SUBMISSION, "%s", "about to delay");
				// update the velocity_state to DEACCELERATE
				train->velocity_state = DEACCELERATE;

				// delay
                Delay_request delay_req;
                Handshake handshake = HANDSHAKE_AKG;
                int velocity = train->velocity_model.velocity[train->speed];
                int park_delay_time = train->park_delay_distance/velocity;
                delay_req.delay_time = park_delay_time; 
                delay_req.train_id   = train->id;
                Send(delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));
            }

            // some condition not takes into account here, like lifo
            // contains the same sensor multiple times 
            Train_br_switch br_switch;
            int temp = peek_br_lifo(&train->br_lifo_struct, &br_switch);
            while( temp == 0 && current_stop == br_switch.sensor_stop ){
                pop_br_lifo(&train->br_lifo_struct);
                /*bwprintf(COM2, "about to sensor %d, switch %d, status %d", br_switch.sensor_stop, br_switch.id, br_switch.state);*/
                Command sw_cmd = get_sw_command(br_switch.id, br_switch.state);
                push_cmd_fifo(train_server, sw_cmd);
                temp = peek_br_lifo(&train->br_lifo_struct, &br_switch);
            }
		}
	}
}

void go_handle(Train_server *train_server, Command go_cmd)
{
    // get invoked 
    int train_id = go_cmd.arg0;

    // can probably change into if in the future 
    switch(train_server->go_cmd_state){
        case -1:
            train_server->go_cmd_state = 0; 
            train_server->trains[0].id = go_cmd.arg0;
            
            // not setting the speed yet, but sensor_handle will 
            // be responsible to set speed of 2 trains to these when get to
            // state 2 
            train_server->trains[0].speed = GO_CMD_START_SPEED;

            if(go_cmd.arg0 == 58){
                velocity58_initialization(&train_server->trains[0].velocity_model);
            } else if(go_cmd.arg0 == 71) {
                velocity71_initialization(&train_server->trains[0].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 58 and %d", 71);
            }
            break;
        case 0:
            train_server->trains[1].id = go_cmd.arg0;
            train_server->trains[1].speed = GO_CMD_START_SPEED;

            if(go_cmd.arg0 == 58){
                velocity58_initialization(&train_server->trains[1].velocity_model);
            } else if(go_cmd.arg0 == 71) {
                velocity71_initialization(&train_server->trains[1].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 58 and %d", 71);
            }
            train_server->go_cmd_state = 1;
            break;
        default:
            break;
    }

    irq_debug(SUBMISSION, "go car %d", go_cmd.arg0);
    // set the train speed 
    irq_printf(COM1, "%c%c", GO_CMD_START_SPEED+16, go_cmd.arg0);
}

void kc_handle(int train_id, Command kc_cmd, int delay_task_tid)
{
    int speed = kc_cmd.arg0;
    int delay_time = kc_cmd.arg1;
    /*irq_debug(SUBMISSION, "kc: speed = %d, delay_time = %d 100ms", speed, delay_time);*/

    Command tr_cmd = get_tr_command(train_id, speed);
    command_handle(&tr_cmd);

	if (delay_time <= 25) {
    	Delay(delay_time * 10);
	}
	else {
        Delay_request delay_req;
        Handshake handshake = HANDSHAKE_AKG;
        delay_req.delay_time = delay_time; 
        delay_req.train_id   = train_id;
        Send(delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));
	} 
    Command tr_stop_cmd = get_tr_stop_command(train_id);
    command_handle(&tr_stop_cmd);
}

void static_reverse(int train_id){
    irq_debug(DEBUG_K4, "%s", "begin reverse");
    irq_printf(COM1, "%c%c", REVERSE, train_id);
}                                     

void slow_walk(Walk_table *walk_table, int train_id, int speed, int distance, int delay_task_tid)
{
	int delay_time = walk_table_lookup(walk_table, train_id, speed, distance);
    irq_debug(SUBMISSION, "wal: speed = %d, delay_time = %d 100ms", speed, delay_time);
	Command kc_cmd = get_kc_command(speed, delay_time);
	kc_handle(train_id, kc_cmd, delay_task_tid);
}

int track_cmd_handle(Train_server *train_server, TS_request *ts_request, Train *train, int delay_task_tid){
    Track_cmd track_cmd;
    if(is_track_cmd_fifo_empty(&ts_request->track_result.cmd_fifo_struct)){
        return -1;
    } else{
        pop_track_cmd_fifo(&ts_request->track_result.cmd_fifo_struct, &track_cmd);
        if(track_cmd.type == TRACK_REVERSE){
            static_reverse(train->id);
            irq_debug(SUBMISSION, "%s", "reverse operation finished");

            pop_track_cmd_fifo(&ts_request->track_result.cmd_fifo_struct, &track_cmd);
            if(track_cmd.type == TRACK_SLOW_WALK){
				// update the velocity_state to SLOW_WALK
				train->velocity_state = SLOW_WALK;

	            slow_walk(&train_server->walk_table, train->id, SLOW_WALK_SPEED, track_cmd.distance, delay_task_tid);
                irq_debug(SUBMISSION, "slow walk finished, train_id %d, speed %d, distance %d", 
                        train->id, SLOW_WALK_SPEED, track_cmd.distance);
            } else if(track_cmd.type == TRACK_PARK){
                train->park_delay_distance = track_cmd.park_info.delay_distance;
                train->deaccel_stop = track_cmd.park_info.deaccel_stop; 
                irq_debug(SUBMISSION, "park operation, delay distance %d, deaccel_stop %d", 
                        track_cmd.park_info.delay_distance, track_cmd.park_info.deaccel_stop);
                irq_printf(COM1, "%c%c", GO_CMD_FINAL_SPEED+16, train->id); // speed up the current train 
            } else{
                irq_debug(SUBMISSION, "sth is wrong, enter unknown state %d", track_cmd.type);
            }
        } else if(track_cmd.type == TRACK_PARK){
            train->park_delay_distance = track_cmd.park_info.delay_distance;
            train->deaccel_stop = track_cmd.park_info.deaccel_stop; 
            irq_printf(COM1, "%c%c", GO_CMD_FINAL_SPEED+16, train->id); // speed up the current train 
        } else if(track_cmd.type == TRACK_SLOW_WALK){
			// update the velocity_state to SLOW_WALK
			train->velocity_state = SLOW_WALK;
	
            slow_walk(&train_server->walk_table, train->id, SLOW_WALK_SPEED, track_cmd.distance, delay_task_tid);
            irq_debug(SUBMISSION, "slow walk finished, train_id %d, speed %d, distance %d", 
                    train->id, SLOW_WALK_SPEED, track_cmd.distance);
        } else{
            irq_debug(SUBMISSION, "sth is wrong, enter unknown state %d", track_cmd.type);
        }
    }
    return 0;
}

