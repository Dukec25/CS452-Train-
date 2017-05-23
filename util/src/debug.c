#include <debug.h>

void debug_asm(uint32 x)
{
	if (DEBUG <= DEBUG_ASM) {
		bwprintf(COM2, "\r\n!!!ASM: 0x%x\r\n", x);
	}
}
