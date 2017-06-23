#include <debug.h>
#include <cli.h>
#include <train.h>
#include <string.h>
#include <user_functions.h>
#include <kernel.h>

#define TWO_WAY_SWITCH_OFFSET 0x01
#define TWO_WAY_SWITCH_NUM 18
#define THREE_WAY_SWITCH_OFFSET 0x99
#define THREE_WAY_SWITCH_NUM 4

char switch_id_to_byte(uint8 id)
{
	assert(id <= NUM_SWITCHES, "sw: Wrong switch id %d provided!", id);
	if ( id > TWO_WAY_SWITCH_NUM ) {
		return THREE_WAY_SWITCH_OFFSET + ( id - TWO_WAY_SWITCH_NUM - 1 );
	}
	return id;
}

char switch_state_to_byte(char state)
{
	switch( state ) {
	case 's':
	case 'S':
		return STRAIGHT;
	case 'c':
	case 'C':
		return CURVE;
	default:
		/*assert(0, "sw: Wrong switch state provided! %s", state);*/
		break;
	}
	return 0;
}

void initialize_switch()
{
	bw_save();
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		bwputc(COM1, switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C')); // state
		bwputc(COM1, switch_id_to_byte( sw )); // switch
		bw_pos(SWITCH_ROW + sw - 1, RIGHT_BORDER - 1);
		bwputc(COM2, (sw == 16 || sw == 10 || sw == 15|| sw == 19 || sw == 21) ? 'S' : 'C');
		Delay(20);
	}
	bwputc(COM1, SOLENOID_OFF); // turn off solenoid
	bw_restore();
}

// method seems deprecated, currently only use initalize_switch
void test_initialize_switch()
{
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		bwputc(COM1, switch_state_to_byte((sw == 19 || sw == 21) ? 'C' : 'S')); // state
		bwputc(COM1, switch_id_to_byte( sw )); // switch
		Delay(20);
	}
	bwputc(COM1, SOLENOID_OFF); // turn off solenoid
}

static int is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static int is_state(char c)
{
	switch(c) {
	case 'c':
	case 'C':
	case 's':
	case 'S':
		return 1;
	default:
		return 0;
	}
}

void command_clear(Command_buffer *command_buffer)
{
	int i = 0;
	for (i = 0; i <= command_buffer->pos; i++) {
		command_buffer->data[i] = ' ';
	}
	command_buffer->data[command_buffer->pos] = '\0';
//	cli_user_input(command_buffer);
	command_buffer->pos = 0;
//	cli_user_input(command_buffer);
}

int command_parse(Command_buffer *command_buffer, char *ptrain_id, char *ptrain_speed, Command *pcmd)
{
	char args[10];
	int argc = 0;

	if (!strcmp(command_buffer->data, "go", 2)) {
		pcmd->type = GO;
	}
	else if (!strcmp(command_buffer->data, "stop", 4)) {
		pcmd->type = STOP;
	}
	else if (!strcmp(command_buffer->data, "tr", 2) || !strcmp(command_buffer->data, "rv", 2) || !strcmp(command_buffer->data, "sw", 2) || !strcmp(command_buffer->data, "st", 2)) {
		// parse arguments
		int pos = 2;
		char num_buffer[10];
		int i = 0;
		for (i = 0; i < 10; i++){
				num_buffer[i] = '\0';
		}	
		int num_buffer_pos = 0;
		while (pos++ < command_buffer->pos) {
			if (is_digit(command_buffer->data[pos])) {
				// current char is a digit
				num_buffer[num_buffer_pos++] = command_buffer->data[pos];
			}
			else if (is_state(command_buffer->data[pos])) {
				// current char is a switch state
				args[argc++] = command_buffer->data[pos];
			}
			else if (command_buffer->data[pos] == ' ' || (pos == command_buffer->pos)) {
				// skip space
				if (num_buffer_pos != 0) {
					// at the end of a number
					args[argc++] = atoi(num_buffer);
				}
				// clear num_buffer
				num_buffer_pos = 0;
				for (i = 0; i < 10; i++){
					num_buffer[i] = '\0';
				}
			}
			else {
				return -1;
			}
		}
	}
	else {
		return -1;
	}

	// Store parsing result in pcmd, update ptrain_id and ptrain_speed
	switch (command_buffer->data[0]) {
	case 't':
		/*assert(argc == 2, "tr: invalid number of arguments %d", argc);*/
		*ptrain_speed = pcmd->arg1;
		*ptrain_id = pcmd->arg0;
		pcmd->type = TR;
		break;
	case 'r':
		/*assert(argc == 1, "rv: invalid number of arguments %d", argc);*/
		pcmd->type = RV;
		break;
	case 's':
		/*assert(argc == 2, "sw: invalid number of arguments %d", argc);*/
		pcmd->type = SW;
		break;
	}
	pcmd->arg0 = args[0];
	pcmd->arg1 = (pcmd->type == RV) ? *ptrain_speed : args[1];

	return 0;
}

void delay_task()
{
	int train_task_tid;
	Delay_command delay_cmd;
	char reply_msg = 0;
	int tid = MyTid();
	debug(DEBUG_K4, "in delay task %d", tid);
	Receive(&train_task_tid, &delay_cmd, sizeof(delay_cmd));
	debug(DEBUG_K4, "received command from %d", train_task_tid);
	Reply(train_task_tid, &reply_msg, sizeof(reply_msg));
	
	switch(delay_cmd.type) {
	case RV:
    	Delay(delay_cmd.delay_time);
		debug(DEBUG_K4, "reached time limit %d, begin reverse", delay_cmd.delay_time);

		Putc(COM1, REVERSE);
		Putc(COM1, delay_cmd.arg0);

		Delay(delay_cmd.delay_time);
		debug(DEBUG_K4, "reached time limit %d, begin set speed", delay_cmd.delay_time);

		Putc(COM1, delay_cmd.arg1);
		Putc(COM1, delay_cmd.arg0);
		break;
	case SW:
		Delay(delay_cmd.delay_time);
		debug(DEBUG_K4, "reached time limit %d, begin to turn of SOLENOID_OFF", delay_cmd.delay_time);

		Putc(COM1, SOLENOID_OFF);
		break;
	}
	Exit();
}

void command_handle(Command *pcmd, Calibration_package *calibration_package)
{
	debug(DEBUG_K4, "enter %s", "command_handle");
	Delay_command delay_cmd;
	int delay_task_tid = INVALID_TID;
	char reply_msg;

    // pcmd->type get defined at train.h
	switch(pcmd->type) {
	case TR:
		if (pcmd->arg1 <= MAX_SPEED) {
			Putc(COM1, pcmd->arg1);
			Putc(COM1, pcmd->arg0);
        //    cli_update_train(pcmd->arg0, pcmd->arg1);
		} else {
			/*assert(0, "tr: Invalid speed %d", pcmd->arg1);*/
		}
		break;
	case RV:
		Putc(COM1, 0);
		Putc(COM1, pcmd->arg0);

		delay_task_tid = Create(PRIOR_LOW, delay_task);
		debug(DEBUG_K4, "delay_task_tid = %d", delay_task_tid);

		delay_cmd.type = RV;
		delay_cmd.delay_time = 20;
		delay_cmd.arg0 = pcmd->arg0; // train
		delay_cmd.arg1 = pcmd->arg1; // speed

		Send(delay_task_tid, &delay_cmd, sizeof(delay_cmd), &reply_msg, sizeof(reply_msg));
		break;
	case SW:
    	Putc(COM1, switch_state_to_byte(pcmd->arg1));
    	Putc(COM1, switch_id_to_byte(pcmd->arg0));

		delay_task_tid = Create(PRIOR_MEDIUM, delay_task);
		debug(DEBUG_K4, "delay_task_tid = %d", delay_task_tid);

		delay_cmd.type = SW;
		delay_cmd.delay_time = 20;

		debug(DEBUG_K4, "sending command to %d", delay_task_tid);
		Send(delay_task, &delay_cmd, sizeof(delay_cmd), &reply_msg, sizeof(reply_msg));
	
        // cli_update_switch(pcmd->arg0, pcmd->arg1);
		break;
	case GO:
        /*bwprintf(COM2, "%s", "stuck here?");*/
        Putc(COM1, START);
        bwprintf(COM2, "%s", "or here?");
		break;
	case STOP:
		Putc(COM1, HALT);
		break;
    case ST:
        // currently only deal with one train, pcmd->arg0 is the number representation of the sensor 
        *calibration_package->stop_sensor=pcmd->arg0;
        break;
	}
}
