#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__
#include "entity.h"
#include "map.h"
#include <allegro5/allegro.h>

void em_init();
void em_tick();
int em_register_entity(struct entity *(*create)(ALLEGRO_BITMAP *), void (*b)(struct entity *, float *, float *), void(*d)(struct entity *), char i);
struct entity *em_create_entity(unsigned char id, float x, float y);
int em_add_entity_to_chunk(struct map *map, struct entity *e);
int em_remove_entity_from_chunk(struct map *map, struct entity *e);
void em_destroy();

#endif