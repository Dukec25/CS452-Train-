#include <string.h>
#include <heap.h>
#include <bwio.h>
#include <debug.h>
#include <define.h>

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
    for(n=1; n<=h.len; n++){
        debug(DEBUG_INFO, "heap is currently like idx=%d", h.nodes[n].priority);
    }

	node_t root, del;

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
