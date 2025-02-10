#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "emath.h"
#include "sprites.h"
#include "time.h"
#include "debug.h"

static void merge(void *list, int mid, int size, int (*comp)(void *, void *), int datasize);

float math_sqrt(float no)
{
    return sqrtf(no);
}

float math_get_distance(float xcomp, float ycomp)
{
    return math_sqrt((xcomp * xcomp) + (ycomp * ycomp));
}

float math_get_inverse_distance(float xcomp, float ycomp)
{
    return 1.0f / math_get_distance(xcomp, ycomp);
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
    int out = (int)f;
    float cmp = f - out;
    if(f >= 0)
	    return out;
    else
        return cmp == 0 ? out : out - 1;
}

int math_ceil(float f)
{
    int out = (int)f;
    float cmp = f - out;
    if(f > 0)
        return cmp == 0 ? out : out + 1;
    else
        return out;
}

inline int math_in_range(float small, float x, float large)
{
    return small < x && x < large;
}

float math_abs(float f)
{
	if(f < 0)
        return -f;
    else 
        return f;
}

void math_seed(unsigned int seed)
{
    if(seed != 0)
        srand(seed);
    else
    {
        time_t t;
        srand((unsigned)time(&t));
        debug_printf("math_seed %ld\n", t);
    }
}

unsigned int math_get_random(int max)
{
    return rand() % (max + 1);
}

unsigned int math_rand()
{
    return rand();
}

float math_atan2(float x, float y)
{
    return atan2f(y, x);
}

float math_cos(float angle)
{
    return cosf(angle);
}

float math_sin(float angle)
{
    return sinf(angle);
}

float math_cos_d(int angle)
{
    return cosf(angle / 180.0f * M_PI);
}

float math_sin_d(int angle)
{
    return sinf(angle / 180.0f * M_PI);
}

float math_arccos(float ratio)//could be done with binary search
{
    return acos(ratio);
}

float math_get_vec_comp(float from, float to)//always have to think real hard about this
{
    return to - from;
}

int math_vec_norm(float *x, float *y)
{
    if(*x == 0 && *y == 0)
        return 0;
    float invdist = math_get_inverse_distance(*x, *y);
    *x *= invdist;
    *y *= invdist;
    return 1;
}

void math_mergesort(void *list, int size, int (*comp)(void *, void *), int datasize)
{
    if(size < 2)
        return;

    char isodd = size & 1;
    int newsize = size / 2;

    math_mergesort(list, newsize, comp, datasize);
    math_mergesort(list + newsize * datasize, isodd ? newsize + 1 : newsize, comp, datasize);

    merge(list, newsize, size, comp, datasize);
}

static void merge(void *list, int mid, int size, int (*comp)(void *, void *), int datasize)
{
    int i = 0, j = mid, ptr = 0;
    void *new = s_malloc(size * datasize, NULL);

    while(i < mid && j < size)
    {
        if(comp(list + i * datasize, list + j * datasize) <= 0)
            memcpy(new + ptr++ * datasize, list + i++ * datasize, datasize);

        else
            memcpy(new + ptr++ * datasize, list + j++ * datasize, datasize);
    }

    if(i == mid)
        while(j < size)
            memcpy(new + ptr++ * datasize, list + j++ * datasize, datasize);

    if(j == size)
        while(i < mid)
            memcpy(new + ptr++ * datasize, list + i++ * datasize, datasize);

    memcpy(list, new, size * datasize);

    s_free(new, NULL);
}