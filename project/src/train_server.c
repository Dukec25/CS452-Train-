#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>
#include <calculation.h>

void trains_init(Train_server *train_server){
    int i = 0;
    for( ; i < MAX_NUM_TRAINS; i++ ){
        train_server->trains[i].id = 0;
        train_server->trains[i].speed = 0;
        train_server->trains[i].last_stop = -1;
        train_server->trains[i].last_sensor_triggered_time = 0;
    }
}

void train_server_init(Train_server *train_server)
{
	train_server->is_shutdown = 0;

	train_server->cmd_fifo_head = 0;
	train_server->cmd_fifo_tail = 0;

	train_server->cli_req_fifo_head = 0;
	train_server->cli_req_fifo_tail = 0;

    train_server->park_req_fifo_head = 0;
	train_server->park_req_fifo_tail = 0;
    
    train_server->br_lifo_top = -1;
	train_server->last_sensor_triggered_time = 0;
    train_server->cli_map.test = 5;

    // to be removed 
    train_server->deaccelarate_stop = -1;
    train_server->park_delay_time = 0;

    train_server->cli_courier_on_wait = 0;
    train_server->park_courier_on_wait = 0;

    train_server->go_cmd_state = -1;
    train_server->train_idx = 0;

	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server->switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}

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

	/*irq_debug(SUBMISSION, "cli_server %d", cli_server_tid);*/

	Handshake handshake = HANDSHAKE_AKG;
	vint train_server_address = (vint) &train_server;
	/*irq_debug(SUBMISSION, "train_server train_server_address = 0x%x", train_server_address);	 */
    Send(cli_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

	int sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
    /*irq_debug(SUBMISSION, "sensor_reader_tid %d", sensor_reader_tid);*/
	Send(sensor_reader_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

    /*int park_server_tid = Create(PRIOR_MEDIUM, park_server);*/
    /*Send(park_server_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));*/
  
    int delay_task_tid = Create(PRIOR_MEDIUM, delay_task);
    Send(delay_task_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));

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
        else if (ts_request.type == TS_TRAIN_TO_PARK_REQ){
            // from park_server_courier
            Park_request park_req;
            if (train_server.park_req_fifo_head == train_server.park_req_fifo_tail) {
                // no park command issued yet 
                train_server.park_courier_on_wait = requester_tid; 
            }
            else {
                pop_park_req_fifo(&train_server, &park_req);
                //irq_debug(SUBMISSION, "train_server reply tp %d pop cli_req %d", requester_tid, cli_req.type);
                Reply(requester_tid, &park_req, sizeof(park_req));
            }
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
        else if (ts_request.type == TS_PARK_SERVER) {
            train_server.park_delay_time = ts_request.park_result.park_delay_time; 
            train_server.deaccelarate_stop = ts_request.park_result.deaccelarate_stop;
            irq_debug(SUBMISSION, "train deaccelarate_stop = %d, park_delay_time = %d \r\n", train_server.deaccelarate_stop, train_server.park_delay_time);
			Reply(requester_tid, &handshake, sizeof(handshake));
        }
        else if (ts_request.type == TS_DELAY_TIME_UP){
            /*Command tr_cmd = get_tr_stop_command(train_server.train.id);*/
            /*push_cmd_fifo(&train_server, tr_cmd);*/
            /*train_server.deaccelarate_stop = -1;*/
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

			train_server.trains[train_server.train_idx].id = cmd.arg0;
			train_server.trains[train_server.train_idx].speed = cmd.arg1;

            if(cmd.arg0 == 69){
                velocity69_initialization(&train_server.trains[train_server.train_idx].velocity_model);
            } else if(cmd.arg0 == 71){
                velocity71_initialization(&train_server.trains[train_server.train_idx].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 69 and %d", 71);
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
		case STOP:
			command_handle(&cmd);
			break;

		case SENSOR:
			sensor_handle(&train_server, delay_task_tid);
			break;

		case DC:
			break;

		case GO:
            go_handle(&train_server, cmd);
            break;

		case BR:
			br_handle(&train_server, cmd);
			break;

		case PARK:
			/*irq_debug(SUBMISSION, "handle park cmd %c%d", cmd.arg0, cmd.arg1);*/
			br_handle(&train_server, cmd);

            Park_request park_req;
            park_req.park_cmd = cmd;

            push_park_req_fifo(&train_server, park_req);
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

        if (train_server.park_courier_on_wait && 
                train_server.park_req_fifo_head != train_server.park_req_fifo_tail)
        {
            Park_request park_req;
            pop_park_req_fifo(&train_server, &park_req);
            int courier_id =  train_server.park_courier_on_wait;
            Reply(courier_id, &park_req, sizeof(park_req));
            train_server.park_courier_on_wait = 0;
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
			int next_stop = predict_next(train_server->track, current_stop, train_server);

            if( train_server->go_cmd_state == 0 ){
                Train *train = &(train_server->trains[0]);
                train->last_stop = current_stop;
                train->last_sensor_triggered_time = current_sensor_triggered_time;

                irq_debug(SUBMISSION, "stop the train %d", train->id);
                irq_printf(COM1, "%c%c", MIN_SPEED+16, train->id); // stop the train 
                continue;
            } else if (train_server->go_cmd_state == 1){
                Train *train = &(train_server->trains[1]);
                train->last_stop = current_stop;
                train->last_sensor_triggered_time = current_sensor_triggered_time;
                irq_debug(SUBMISSION, "speed up the train %d", train->id);
                irq_printf(COM1, "%c%c", GO_CMD_FINAL_SPEED+16, train->id); // speed up the current train 
                irq_printf(COM1, "%c%c", GO_CMD_FINAL_SPEED+16, train_server->trains[0].id); // start the other train 
                train_server->go_cmd_state = 2;
                continue;
            }

            // sensor attribution, detect which train hits the sensor
            Train *train;
            int i = 0;
            for( ; i < MAX_NUM_TRAINS; i++){
                if(current_stop == train_server->trains[i].predict_stop){
                    train = &(train_server->trains[i]);
                    break;
                }
            }

			int last_stop = train->last_stop;
            int last_sensor_triggered_time = train->last_sensor_triggered_time;
	
			/*if ((current_stop == last_stop) || (last_stop == -1)) { */
            if ((current_stop == last_stop)){
				return;
			}

			// calculate distance, next stop, time, and new_velocity
			int distance = cal_distance(train_server->track, last_stop, current_stop);

			// update 
			train->last_stop = current_stop;
            train->last_sensor_triggered_time = current_sensor_triggered_time; 
            train->predict_stop = next_stop;

            int time = current_sensor_triggered_time - last_sensor_triggered_time;
            double real_velocity = (double) distance / (double) time;

			// update velocity_data
            velocity_update(train->speed, real_velocity, &train->velocity_model);

			Cli_request update_sensor_request = get_update_sensor_request(sensor, last_stop, next_stop);
			push_cli_req_fifo(train_server, update_sensor_request);

            Cli_request update_calibration_request = get_update_calibration_request(last_stop, current_stop, distance,
                (int) real_velocity, (int) train->velocity_model.velocity[train->speed]); 
            push_cli_req_fifo(train_server, update_calibration_request);

            if (current_stop == train_server->deaccelarate_stop){
                irq_debug(SUBMISSION, "%s", "about to delay");

                Delay_request delay_req;
                Handshake handshake = HANDSHAKE_AKG;
                delay_req.delay_time = train_server->park_delay_time; 
                Send(delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));
            }

            // some condition not takes into account here, like lifo
            // contains the same sensor multiple times 
            Train_br_switch br_switch;
            int temp = peek_br_lifo(train_server, &br_switch);
            if( current_stop == br_switch.sensor_stop){
                train_server->br_lifo_top -= 1; // equivalent with pop 
                irq_debug(SUBMISSION, "about to sensor %d, switch %d, status %d", br_switch.sensor_stop, br_switch.id, br_switch.state);
                Command sw_cmd = get_sw_command(br_switch.id, br_switch.state);
                push_cmd_fifo(train_server, sw_cmd);
            }

		}
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
	/*irq_debug(SUBMISSION, "num_switch = %d", num_switch);*/
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

            if(go_cmd.arg0 == 69){
                velocity69_initialization(&train_server->trains[0].velocity_model);
            } else if(go_cmd.arg0 == 71) {
                velocity71_initialization(&train_server->trains[0].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 69 and %d", 71);
            }
            break;
        case 0:
            train_server->trains[1].id = go_cmd.arg0;
            train_server->trains[1].speed = GO_CMD_START_SPEED;

            if(go_cmd.arg0 == 69){
                velocity69_initialization(&train_server->trains[1].velocity_model);
            } else if(go_cmd.arg0 == 71) {
                velocity71_initialization(&train_server->trains[1].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 69 and %d", 71);
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
