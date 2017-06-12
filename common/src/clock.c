#include <clock.h>
#include <debug.h>

void clock_init(Clock *pclock)
{
	pclock->tenth_sec = 0;
	pclock->sec = 0;
	pclock->min = 0;
}

void clock_update(Clock *pclock, int elapsed_tenth_sec)
{
	pclock->min = elapsed_tenth_sec / 600;
	pclock->sec = (elapsed_tenth_sec % 600) / 10;
	pclock->tenth_sec = (elapsed_tenth_sec % 600) % 10;
}
