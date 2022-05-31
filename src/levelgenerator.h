#ifndef __LEVELGENERATOR_H__
#define __LEVELGENERATOR_H__
#include "dictionary.h"

void lg_generate_level(char newmap);
void lg_destroy_level();
void lg_tick();

struct warptableentry
{
	struct map *map;
	int x;
	int y;
};

struct dict *lg_get_warp_table();

#endif