#include <stdio.h>
#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../spritemanager.h"
#include "entities.h"

void knight_behaviour(struct entity *e, float *dx, float *dy)
{
    struct sprite *sprite = e->sprite;
    struct knightdata *data = e->data;
    float up = kb_get_up(), down = kb_get_down(), left = kb_get_left(), right = kb_get_right();
    if(left && !right)
        sprite->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(right && !left)
        sprite->alflags = 0;

    *dx = right - left;
    *dy = up - down;

    if(*dx == 0 && *dy == 0)
        data->idle = 1;
    else
        data->idle = 0;

    sprite->i = data->idle;
}

struct entity *knight_create()
{
    struct knightdata *kd = s_malloc(sizeof(struct knightdata), NULL);
    kd->idle = 1;

    ALLEGRO_CONFIG *cfg = al_load_config_file(s_get_full_path_with_dir("config/entities", "knight.cfg"));
    struct animation *an = e_load_animations_from_config(cfg);

    struct entity *out = e_create(0, 0, an, kd);
    
    e_load_stats_from_config(cfg, out);

    al_destroy_config(cfg);
    return out;
}

void knight_destroy(struct entity *knight)
{
    s_free(knight->sprite->an, NULL);
    e_destroy(knight);
}