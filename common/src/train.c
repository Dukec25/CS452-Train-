#include <debug.h>
#include <train.h>
#include <string.h>
#include <user_functions.h>

#define TWO_WAY_SWITCH_OFFSET 0x01
#define TWO_WAY_SWITCH_NUM 18
#define THREE_WAY_SWITCH_OFFSET 0x99
#define THREE_WAY_SWITCH_NUM 4

char switch_id_to_byte(uint8 id)
{
	assert(id <= NUM_SWITCHES, "sw: Wrong switch id provided!\n");
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
		assert(0, "sw: Wrong switch state provided!\n");
	}
	return 0;
}

void initialize_switch()
{
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		bwputc(COM1, switch_state_to_byte((sw == 19 || sw == 21) ? 'S' : 'C')); // state
		bwputc(COM1, switch_id_to_byte( sw )); // switch
	}
	bwputc( COM1, SOLENOID_OFF); // turn off solenoid
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

int command_parse(char *command_buffer, int *pcommand_irq_pos, char *ptrain_id, char *ptrain_speed, Command *pcmd)
{
	char args[10];
	int argc = 0;

	if (command_buffer[0] == 'q') {
		// quit
		return 1;
	}
	else if (!strcmp(command_buffer, "go", 2)) {
		pcmd->type = GO;
	}
	else if (!strcmp(command_buffer, "stop", 4)) {
		pcmd->type = STOP;
	}
	else if (!strcmp(command_buffer, "tr", 2) || !strcmp(command_buffer, "rv", 2) || !strcmp(command_buffer, "sw", 2)) {
		// parse arguments
		int pos = 2;
		char num_buffer[10];
		int num_irq_pos = 0;
		while (pos++ < *pcommand_irq_pos) {
			if (is_digit(command_buffer[pos])) {
				// current char is a digit
				num_buffer[num_irq_pos++] = command_buffer[pos];
			}
			else if (is_state(command_buffer[pos])) {
				// current char is a switch state
				args[argc++] = command_buffer[pos];
			}
			else if (command_buffer[pos] == ' ' || command_buffer[pos] == '\n' || command_buffer[pos] == '\r') {
				// skip non-digit characters: space, new line, and carriage return
				if (!num_irq_pos) {
					// at the end of a number
					args[argc++] = atoi(num_buffer);
				}
				// clear num_buffer
				while (!num_irq_pos) {
					num_buffer[num_irq_pos--] = 0;
				}
			}
			else {
				assert(0, "Invalid command\n");
				return -1;
			}
		}
	}
	else {
		assert(0, "Invalid command\n");
		return -1;
	}

	// clear command buffer
	while(!pcommand_irq_pos) {
		command_buffer[*pcommand_irq_pos--] = 0;
	}

	// Store parsing result in pcmd, update ptrain_id and ptrain_speed
	switch (command_buffer[0]) {
	case 't':
		assert(argc == 2, "tr: invalid arguments\n");
		*ptrain_speed = pcmd->arg1;
		*ptrain_id = pcmd->arg0;
		pcmd->type = TR;
	case 'r':
		assert(argc == 1, "rv: invalid arguments\n");
		pcmd->type = RV;
	case 's':
		assert(argc == 2, "sw: invalid arguments\n");
		pcmd->type = SW;
	default: // fall through
		pcmd->arg0 = args[0];
		pcmd->arg1 = (pcmd->type == RV) ? *ptrain_speed : args[1];
	}

	return 0;	
}

void command_handle(Command *pcmd)
{
	switch(pcmd->type) {
	case TR:
		if (pcmd->arg1 <= MAX_SPEED) {
			Putc(COM1, pcmd->arg1); // speed
			Putc(COM1, pcmd->arg0); // train
		} else {
			assert(0, "tr: Invalid speed %d\n", pcmd->arg1);
		}
		break;
	case RV:
		Putc(COM1, STOP); 	 		// stop	
		Putc(COM1, pcmd->arg0); 	// train
		// NEED TO MOVE delay into separate task, otherwise will block, ask Slavik tomorrow if you can!!!
		elay(50);					// Delay 0.5 second

		Putc(COM1, REVERSE); 	 	// reverse	
		Putc(COM1, pcmd->arg0); 	// train
		Delay(50);					// Delay 1 second

		Putc(COM1, pcmd->arg1); 	// speed
		Putc(COM1, pcmd->arg0); 	// train
		break;
	case SW:
		Putc(COM1, switch_state_to_byte(pcmd->arg1));	// state
		Putc(COM1, switch_id_to_byte(pcmd->arg0)); 		// switch
		Putc(COM1, SOLENOID_OFF); 						// turnoff solenoid
		break;
	case GO:
		Putc(COM1, START);
		break;
	case STOP:
		Putc(COM1, HALT);
		break;
	}
}
