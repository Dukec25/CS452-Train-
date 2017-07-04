#ifndef __LOG_H__
#define __LOG_H__
#include <define.h>
#include <debug.h>

#define DUMP_INDEX		0x50000 - 4
#define DUMP_ADDRESS	0x50000

int dump_putc(char c);
void dump_putw(int n, char fc, char *bf);
void dump_format(char *fmt, va_list va);
void dump_printf(char *fmt, ...);

#define dump(level, fmt, ...)														\
		do {																		\
			if (level >= LOG)														\
				dump_printf("%s:%d " fmt "\r\n", __FILE__, __LINE__, __VA_ARGS__);	\
			}																		\
		while (0)
#endif // __LOG_H__
