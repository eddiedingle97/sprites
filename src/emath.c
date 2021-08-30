#include "emath.h"

float math_fast_inverse_sqrt(float no)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = no * .5F;
    y = no;
    i = *(long *) &y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *) &i;
    y = y * (threehalfs - (x2 * y * y));

    return y;
}

int math_round(float f)
{
	int out = (int)f;
	float cmp = f - out;
	if(cmp < .5)
		return out;
	else
		return ++out;
}

int math_floor(float f)
{
	return (int)f;
}

float math_abs(float f)
{
	if(f < 0)
        return -f;

    else 
        return f;
}