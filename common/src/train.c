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

int sensor_to_num(Sensor sensor)
{
	return sensor.group * SENSORS_PER_GROUP + sensor.id - 1;
}

Sensor num_to_sensor(int num)
{
	Sensor sensor;
	sensor.group = num / SENSORS_PER_GROUP;
	sensor.id = num % SENSORS_PER_GROUP + 1;
	return sensor;
}

int track_node_name_to_num(char *name)
{
	char group_buf[10];
	int group_idx = 0;
	char id_buf[10];
	int id_idx = 0;
	int idx = 0;
	while(name[idx] != '\0') {
		if (is_alpha(name[idx])) {
			group_buf[group_idx++] = name[idx];
		}
		else {
			id_buf[id_idx++] = name[idx];
		}
		idx++;
	}
	group_buf[group_idx] = '\0';
	id_buf[id_idx] = '\0';
	//debug(SUBMISSION, "name = %s, group_buf = %s, id_buf = %s", name, group_buf, id_buf);

	int num = -1;
	int id = atoi(id_buf);
	if (group_idx == 1) {
		// A1 - E16
		Sensor sensor;
		sensor.group = group_buf[0] - SENSOR_LABEL_BASE;
		sensor.id = id;
		num = sensor_to_num(sensor);
	}
	else if (group_idx == 2) {
		// BR, MR, EN, EX
		if ((!strcmp(group_buf, "BR", 2)) && (id <= 18)) {
			// BR1 - 18
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS;
			int base = 1;
			num = offset + 2 * (id - base);
		}
		else if ((!strcmp(group_buf, "MR", 2)) && (id <= 18)) {
			// MR1 - 18
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS;
			int base = 1;
			num = offset + 2 * (id - base) + 1;
		}
		else if ((!strcmp(group_buf, "BR", 2)) && (id <= 156)) {
			// BR153 - 156
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS + 18 * 2;
			int base = 153;
			num = offset + 2 * (id - base);
		}
		else if ((!strcmp(group_buf, "MR", 2)) && (id <= 156)) {
			// MR153 - 156
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS + 18 * 2;
			int base = 153;
			num = offset + 2 * (id - base) + 1;
		}
		else if ((!strcmp(group_buf, "EN", 2)) && (id <= 10)) {
			// EN1 - 10
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS + 18 * 2 + 4 * 2;
			int base = 1;
			num = offset + 2 * (id - base);
		}
		else if ((!strcmp(group_buf, "EX", 2)) && (id <= 10)) {
			// EX1 - 10
			int offset = SENSORS_PER_GROUP * SENSOR_GROUPS + 18 * 2 + 4 * 2;
			int base = 1;
			num = offset + 2 * (id - base) + 1;
		}
	}
	//debug(SUBMISSION, "num = %d", num);
	return num; 
}
 
int velocity14_initialization(Velocity_data *velocity_data)
{
	int i;
	for (i = 0; i < TRACK_MAX; i++) {
		velocity_data->node[i].src = i;
		velocity_data->node[i].num_velocity = 0;
	}
 
	velocity_data->stopping_distance = 940;

	int index;
	index = track_node_name_to_num("A3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C11");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("A4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B16");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D14");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C2");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C9");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B13");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 8;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("B14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D16");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A3");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C10");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C5");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B4");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 8;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D12");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B15");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B1");
	velocity_data->node[index].velocity[0] = 2;
	velocity_data->node[index].dest[1] = track_node_name_to_num("B3");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C11");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E16");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C12");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E4");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B6");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E6");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E10");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D8");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E8");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D12");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B13");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("E2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E15");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E5");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D6");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C4");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].dest[2] = track_node_name_to_num("E3");
	velocity_data->node[index].velocity[2] = 7;
	velocity_data->node[index].num_velocity = 3;

	index = track_node_name_to_num("E8");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C14");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 5;
	velocity_data->node[index].num_velocity = 2;
	
	index = track_node_name_to_num("E10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E13");
	velocity_data->node[index].velocity[0] = 9;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E11");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D10");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E13");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D15");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E9");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E1");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("MR9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 5;
	velocity_data->node[index].num_velocity = 2;
}
 
int velocity_lookup(int src, int dest, Velocity_data *velocity_data)
{

	if (velocity_data->node[src].num_velocity == 0) {
		//debug(SUBMISSION, "velocity_lookup src = %c%d, velocity = %d",
		//	num_to_sensor(src).group + SENSOR_LABEL_BASE, num_to_sensor(src).id, -1);
		return -1;
	}

	int i;
	for (i = 0; i < velocity_data->node[src].num_velocity; i++) {
		if (velocity_data->node[src].dest[i] == dest) {
			//debug(SUBMISSION, "velocity_lookup src = %c%d, dest = %c%d, velocity = %d",
			//	num_to_sensor(src).group + SENSOR_LABEL_BASE, num_to_sensor(src).id,
			//	num_to_sensor(velocity_data->node[src].dest[i]).group + SENSOR_LABEL_BASE,
			//	num_to_sensor(velocity_data->node[src].dest[i]).id,
			//	velocity_data->node[src].velocity[0]);
			return velocity_data->node[src].velocity[i];
		}
	}
	return -1;
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
		return 0;
	}
	else if (!strcmp(command_buffer->data, "stop", 4)) {
		pcmd->type = STOP;
		return 0;
	}
	else if (!strcmp(command_buffer->data, "tr", 2) || !strcmp(command_buffer->data, "rv", 2) ||
			 !strcmp(command_buffer->data, "sw", 2 ) || !strcmp(command_buffer->data, "park", 4) ||
			 !strcmp(command_buffer->data, "dc", 2)) {
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
			else if (is_alpha(command_buffer->data[pos])) {
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
	case 'p':
		pcmd->type = PARK;
		debug(SUBMISSION, "parse PARK cmd, arg0 = %d, arg1 = %d", args[0], args[1]);
		break;
	case 'd':
		pcmd->type = DC;
		//debug(SUBMISSION, "parse DC cmd, arg0 = %d, arg1 = %d", args[0], args[1]);
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
        // track switches status
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
	default:
		break;
	}
}
