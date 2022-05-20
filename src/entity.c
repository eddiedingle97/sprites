#include <stdio.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "spritemanager.h"
#include "entity.h"

struct entity *e_create(ALLEGRO_BITMAP *bitmap, float x, float y, struct animation *an, void *data)
{
    struct entity *out = s_malloc(sizeof(struct entity), "e_create");
    out->sprite = sm_create_global_dynamic_sprite(bitmap, an, x, y, PLAYER, CENTERED);
    out->data = data;
    out->chunk = NULL;

    return out;
}

void e_destroy(struct entity *e)
{
    if(e->data)
        s_free(e->data, NULL);

    sm_destroy_sprite(e->sprite);

    s_free(e, NULL);
}