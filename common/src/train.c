#include <debug.h>
#include <cli.h>
#include <train.h>
#include <string.h>
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

// if here changes, need to change reverse_initialize_swtich below, 
// and train_server_init within train_server.c
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
void reverse_initialize_switch()
{
	int sw;
	for (sw = 1; sw <= NUM_SWITCHES ; sw++) {
		bwputc(COM1, switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'C' : 'S')); // state
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
	return num; 
}
 
void velocity69_initialization(Velocity_model *velocity_model) 
{
    velocity_model->train_id = 69;

    int i = 0;
    for ( ; i < MAX_SPEED + 1; i++) {
        velocity_model->velocity[i] = 0.;   // to be hardcoded
        velocity_model->stopping_distance[i] = 0;
    }

    velocity_model->stopping_distance[6] = 405;
    velocity_model->stopping_distance[8] = 550;
    velocity_model->stopping_distance[10] = 685;
    velocity_model->stopping_distance[14] = 935;
}

void velocity71_initialization(Velocity_model *velocity_model) 
{
    velocity_model->train_id = 71;

    int i = 0;
    for ( ; i < MAX_SPEED + 1; i++) {
        velocity_model->velocity[i] = 0.;   // to be hardcoded
        velocity_model->stopping_distance[i] = 0;
    }

    velocity_model->stopping_distance[6] = 90;
    velocity_model->stopping_distance[8] = 210;
    velocity_model->stopping_distance[10] = 399;
    velocity_model->stopping_distance[14] = 1265;
}

void velocity_update(int speed, double real_velocity, Velocity_model *velocity_model)
{
    if (real_velocity <= 0 || real_velocity >= MAX_VELOCITY) {
        return;
    }
    if (velocity_model->velocity[speed] == 0) {
        velocity_model->velocity[speed] = real_velocity;
    }
    else {
        double alpha = ALPHA;
        velocity_model->velocity[speed] = alpha * real_velocity + velocity_model->velocity[speed] * (1 - alpha);
    }
}

int train_id_to_idx(int train_id)
{
	switch(train_id) {
	case 69:
		return 0;
	case 71:
		return 1;
	case 58:
		return 2;
	default:
		return -1;
	}
}

int speed_to_idx(int speed)
{
	switch(speed) {
	case 6:
		return 0;
	case 7:
		return 1;
	case 8:
		return 2;
	default:
		return -1;
	}
}

int distance_to_idx(int distance)
{
	if (distance <= 10 || distance >= 95) {
		return -1;
	}
	else if (distance % 10 == 0) {
		return distance / 10 - 1;
	}
	else if (distance % 10 < 5) {
		return distance / 10;
	}
	else if (distance % 10 >= 5) {
		return distance / 10 + 1;
	}
}

void walk_table_initialization(Walk_table *walk_table)
{
/*
	walk_table->walk_data[0][0][0] = ; // 69, 6, 10
	walk_table->walk_data[0][0][1] = ; // 69, 6, 20
	walk_table->walk_data[0][0][2] = ; // 69, 6, 30
	walk_table->walk_data[0][0][3] = ; // 69, 6, 40
	walk_table->walk_data[0][0][4] = ; // 69, 6, 50
	walk_table->walk_data[0][0][5] = ; // 69, 6, 60
	walk_table->walk_data[0][0][6] = ; // 69, 6, 70
	walk_table->walk_data[0][0][7] = ; // 69, 6, 80
	walk_table->walk_data[0][0][8] = ; // 69, 6, 90

	walk_table->walk_data[0][1][0] = ; // 69, 7, 10
	walk_table->walk_data[0][1][1] = ; // 69, 7, 20
	walk_table->walk_data[0][1][2] = ; // 69, 7, 30
	walk_table->walk_data[0][1][3] = ; // 69, 7, 40
	walk_table->walk_data[0][1][4] = ; // 69, 7, 50
	walk_table->walk_data[0][1][5] = ; // 69, 7, 60
	walk_table->walk_data[0][1][6] = ; // 69, 7, 70
	walk_table->walk_data[0][1][7] = ; // 69, 7, 80
	walk_table->walk_data[0][1][8] = ; // 69, 7, 90

	walk_table->walk_data[0][2][0] = ; // 69, 8, 10
	walk_table->walk_data[0][2][1] = ; // 69, 8, 20
	walk_table->walk_data[0][2][2] = ; // 69, 8, 30
	walk_table->walk_data[0][2][3] = ; // 69, 8, 40
	walk_table->walk_data[0][2][4] = ; // 69, 8, 50
	walk_table->walk_data[0][2][5] = ; // 69, 8, 60
	walk_table->walk_data[0][2][6] = ; // 69, 8, 70
	walk_table->walk_data[0][2][7] = ; // 69, 8, 80
	walk_table->walk_data[0][2][8] = ; // 69, 8, 90

	walk_table->walk_data[1][0][0] = ; // 71, 6, 10
	walk_table->walk_data[1][0][1] = ; // 71, 6, 20
	walk_table->walk_data[1][0][2] = ; // 71, 6, 30
	walk_table->walk_data[1][0][3] = ; // 71, 6, 40
	walk_table->walk_data[1][0][4] = ; // 71, 6, 50
	walk_table->walk_data[1][0][5] = ; // 71, 6, 60
	walk_table->walk_data[1][0][6] = ; // 71, 6, 70
	walk_table->walk_data[1][0][7] = ; // 71, 6, 80
	walk_table->walk_data[1][0][8] = ; // 71, 6, 90

	walk_table->walk_data[1][1][0] = ; // 71, 7, 10
	walk_table->walk_data[1][1][1] = ; // 71, 7, 20
	walk_table->walk_data[1][1][2] = ; // 71, 7, 30
	walk_table->walk_data[1][1][3] = ; // 71, 7, 40
	walk_table->walk_data[1][1][4] = ; // 71, 7, 50
	walk_table->walk_data[1][1][5] = ; // 71, 7, 60
	walk_table->walk_data[1][1][6] = ; // 71, 7, 70
	walk_table->walk_data[1][1][7] = ; // 71, 7, 80
	walk_table->walk_data[1][1][8] = ; // 71, 7, 90

	walk_table->walk_data[1][2][0] = ; // 71, 8, 10
	walk_table->walk_data[1][2][1] = ; // 71, 8, 20
	walk_table->walk_data[1][2][2] = ; // 71, 8, 30
	walk_table->walk_data[1][2][3] = ; // 71, 8, 40
	walk_table->walk_data[1][2][4] = ; // 71, 8, 50
	walk_table->walk_data[1][2][5] = ; // 71, 8, 60
	walk_table->walk_data[1][2][6] = ; // 71, 8, 70
	walk_table->walk_data[1][2][7] = ; // 71, 8, 80
	walk_table->walk_data[1][2][8] = ; // 71, 8, 90

	walk_table->walk_data[2][0][0] = ; // 58, 6, 10
	walk_table->walk_data[2][0][1] = ; // 58, 6, 20
	walk_table->walk_data[2][0][2] = ; // 58, 6, 30
	walk_table->walk_data[2][0][3] = ; // 58, 6, 40
	walk_table->walk_data[2][0][4] = ; // 58, 6, 50
	walk_table->walk_data[2][0][5] = ; // 58, 6, 60
	walk_table->walk_data[2][0][6] = ; // 58, 6, 70
	walk_table->walk_data[2][0][7] = ; // 58, 6, 80
	walk_table->walk_data[2][0][8] = ; // 58, 6, 90

	walk_table->walk_data[2][1][0] = ; // 58, 7, 10
	walk_table->walk_data[2][1][1] = ; // 58, 7, 20
	walk_table->walk_data[2][1][2] = ; // 58, 7, 30
	walk_table->walk_data[2][1][3] = ; // 58, 7, 40
	walk_table->walk_data[2][1][4] = ; // 58, 7, 50
	walk_table->walk_data[2][1][5] = ; // 58, 7, 60
	walk_table->walk_data[2][1][6] = ; // 58, 7, 70
	walk_table->walk_data[2][1][7] = ; // 58, 7, 80
	walk_table->walk_data[2][1][8] = ; // 58, 7, 90

	walk_table->walk_data[2][2][0] = ; // 58, 8, 10
	walk_table->walk_data[2][2][1] = ; // 58, 8, 20
	walk_table->walk_data[2][2][2] = ; // 58, 8, 30
	walk_table->walk_data[2][2][3] = ; // 58, 8, 40
	walk_table->walk_data[2][2][4] = ; // 58, 8, 50
	walk_table->walk_data[2][2][5] = ; // 58, 8, 60
	walk_table->walk_data[2][2][6] = ; // 58, 8, 70
	walk_table->walk_data[2][2][7] = ; // 58, 8, 80
	walk_table->walk_data[2][2][8] = ; // 58, 8, 90
*/
}

int walk_table_lookup(Walk_table *walk_table, int train_id, int speed, int distance)
{
	int idx1 = train_id_to_idx(train_id);
	int idx2 = speed_to_idx(speed);
	int idx3 = distance_to_idx(distance);

	if (idx1 == -1 || idx2 == -1 || idx3 == -1) {
		return -1;
	}
	
	return walk_table->walk_data[idx1][idx2][idx3];
}

Command get_tr_command(char id, char speed)
{
    Command tr_cmd;
    tr_cmd.type = TR;
    tr_cmd.arg0 = id;
    tr_cmd.arg1 = speed;
    return tr_cmd;
}

Command get_sw_command(char id, char state)
{
	Command sw_cmd;
	sw_cmd.type = SW;
	sw_cmd.arg0 = id;
	sw_cmd.arg1 = state;
	return sw_cmd;
}

Command get_sensor_command()
{
	Command sensor_cmd;
	sensor_cmd.type = SENSOR;
	return sensor_cmd;
}

Command get_tr_stop_command(char id)
{
	Command tr_stop_cmd;
	tr_stop_cmd.type = TR;
	tr_stop_cmd.arg0 = id;
	tr_stop_cmd.arg1 = MIN_SPEED;
	return tr_stop_cmd;
}

Command get_br_command(char group, char id)
{
	Command br_cmd;
	br_cmd.type = BR;
	br_cmd.arg0 = group;
	br_cmd.arg1 = id;
	return br_cmd;
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
			 !strcmp(command_buffer->data, "sw", 2 ) || !strcmp(command_buffer->data, "dc", 2) ||
			 !strcmp(command_buffer->data, "br", 2) || !strcmp(command_buffer->data, "park", 4) ||
             !strcmp(command_buffer->data, "map", 3) || !strcmp(command_buffer->data, "kc", 2)) {
		// parse arguments
		int pos = !strcmp(command_buffer->data, "park", 4) ? 4 : 2;

        if ( !strcmp(command_buffer->data, "map", 3) ){
            pos = 3; 
        } 

		char num_buffer[10];
		int i = 0;
		for (i = 0; i < 10; i++) {
				num_buffer[i] = '\0';
		}	
		int num_buffer_pos = 0;

        if(command_buffer->data[pos] != ' '){
            // if the next character after command not space
            return -1;
        }

        // skip the next character after command
		while (pos++ < command_buffer->pos) {
			if (is_digit(command_buffer->data[pos])) {
				// current char is a digit
				num_buffer[num_buffer_pos++] = command_buffer->data[pos];
			}
			else if (is_alpha(command_buffer->data[pos])) {
				// current char is a switch state
                int temp = argc;
				args[argc++] = command_buffer->data[pos];
                /*irq_debug(SUBMISSION, "alpha data=%c\r\n", args[temp]);*/
			}
			else if (command_buffer->data[pos] == ' ' || (pos == command_buffer->pos)) {
				// skip space
				if (num_buffer_pos != 0) {
					// at the end of a number
                    int temp = argc;
					args[argc++] = atoi(num_buffer);
                    /*irq_debug(SUBMISSION, "converted value=%d\r\n", args[temp]);*/
				}
				// clear num_buffer
				num_buffer_pos = 0;
				for (i = 0; i < 10; i++) {
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
    /*debug(SUBMISSION, "args%s\r\n", args);*/
    /*debug(SUBMISSION, "argc num = %d\r\n", argc);*/
    
    // currently no commands need more than three arguments
    // there will probably won't be in the future as well
    if(argc > 3){
        return -1;
    }

	// Store parsing result in pcmd, update ptrain_id and ptrain_speed
	switch (command_buffer->data[0]) {
	case 't':
        if (argc != 2){
            return -1;
        }
		if (args[1] > MAX_SPEED) {
			return -1;
		}
		pcmd->type = TR;
		ptrain->speed = args[1];
		ptrain->id = args[0];
		break;
	case 'r':
        if (argc != 1){
            return -1;
        }
		pcmd->type = RV;
		break;
	case 's':
        if (argc != 2){
            return -1;
        }
		if (switch_state_to_byte(pcmd->arg1) == -1 ||  switch_id_to_byte(pcmd->arg0) == -1) {
			return -1;
		}
		pcmd->type = SW;
		break;
	case 'b':
        if (argc != 2){
            return -1;
        }
        if(args[1] > 16){
            return -1;
        }
		pcmd->type = BR;
		break;
	case 'd':
        if (argc != 2){
            return -1;
        }
        if(args[1] > 16){
            return -1;
        }
		pcmd->type = DC;
		break;
	case 'p':
        if (argc != 3){
            return -1;
        }
        if(args[1] > 16){
            return -1;
        }
		pcmd->type = PARK;
		break;
    case 'm':
        /*bwprintf(COM2, "TRIGGER train.c, 1%c 2%c", args[0], args[1]); */
        if (argc != 1){
            return -1;
        }
        if (args[0] != 'A' && args[0] != 'B' && args[0] != 'a' && args[0] != 'b'){
            return -1;
        }
        pcmd->type = MAP;
        break;
    case 'k':
        if (argc != 2) {
            return -1;
        }
        if (args[0] > MAX_SPEED) {
            return -1;
        }
        pcmd->type = KC;
        break;
	}
	pcmd->arg0 = args[0];
	pcmd->arg1 = (pcmd->type == RV) ? ptrain->speed : args[1];
    pcmd->arg2 = args[2];

	return 0;
}

void command_handle(Command *pcmd)
{
	irq_debug(DEBUG_K4, "enter %s", "command_handle");

	// pcmd->type get defined at train.h
	switch(pcmd->type) {
	case TR:
        // add 16 here to turn on the headlight of the train
		irq_printf(COM1, "%c%c", pcmd->arg1+16, pcmd->arg0);
		break;
	case RV:
		irq_printf(COM1, "%c%c", MIN_SPEED, pcmd->arg0);

		Delay(20);
		irq_debug(DEBUG_K4, "%s", "reached time limit, begin reverse");
		irq_printf(COM1, "%c%c", REVERSE, pcmd->arg0);

		Delay(20);
		irq_debug(DEBUG_K4, "%s", "reached time limit, begin set speed");
		irq_printf(COM1, "%c%c", pcmd->arg1+16, pcmd->arg0);

		break;
	case SW:
		irq_printf(COM1, "%c%c", switch_state_to_byte(pcmd->arg1), switch_id_to_byte(pcmd->arg0));
        // track switches status
		Delay(20);
		irq_debug(DEBUG_K4, "%s", "reached time limit, begin to turn of SOLENOID_OFF");
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
