#include <log.h>
#include <bwio.h>

int main()
{
	int i = 0;
	vint *index = (vint *) DUMP_INDEX;
	int num_char = *index;
	for (i = 0; i < num_char; i++) {
		vchar *data = (vchar *) (DUMP_ADDRESS + i);
		char c = *data; 
		bwputc(COM2, c);
	}
}
