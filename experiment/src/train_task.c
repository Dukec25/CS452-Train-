#include <train_task.h>

void train_task_startup()
{
	irq_io_tasks_cluster();

	cli_startup();
    bwputc(COM1, START); // switches won't work without start command

	initialize_switch();
	sensor_initialization();

	int tid;

	tid = Create(PRIOR_MEDIUM, cli_server);
	debug(DEBUG_K4, "created sensor_task taskId = %d", tid);

	tid = Create(PRIOR_MEDIUM, train_server);
	debug(DEBUG_K4, "created train_task taskId = %d", tid);
	
	Exit();
}

int sensor_to_num(int group, int num){
    return group*16 + num - 1;
}

void train_server()
{
	Train_server train_server;

    // initialization 
	fifo_init(&train_server.cmd_fifo);
	train_server.sensor_lifo_top = -1;
	train_server.is_shutdown = 0;
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
        // be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server.switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}

    train_server.switches_status;

    int last_stop = -1;
	char sensor_group = 0;

    track_node track[TRACK_MAX];
    init_tracka(track);

	int result = RegisterAs("TRAIN_SERVER");
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
//	irq_printf(COM2, "cli_server %d\r\n", cli_server_tid);

	train_server.sensor_reader_tid = Create(PRIOR_MEDIUM, sensor_reader_task);
//	irq_printf(COM2, "sensor_reader_tid %d\r\n", train_server.sensor_reader_tid);

	Handshake cli_server_handshake = HANDSHAKE_AKG;
	Handshake requester_handshake = HANDSHAKE_AKG;

	while(!train_server.is_shutdown) {
		int requester_tid = INVALID_TID;
		Command request;
		Receive(&requester_tid, &request, sizeof(request));
		requester_handshake = train_server.is_shutdown ? HANDSHAKE_SHUTDOWN : HANDSHAKE_AKG;
		Reply(requester_tid, &requester_handshake, sizeof(requester_handshake));
		fifo_put(&train_server.cmd_fifo, &request);
		//irq_printf(COM2, "ts put cmd\r\n");

		if (is_fifo_empty(&train_server.cmd_fifo)) {
			continue;
		}
		
		Command *cmd;
		fifo_get(&train_server.cmd_fifo, &cmd);
		//irq_printf(COM2, "ts get cmd type %d\r\n", cmd->type);
		Cli_request cli_update_request;
		switch (cmd->type) {
		case TR:
			//irq_printf(COM2, "handle tr cmd\r\n");
			command_handle(cmd, &train_server);
			cli_update_request.type = CLI_UPDATE_TRAIN;
			cli_update_request.train_update.id = cmd->arg0; 
			cli_update_request.train_update.speed = cmd->arg1;
			//irq_printf(COM2, "send tr update\r\n");
			Send(cli_server_tid, &cli_update_request, sizeof(cli_update_request), &cli_server_handshake, sizeof(cli_server_handshake));
			break;
		case RV:
			//irq_printf(COM2, "handle rv cmd\r\n");
			command_handle(cmd, &train_server);
			//irq_printf(COM2, "sent rv cmd\r\n");
			break;
		case SW:
			//irq_printf(COM2, "handle sw cmd\r\n");
			command_handle(cmd, &train_server);
			cli_update_request.type = CLI_UPDATE_SWITCH;
			cli_update_request.switch_update.id = cmd->arg0; 
			cli_update_request.switch_update.state = cmd->arg1;
			//irq_printf(COM2, "send sw update\r\n");
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

		if (cmd->type == SENSOR) {
			uint16 sensor_data[SENSOR_GROUPS];
			//irq_printf(COM2, "sensor cmd\r\n");
			Putc(COM1, SENSOR_QUERY);
			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				char lower = Getc(COM1);
				char upper = Getc(COM1);
				sensor_data[sensor_group] = upper << 8 | lower;
			}
			//irq_printf(COM2, "sensor queried\r\n");

			for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
				if (sensor_data[sensor_group] == 0) {
					continue;
				}
				//irq_printf(COM2, "sensor_data[%d] = %d\r\n", sensor_group, sensor_data[sensor_group]);
				char bit = 0;
				for (bit = 0; bit < SENSORS_PER_GROUP; bit++) {
					//sensor_data actually looks like 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
					if (!(sensor_data[sensor_group] & (0x1 << bit))) {
						continue;
					}
					Sensor sensor;
					sensor.group = sensor_group;
					sensor.triggered_time = Time();
					if (bit + 1 <= 8) {
						sensor.id = 8 - bit;
					}
					else {
						sensor.id = 8 + 16 - bit;
					}

                    int current_location = sensor_to_num(sensor.group, sensor.id);
                    /*irq_printf(COM2, "current_location = %d\r\n", current_location);*/
                    /*int distance = cal_distance(track, last_stop, current_location);*/
                    /*irq_printf(COM2, "distance = %d\r\n", distance);*/
                    last_stop = current_location;
	
					Cli_request update_request;
					update_request.type = CLI_UPDATE_SENSOR;
					update_request.sensor_update = sensor;
					Send(cli_server_tid, &update_request, sizeof(update_request), &cli_server_handshake, sizeof(cli_server_handshake));

					if (train_server.sensor_lifo_top != SENSOR_LIFO_SIZE - 1) {
						train_server.sensor_lifo_top += 1;
						train_server.sensor_lifo[train_server.sensor_lifo_top] = sensor;
						//irq_printf(COM2, "insert sensor group = %d, id = %d, time = %d\r\n",
						//sensor.group, sensor.id, sensor.triggered_time);
					}
				}
			}
			
			if (train_server.sensor_lifo_top != -1) {
				Sensor sensor;
				sensor = train_server.sensor_lifo[train_server.sensor_lifo_top];
				train_server.sensor_lifo_top -= 1;
				//irq_printf(COM2, "pop sensor group = %d, id = %d, time = %d\r\n", sensor.group, sensor.id, sensor.triggered_time);
			}
		
		}
	}
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

	cli_server.cli_clock_tid = Create(PRIOR_LOW, cli_clock_task); 
	cli_server.cli_io_tid = Create(PRIOR_MEDIUM, cli_io_task);

	Handshake train_server_handshake = HANDSHAKE_AKG;
	Handshake requester_handshake = HANDSHAKE_AKG;

	int num_sensor_updates = 0;
	do {
		int requester_tid = INVALID_TID;
		Cli_request request;
		Receive(&requester_tid, &request, sizeof(request));
		requester_handshake = cli_server.is_shutdown ? HANDSHAKE_SHUTDOWN : HANDSHAKE_AKG;
		Reply(requester_tid, &requester_handshake, sizeof(requester_handshake));

		switch (request.type) {
		case CLI_TRAIN_COMMAND:
			fifo_put(&cli_server.cmd_fifo, &request);
			// irq_printf(COM2, "cli_server train cmd cli req\r\n");
			break;

		case CLI_UPDATE_TRAIN:
		case CLI_UPDATE_SENSOR:
		case CLI_UPDATE_SWITCH:
		case CLI_UPDATE_CLOCK:
			fifo_put(&cli_server.status_update_fifo, &request);
			// if (request.type != CLI_UPDATE_CLOCK) irq_printf(COM2, "update cli req %d\r\n", request.type);
			break;

		case CLI_SHUTDOWN:
			cli_server.is_shutdown = 1;
			irq_printf(COM2, "shutdown cli req\r\n");
			break;
		}
		
		if (!is_fifo_empty(&cli_server.cmd_fifo)) {
			//irq_printf(COM2, "cli send train cmd\r\n");
			Cli_request *cli_cmd_request;
			fifo_get(&cli_server.cmd_fifo, &cli_cmd_request);
			Command train_cmd = cli_cmd_request->cmd;
			//irq_printf(COM2, "cli get train cmd %d\r\n", train_cmd.type);
			Send(train_server_tid, &train_cmd, sizeof(train_cmd), &train_server_handshake, sizeof(train_server_handshake));
		}

		if (!is_fifo_empty(&cli_server.status_update_fifo)) {
			Cli_request *update_request;
			fifo_get(&cli_server.status_update_fifo, &update_request);
			switch (update_request->type) {
			case CLI_UPDATE_CLOCK:
				// irq_printf(COM2, "cli pop clock update req\r\n");
				cli_update_clock(update_request->clock_update);
				break;
			case CLI_UPDATE_TRAIN:
				//irq_printf(COM2, "cli pop train update req\r\n");
				cli_update_train(update_request->train_update);
				break;
			case CLI_UPDATE_SWITCH:
				//irq_printf(COM2, "cli pop switch update req\r\n");
				cli_update_switch(update_request->switch_update);
				break;
			case CLI_UPDATE_SENSOR:
				//irq_printf(COM2, "cli pop sensor group = %d, id = %d, time = %d\r\n",
				//			update_request->sensor_update.group, update_request->sensor_update.id,
				//			update_request->sensor_update.triggered_time);		
				cli_update_sensor(update_request->sensor_update, num_sensor_updates++);
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
		// irq_printf(COM2, "send clock\r\n");
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
			//irq_printf(COM2, "io entered q, send shutdown\r\n");
			Send(cli_server_tid, &cli_request, sizeof(cli_request), &handshake, sizeof(handshake));
		}
		else if (c == '\r') {
			// user hits ENTER
			// parse command
			Command cmd;
			parse_result = command_parse(&command_buffer, &train, &cmd);
			//irq_printf(COM2, "io entered ENTER, parse_result = %d\r\n", parse_result);
			if (parse_result != -1) {
				Cli_request cli_cmd_request;
				cli_cmd_request.type = CLI_TRAIN_COMMAND;
				cli_cmd_request.cmd = cmd;
				//irq_printf(COM2, "io entered train cmd, send cmd\r\n");
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

/*
void train_task_startup()
{
//	cli_startup();
	irq_io_tasks_cluster();

//	Putc(COM1, START); // switches won't work without start command
//	test_initialize_switch();
//	initialize_switch();
//	sensor_initialization();
//	debug(DEBUG_K4, "%s", "initialized switches");

	int tid;
//	tid  = Create(PRIOR_LOW, clock_task);
//	debug(DEBUG_K4, "created clock_task taskId = %d", tid);

	tid = Create(PRIOR_LOW, sensor_task);
	debug(DEBUG_K4, "created sensor_task taskId = %d", tid);

//   tid = Create(PRIOR_LOW, test_task);
//	debug(DEBUG_K4, "created train_task taskId = %d", tid);
	
	tid = Create(PRIOR_LOW, train_task);
	debug(DEBUG_K4, "created train_task taskId = %d", tid);
	
	Exit();
}
*/

/*
void sensor_task() {

	int updates = 0;

	vint register_result = RegisterAs("SENSOR_TASK");
	vint requester;
	Calibration_package cali_pkg;
	Delivery reply_msg;
	Receive(&requester, &cali_pkg, sizeof(cali_pkg));
	Reply(requester, &reply_msg, sizeof(reply_msg));

	track_node tracka[TRACK_MAX];
	init_tracka(tracka);

	while (1) {
		irq_printf(COM2, "%s", "sensor");
		//debug(SUBMISSION, "%s", "sensor is printing");
		//irq_printf(COM2, "%s", "sensor is printing");
		Delay(20);	// delay 0.2 second
		Putc(COM1, SENSOR_QUERY);
		//irq_printf(COM2, "%s", "after SENSOR_QUERY\r\n");
		int sensor_data[SENSOR_GROUPS];
		int group = 0;
		for (group = 0; group < SENSOR_GROUPS; group++) {
			char lower = Getc(COM1);
		   // Putc(COM1, 0); // speed
			char upper = Getc(COM1);
			sensor_data[group] = upper << 8 | lower;
		}

		for (group = 0; group < SENSOR_GROUPS; group++) {
			int id = 0;
			for (id = 0; id < SENSORS_PER_GROUP; id++) {
				// irq_printf(COM2, "%s", "Sensor group  "); 
				if (sensor_data[group] & (0x1 << id)) {
					int actual_id; 
					if( id + 1 <= 8){
						actual_id = 8 - id;
					} else{
						actual_id = 8 + 16 - id;
					}
					//sensor_data actually looks like 
					// 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
					cli_update_sensor(group, actual_id, updates++);
					//irq_printf(COM2, "%s", "sensor is printing");
					
					if(group*16-1 + actual_id  == *(cali_pkg.stop_sensor)){
						Putc(COM1, 0); 	 	// stop	
						Putc(COM1, 69); 	// train
					}
				}
			}
		}
	}
	Exit();
}
*/

/*
void train_task() {
	// command
	Command_buffer command_buffer;
	command_buffer.pos = 0;
	char train_id = 0;
	char train_speed = 0;

	vint stop_sensor = -1;
	vint last_stop = -1;

	Calibration_package calibration_package;
	calibration_package.stop_sensor = &stop_sensor;
	calibration_package.last_stop = &last_stop;

	int sensor_task_tid = WhoIs("SENSOR_TASK");
	Delivery reply_msg;
	Send(sensor_task_tid, &calibration_package, sizeof(Calibration_package), &reply_msg, sizeof(reply_msg));

	while(1) {
		// user I/O and send command to train
		char c = Getc(COM2);
		if (c == 'q') {
			// user hits 'q', exit
			irq_printf(COM2, "user hits q, terminate the program");	
			Terminate();
			break;
		}
		else if (c == '\r') {
			// user hits ENTER
			// parse command
			Command cmd;
			int result = command_parse(&command_buffer, &train_id, &train_speed, &cmd);
			if (result != -1) {
				// user entered a valid command, sends command to train and updates user interface
				// command_handle is within common/src/train.c
			  	command_handle(&cmd, &calibration_package);
			}
			// clears command_buffer
			command_clear(&command_buffer);
		}
		else {
			command_buffer.data[command_buffer.pos++] = c;
//			Putc(COM2, c);
		}
	}
	Exit();
}*/
