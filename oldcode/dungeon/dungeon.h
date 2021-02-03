#include <gsl/gsl_rng.h>
#ifndef __DUNGEON_H__
#define __DUNGEON_H__

#define RING 12000

struct areaflags{
	unsigned int spawn:1;
	unsigned int warp:4;
	unsigned int exit:3;
	unsigned int mobs:8;
	unsigned int treas:5;
	unsigned int boss:2;
	unsigned int spooky:1;
	unsigned int type:8;
};

struct area{
	unsigned char *tiles;
	int height;
	int width;
	double distance;
	double angle;
	int x;
	int y;
	char z;
	struct areaflags flags;
};

enum tiles{OPEN = 0, WALL = 1, WARP = 2, UWALL = 3, START = 4, SPAWN = 5};

struct coord{
	int x;
	int y;
	char z;
};

struct loot{
	int lootno;
	struct coord c;
};

struct dungeon{
	char amtlevels;
	int seed;
	struct list *bitmaps;
       	struct list *tilemaps;
	struct list *maps;
	struct coord start;
	struct list *loots;
};

struct tilemap{
	unsigned char *tiles;
	int x;
	int y;
};

struct dungeon *create_dungeon(int level, gsl_rng *r);
struct dungeon *destroy_dungeon(struct dungeon *d);
int destroy_area(struct dungeon *d, struct coord c, int strength);
unsigned char get_coord(struct tilemap *t, int x, int y);
void set_coord(struct tilemap *t, int x, int y, unsigned char tile);

//shape.c
void _shape(struct area *a, gsl_rng *r);
void shape_fill(unsigned char *tiles, int size, int type);

#define CIRCLE_WEIGHT 100
#define RECTANGLE_WEIGHT 100
#define L_WEIGHT 100
#define OPEN_WEIGHT 100

#endif
