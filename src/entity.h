#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <allegro5/allegro.h>
#include "spritemanager.h"

/*struct spritesheetdata
{
    unsigned short width;
    unsigned short height;
    unsigned short y;
    unsigned short x;
    unsigned char spritecount;
};*/

struct entity
{
    struct sprite *sprite;
    void *data;
    void (*behaviour)(struct sprite *sprite, void *data);
};

struct entity *e_create(ALLEGRO_BITMAP *bitmap, void (*draw)(struct sprite *sprite, int tick), void (*behaviour)(struct sprite *sprite, void *data), float x, float y, struct animation *an, void *data);
void e_destroy(struct entity *e);

#endif