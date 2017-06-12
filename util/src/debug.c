#include <debug.h>

void debug_asm(uint32 x)
{
	if (DEBUG <= DEBUG_ASM) {
		bwprintf(COM2, "\r\n!!!ASM: 0x%x\r\n", x);
	}
}

/*
void assert(int expr, char *message, ...)
{
	va_list va;
	va_start(va,message);
	if (!expr) {
		bwprintf(COM2, "assertION FAILURE! %s:%d ", __FILE__, __LINE__);
		bwformat(COM2, message, va);
		while(1);
	}
	va_end(va);
}
*/
