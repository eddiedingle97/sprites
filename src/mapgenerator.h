#ifndef __MAPGENERATOR_H__
#define __MAPGENERATOR_H__
#include "map.h"

struct map *mg_create_map(int w, int h);
struct map *mg_create_island_map(int w, int h);

struct coord
{
    int x;
    int y;
};

struct room
{
    int w;
    int h;
    int x;
    int y;
    struct coord *exit;
    int noexits;
    int enemies;
};

struct area
{
    int x1;
    int y1;
    int x2;
    int y2;
};

int mg_room_center_x(struct room *room);
int mg_room_center_y(struct room *room);

#endif