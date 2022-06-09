#include <stdio.h>
#include <math.h>
#include "sprites.h"
#include "map.h"
#include "emath.h"
#include "mapmanager.h"
#include "mapgenerator.h"
#include "entitymanager.h"
#include "dictionary.h"
#include "levelgenerator.h"
#include "tilefunctions.h"
#include "graph.h"
#include "entities/entities.h"
#include "items/items.h"

static struct dict *warptable;
static struct entity *knight;
static struct entity *sword;
void lg_add_enemies(struct map *map);
int wte_comp(struct warptableentry *one, struct warptableentry *two);

void lg_generate_level(char newmap)
{
	warptable = dict_create(wte_comp);
    mm_add_tile_map_to_list("DungeonTilesetIItiles.png", 16);
    mm_register_tile_function(tf_warp);
	struct map *map = mg_create_map(30, 30);
	mm_add_map(map);
	mm_set_top_map(0);
    em_register_entity(knight_create, knight_behaviour, knight_destroy, 0);
    em_register_entity(orc_create, orc_behaviour, orc_destroy, 0);
	knight = em_create_entity(0, 0, 0);
	em_add_entity_to_map(map, knight);

	lg_add_enemies(map);

	em_register_entity(sword_create, sword_behaviour, sword_destroy, 1);
	sword = em_create_entity(2, 32.0f, 0.0f);
	em_add_entity_to_map(map, sword);
	
}

void lg_tick()
{
	//sword->angvel += .02;
}

struct dict *lg_get_warp_table()
{
    return warptable;
}

void lg_add_enemies(struct map *map)
{
	int i, j;
	for(j = 0; j < map->graph->novertices; j++)
	{
		struct vertex *v = graph_get_vertex(map->graph, j);
		if(!v)
			continue;
		struct room *room = v->p;
		for(i = 0; i < room->enemies; i++)
		{
			
			int x = (room->x + 1 + math_get_random(room->w - 3)) * 16;
			int y = (room->y - 1 - math_get_random(room->h - 3)) * 16;
			//printf("adding enemy %d %d %d %d %d %d %.2f %.2f\n", room->x, room->w, room->y, room->h, x, y, (float)x, (float)y);
			struct entity *e = em_create_entity(1, x, y);
			struct orcdata *od = e->data;
			od->target = knight;
			map_add_entity_to_chunk(map, e);
		}
	}
}

int wte_comp(struct warptableentry *one, struct warptableentry *two)
{
    if(one->x == two->x)
    {
        if(one->y == two->y)
        {
            if(one->map == two->map)
                return 0;
            return one->map < two->map ? -1 : 1;
        }
        return one->y - two->y;
    }
    return one->x - two->x;
}

void lg_add_warp(struct warptableentry *wteone, struct warptableentry *wtetwo)
{
    dict_add_entry(warptable, wteone, wtetwo);
}

void lg_destroy_level()
{
	int i;
	for(i = 0; i < warptable->size; i++)
    {
        s_free(warptable->keys[i], NULL);
        s_free(warptable->p[i], NULL);
    }
    dict_destroy(warptable);
}
