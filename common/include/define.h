#ifndef __DEFINE_H__
#define __DEFINE_H__

/* typedefs */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef volatile int vint;
typedef unsigned int size_t;

/* defines */
#define COM1	1
#define COM2	2
#define ON	1
#define	OFF	0
#define NULL	0
#define TASK_SIZE 102400 // 100kb
#define TASK_START_LOCATION 0x10000000 // value to be determined
#define NUM_TASK 64 // assume there will be 64 task in the kernel
/* task priorities */
#define PRIOR_LOWEST    0
#define PRIOR_LOW   1 
#define PRIOR_MEDIUM    2 
#define PRIOR_HIGH  3 

#endif //__DEFINE_H__
