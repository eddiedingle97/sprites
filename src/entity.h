#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <allegro5/allegro.h>
#include "spritemanager.h"

struct entity
{
    struct sprite *sprite;
    void *data;
    struct chunk *chunk;
    unsigned char id;
};

struct entity *e_create(ALLEGRO_BITMAP *bitmap, void (*draw)(struct sprite *sprite, int tick), float x, float y, struct animation *an, void *data);
void e_destroy(struct entity *e);

#endif