#ifndef __TILELIST_H__
#define __TILELIST_H__
#include "map.h"

enum TILES {FLOOR, LEFTWALL, RIGHTWALL, TOPWALL, BOTTOMWALL, WALL, ERROR};
struct tile dungeontiles[] = 
{
    {16, 64, 3, 0, 0},
    {0, 128, 3, SOLID, 0},
    {16, 128, 3, SOLID, 0},
    {64, 160, 3, SOLID, 0},
    {32, 160, 3, SOLID, 0},
    {32, 160, 3, SOLID, 0},
    {0, 0, 1, 0, 0}
};

struct tile *mg_get_tile(int tile)
{
    return &dungeontiles[tile];
}

#endif