#include <stdio.h>
#include <allegro5/allegro.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "../list/list.h"
#include "dungeon.h"
#define PI 3.1415926535

const int pixels_per_c = 40;

struct dungeon *create_dungeon(int level, gsl_rng *r)
{
	struct dungeon *d = malloc(sizeof(struct dungeon));
	d->tilemaps = list_create();
	int z = 1 + gsl_rng_uniform_int(r, level + 3);
	d->amtlevels = (char)z;

	int i, floor;
	for(floor = 0; floor < d->amtlevels; floor++)
	{
		struct list *lareas = list_create();
		int upperbound = 51 + level * level * 4;
		int amtareas = 30 + gsl_rng_uniform_int(r, upperbound);

		int lx = 0, sx = 0, ly = 0, sy = 0;//largest x, smallest x, etc.

		for(i = 0; i < amtareas; i++)
		{
			struct area *a = malloc(sizeof(struct area));
			list_append(lareas, a);

			double zscore = abs(gsl_ran_gaussian(r, 1));
			
			/*
			 * loop until a suitable zscore is found, this will determine a distance, as well as set flags for
			 * interesting or rare things to appear in a given room
			*/
			int done = 0;
			while(!done)
			{
				if(zscore >= 3.0)
				{
					a->flags.spooky = 1;
					zscore = abs(gsl_ran_gaussian(r, 1));
				}

				else if(zscore >= 2.0)
				{
					zscore = abs(gsl_ran_gaussian(r, 1));
				}

				else if(zscore >= 1.15)
				{
					a->flags.spawn = 1;
					a->distance = zscore * RING / 1.15;
					done = 1;
				}

				else
				{
					a->distance = zscore * RING / 1.15;
					done = 1;
				}
			}

			a->angle = gsl_rng_uniform(r) * 2 * PI;//assign random angle

			a->x = cos(a->angle) * a->distance * RING;//assign x coordinate
			a->y = sin(a->angle) * a->distance * RING;//assign y coordinate
			
			
			//record smallest and largest x's and y's, so we know the total map size to create	
			if(a->x + a->width > lx)
				lx = a->x;
			if(a->x < sx)
				sx = a->x;
			if(a->y > ly)
				ly = a->y;
			if(a->y - a->height < sy)
				sy = a->y;

			a->width = gsl_rng_uniform_int(r, 21) + 30;
			a->height = gsl_rng_uniform_int(r, 21) + 30;
			
			a->tiles = malloc(a->width * a->height);//create tile map
			_shape(a, r);
			
		}
		
		int mapsize = (lx - sx) * (ly - sy);

		unsigned char *map = malloc(mapsize);
		
		fill_shape(map, mapsize, WALL);
		
		struct tilemap *t = malloc(sizeof(struct tilemap));
		t->x = (lx - sx);
		t->y = (ly - sy);
		t->tiles = map;

		list_append(d->tilemaps, (void *)t);

		for(i = 0; i < amtareas; i++)
		{
			struct dungeon *a = list_get(lareas, i);
			
			

			free(a);
		}
	}

	return d;
}

struct dungeon *destroy_dungeon(struct dungeon *d)
{
	return d;
}

int explode_area(struct dungeon *d, struct coord *c, int strength)
{
	return 0;
}

int is_filled(struct area *a, struct tilemap *t)
{
	

	return 0;
}

unsigned char get_coord(struct tilemap *t, int x, int y)
{
	x += t->x >> 1;
	y -= t->y >> 1;

	if(y > t->y || y < 0)
		return 0xFF;

	if(x > t->x || x < 0)
		return 0xFF;

	return t->tiles[y * t->x + x];
}

void set_coord(struct tilemap *t, int x, int y, unsigned char tile)
{
	x += t->x >> 1;
	y -= t->y >> 1;

	if(y > t->y || y < 0)
		return;

	if(x > t->x || x < 0)
		return;

	t->tiles[y * t->x + x] = tile;
}