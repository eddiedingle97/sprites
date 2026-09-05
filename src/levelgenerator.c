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
#include "constellation.h"
#include "entities/entities.h"
#include "items/items.h"

#define REGISTER_KNIGHT em_register_entity(knight_create, knight_behaviour, knight_destroy, CANHOLD | CIRCULARHITBOX);
#define REGISTER_SWORD em_register_entity(sword_create, sword_behaviour, sword_destroy, ITEM | HOLDABLE);
#define REGISTER_ORC em_register_entity(orc_create, orc_behaviour, orc_destroy, CANHOLD | CIRCULARHITBOX);
#define REGISTER_KNIFE em_register_entity(knife_create, knife_behaviour, knife_destroy, ITEM | HOLDABLE);

static struct dict *warptable;
static struct entity *knight;
static struct entity *sword;
static struct sky *sky;
void lg_add_enemies(struct map *map);
int wte_comp(struct warptableentry *one, struct warptableentry *two);

struct map *lg_generate_level(char newmap)
{
	warptable = dict_create(wte_comp);
        mm_add_tile_map_to_list("DungeonTilesetIItiles.png", 16);

	REGISTER_KNIGHT
	//REGISTER_SWORD
	//REGISTER_ORC
	//REGISTER_KNIFE

	struct map *map = mg_create_map(50, 50);
	mm_add_map(map);
	mm_set_top_map(0);
	
	struct room *center = &map->rooms[map->graph->novertices / 2];
	knight = em_create_entity(0, 16.0f * mg_room_center_x(center), 16.0f * mg_room_center_y(center));
	em_add_entity_to_map(map, knight);
	knight->health = 10;
	//sword = em_create_entity(1, 16.0f * mg_room_center_x(center) + 16.0f, 16.0f * mg_room_center_y(center));
	//em_add_entity_to_map(map, sword);

	/*sky = cons_create_sky();
	cons_gen_sky(sky);
	sm_add_sprite_to_layer(sky->canvas);*/
	
        //em_register_entity(orc_create, orc_behaviour, orc_destroy, 12);
	//em_register_entity(knife_create, knife_behaviour, knife_destroy, 3);

	//lg_add_enemies(map);

	/*em_register_entity(sword_create, sword_behaviour, sword_destroy, 3);
	em_register_entity(lizard_create, lizard_behaviour, lizard_destroy, 12);
	sword = em_create_entity(3, 0.0f, 0.0f);
	em_add_entity_to_map(map, sword);
	knight->hand = sword;
	sword->holder = knight;
	struct entity *lizard = em_create_entity(4, 0.0f, -100.0f);
	em_add_entity_to_map(map, lizard);

	em_register_entity(necro_create, necro_behaviour, necro_destroy, 12);
	em_register_entity(potion_create, potion_behaviour, potion_destroy, CIRCULARHITBOX | CALLONCOLLIDE | KILLONNEGATIVEHEALTH);
	struct entity *necro = em_create_entity(5, 100.0f, 0.0f);
	em_add_entity_to_map(map, necro);
	struct necrodata *nd = necro->data;
	nd->target = knight;*/

	return map;
}

void lg_tick()
{
	//sword->angvel += .02;
	//sky->canvas->rot += .001;
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
	//cons_destroy_sky(sky);
}
