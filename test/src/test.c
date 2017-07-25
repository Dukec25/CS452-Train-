#include <string.h>
#include <heap.h>
#include <bwio.h>
#include <debug.h>
#include <define.h>
#include <lifo.h>
#include <track_data.h>
#include <track_server.h>
#include <train.h>
#include <calculation.h>
#include <define.h>
static uint32 choice_seed = 0;

#define test_assert_str(cond, msg)		 										 			\
		do {																			\
			if (!cond)																	\
				bwprintf(COM2, "%s:%d %s\r\n", __FILE__, __LINE__, msg);				\
			}																			\
		while (0)

#define test_assert(cond, fmt, ...) 										 					\
		do {																			\
			if (!cond)																	\
				bwprintf(COM2, "%s:%d " fmt "\r\n", __FILE__, __LINE__, __VA_ARGS__);	\
			}																			\
		while (0)

void string_test()
{
	int i;

	// atoi
	test_assert_str(1 == atoi("1"), "atoi test1 failed"); 
	test_assert_str(123456 == atoi("123456"), "atoi test2 failed");
	test_assert_str(7890 == atoi("7890"), "atoi test2 failed");
	debug(DEBUG_INFO, "%s test passed", "atoi");

	// toupper
	test_assert_str('A' == toupper('a'), "toupper test1 failed");
	test_assert_str('S' == toupper('s'), "toupper test2 failed");
	test_assert_str('Z' == toupper('z'), "toupper test3 failed");
	test_assert_str('A' == toupper('A'), "toupper test4 failed");
	test_assert_str('S' == toupper('S'), "toupper test5 failed");
	test_assert_str('Z' == toupper('Z'), "toupper test6 failed");
	debug(DEBUG_INFO, "%s test passed", "toupper");

	// strcmp
	test_assert_str(strcmp("abcABC", "abcABC", 6) == 0, "strcmp test1 failed");
	test_assert_str(strcmp("abcABC", "abcABD", 6) == 1, "strcmp test2 failed");
	test_assert_str(!strcmp("abcABB", "abcABC", 6) == -1, "strcmp test3 failed");
	test_assert_str(!strcmp("abcABCDDDDD", "abcABCCCCCC", 6), "strcmp test4 failed");
	debug(DEBUG_INFO, "%s test passed", "strcmp");
}

void Memcpy_test()
{
	// Memcpy
	const size_t source_num = 70;
	char source[source_num + 1];
	int Memcpy_idx;
	for(Memcpy_idx = 0; Memcpy_idx < source_num; Memcpy_idx++) {
		source[Memcpy_idx] = 'a' + (Memcpy_idx % 26);
	}
	source[source_num] = '\0';
	debug(DEBUG_TRACE, "Memcpy source = %s, source_num = %d", source, source_num); 
	// char Memcpy
	const size_t dest1_num = 1; // test char Memcpy
	char dest1[dest1_num + 1];
	Memcpy(dest1, source, dest1_num);
	dest1[dest1_num] = '\0';
	test_assert(!strcmp(source, dest1, dest1_num), "Memcpy test1 failed, dest = %s", dest1);
	// short Memcpy
	const size_t dest2_num = 2; // test short Memcpy
	char dest2[dest2_num + 1];
	Memcpy(dest2, source, dest2_num);
	dest2[dest2_num] = '\0';
	test_assert(!strcmp(source, dest2, dest2_num), "Memcpy test2 failed, dest = %s", dest2);
	// int Memcpy
	const size_t dest3_num = 4;
	char dest3[dest3_num + 1];
	Memcpy(dest3, source, dest3_num);
	dest3[dest3_num] = '\0';
	test_assert(!strcmp(source, dest3, dest3_num), "Memcpy test3 failed, dest = %s", dest3);
	// long long Memcpy
	const size_t dest4_num = 8;
	char dest4[dest4_num + 1];
	Memcpy(dest4, source, dest4_num);
	dest4[dest4_num] = '\0';
	test_assert(!strcmp(source, dest4, dest4_num), "Memcpy test4 failed, dest = %s", dest4);
	// long long aligned Memcpy
	const size_t dest5_num = 8 * 5;
	char dest5[dest5_num + 1];
	Memcpy(dest5, source, dest5_num);
	dest5[dest5_num] = '\0';
	test_assert(!strcmp(source, dest5, dest5_num), "Memcpy test5 failed, dest = %s", dest5);
	// int aligned Memcpy
	const size_t dest6_num = 8 * 5 + 4;
	char dest6[dest6_num + 1];
	Memcpy(dest6, source, dest6_num);
	dest6[dest6_num] = '\0';
	test_assert(!strcmp(source, dest6, dest6_num), "Memcpy test6 failed, dest = %s", dest6);
	// short aligned Memcpy
	const size_t dest7_num = 8 * 5 + 4 + 2;
	char dest7[dest7_num + 1];
	Memcpy(dest7, source, dest7_num);
	dest7[dest7_num] = '\0';
	test_assert(!strcmp(source, dest7, dest7_num), "Memcpy test7 failed, dest = %s", dest7);
	// char aligned Memcpy
	const size_t dest8_num = 8 * 5 + 4 + 2 + 1;
	char dest8[dest8_num + 1];
	Memcpy(dest8, source, dest8_num);
	dest8[dest8_num] = '\0';
	test_assert(!strcmp(source, dest8, dest8_num), "Memcpy test8 failed, dest = %s", dest8);
	// null terminator Memcpy
	const size_t dest9_num = source_num + 1;
	char dest9[dest9_num];
	Memcpy(dest9, source, dest9_num);
	test_assert(!strcmp(source, dest9, dest9_num) && (dest9[dest9_num - 1] == '\0'), "Memcpy test9 failed, dest = %s", dest9);
	debug(DEBUG_INFO, "%s test passed", "Memcpy");
}

void lifo_test()
{
    Lifo_t stack; 
    lifo_init(&stack);

	int i;
    for(i = 0; i < 10; i++){
        lifo_push(&stack, (void *)i);
    }
    while(!is_lifo_empty(&stack)){
        int a;
        lifo_pop(&stack, &a);
        test_assert((100 == a), "fifo test failed, int = %d", a);
    }
}

void heap_test()
{
	// heap test
	const size_t heap_size = 12;
	node_t data[heap_size];
	heap_t h = heap_init(data, heap_size);
	test_assert_str(0 == heap_insert(&h, 1, "test1"), "heap insert test 1 failed");
	test_assert_str(0 == heap_insert(&h, 8, "test8"), "heap insert test 8 failed");
	test_assert_str(0 == heap_insert(&h, 2, "test2"), "heap insert test 2 failed");
	test_assert_str(0 == heap_insert(&h, 4, "test4"), "heap insert test 4 failed");
	test_assert_str(0 == heap_insert(&h, 9, "test9dup"), "heap insert test 9dup failed");
	test_assert_str(0 == heap_insert(&h, 6, "test6"), "heap insert test 6 failed");
	test_assert_str(0 == heap_insert(&h, 3, "test3dup"), "heap insert test 3dup failed");
	test_assert_str(0 == heap_insert(&h, 5, "test5"), "heap insert test 5 failed");
	test_assert_str(0 == heap_insert(&h, 9, "test9"), "heap insert test 9 failed");
	test_assert_str(0 == heap_insert(&h, 3, "test3"), "heap insert test 3 failed");
	test_assert_str(0 == heap_insert(&h, 7, "test7"), "heap insert test 7 failed");

    node_t root, del;
    root = heap_root(&h);
    while(root.priority <= 100)
    {
        int isEmpty = heap_delete(&h, &del);
        debug(DEBUG_INFO, "heap is currently like idx=%d", del.priority);
        if(is_heap_empty(&h)){
            break;
        }
        root = heap_root(&h);
    }

	// heap duplicate priority fifo test
	root = heap_root(&h);
	test_assert(strcmp("test1", root.data, 6), "heap root duplicate test 9 failed, root.data = %s", root.data);
	test_assert(0 == heap_delete(&h, &del), "heap delete duplicate test 9(A) failed, del.data = %s", del.data); 
	test_assert(strcmp("test1", del.data, 6), "heap delete duplicate test 9(B) failed, del.data = %s", del.data); 
	bwputstr(COM2, "\r\n");

	int heap_idx;
	char test[6];
	test[0] = 'q'; test[1] = 'a'; test[2] = 's'; test[3] = 't';
	for (heap_idx = 11; heap_idx >= 2; heap_idx--) {
		test[4] = c2x(heap_idx);
		test[5] = '\0';
		root = heap_root(&h);
		test_assert(!strcmp(test, root.data, 6), "heap root test %d failed, root.data = %s", heap_idx, root.data);
		test_assert(0 == heap_delete(&h, &del), "heap delete test %d(A) failed", heap_idx); 
		test_assert(!strcmp(test, del.data, 6), "heap delete test %d(B) failed, del.data = %s", heap_idx, del.data); 
	}
	root = heap_root(&h);
	test_assert_str(!strcmp("test3dup", root.data, 8), "heap root duplicate test 3 failed");
	test_assert_str(!heap_delete(&h, &del), "heap delete duplicate test 3(A) failed"); 
	test_assert_str(!strcmp("test3dup", del.data, 8), "heap delete duplicate test 3(B) failed"); 

	for (heap_idx = 3; heap_idx >= 1; heap_idx--) {
		char test[5] = "test";
		test[5] = c2x(heap_idx);
		root = heap_root(&h);
		test_assert(!strcmp(test, root.data, 5), "heap root test %d failed", heap_idx);
		test_assert(!heap_delete(&h, &del), "heap delete test %d(A) failed", heap_idx); 
		test_assert(!strcmp(test, del.data, 5), "heap delete test %d(B) failed", heap_idx); 
	}
}

void rand_test(){
    int n = 0;
    for( ; n < 10; n++){
        int choice = abs(rand(&choice_seed)%100);
        test_assert( 0, "rand value = %d", choice);
    }
}

void calculation_test(){
    int stop = choose_rand_destination();
    debug(SUBMISSION, "choosed stop %d\r\n", stop);
    int resource_available[144]; 
    Train_server train_server;
    init_tracka(train_server.track);
    Br_lifo br_lifo_struct;
    br_lifo_struct.br_lifo_top = -1;
    int i = 0;
    for( ;i < 144; i++){
        resource_available[i] = 1;
    }
    /*resource_available[19] = 0;*/
    int sw;
	for (sw = 1; sw <= 22 ; sw++) {
		// be careful that if switch initialize sequence changes within initialize_switch(), here need to change 
		train_server.switches_status[sw-1] = switch_state_to_byte((sw == 16 || sw == 10 || sw == 19 || sw == 21) ? 'S' : 'C');
	}
    // c10 and e2
    /*track_node *node = find_path_with_blocks(train_server.track, 41, 65, resource_available);*/
    // c10 and a6
    track_node *node = find_path_with_blocks(train_server.track, 41, 5, resource_available);
    track_node *temp_node = node;
    while(temp_node->num != 40 && temp_node->num != 41) {
        debug(SUBMISSION, "%s", temp_node->name);
        temp_node = temp_node->previous;
    }
    /*debug(SUBMISSION, "%s", temp_node->name);*/
    /*int num_switch = switches_need_changes(41, node, &train_server, &br_lifo_struct);*/
    /*debug(SUBMISSION, "num_switch %d", num_switch);*/

    Train train;
    train.last_stop = 41;
    TS_request ts_request;
    put_cmd_fifo(train_server.track, 5, resource_available, node, &train, &ts_request);

    Track_cmd_fifo_struct *cmd_struct = &ts_request.track_result.cmd_fifo_struct;
    while(cmd_struct->track_cmd_fifo_head != cmd_struct->track_cmd_fifo_tail){
        Track_cmd track_cmd;
        pop_track_cmd_fifo(cmd_struct, &track_cmd);
        debug(SUBMISSION, "cmd_type is %d", track_cmd.type);
    }
    

    /*Train_br_switch temp;*/
    /*while(br_lifo_struct.br_lifo_top != -1){*/
        /*peek_br_lifo(&br_lifo_struct, &temp);*/
        /*pop_br_lifo(&br_lifo_struct);*/
        /*debug(SUBMISSION, "sensor_stop %d, id %d, state %c", temp.sensor_stop, temp.id, temp.state);*/
    /*}  */

    /*Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];*/
    /*int num_park_stops = find_stops_by_distance(train_server.track, 41, 65, 935, park_stops, resource_available);*/

    /*if(num_park_stops == -1){*/
        /*return; // there is error, ignore this iteration*/
    /*}*/

    /*// retrieve the sensor_to_deaccelate_train*/
    /*int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; */
    /*// calculate the delta = the distance between sensor_to_deaccelate_train*/
    /*// calculate average velocity measured in [tick]*/
    /*int delta = 0;*/

    /*int i = 0;*/

    /*for ( ; i < num_park_stops; i++) {*/
        /*delta += park_stops[i].distance;*/
    /*}*/

    /*int park_delay_time = (delta - stopping_distance * 1000) / 1000;*/
}

int main()
{
	bwsetfifo(COM2, OFF);
	/*track_test();*/
    calculation_test();
	return 0;
}
