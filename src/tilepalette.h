#ifndef __TILEPALETTE_H__
#define __TILEPALETTE_H__
#include "map.h"

enum PALETTETYPES {PT_DUNGEONTILES, PT_ISLANDTILES};
enum DT_DUNGEONTILES {DT_EMPTY, DT_FLOOR, DT_LEFTWALL, DT_RIGHTWALL, DT_TOPWALL, DT_BOTTOMWALL, DT_WALL, DT_ERROR};
static const char *DT_NAMES[] = {"empty", "floor", "leftwall", "rightwall", "topwall", "bottomwall", "wall"};
static const char DT_TILETYPE[] = {0, 0, SOLID, SOLID, SOLID, SOLID, SOLID, 0};
enum IT_ISLANDTILES {IT_SAND, IT_WATER, IT_GRASS, IT_DIRT, IT_ERROR};
static const char *IT_NAMES[] = {"sand", "water", "grass", "dirt"};
static const char IT_TILETYPE[] = {0, 0, 0, 0, 0};

static const int DT_SIZE = DT_ERROR;
static const int IT_SIZE = IT_ERROR;
/*struct tile dungeontiles[] = 
{
    {16, 64, 3, 0, 0},
    {0, 128, 3, SOLID, 0},
    {16, 128, 3, SOLID, 0},
    {64, 160, 3, SOLID, 0},
    {32, 160, 3, SOLID, 0},
    {32, 160, 3, SOLID, 0},
    {0, 0, 1, 0, 0}
};*/

void tp_change_palette(char *configfile, int type);
void tp_destroy();
struct palette *tp_get_palette_copy();
struct tile *tp_get_tile(int tile);

#endif