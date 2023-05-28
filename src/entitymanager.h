#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__
#include "entity.h"
#include "map.h"
#include <allegro5/allegro.h>

void em_init();
void em_tick();
int em_register_entity(struct entity *(*create)(ALLEGRO_BITMAP *), void (*b)(struct map *, struct entity *, float *, float *), void(*d)(struct entity *), char i);
struct entity *em_create_entity(unsigned char id, float x, float y);
int em_add_entity_to_map(struct map *map, struct entity *e);
int em_remove_entity_from_map(struct map *map, struct chunk *chunk, struct entity *e);
void em_destroy();

enum EM_ATTRIBUTES {ITEM = 1, HOLDABLE = 2, CANHOLD = 4, CIRCULARHITBOX = 8, CALLONCOLLIDE = 16};
enum EM_FLAGS {AIRBORNE = 1};

#endif