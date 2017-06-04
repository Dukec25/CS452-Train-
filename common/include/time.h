#ifndef __TIME_H__
#define __TIME_H__

#include <define.h>

#define TIMER4_MAX			0x7FFFFFFFFF
#define	TIMER4_REQUENCY		983000
#define TIMER4_LOW			0x80810060
#define TIMER4_HIGH			0x80810064
#define TIMER4_ENABLE		(0x1 << 8) 

/*
 * Read the count up timer4 value
 */
uint64 timer4_read();

/*
 * Start the count up timer with TIMER4_MAX as the preload value
 */
void timer4_start();

/*
 * Stop the count up timer4
 */
void timer4_stop();

#endif // __TIME_H__
