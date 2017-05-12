#include <string.h>
#include <heap.h>
#include <io.h>

int main()
{
	// atoi
	assert(1 == atoi("1"), "atoi test1 failed\n"); 
	assert(123456 == atoi("123456"), "atoi test2 failed\n");
	assert(7890 == atoi("7890"), "atoi test2 failed\n");

	// toupper
	assert('A' == toupper('a'), "toupper test1 failed\n");
	assert('S' == toupper('s'), "toupper test2 failed\n");
	assert('Z' == toupper('z'), "toupper test3 failed\n");
	assert('A' == toupper('A'), "toupper test4 failed\n");
	assert('S' == toupper('S'), "toupper test5 failed\n");
	assert('Z' == toupper('Z'), "toupper test6 failed\n");

	// strcmp
	assert(strcmp("abcABC", "abcABC", 6) == 0, "strcmp test1 failed\n");
	assert(strcmp("abcABC", "abcABD", 6) == 1, "strcmp test2 failed\n");
	assert(!strcmp("abcABB", "abcABC", 6) == -1, "strcmp test3 failed\n");
	assert(!strcmp("abcABCDDDDD", "abcABCCCCCC", 6), "strcmp test4 failed\n");

	// memcpy
	const size_t source_num = 300;
	char source[source_num];
	int memcpy_idx;
	for(memcpy_idx = 0; memcpy_idx < source_num; memcpy_idx++) {
		source[memcpy_idx] = memcpy_idx % 128;
	}
	const size_t dest1_num = 64 * 4;
	char dest1[dest1_num];
	assert(!strcmp(source, memcpy(dest1, source, dest1_num), dest1_num), "memcpy test1 failed\n");
	const size_t dest2_num = 64 * 4 + 32;
	char dest2[dest2_num];
	assert(!strcmp(source, memcpy(dest2, source, dest2_num), dest2_num), "memcpy test2 failed\n");
	const size_t dest3_num = 64 * 4 + 32 + 16;
	char dest3[dest3_num];
	assert(!strcmp(source, memcpy(dest3, source, dest3_num), dest3_num), "memcpy test3 failed\n");
	const size_t dest4_num = 64 * 4 + 32 + 16 + 8;
	char dest4[dest4_num];
	assert(!strcmp(source, memcpy(dest4, source, dest4_num), dest4_num), "memcpy test4 failed\n");

	// heap test
	const size_t heap_size = 12;
	node_t data[heap_size];
	heap_t h = heap_init(data, heap_size);
	assert(0 == heap_insert(&h, 1, "test1"), "heap insert test 1 failed");
	assert(0 == heap_insert(&h, 8, "test8"), "heap insert test 8 failed");
	assert(0 == heap_insert(&h, 2, "test2"), "heap insert test 2 failed");
	assert(0 == heap_insert(&h, 7, "test7"), "heap insert test 7 failed");
	assert(0 == heap_insert(&h, 4, "test4"), "heap insert test 4 failed");
	assert(0 == heap_insert(&h, 9, "test9dup"), "heap insert test 9dup failed");
	assert(0 == heap_insert(&h, 6, "test6"), "heap insert test 6 failed");
	assert(0 == heap_insert(&h, 3, "test3dup"), "heap insert test 3dup failed");
	assert(0 == heap_insert(&h, 5, "test5"), "heap insert test 5 failed");
	assert(0 == heap_insert(&h, 9, "test9"), "heap insert test 9 failed");
	assert(0 == heap_insert(&h, 3, "test3"), "heap insert test 3 failed");

	node_t root, del;

	root = heap_root(&h);
	assert(!strcmp("test9dup", root.data, 8), "heap root duplicate test 9 failed\n");
	assert(!heap_delete(&h, &del), "heap delete duplicate test 9(A) failed\n"); 
	assert(!strcmp("test9dup", del.data, 8), "heap delete duplicate test 9(B) failed\n"); 

	int heap_idx;
	for (heap_idx = 9; heap_idx >= 4; heap_idx--) {
		char test[5] = "test";
		test[5] = c2x(heap_idx);
		root = heap_root(&h);
		assert(!strcmp(test, root.data, 5), "heap root test %d failed\n", heap_idx);
		assert(!heap_delete(&h, &del), "heap delete test %d(A) failed\n", heap_idx); 
		assert(!strcmp(test, del.data, 5), "heap delete test %d(B) failed\n", heap_idx); 
	}

	root = heap_root(&h);
	assert(!strcmp("test3dup", root.data, 8), "heap root duplicate test 3 failed\n");
	assert(!heap_delete(&h, &del), "heap delete duplicate test 3(A) failed\n"); 
	assert(!strcmp("test3dup", del.data, 8), "heap delete duplicate test 3(B) failed\n"); 

	for (heap_idx = 3; heap_idx >= 1; heap_idx--) {
		char test[5] = "test";
		test[5] = c2x(heap_idx);
		root = heap_root(&h);
		assert(!strcmp(test, root.data, 5), "heap root test %d failed\n", heap_idx);
		assert(!heap_delete(&h, &del), "heap delete test %d(A) failed\n", heap_idx); 
		assert(!strcmp(test, del.data, 5), "heap delete test %d(B) failed\n", heap_idx); 
	}
	return 0;
}
