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

	struct map *island = mg_create_island_map(500, 500);
	mm_add_map(island);
	mm_set_top_map(0);
    em_register_entity(knight_create, knight_behaviour, knight_destroy, 12);
	knight = em_create_entity(0, 0, 0);
	knight->health = 10;
	em_add_entity_to_map(island, knight);
	em_register_entity(lizard_create, lizard_behaviour, lizard_destroy, 12);
	int r, c;
	for(r = 0; r < island->height; r++)
	{
		for(c = 0; c < island->width; c++)
		{
			if(island->chunks[r][c].flags & MG_HABITABLE)
			{
				//em_add_entity_to_map(island, em_create_entity(1, c * island->chunksize * island->tilesize - island->width * island->tilesize / 2, -r * island->chunksize * island->tilesize - island->height * island->tilesize / 2));
			}
		}
	}
	

    //mm_register_tile_function(tf_warp);
	/*struct map *map = mg_create_map(50, 50);
	mm_add_map(map);
	mm_set_top_map(0);
    em_register_entity(knight_create, knight_behaviour, knight_destroy, 12);
    em_register_entity(orc_create, orc_behaviour, orc_destroy, 12);
	em_register_entity(knife_create, knife_behaviour, knife_destroy, 3);
	
	knight = em_create_entity(0, 0, 0);
	em_add_entity_to_map(map, knight);
	knight->health = 10;

	lg_add_enemies(map);

	em_register_entity(sword_create, sword_behaviour, sword_destroy, 3);
	em_register_entity(lizard_create, lizard_behaviour, lizard_destroy, 12);
	sword = em_create_entity(3, 32.0f, 0.0f);
	em_add_entity_to_map(map, sword);
	struct entity *lizard = em_create_entity(4, 0.0f, -100.0f);
	em_add_entity_to_map(map, lizard);*/
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
	if(map->graph)
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
				if(e)
				{
					struct orcdata *od = e->data;
					od->target = knight;
					em_add_entity_to_map(map, e);
					if(math_get_random(1))
					{
						e->hand = em_create_entity(2, x, y);
						e->hand->holder = e;
						em_add_entity_to_map(map, e->hand);
					}
				}
			}
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
