#include <stdio.h>
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include "dungeon.h"

void printmap(struct area *a);

int main(int argc, char *argv[])
{
	int seed = 1;
	if(argc > 1)
		seed = atoi(argv[1]);

	struct area a;
	a.height = 30;
	a.width = 30;
	unsigned char map3[30 * 30];
	a.tiles = map3;

	gsl_rng *r = gsl_rng_alloc(gsl_rng_mt19937);
	gsl_rng_set(r, seed);

	_shape(&a, r);

	gsl_rng_free(r);
		
	printmap(&a);

	printf("\n");

	//rotate(&a);

	//printmap(&a);

	return 0;
}

void printmap(struct area *a)
{
	int r, c;
	for(r = 0; r < a->height; r++)
	{
		for(c = 0; c < a->width; c++)
		{
			printf("%c", a->tiles[r * a->width + c] + 48);
		}
		printf("\n");
	}
}








