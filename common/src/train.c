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
 
void velocity6_initialization(Velocity_data *velocity_data)
{
    int i;
    for (i = 0; i < TRACK_MAX; i++) {
        velocity_data->node[i].src = i;
        velocity_data->node[i].num_velocity = 0;
        int j;
        for (j = 0; j < MAX_NUM_VELOCITIES; j++) {
            velocity_data->node[i].updates[j] = 1;
        }
    }
 
    velocity_data->stopping_distance = 405;

}

void velocity8_initialization(Velocity_data *velocity_data)
{
    int i;
    for (i = 0; i < TRACK_MAX; i++) {
        velocity_data->node[i].src = i;
        velocity_data->node[i].num_velocity = 0;
        int j;
        for (j = 0; j < MAX_NUM_VELOCITIES; j++) {
            velocity_data->node[i].updates[j] = 1;
        }
    }
 
    velocity_data->stopping_distance = 550;

    int index;
}

void velocity10_initialization(Velocity_data *velocity_data)
{
	int i;
	for (i = 0; i < TRACK_MAX; i++) {
		velocity_data->node[i].src = i;
		velocity_data->node[i].num_velocity = 0;
		int j;
		for (j = 0; j < MAX_NUM_VELOCITIES; j++) {
			velocity_data->node[i].updates[j] = 1;
		}
	}
 
	velocity_data->stopping_distance = 685;

	int index;
}

void velocity14_initialization(Velocity_data *velocity_data)
{
	int i;
	for (i = 0; i < TRACK_MAX; i++) {
		velocity_data->node[i].src = i;
		velocity_data->node[i].num_velocity = 0;
		int j;
		for (j = 0; j < MAX_NUM_VELOCITIES; j++) {
			velocity_data->node[i].updates[j] = 1;
		}
	}
 
	velocity_data->stopping_distance = 935;

	int index;
	index = track_node_name_to_num("A3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C11");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 5;

	index = track_node_name_to_num("A4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B16");
	velocity_data->node[index].velocity[0] = 80;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 1;

	index = track_node_name_to_num("B1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D14");
	velocity_data->node[index].velocity[0] = 70;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 8;

	index = track_node_name_to_num("B3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C2");
	velocity_data->node[index].velocity[0] = 40;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 1;

	index = track_node_name_to_num("B4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C9");
	velocity_data->node[index].velocity[0] = 64;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 5;

	index = track_node_name_to_num("B6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 56;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 6;

	index = track_node_name_to_num("B13");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 64;
	velocity_data->node[index].updates[0] = 4;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 80;
	velocity_data->node[index].updates[1] = 2;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("B14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D16");
	velocity_data->node[index].velocity[0] = 25;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 21;

	index = track_node_name_to_num("B15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A3");
	velocity_data->node[index].velocity[0] = 72;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 5;

	index = track_node_name_to_num("B16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C10");
	velocity_data->node[index].velocity[0] = 55;
	velocity_data->node[index].updates[0] = 29;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C5");
	velocity_data->node[index].velocity[1] = 68;
	velocity_data->node[index].num_velocity = 2;
	velocity_data->node[index].updates[1] = 1;

	index = track_node_name_to_num("C1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B4");
	velocity_data->node[index].velocity[0] = 32;
	velocity_data->node[index].num_velocity = 1;
	velocity_data->node[index].updates[0] = 5;

	index = track_node_name_to_num("C2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 1;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 70;
	velocity_data->node[index].updates[1] = 17;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D12");
	velocity_data->node[index].velocity[0] = 100;
	velocity_data->node[index].updates[0] = 2;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[1] = 165;
	velocity_data->node[index].updates[1] = 2;
	velocity_data->node[index].dest[2] = track_node_name_to_num("C15");
	velocity_data->node[index].velocity[2] = 45;
	velocity_data->node[index].updates[2] = 4;
	velocity_data->node[index].num_velocity = 3;

	index = track_node_name_to_num("C9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B15");
	velocity_data->node[index].velocity[0] = 66;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B1");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 8;
	velocity_data->node[index].dest[1] = track_node_name_to_num("B3");
	velocity_data->node[index].velocity[1] = 55;
	velocity_data->node[index].updates[1] = 20;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C11");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E16");
	velocity_data->node[index].velocity[0] = 56;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C12");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 52;
	velocity_data->node[index].updates[0] = 27;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 91;
	velocity_data->node[index].updates[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D12");
	velocity_data->node[index].velocity[0] = 75;
	velocity_data->node[index].updates[0] = 4;
	velocity_data->node[index].num_velocity = 1;
    
	index = track_node_name_to_num("D1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 71;
	velocity_data->node[index].updates[0] = 19;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 80;
	velocity_data->node[index].updates[1] = 1;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E4");
	velocity_data->node[index].velocity[0] = 20;
	velocity_data->node[index].updates[0] = 4;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B6");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E6");
	velocity_data->node[index].velocity[0] = 58;
	velocity_data->node[index].updates[0] = 28;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E10");
	velocity_data->node[index].velocity[0] = 100;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D8");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E8");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 140;
	velocity_data->node[index].updates[0] = 1;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 128;
	velocity_data->node[index].updates[1] = 5;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D12");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[0] = 40;
	velocity_data->node[index].updates[0] = 2;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 40;
	velocity_data->node[index].updates[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B13");
	velocity_data->node[index].velocity[0] = 20;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 40;
	velocity_data->node[index].updates[0] = 21;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E1");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 80;
	velocity_data->node[index].updates[0] = 1;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 80;
	velocity_data->node[index].updates[1] = 1;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("E2");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E15");
	velocity_data->node[index].velocity[0] = 25;
	velocity_data->node[index].updates[0] = 22;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E3");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 20;
	velocity_data->node[index].updates[0] = 1;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E4");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E5");
	velocity_data->node[index].velocity[0] = 44;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E5");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D6");
	velocity_data->node[index].velocity[0] = 56;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E6");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 71;
	velocity_data->node[index].updates[0] = 24;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D4");
	velocity_data->node[index].velocity[1] = 43;
	velocity_data->node[index].updates[1] = 6;
	velocity_data->node[index].dest[2] = track_node_name_to_num("E3");
	velocity_data->node[index].velocity[2] = 4;
	velocity_data->node[index].updates[2] = 1;
	velocity_data->node[index].num_velocity = 3;

	index = track_node_name_to_num("E8");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C14");
	velocity_data->node[index].velocity[0] = 141;
	velocity_data->node[index].updates[0] = 8;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 101;
	velocity_data->node[index].updates[0] = 27;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 120;
	velocity_data->node[index].updates[0] = 2;
	velocity_data->node[index].num_velocity = 2;
	
	index = track_node_name_to_num("E10");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E13");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E11");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D10");
	velocity_data->node[index].velocity[0] = 60;
	velocity_data->node[index].updates[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E13");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D15");
	velocity_data->node[index].velocity[0] = 40;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E14");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E9");
	velocity_data->node[index].velocity[0] = 58;
	velocity_data->node[index].updates[0] = 34;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E15");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 50;
	velocity_data->node[index].updates[0] = 21;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E16");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("E1");
	velocity_data->node[index].velocity[0] = 24;
	velocity_data->node[index].updates[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("MR9");
	velocity_data->node[index].src = index;
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 100;
	velocity_data->node[index].updates[0] = 1;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 120;
	velocity_data->node[index].updates[1] = 1;
	velocity_data->node[index].num_velocity = 2;
}
 
int velocity_lookup(int src, int dest, Velocity_data *velocity_data)
{
	/*if (!is_found) {*/
        /*bwprintf(COM2, "not found velocity%d\r\n", new_velocity);*/
		/*int idx = velocity_data->node[src].num_velocity;*/
		/*velocity_data->node[src].dest[idx] = dest;*/
		/*velocity_data->node[src].velocity[idx] = new_velocity;*/
		/*velocity_data->node[src].num_velocity++;*/
	/*}*/
	int i;
    /*bwprintf(COM2, "src%d dest%d\r\n", src, dest);*/
    /*bwprintf(COM2, "num_velocity%d\r\n",velocity_data->node[src].num_velocity);*/
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
    /*bwprintf(COM2, "!not found\r\n");*/
	return -1;
}

void velocity_update(int src, int dest, int new_velocity, Velocity_data *velocity_data)
{
    if (new_velocity > 600){
        // indicates the trains once stopped 
        // currently only take into consideration speed as low as 6 
        // probably need a smarter way to determine this in the future
        return;
    }

	int is_found = 0;
	int dest_idx = -1;

	int i;
	for (i = 0; i < velocity_data->node[src].num_velocity; i++) {
		if (velocity_data->node[src].dest[i] == dest) {
			is_found = 1;
			dest_idx = i;
		}
	}

	if (!is_found) {
        /*bwprintf(COM2, "src=%d dest=%d\r\n", src, dest);*/
		int idx = velocity_data->node[src].num_velocity;
		velocity_data->node[src].dest[idx] = dest;
		velocity_data->node[src].velocity[idx] = new_velocity;
		velocity_data->node[src].num_velocity++;
	}
	else {
        /*bwprintf(COM2, "is found velocity%d\r\n", new_velocity);*/
		int hit = velocity_data->node[src].updates[dest_idx];
		velocity_data->node[src].updates[dest_idx]++;

		int old_velocity = velocity_data->node[src].velocity[dest_idx];
		if (new_velocity != old_velocity) {
			if (((new_velocity + hit * old_velocity) % (hit + 1)) >= ((hit + 1) / 2)) {
				velocity_data->node[src].velocity[dest_idx] = 1 + (new_velocity + hit * old_velocity) / (hit + 1);
			}
			else {
				velocity_data->node[src].velocity[dest_idx] = (new_velocity + hit * old_velocity) / (hit + 1);
			}
		}
	}
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
			 !strcmp(command_buffer->data, "br", 2) || !strcmp(command_buffer->data, "park", 4)) {
		// parse arguments
		int pos = !strcmp(command_buffer->data, "park", 4) ? 4 : 2;
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
                /*debug(SUBMISSION, "alpha data=%c\r\n", args[temp]);*/
			}
			else if (command_buffer->data[pos] == ' ' || (pos == command_buffer->pos)) {
				// skip space
				if (num_buffer_pos != 0) {
					// at the end of a number
                    int temp = argc;
					args[argc++] = atoi(num_buffer);
                    /*debug(SUBMISSION, "converted value=%d\r\n", args[temp]);*/
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
    /*bwprintf(COM2, "args%s\r\n", args);*/
    /*bwprintf(COM2, "argc num = %d\r\n", argc);*/
    
    // currently no commands need more than two arguments
    // there will probably won't be in the future as well
    if(argc > 2){
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
        if (argc != 2){
            return -1;
        }
        if(args[1] > 16){
            return -1;
        }
		pcmd->type = PARK;
		break;
	}
	pcmd->arg0 = args[0];
	pcmd->arg1 = (pcmd->type == RV) ? ptrain->speed : args[1];

	return 0;
}

void command_handle(Command *pcmd)
{
	debug(DEBUG_K4, "enter %s", "command_handle");

	// pcmd->type get defined at train.h
	switch(pcmd->type) {
	case TR:
        // add 16 here to turn on the headlight of the train
		irq_printf(COM1, "%c%c", pcmd->arg1+16, pcmd->arg0);
		break;
	case RV:
		irq_printf(COM1, "%c%c", MIN_SPEED, pcmd->arg0);

		Delay(20);
		debug(DEBUG_K4, "%s", "reached time limit, begin reverse");
		irq_printf(COM1, "%c%c", REVERSE, pcmd->arg0);

		Delay(20);
		debug(DEBUG_K4, "%s", "reached time limit, begin set speed");
		irq_printf(COM1, "%c%c", pcmd->arg1+16, pcmd->arg0);

		break;
	case SW:
		irq_printf(COM1, "%c%c", switch_state_to_byte(pcmd->arg1), switch_id_to_byte(pcmd->arg0));
        // track switches status
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
