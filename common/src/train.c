#include <debug.h>
#include <cli.h>
#include <train.h>
#include <string.h>
#include <user_functions.h>
#include <kernel.h>
#include <cursor.h>
#include <irq_io.h>

#define TWO_WAY_SWITCH_OFFSET 0x01
#define TWO_WAY_SWITCH_NUM 18
#define THREE_WAY_SWITCH_OFFSET 0x99
#define THREE_WAY_SWITCH_NUM 4

char switch_id_to_byte(uint8 id)
{
	if (id > NUM_SWITCHES) {
		return -1;
	}
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
		break;
	}
	return -1;
}

void initialize_switch()
{
	bw_save();
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		bwputc(COM1, switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C')); // state
		bwputc(COM1, switch_id_to_byte( sw )); // switch
		bw_pos(SWITCH_ROW + sw - 1, RIGHT_BORDER - 1);
		bwputc(COM2, (sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
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

void sensor_initialization()
{
	Delay(20);	// delay 0.2 second
	// clear up any unread sensor data
	Putc(COM1, SENSOR_QUERY);
	int group = 0;
	for (group = 0; group < SENSOR_GROUPS; group++) {
		Getc(COM1);
		Getc(COM1);
	} 
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
	cli_user_input(command_buffer);
	command_buffer->pos = 0;
	cli_user_input(command_buffer);
}

int command_parse(Command_buffer *command_buffer, Train *ptrain, Command *pcmd)
{
	char args[10];
	int argc = 0;

	if (!strcmp(command_buffer->data, "go", 2)) {
		pcmd->type = GO;
	}
	else if (!strcmp(command_buffer->data, "stop", 4)) {
		pcmd->type = STOP;
	}
	else if (!strcmp(command_buffer->data, "tr", 2) || !strcmp(command_buffer->data, "rv", 2) || !strcmp(command_buffer->data, "sw", 2 ) || !strcmp(command_buffer->data, "br", 2)) {
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
		if (args[1] > MAX_SPEED) {
			return -1;
		}
		ptrain->speed = args[1];
		ptrain->id = args[0];
		pcmd->type = TR;
		break;
	case 'r':
		pcmd->type = RV;
		break;
	case 's':
		if (switch_state_to_byte(pcmd->arg1) == -1 ||  switch_id_to_byte(pcmd->arg0) == -1) {
			return -1;
		}
		pcmd->type = SW;
		break;
    case 'b':
        pcmd->type = BR;
        break;
	}
	pcmd->arg0 = args[0];
	pcmd->arg1 = (pcmd->type == RV) ? ptrain->speed : args[1];

	return 0;
}

void command_handle(Command *pcmd, Train_server *train_server)
{
	debug(DEBUG_K4, "enter %s", "command_handle");

	// pcmd->type get defined at train.h
	switch(pcmd->type) {
	case TR:
		irq_printf(COM1, "%c%c", pcmd->arg1, pcmd->arg0);
		break;
	case RV:
		irq_printf(COM1, "%c%c", MIN_SPEED, pcmd->arg0);

		Delay(20);
		debug(DEBUG_K4, "%s", "reached time limit, begin reverse");
		irq_printf(COM1, "%c%c", REVERSE, pcmd->arg0);

		Delay(20);
		debug(DEBUG_K4, "%s", "reached time limit, begin set speed");
		irq_printf(COM1, "%c%c", pcmd->arg1, pcmd->arg0);

		break;
	case SW:
		irq_printf(COM1, "%c%c", switch_state_to_byte(pcmd->arg1), switch_id_to_byte(pcmd->arg0));
        train_server->switches_status[pcmd->arg0-1] = switch_state_to_byte(pcmd->arg1);
		Delay(20);
		debug(DEBUG_K4, "%s", "reached time limit, begin to turn of SOLENOID_OFF");
		Putc(COM1, SOLENOID_OFF);
		break;
	case GO:
		Putc(COM1, START);
		break;
	case STOP:
		Putc(COM1, HALT);
		break;
	}
}
