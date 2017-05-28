#ifndef __TIME_H__
#define __TIME_H__

#include <define.h>

#define CLOCK_FREQ 	508000
#define TIMER_MAX	0xFFFFFFFF

typedef struct {
	uint32 prev_tenth_sec;	/* last tenth_sec */
	uint32 tenth_sec;	/* elapsed time measured in tenth of seconds */
	uint32 sec;		/* elapsed time measured in seconds */
	uint32 min;		/* elapsed time measured in minutes */
} Time;

/*
 * Returns pointer to the count down timer value
 */
vint *timer();

/*
 * Start the count down timer with TIMER_MAX as the preload value
 */
void timer_start();

/*
 * Stop the count down timer
 */
void timer_stop();

/*
 * Return an initialized time
 */
Time time_init();

/*
 * Update the time pt every tenth of second.
 * Return 1 if pt is updated, 0 otherwise.
 */
int time_update(Time *pt);

#endif // __TIME_H__
