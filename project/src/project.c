#include <debug.h>
#include <user_functions.h>
#include <train.h>
#include <train_task.h>

void project()
{
	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	
	irq_debug(SUBMISSION, "%s", "GAME START");
	irq_debug(SUBMISSION, "%s", "Please enter player train id: ");
	char player_buf[3];
	player_buf[0] = Getc(COM2);
	player_buf[1] = Getc(COM2);
	player_buf[2] = '\0';
	int player = atoi(player_buf);
	int predator = (player == 58 ? 71 : 58);
	int train_idx = (player == 58 ? 0 : 1);
	irq_debug(SUBMISSION, "You have choosen player to be %d", player);
	irq_debug(SUBMISSION, "Predator is %d", predator);

	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;

	int golds[6];
	golds[0] = 59;
	golds[1] = 61;
	golds[2] = 41;
	golds[3] = 37;
	golds[4] = 21;
	golds[5] = 45;
	irq_debug(SUBMISSION, "%s", "list of golds: ");
	int i = 0;
	for (; i < 6; i++) {
		Sensor sensor = num_to_sensor(golds[i]);
		irq_printf(COM2, "%c%d ", sensor.group, sensor.id);
	}
	irq_printf(COM2, "%s", "\r\n");
	for (i = 0; i < 6; i++) {
	    irq_pos(train_server->cli_map.sensors[i].row, train_server->cli_map.sensors[i].col);
	    irq_printf(COM2, "\035[31m"); // make sensor display magenta
    	Putc(COM2, 'X');
    	irq_printf(COM2, "\035[0m"); // reset special format
	}

	int current_score = 0;

	Command go58_cmd;
	go58_cmd.type = GO;
	go58_cmd.arg0 = 58;
	Cli_request go58_request;
	go58_request.type = CLI_TRAIN_COMMAND;
	go58_request.cmd = go58_cmd;
	Send(cli_server_tid, &go58_request, sizeof(go58_request), &handshake, sizeof(handshake));

	Command go71_cmd;
	go71_cmd.type = GO;
	go71_cmd.arg0 = 71;
	Cli_request go71_request;
	go71_request.type = CLI_TRAIN_COMMAND;
	go71_request.cmd = go71_cmd;
	Send(cli_server_tid, &go71_request, sizeof(go71_request), &handshake, sizeof(handshake));

	int exit = 0;
	while(exit == 0) {
		int scored = 0;
		int pick_up = 0;
		for (i = 0; i < 6; i++) {
			if (train_server->trains[train_idx].last_stop == golds[i]) {
				current_score++;
				scored = 1;
				pick_up = i;
				break;
			}
		}
		if (scored == 1) {
			Sensor sensor = num_to_sensor(pick_up);
			irq_printf(SUBMISSION, "picked up %c%d, current_score = %d", sensor.group, sensor.id, current_score);
		}
		if (current_score == 6) {
			exit = 1;
		}
	}

	debug(SUBMISSION, "%s", "GAME OVER");
	Exit();
}
