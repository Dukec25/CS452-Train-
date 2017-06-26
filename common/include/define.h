#ifndef __DEFINE_H__
#define __DEFINE_H__

typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

/* typedefs */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef volatile int vint;
typedef volatile char vchar;
typedef unsigned int size_t;

/* defines */
#define COM1	1
#define COM2	2
#define ON		1
#define	OFF		0
#define NULL	0

/* debug */
#define DEBUG	13
#define LOG		13

#endif //__DEFINE_H__
