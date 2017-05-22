#include <debug.h>

void debug_asm(uint32 x)
{
	if (DEBUG <= DEBUG_ASM) {
		bwprintf(COM2, "\r\n0x%x", x);
	}
}
