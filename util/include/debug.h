#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <bwio.h>
#include <define.h>

typedef enum debug_level
{
	DEBUG_ALL,
	DEBUG_PRIOR_FIFO,
	DEBUG_ASM,
	DEBUG_TRACE,
	DEBUG_SYSCALL,
	DEBUG_IRQ,
	DEBUG_TASK,
    DEBUG_SERVER,
    DEBUG_TIME,
    SUBMISSION,
} debug_level;

/* debug */
#define debug(level, fmt, ...) 												 			\
		do {																			\
			if (level >= DEBUG && level < SUBMISSION)									\
				bwprintf(COM2, "%s:%d " fmt "\r\n", __FILE__, __LINE__, __VA_ARGS__);	\
			else if (level >= SUBMISSION)												\
				bwprintf(COM2, fmt "\r\n", __VA_ARGS__);								\
			}																			\
		while (0)

void debug_asm(uint32 x);

#endif // __DEBUG_H__
