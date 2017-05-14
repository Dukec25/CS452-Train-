#include <bwio.h>

void first( ) {
	bwprintf(COM2, "first.c: initializing\n\r");
	while(1) {
	bwprintf(COM2, "first.c: good-bye\n\r");
	bwprintf(COM2, "first.c: hello\n\r");
	}
}

