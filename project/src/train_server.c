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
        train_server->trains[i].deaccel_stop = -1;
        train_server->trains[i].park_delay_distance = -1;
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

    // TODO, branch_delay_task, detail see final_idea_generation
    /*int br_delay_task_tid = Create(PRIOR_MEDIUM, delay_task);*/
    /*Send(br_delay_task_tid, &train_server_address, sizeof(train_server_address), &handshake, sizeof(handshake));*/

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
            debug(SUBMISSION, "%s", "receive from track_server");
            // result from track server
            int i = 0;
            Train *train;
            for(i = 0; i < MAX_NUM_TRAINS; i++){
                if(ts_request.track_result.train_id == train_server.trains[i].id){
                    train = &(train_server.trains[i]);
                    break;
                }
            }
            train->cmd_fifo_struct = ts_request.track_result.cmd_fifo_struct;
            train->br_lifo_struct  = ts_request.track_result.br_lifo_struct;
            int no_more_command = track_cmd_handle(&train_server, train);
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
			Reply(requester_tid, &handshake, sizeof(handshake));
            Train *train;
            int i = 0;
            for( ; i < MAX_NUM_TRAINS; i++){
                if(ts_request.delay_result.train_id == train_server.trains[i].id){
                    train = &(train_server.trains[i]);
                    break;
                }
            }
            debug(SUBMISSION, "%s", "delay is over");
            Command tr_cmd = get_tr_stop_command(train->id);
            push_cmd_fifo(&train_server, tr_cmd);
            /*train->deaccel_stop = -1;*/ // TODO, case slow walk and park
            int no_more_command = track_cmd_handle(&train_server, train);
            if(no_more_command == -1){
                //TODO no_more_command, ask track_server_for_more
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
            debug(SUBMISSION, "%s", "go_handle get triggered");
            go_handle(&train_server, cmd);
            break;

        case WALK:
            irq_debug(SUBMISSION, "%s", "WALK GET TRIGGERED, currently disabled");
            /*walk_handle(&train_server.walk_table, 71, cmd);*/
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
			int next_stop = predict_next(train_server->track, current_stop, train_server);

            // TODO uncomment those
            /*// send sensor info to track for resource management*/
            /*Track_request track_req; */
            /*track_req.type = TRACK_SENSOR_HIT; */
            /*track_req.sensor_num = current_stop;*/
            /*push_track_req_fifo(train_server, track_req);*/

            // code for initializing two trains 
            if( train_server->go_cmd_state == 0 ){
                Train *train = &(train_server->trains[0]);
                train->last_stop = current_stop;
                train->last_sensor_triggered_time = current_sensor_triggered_time;
                //TODO, fix this in the future
                train->delay_task_tid = delay_task_tid;

                debug(SUBMISSION, "stop the train %d", train->id);
                irq_printf(COM1, "%c%c", MIN_SPEED+16, train->id); // stop the train 

                // for testing purpose, one train only
                train_server->go_cmd_state = 1;
                Track_request track_req_first_train; 
                track_req_first_train.type = TRAIN_WANT_GUIDANCE; 
                track_req_first_train.train = &(train_server->trains[0]);
                debug(SUBMISSION, "%s", "sending request to track");
                push_track_req_fifo(train_server, track_req_first_train);

                continue;
            } /*else if (train_server->go_cmd_state == 1){*/
                /*Train *train = &(train_server->trains[1]);*/
                /*train->last_stop = current_stop;*/
                /*train->last_sensor_triggered_time = current_sensor_triggered_time;*/
                /*train_server->go_cmd_state = 2;*/

                /*Track_request track_req_first_train; */
                /*track_req_first_train.type = TRAIN_WANT_GUIDANCE; */
                /*track_req_first_train.train = &(train_server->trains[0]);*/
                /*push_track_req_fifo(train_server, track_req_first_train);*/

                /*Track_request track_req_second_train; */
                /*track_req_second_train.type = TRAIN_WANT_GUIDANCE; */
                /*track_req_second_train.train = train;*/
                /*push_track_req_fifo(train_server, track_req_second_train);*/
                /*continue;*/
            /*}*/

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

            /*if (current_stop == train->deaccel_stop){*/
                /*irq_debug(SUBMISSION, "%s", "about to delay");*/
                /*Delay_request delay_req;*/
                /*Handshake handshake = HANDSHAKE_AKG;*/
                /*int velocity = train->velocity_model.velocity[train->speed];*/
                /*int park_delay_time = train->park_delay_distance/velocity;*/
                /*delay_req.delay_time = park_delay_time; */
                /*delay_req.train_id   = train->id;*/
                /*Send(delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));*/
            /*}*/

            // some condition not takes into account here, like lifo
            // contains the same sensor multiple times 
            /*Train_br_switch br_switch;*/
            /*int temp = peek_br_lifo(&train->br_lifo_struct, &br_switch);*/
            /*while( temp == 0 && current_stop == br_switch.sensor_stop ){*/
                /*pop_br_lifo(&train->br_lifo_struct);*/
                /*[>bwprintf(COM2, "about to sensor %d, switch %d, status %d", br_switch.sensor_stop, br_switch.id, br_switch.state);<]*/
                /*Command sw_cmd = get_sw_command(br_switch.id, br_switch.state);*/
                /*push_cmd_fifo(train_server, sw_cmd);*/
                /*temp = peek_br_lifo(&train->br_lifo_struct, &br_switch);*/
            /*}*/
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

            if(go_cmd.arg0 == 69){
                velocity69_initialization(&train_server->trains[0].velocity_model);
            } else if(go_cmd.arg0 == 71) {
                velocity71_initialization(&train_server->trains[0].velocity_model);
            } else{
                irq_debug(SUBMISSION, "incorrect train id, only 69 and %d", 71);
            }
            break;
        /*case 0:*/
            /*train_server->trains[1].id = go_cmd.arg0;*/
            /*train_server->trains[1].speed = GO_CMD_START_SPEED;*/

            /*if(go_cmd.arg0 == 69){*/
                /*velocity69_initialization(&train_server->trains[1].velocity_model);*/
            /*} else if(go_cmd.arg0 == 71) {*/
                /*velocity71_initialization(&train_server->trains[1].velocity_model);*/
            /*} else{*/
                /*irq_debug(SUBMISSION, "incorrect train id, only 69 and %d", 71);*/
            /*}*/
            /*train_server->go_cmd_state = 1;*/
            /*break;*/
        default:
            break;
    }

    irq_debug(SUBMISSION, "go car %d", go_cmd.arg0);
    // set the train speed 
    irq_printf(COM1, "%c%c", GO_CMD_START_SPEED+16, go_cmd.arg0);
}

void kc_handle(Train *train, Command kc_cmd)
{
    int speed = kc_cmd.arg0;
    int delay_time = kc_cmd.arg1;
    irq_debug(SUBMISSION, "kc: speed = %d, delay_time = %d 100ms, train_id = %d", 
            speed, delay_time, train->id);
    Delay_request delay_req;
    Handshake handshake = HANDSHAKE_AKG;
    delay_req.delay_time = delay_time * 10;//TODO ERROR PRONE
    delay_req.train_id   = train->id;
    Send(train->delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));

    Command tr_cmd = get_tr_command(train->id, speed);
    command_handle(&tr_cmd);
    Send(train->delay_task_tid, &delay_req, sizeof(delay_req), &handshake, sizeof(handshake));

    /*Delay(delay_time * 10);*/
    /*Command tr_stop_cmd = get_tr_stop_command(train_id);*/
    /*command_handle(&tr_stop_cmd);*/
}

void walk_handle(Walk_table *walk_table, int train_id, Command walk_cmd)
{
	int speed = walk_cmd.arg0;
	int distance = walk_cmd.arg1;
	int delay_time = walk_table_lookup(walk_table, train_id, speed, distance);
    irq_debug(SUBMISSION, "wal: speed = %d, delay_time = %d 100ms", speed, delay_time);
	Command kc_cmd = get_kc_command(speed, delay_time);
	kc_handle(train_id, kc_cmd);
}

void static_reverse(int train_id){
    /*Delay(20);*/
    /*irq_debug(SUBMISSION, "%s", "reverse and delay");*/
    /*irq_printf(COM1, "%c%c", REVERSE, train_id);*/
    /*Delay(20);*/
    Command rv_cmd = get_rv_command(train_id);
    command_handle(&rv_cmd);
}                                     

void slow_walk(Walk_table *walk_table, Train *train, int speed, int distance)
{
    // walk_table_loopup used distance is in cm, need to convert 
	int delay_time = walk_table_lookup(walk_table, train->id, speed, distance/10000);
    irq_debug(SUBMISSION, "wal: speed = %d, delay_time = %d 100ms, distance = %d", 
            speed, delay_time, distance);
	Command kc_cmd = get_kc_command(speed, delay_time);
	kc_handle(train, kc_cmd);
}

int track_cmd_handle(Train_server *train_server, Train *train){
    irq_debug(SUBMISSION, "%s", "enter track_cmd_handle_routine");
    Track_cmd track_cmd;
    if(is_track_cmd_fifo_empty(&train->cmd_fifo_struct)){
        irq_debug(SUBMISSION, "%s", "cmd_list is empty");
        return -1;
    } else{
        pop_track_cmd_fifo(&train->cmd_fifo_struct, &track_cmd);
        if(track_cmd.type == TRACK_REVERSE){
            irq_debug(SUBMISSION, "%s", "before enter static_reverse");
            static_reverse(train->id);
            irq_debug(SUBMISSION, "%s", "reverse operation finished");

            pop_track_cmd_fifo(&train->cmd_fifo_struct, &track_cmd);
            if(track_cmd.type == TRACK_SLOW_WALK){
                // need to reduce train length due to train reverse
                int stop_dist = track_cmd.distance - TRAIN_LENGTH;
                slow_walk(&train_server->walk_table, train, SLOW_WALK_SPEED, stop_dist);
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
            irq_debug(SUBMISSION, "park operation, delay distance %d, deaccel_stop %d", 
                    track_cmd.park_info.delay_distance, track_cmd.park_info.deaccel_stop);
        } else if(track_cmd.type == TRACK_SLOW_WALK){
            slow_walk(&train_server->walk_table, train, SLOW_WALK_SPEED, track_cmd.distance);
            irq_debug(SUBMISSION, "slow walk finished, train_id %d, speed %d, distance %d", 
                    train->id, SLOW_WALK_SPEED, track_cmd.distance);
        } else{
            irq_debug(SUBMISSION, "sth is wrong, enter unknown state %d", track_cmd.type);
        }
    }
    return 0;
}

