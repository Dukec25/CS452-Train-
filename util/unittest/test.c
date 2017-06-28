#include <string.h>
#include <heap.h>
#include <bwio.h>
#include <debug.h>
#include <define.h>
#include <lifo.h>

/* Velocity */
#define VELOCITY_DATA_LENGTH	80
#define MAX_NUM_VELOCITIES		3
#define SENSOR_GROUPS 5
#define SENSORS_PER_GROUP 16
typedef struct Velocity_node {
	int src;
	int num_velocity;
	int dest[MAX_NUM_VELOCITIES];
	int velocity[MAX_NUM_VELOCITIES];
} Velocity_node;
typedef struct Velocity_data {
	Velocity_node node[144];	// [mm] / [tick] = [mm] / [10ms]  
	int stopping_distance;	// mm
} Velocity_data;

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
		int group = group_buf[0] - 'A';
		int id = id;
		num = group * SENSORS_PER_GROUP + id - 1;
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
	for (i = 0; i < 144; i++) {
		velocity_data->node[i].src = i;
		velocity_data->node[i].num_velocity = 0;
	}
 
	velocity_data->stopping_distance = 940;

	int index;
	index = track_node_name_to_num("A3");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C11");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("A4");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B16");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B1");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D14");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B3");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C2");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B4");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C9");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B6");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B13");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 8;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("B14");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D16");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B15");
	velocity_data->node[index].dest[0] = track_node_name_to_num("A3");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("B16");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C10");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C5");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C1");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B4");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C2");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D2");
	velocity_data->node[index].velocity[0] = 8;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E2");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C5");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D12");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C9");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B15");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C10");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B1");
	velocity_data->node[index].velocity[0] = 2;
	velocity_data->node[index].dest[1] = track_node_name_to_num("B3");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("C11");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E16");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C12");
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("C14");
	velocity_data->node[index].dest[0] = track_node_name_to_num("A4");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D1");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D2");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E4");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D4");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B6");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D5");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E6");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D6");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E10");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D8");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E8");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D10");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("D12");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E11");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D14");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D15");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B13");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("D16");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E14");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E1");
	velocity_data->node[index].dest[0] = track_node_name_to_num("B14");
	velocity_data->node[index].velocity[0] = 5;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C1");
	velocity_data->node[index].velocity[1] = 6;
	velocity_data->node[index].num_velocity = 2;

	index = track_node_name_to_num("E2");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E15");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E3");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E4");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E5");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E5");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D6");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E6");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D1");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].dest[1] = track_node_name_to_num("C4");
	velocity_data->node[index].velocity[1] = 7;
	velocity_data->node[index].dest[2] = track_node_name_to_num("E3");
	velocity_data->node[index].velocity[2] = 7;
	velocity_data->node[index].num_velocity = 3;

	index = track_node_name_to_num("E8");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C14");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E9");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 5;
	velocity_data->node[index].num_velocity = 2;
	
	index = track_node_name_to_num("E10");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E13");
	velocity_data->node[index].velocity[0] = 9;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E11");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D10");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E13");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D15");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E14");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E9");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E15");
	velocity_data->node[index].dest[0] = track_node_name_to_num("C12");
	velocity_data->node[index].velocity[0] = 7;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("E16");
	velocity_data->node[index].dest[0] = track_node_name_to_num("E1");
	velocity_data->node[index].velocity[0] = 10;
	velocity_data->node[index].num_velocity = 1;

	index = track_node_name_to_num("MR9");
	velocity_data->node[index].dest[0] = track_node_name_to_num("D5");
	velocity_data->node[index].velocity[0] = 6;
	velocity_data->node[index].dest[1] = track_node_name_to_num("D8");
	velocity_data->node[index].velocity[1] = 5;
	velocity_data->node[index].num_velocity = 2;
}

#define assert_str(cond, msg)		 										 			\
		do {																			\
			if (!cond)																	\
				bwprintf(COM2, "%s:%d %s\r\n", __FILE__, __LINE__, msg);				\
			}																			\
		while (0)

#define assert(cond, fmt, ...) 										 					\
		do {																			\
			if (!cond)																	\
				bwprintf(COM2, "%s:%d " fmt "\r\n", __FILE__, __LINE__, __VA_ARGS__);	\
			}																			\
		while (0)


int main()
{
	bwsetfifo(COM2, OFF);

	// atoi
	assert_str(1 == atoi("1"), "atoi test1 failed"); 
	assert_str(123456 == atoi("123456"), "atoi test2 failed");
	assert_str(7890 == atoi("7890"), "atoi test2 failed");
	debug(DEBUG_INFO, "%s test passed", "atoi");

	// toupper
	assert_str('A' == toupper('a'), "toupper test1 failed");
	assert_str('S' == toupper('s'), "toupper test2 failed");
	assert_str('Z' == toupper('z'), "toupper test3 failed");
	assert_str('A' == toupper('A'), "toupper test4 failed");
	assert_str('S' == toupper('S'), "toupper test5 failed");
	assert_str('Z' == toupper('Z'), "toupper test6 failed");
	debug(DEBUG_INFO, "%s test passed", "toupper");

	// strcmp
	assert_str(strcmp("abcABC", "abcABC", 6) == 0, "strcmp test1 failed");
	assert_str(strcmp("abcABC", "abcABD", 6) == 1, "strcmp test2 failed");
	assert_str(!strcmp("abcABB", "abcABC", 6) == -1, "strcmp test3 failed");
	assert_str(!strcmp("abcABCDDDDD", "abcABCCCCCC", 6), "strcmp test4 failed");
	debug(DEBUG_INFO, "%s test passed", "strcmp");

	// memcpy
	const size_t source_num = 70;
	char source[source_num + 1];
	int memcpy_idx;
	for(memcpy_idx = 0; memcpy_idx < source_num; memcpy_idx++) {
		source[memcpy_idx] = 'a' + (memcpy_idx % 26);
	}
	source[source_num] = '\0';
	debug(DEBUG_TRACE, "memcpy source = %s, source_num = %d", source, source_num); 
	// char memcpy
	const size_t dest1_num = 1; // test char memcpy
	char dest1[dest1_num + 1];
	memcpy(dest1, source, dest1_num);
	dest1[dest1_num] = '\0';
	assert(!strcmp(source, dest1, dest1_num), "memcpy test1 failed, dest = %s", dest1);
	// short memcpy
	const size_t dest2_num = 2; // test short memcpy
	char dest2[dest2_num + 1];
	memcpy(dest2, source, dest2_num);
	dest2[dest2_num] = '\0';
	assert(!strcmp(source, dest2, dest2_num), "memcpy test2 failed, dest = %s", dest2);
	// int memcpy
	const size_t dest3_num = 4;
	char dest3[dest3_num + 1];
	memcpy(dest3, source, dest3_num);
	dest3[dest3_num] = '\0';
	assert(!strcmp(source, dest3, dest3_num), "memcpy test3 failed, dest = %s", dest3);
	// long long memcpy
	const size_t dest4_num = 8;
	char dest4[dest4_num + 1];
	memcpy(dest4, source, dest4_num);
	dest4[dest4_num] = '\0';
	assert(!strcmp(source, dest4, dest4_num), "memcpy test4 failed, dest = %s", dest4);
	// long long aligned memcpy
	const size_t dest5_num = 8 * 5;
	char dest5[dest5_num + 1];
	memcpy(dest5, source, dest5_num);
	dest5[dest5_num] = '\0';
	assert(!strcmp(source, dest5, dest5_num), "memcpy test5 failed, dest = %s", dest5);
	// int aligned memcpy
	const size_t dest6_num = 8 * 5 + 4;
	char dest6[dest6_num + 1];
	memcpy(dest6, source, dest6_num);
	dest6[dest6_num] = '\0';
	assert(!strcmp(source, dest6, dest6_num), "memcpy test6 failed, dest = %s", dest6);
	// short aligned memcpy
	const size_t dest7_num = 8 * 5 + 4 + 2;
	char dest7[dest7_num + 1];
	memcpy(dest7, source, dest7_num);
	dest7[dest7_num] = '\0';
	assert(!strcmp(source, dest7, dest7_num), "memcpy test7 failed, dest = %s", dest7);
	// char aligned memcpy
	const size_t dest8_num = 8 * 5 + 4 + 2 + 1;
	char dest8[dest8_num + 1];
	memcpy(dest8, source, dest8_num);
	dest8[dest8_num] = '\0';
	assert(!strcmp(source, dest8, dest8_num), "memcpy test8 failed, dest = %s", dest8);
	// null terminator memcpy
	const size_t dest9_num = source_num + 1;
	char dest9[dest9_num];
	memcpy(dest9, source, dest9_num);
	assert(!strcmp(source, dest9, dest9_num) && (dest9[dest9_num - 1] == '\0'), "memcpy test9 failed, dest = %s", dest9);
	debug(DEBUG_INFO, "%s test passed", "memcpy");

    //calculate distance between two sensor
//    track_node tracka[TRACK_MAX];
//    init_tracka(tracka);
//    int b = cal_distance(tracka, 0, 44);
//    assert((100 == b), "cal_distance test failed, int = %d", b);

	// track_node_name_to_num
	int sensor = 0;
	for (sensor = 0; sensor < 80; sensor++) {
		char name[4];
		name[0] = sensor / 16 + 'A';
		int id = sensor % 16 + 1;
		if (id > 9) {
			name[1] = '1';
			name[2] = id % 10 + '0';
			name[3] = '\0';
		}
		else {
			name[1] = id + '0';
			name[2] = '\0';
		}
		assert(0, "%d: %s is %d", sensor, name, track_node_name_to_num(name));
	}
	// velocity initialization
	Velocity_data velocity_data;
	velocity14_initialization(&velocity_data); 
	for (sensor = 0; sensor < 80; sensor++) {
		int i = 0;
		for (i = 0; i < velocity_data.node[sensor].num_velocity; i++) {
			assert(0, "%d->%d: %d",
					velocity_data.node[sensor].src, velocity_data.node[sensor].dest[i], velocity_data.node[sensor].velocity[i]);
		}
	}

    //fifo test 
    Lifo_t stack; 
    lifo_init(&stack);

    int i;
    for(i = 0; i < 10; i++){
        lifo_push(&stack, (void *)i);
    }
    while(!is_lifo_empty(&stack)){
        int a;
        lifo_pop(&stack, &a);
        assert((100 == a), "fifo test failed, int = %d", a);
    }
	// heap test
	const size_t heap_size = 12;
	node_t data[heap_size];
	heap_t h = heap_init(data, heap_size);
	assert_str(0 == heap_insert(&h, 1, "test1"), "heap insert test 1 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 8, "test8"), "heap insert test 8 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 2, "test2"), "heap insert test 2 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 4, "test4"), "heap insert test 4 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 9, "test9dup"), "heap insert test 9dup failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 6, "test6"), "heap insert test 6 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 3, "test3dup"), "heap insert test 3dup failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 5, "test5"), "heap insert test 5 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 9, "test9"), "heap insert test 9 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 3, "test3"), "heap insert test 3 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
	assert_str(0 == heap_insert(&h, 7, "test7"), "heap insert test 7 failed");
	/*heap_print(&h, 0);*/
	/*bwputstr(COM2, "\r\n");*/
    int n;
    /*for(n=1; n<=h.len; n++){*/
        /*debug(DEBUG_INFO, "heap is currently like idx=%d", h.nodes[n].priority);*/
    /*}*/

    node_t root, del;
    root = heap_root(&h);
    while(root.priority <= 100)
    {
        /*debug(SUBMISSION, "about to unblock task tid = %d", tid);*/
        int isEmpty = heap_delete(&h, &del);
        debug(DEBUG_INFO, "heap is currently like idx=%d", del.priority);
        if(is_heap_empty(&h)){
            break;
        }
        root = heap_root(&h);
    }

	// heap duplicate priority fifo test
	root = heap_root(&h);
	assert(strcmp("test1", root.data, 6), "heap root duplicate test 9 failed, root.data = %s", root.data);
	assert(0 == heap_delete(&h, &del), "heap delete duplicate test 9(A) failed, del.data = %s", del.data); 
	assert(strcmp("test1", del.data, 6), "heap delete duplicate test 9(B) failed, del.data = %s", del.data); 
    /*for(n=1; n<=h.len; n++){*/
        /*debug(DEBUG_INFO, "heap is currently like idx=%d", h.nodes[n].priority);*/
    /*}*/
	/*heap_print(&h, 0);*/
	bwputstr(COM2, "\r\n");

	int heap_idx;
	char test[6];
	test[0] = 'q'; test[1] = 'a'; test[2] = 's'; test[3] = 't';
	for (heap_idx = 11; heap_idx >= 2; heap_idx--) {
		test[4] = c2x(heap_idx);
		test[5] = '\0';
		root = heap_root(&h);
		assert(!strcmp(test, root.data, 6), "heap root test %d failed, root.data = %s", heap_idx, root.data);
		assert(0 == heap_delete(&h, &del), "heap delete test %d(A) failed", heap_idx); 
		assert(!strcmp(test, del.data, 6), "heap delete test %d(B) failed, del.data = %s", heap_idx, del.data); 
        /*for(n=1; n<=h.len; n++){*/
            /*debug(DEBUG_INFO, "heap is currently like idx=%d", h.nodes[n].priority);*/
        /*}*/
	}
/*
	root = heap_root(&h);
	assert_str(!strcmp("test3dup", root.data, 8), "heap root duplicate test 3 failed");
	assert_str(!heap_delete(&h, &del), "heap delete duplicate test 3(A) failed"); 
	assert_str(!strcmp("test3dup", del.data, 8), "heap delete duplicate test 3(B) failed"); 

	for (heap_idx = 3; heap_idx >= 1; heap_idx--) {
		char test[5] = "test";
		test[5] = c2x(heap_idx);
		root = heap_root(&h);
		assert(!strcmp(test, root.data, 5), "heap root test %d failed", heap_idx);
		assert(!heap_delete(&h, &del), "heap delete test %d(A) failed", heap_idx); 
		assert(!strcmp(test, del.data, 5), "heap delete test %d(B) failed", heap_idx); 
	}
*/
	return 0;
}
