#include <cli_server.h>
#include <train_server.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <train.h>
#include <cli.h>
#include <calculation.h>

Cli_request get_train_command_request(Command cmd)
{
	Cli_request train_cmd_request;
	train_cmd_request.type = CLI_TRAIN_COMMAND;
	train_cmd_request.cmd = cmd;
	return train_cmd_request;	
}

Cli_request get_update_train_request(char id, char speed)
{
	Cli_request update_train_request;
	update_train_request.type = CLI_UPDATE_TRAIN;
	update_train_request.train_update.id = id; 
	update_train_request.train_update.speed = speed;
	return update_train_request;
}

Cli_request get_update_switch_request(char id, char state)
{
	Cli_request update_switch_request;
	update_switch_request.type = CLI_UPDATE_SWITCH;
	update_switch_request.switch_update.id = id; 
	update_switch_request.switch_update.state = state;	
	return update_switch_request;
}

Cli_request get_update_sensor_request(Sensor sensor, int last_stop, int next_stop)
{
	Cli_request update_sensor_request;
	update_sensor_request.type = CLI_UPDATE_SENSOR;
	update_sensor_request.sensor_update = sensor;
	update_sensor_request.last_sensor_update = last_stop;
	update_sensor_request.next_sensor_update = next_stop;
	return update_sensor_request;	
}

Cli_request get_update_calibration_request(int last_stop, int current_stop, int distance, int time, int velocity)
{
	Cli_request update_calibration_request;
	update_calibration_request.type = CLI_UPDATE_CALIBRATION;
	update_calibration_request.calibration_update.src = last_stop;
	update_calibration_request.calibration_update.dest = current_stop;
	update_calibration_request.calibration_update.distance = distance;
	update_calibration_request.calibration_update.time = time;
	update_calibration_request.calibration_update.velocity = velocity;
	return update_calibration_request;
}

Cli_request get_update_clock_request(Clock clock)
{
	Cli_request update_clock_request;
	update_clock_request.type = CLI_UPDATE_CLOCK;
	update_clock_request.clock_update = clock;
	return update_clock_request;	
}

Cli_request get_shutdown_request()
{
	Cli_request shutdown_request;
	shutdown_request.type = CLI_SHUTDOWN;	
	return shutdown_request;
}

void cli_server()
{
	Handshake kill_all_reply = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
	Handshake *kill_all = kill_all_addr;

	Cli_server cli_server;
	cli_server.is_shutdown = 0;
	fifo_init(&cli_server.cmd_fifo);
	fifo_init(&cli_server.status_update_fifo);

	RegisterAs("CLI_SERVER");
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	Handshake handshake = HANDSHAKE_AKG;
	vint cli_server_address = (vint) &cli_server;
	/*dump(SUBMISSION, "cli_server cli_server_address = 0x%x", cli_server_address);	 */

	int cli_clock_tid = Create(PRIOR_MEDIUM, cli_clock_task); 
	Send(cli_clock_tid, &cli_server_address, sizeof(cli_server_address), &handshake, sizeof(handshake));
	/*dump(SUBMISSION, "cli_clock_tid %d", cli_clock_tid);*/

	int cli_io_tid = Create(PRIOR_MEDIUM, cli_io_task);
	Send(cli_io_tid, &cli_server_address, sizeof(cli_server_address), &handshake, sizeof(handshake));
	/*dump(SUBMISSION, "cli_io_tid %d", cli_io_tid);*/

	int num_track_updates = 0;

	while (*kill_all != HANDSHAKE_SHUTDOWN) {
		int requester_tid = INVALID_TID;
		Cli_request request;
		Receive(&requester_tid, &request, sizeof(request));

		switch (request.type) {
		case CLI_TRAIN_COMMAND:
			// from cli_io_task
			fifo_put(&cli_server.cmd_fifo, &request);
			handshake = HANDSHAKE_AKG;
			Reply(requester_tid, &handshake, sizeof(handshake));
			/*dump(SUBMISSION, "%s", "cli_server put train cmd cli req");*/
			break;

		case CLI_UPDATE_CLOCK:
			// from cli_clock_task
			fifo_put(&cli_server.status_update_fifo, &request);
			handshake = HANDSHAKE_AKG;
			Reply(requester_tid, &handshake, sizeof(handshake));
   			break;
 
		case CLI_UPDATE_TRAIN:
		case CLI_UPDATE_SENSOR:
		case CLI_UPDATE_SWITCH:
		case CLI_UPDATE_CALIBRATION:
			// from cli_request_courier
			fifo_put(&cli_server.status_update_fifo, &request);
			handshake = HANDSHAKE_AKG;
			Reply(requester_tid, &handshake, sizeof(handshake));
            /*dump(SUBMISSION, "update cli req %d", request.type);*/
			break;

		case CLI_SHUTDOWN:
			// from cli_io_task
			*kill_all = HANDSHAKE_SHUTDOWN;
			break;
		
		default:
			break;
		}

		if (request.type != CLI_WANT_COMMAND) {
			handshake = HANDSHAKE_AKG;
			Reply(requester_tid, &handshake, sizeof(handshake));
			bwprintf(COM2, "%s", "shutdown...");
		}		
		else if (!is_fifo_empty(&cli_server.cmd_fifo)) {
			Cli_request *cli_cmd_request;
			fifo_get(&cli_server.cmd_fifo, &cli_cmd_request);
			TS_request ts_request;
			ts_request.type = TS_COMMAND;
			ts_request.cmd = cli_cmd_request->cmd;
			/*dump(SUBMISSION, "cli send train cmd %d", ts_request.cmd.type);*/
			Reply(requester_tid, &ts_request, sizeof(ts_request));
		}

		if (!is_fifo_empty(&cli_server.status_update_fifo)) {
			Cli_request *update_request;
			fifo_get(&cli_server.status_update_fifo, &update_request);
			switch (update_request->type) {
			case CLI_UPDATE_CLOCK:
				cli_update_clock(update_request->clock_update);
				break;
			case CLI_UPDATE_TRAIN:
				/*dump(SUBMISSION, "%s", "cli pop train update req");*/
				cli_update_train(update_request->train_update);
				break;
			case CLI_UPDATE_SWITCH:
				/*dump(SUBMISSION, "%s", "cli pop switch update req");*/
				cli_update_switch(update_request->switch_update);
				break;
			case CLI_UPDATE_SENSOR:
				/*dump(SUBMISSION, "%s", "cli pop sensor group = %d, id = %d, time = %d",*/
							/*update_request->sensor_update.group, update_request->sensor_update.id,*/
							/*update_request->sensor_update.triggered_time);		*/
				cli_update_sensor(update_request->sensor_update, update_request->last_sensor_update, update_request->next_sensor_update);
				break;
			case CLI_UPDATE_CALIBRATION:
				/*dump(SUBMISSION, "%s", "cli pop calibration update req");*/
				cli_update_track(update_request->calibration_update, num_track_updates++);
				break;
			default:
				break;
			}
		}
	}

	cli_server.is_shutdown = 1;

	int expected_num_exit = 2;
	int num_exit = 0;
	int exit_list[2];
	exit_list[0] = cli_clock_tid;
	exit_list[1] = cli_io_tid;
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
		}
	}
	
	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 	

	Exit();
}

void cli_clock_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int cli_server_tid = INVALID_TID;
	vint cli_server_address;
	Receive(&cli_server_tid, &cli_server_address, sizeof(cli_server_address));
	Reply(cli_server_tid, &handshake, sizeof(handshake));
	Train_server *cli_server = (Cli_server *) cli_server_address;
	/*dump(SUBMISSION, "cli_clock_task cli_server_address = 0x%x", cli_server_address);	 */

	// digital clock
	vint elapsed_tenth_sec = 0;
	Clock clock;
	clock_init(&clock);

	while (cli_server->is_shutdown == 0) {
		Delay(10);	// update every 100ms
		elapsed_tenth_sec++;
		clock_update(&clock, elapsed_tenth_sec);

		Cli_request update_clock_request = get_update_clock_request(clock);
		Send(cli_server_tid, &update_clock_request, sizeof(update_clock_request), &handshake, sizeof(handshake));
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(cli_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

void cli_io_task()
{
	Handshake handshake = HANDSHAKE_AKG;
	int cli_server_tid = INVALID_TID;
	vint cli_server_address;
	Receive(&cli_server_tid, &cli_server_address, sizeof(cli_server_address));
	Reply(cli_server_tid, &handshake, sizeof(handshake));
	Train_server *cli_server = (Cli_server *) cli_server_address;
	/*dump(SUBMISSION, "cli_io_task cli_server_address = 0x%x", cli_server_address);*/

	// command
	Command_buffer command_buffer;
	command_buffer.pos = 0;
	Train train;
	int parse_result = 0;

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
				/*dump(SUBMISSION, "%s", "io entered train cmd, send cmd");*/
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

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(cli_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}
