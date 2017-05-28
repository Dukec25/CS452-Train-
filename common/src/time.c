#include <ts7200.h>
#include <define.h>
#include <time.h>

vint *timer()
{
	vint *timer = (vint *) (TIMER3_BASE + VAL_OFFSET);;
	return timer;
}

void timer_start()
{
	vint *load, *control;

	// Load the pre-load value into the 32-bit timer
	load = (vint *) (TIMER3_BASE + LDR_OFFSET);
	*load = TIMER_MAX;
	
	// Enable the 32-bit timer
	control = (vint *) (TIMER3_BASE + CRTL_OFFSET);
	*control |= MODE_MASK;
	*control |= CLKSEL_MASK;
	*control |= ENABLE_MASK; 
}

void timer_stop()
{
	vint *control;

	control = (vint *) (TIMER3_BASE + CRTL_OFFSET);
	*control = 0x0;
}

Time time_init()
{
	Time t;
	t.prev_tenth_sec = 0;
	t.tenth_sec = 0;
	t.sec = 0;
	t.min = 0;
	return t;
}

int time_update(Time *pt)
{
	vint *ptimer = timer();
	uint32 timer_output = TIMER_MAX - *ptimer;

	pt->tenth_sec = timer_output / (CLOCK_FREQ / 10);
	pt->sec = timer_output / CLOCK_FREQ;
	pt->min = pt->sec / 60;
	
	if ( pt->prev_tenth_sec < pt->tenth_sec ) {
		pt->prev_tenth_sec = pt->tenth_sec;
		return 1;
	}
	return 0;
}

int get_microsecond(){
	vint *ptimer = timer();
	uint32 timer_output = TIMER_MAX - *ptimer;
    return timer_output;
}
