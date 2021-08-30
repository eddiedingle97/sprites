#include <stdio.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "spritemanager.h"
#include "entity.h"

struct entity *e_create(ALLEGRO_BITMAP *bitmap, void (*draw)(struct sprite *sprite, int tick), void (*behaviour)(struct sprite *sprite, void *data), float x, float y, struct animation *an, void *data)
{
    struct entity *out = s_malloc(sizeof(struct entity), "e_create");
    out->sprite = sm_create_global_dynamic_sprite(bitmap, draw, an, x, y, PLAYER, CENTERED);
    out->behaviour = behaviour;
    out->data = data;

    return out;
}

void e_destroy(struct entity *e)
{
    if(e->data)
        s_free(e->data, "Freeing entity data");

    sm_destroy_sprite(e->sprite);

    s_free(e, "Freeing entity");
}