#include <stdio.h>
#include "tilefunctions.h"
#include "spritemanager.h"
#include "mapmanager.h"
#include "entity.h"
#include "map.h"
#include "dictionary.h"
#include "emath.h"

void tf_warp(struct map *map, struct entity *e)
{
	printf("warping %.2f %.2f\n", e->sprite->x, e->sprite->y);

	struct dict *warptable = mm_get_warp_table();
	struct warptableentry wte;
	wte.map = map;
	wte.x = math_floor(e->sprite->x / map->tilesize);
	wte.y = math_ceil(e->sprite->y / map->tilesize);

	struct warptableentry *wtout = dict_get_entry(warptable, &wte);

	if(wtout)
	{
		map_remove_entity_from_chunk(map, e);
		e->sprite->x = wtout->x * 16.0f;
		e->sprite->y = wtout->y * 16.0f;
		map_add_entity_to_chunk(wtout->map, e);

		if(e->id == 0)
		{
			mm_set_top_map(1);
			sm_set_coord(e->sprite->x, e->sprite->y);
		}
	}
	else
		printf("wte not found %d %d\n", wte.x, wte.y);
}
