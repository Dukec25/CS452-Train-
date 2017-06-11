#ifndef __CLOCK_H__
#define __CLOCK_H__

typedef struct Clock {
	int tenth_sec;	/* total elapsed time in terms of tenth_sec */
	int sec;			/* total elapsed time in terms of sec */
	int min;			/* total elapsed time in terms of min */
} Clock;

void clock_init(Clock *pclock);
void clock_update(Clock *pclock, int elapsed_tenth_sec);

#endif // __CLOCK_H__
