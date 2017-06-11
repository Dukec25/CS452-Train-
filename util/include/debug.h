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
	DEBUG_TASK,
    DEBUG_SERVER,
    DEBUG_TIME,
    DEBUG_CLOCK,
    DEBUG_INFO,
	DEBUG_IRQ,
    DEBUG_UART_IRQ,
    DEBUG_K4,
    SUBMISSION
} debug_level;

/* debug */
#define debug(level, fmt, ...) 												 			\
		do {																			\
			if (level >= DEBUG && level < 100)									\
				bwprintf(COM2, "%s:%d " fmt "\r\n", __FILE__, __LINE__, __VA_ARGS__);	\
			else if (level >= SUBMISSION)												\
				bwprintf(COM2, fmt "\r\n", __VA_ARGS__);								\
			}																			\
		while (0)

void debug_asm(uint32 x);
void assert(int expr, char *message, ...);
#endif // __DEBUG_H__
