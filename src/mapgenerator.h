#ifndef __MAPGENERATOR_H__
#define __MAPGENERATOR_H__
#include "map.h"

struct map *mg_create_map(int w, int h);
struct map *mg_create_island_map(int w, int h);

struct room
{
    int x;//of the topleft corner
    int y;//of the topleft corner
    int w;
    int h;
    struct coord *exit;
    int noexits;
    int enemies;
};

struct area
{
    int x1;//topleft
    int y1;
    int x2;//bottomright
    int y2;
};

int mg_room_center_x(struct room *room);
int mg_room_center_y(struct room *room);
struct coord *mg_a_star(struct map *map, struct coord *start, struct coord *end, int *no, unsigned char typemask);
enum CHUNKFLAGS {MG_OCEAN = 1, MG_SHORE = 2, MG_MAINLAND = 4, MG_SMALLISLAND = 8, MG_HABITABLE = 16, MG_HASWATER = 32, MG_CHECKED = 64};

#endif