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
		assert(0, "sw: Wrong switch state provided! %s", state);
		break;
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

int command_parse(char *command_buffer, int *pcommand_buffer_pos, char *ptrain_id, char *ptrain_speed, Command *pcmd)
{
	char args[10];
	int argc = 0;

	if (!strcmp(command_buffer, "go", 2)) {
		pcmd->type = GO;
	}
	else if (!strcmp(command_buffer, "stop", 4)) {
		pcmd->type = STOP;
	}
	else if (!strcmp(command_buffer, "tr", 2) || !strcmp(command_buffer, "rv", 2) || !strcmp(command_buffer, "sw", 2)) {
		// parse arguments
		int pos = 2;
		char num_buffer[10];
		int i = 0;
		for (i = 0; i < 10; i++){
				num_buffer[i] = '\0';
		}	
		int num_buffer_pos = 0;
		debug(DEBUG_K4, "%s", "start of a valid command");
		while (pos++ < *pcommand_buffer_pos) {
			debug(DEBUG_K4, "pos = %d", pos);
			if (is_digit(command_buffer[pos])) {
				// current char is a digit
				num_buffer[num_buffer_pos++] = command_buffer[pos];
				debug(DEBUG_K4, "num_buffer = %s", num_buffer);
			}
			else if (is_state(command_buffer[pos])) {
				// current char is a switch state
				args[argc++] = command_buffer[pos];
				debug(DEBUG_K4, "args[%d] = %d", argc - 1, args[argc - 1]);
			}
			else if (command_buffer[pos] == ' ' || (pos == *pcommand_buffer_pos)) {
				// skip space
				if (num_buffer_pos != 0) {
					// at the end of a number
					args[argc++] = atoi(num_buffer);
					debug(DEBUG_K4, "args[%d] = %d", argc - 1, args[argc - 1]);
				}
				// clear num_buffer
				num_buffer_pos = 0;
				for (i = 0; i < 10; i++){
					num_buffer[i] = '\0';
				}
			}
			else {
				assert(0, "Invalid char %d at pos = %d", command_buffer[pos - 1], pos - 1);
				return -1;
			}
		}
	}
	else {
		assert(0, "Invalid command %s", command_buffer);
		return -1;
	}

	// Store parsing result in pcmd, update ptrain_id and ptrain_speed
	switch (command_buffer[0]) {
	case 't':
		assert(argc == 2, "tr: invalid number of arguments %d", argc);
		*ptrain_speed = pcmd->arg1;
		*ptrain_id = pcmd->arg0;
		pcmd->type = TR;
		break;
	case 'r':
		assert(argc == 1, "rv: invalid number of arguments %d", argc);
		pcmd->type = RV;
		break;
	case 's':
		assert(argc == 2, "sw: invalid number of arguments %d", argc);
		pcmd->type = SW;
		break;
	}
	pcmd->arg0 = args[0];
	pcmd->arg1 = (pcmd->type == RV) ? *ptrain_speed : args[1];

	// clear command buffer
	int i = 0;
	for (i = 0; i <= *pcommand_buffer_pos; i++) {
		command_buffer[i] = '\0';
	}
	*pcommand_buffer_pos = 0;

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
			assert(0, "tr: Invalid speed %d", pcmd->arg1);
		}
		break;
	case RV:
		Putc(COM1, STOP); 	 		// stop	
		Putc(COM1, pcmd->arg0); 	// train
		Delay(50);					// Delay 0.5 second

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
