#ifndef __ENTITYMANAGER_H__
#define __ENTITYMANAGER_H__
#include "entity.h"

void em_init();
void em_tick();
struct entity *em_create_enemy(float x, float y);
int em_add_entity_to_chunk(struct entity *e);
int em_remove_entity_from_chunk(struct entity *e);
void em_destroy();

#endif