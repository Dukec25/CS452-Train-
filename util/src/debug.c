#include <debug.h>

void debug_asm(uint32 x)
{
	if (DEBUG <= DEBUG_ASM) {
		bwprintf(COM1, "\r\n!!!ASM: 0x%x\r\n", x);
	}
}
