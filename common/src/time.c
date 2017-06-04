#include <ts7200.h>
#include <define.h>
#include <time.h>

uint64 timer4_read()
{
	vint *timer4_low = (vint *) TIMER4_LOW;
	vint *timer4_high = (vint *) TIMER4_HIGH;
	uint64 timer4 = ((uint64) *timer4_high << 32) | ((uint64) *timer4_low);
	return timer4;
}

void timer4_start()
{
	vint *timer4_high = (vint *) TIMER4_HIGH;
	*timer4_high |= TIMER4_ENABLE;
}

void timer4_stop()
{
	vint *timer4_high = (vint *) TIMER4_HIGH;
	*timer4_high &= ~TIMER4_ENABLE;
}
