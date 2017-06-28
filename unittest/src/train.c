#include <debug.h>
#include <train.h>
#include <string.h>
#include <irq_io.h>

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
		sensor.group = group_buf[0] - 'A';
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
 
void velocity14_initialization(Velocity_data *velocity_data)
{
	int i;
	for (i = 0; i < TRACK_MAX; i++) {
		velocity_data->node[i].src = i;
		velocity_data->node[i].updates = 1;
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

void velocity_update(int src, int dest, int new_velocity, Velocity_data *velocity_data)
{
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
		int idx = velocity_data->node[src].num_velocity;
		velocity_data->node[src].dest[idx] = dest;
		velocity_data->node[src].velocity[idx] = new_velocity;
		velocity_data->node[src].num_velocity++;
	}
	else {
		int hit = velocity_data->node[src].updates;
		velocity_data->node[src].updates++;

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
