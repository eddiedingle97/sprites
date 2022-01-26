#ifndef __EMATH_H__
#define __EMATH_H__

float math_fast_inverse_sqrt(float no);
int math_round(float f);
int math_floor(float f);
int math_ceil(float f);
int math_in_range(float small, float x, float large);
float math_abs(float f);
void math_seed(unsigned int seed);
unsigned int math_get_random(int max);
float math_cos_d(int angle);
float math_sin_d(int angle);
void math_mergesort(void *list, int size, int (*comp)(void *, void *), int datasize);

#endif