#include <math.h>
#include <bwio.h>
#include <debug.h>

uint8 popcount(uint32 x) {
   x = x - ((x >> 1) & 0x55555555);
   x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
   x = (x + (x >> 4)) & 0x0F0F0F0F;
   x = x + (x << 8);
   x = x + (x << 16);
   return x >> 24;
}

uint8 clz(uint32 x)
{
	uint8 lz = 0;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >>16);
	/*debug("in clz, x = 0x%x", x);*/
	/*debug("in clz, ~x = 0x%x", ~x);*/
	lz = popcount(~x);
	/*debug("in clz, lz = %d", lz);*/
	return lz;
}

uint32 rand(uint32 state[static 1])
{
	uint32 x = state[0];
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	state[0] = x;
	return x >= x ? x : -x;
}

int sqrt(double n) {
	double x = n;
	double y = 1.;
	double e = 0.001; /* e decides the accuracy level*/
	double diff = (x > y) ? (x - y) : (y - x);
	while (diff > e) {
		x = (x + y)/2;
		y = n/x;
		diff = (x > y) ? (x - y) : (y - x);
	}
	int retval = (int) x;
	return retval;
}
