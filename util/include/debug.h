#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <bwio.h>
#include <define.h>

typedef enum debug_level
{
	DEBUG_ALL = 0,
	DEBUG_TRACE = 1,
	DEBUG_ASM = 2,
	DEBUG_SCHEDULER = 3,
	DEBUG_SYSCALL = 4,
	DEBUG_TASK = 5
} debug_level;

/* debug */
#define debug(level, fmt, ...) 												 		\
		do {																		\
			if (level >= DEBUG)														\
				bwprintf(COM2, "\r\n%s:%d " fmt, __FILE__, __LINE__, __VA_ARGS__);	\
			}																		\
		while (0)

void debug_asm(uint32 x);

#endif // __DEBUG_H__
