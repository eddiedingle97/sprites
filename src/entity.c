#include <stdio.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "spritemanager.h"
#include "entity.h"
#include "util.h"

struct entity *e_create(float x, float y, struct animation *an, void *data)
{
    struct entity *out = s_malloc(sizeof(struct entity), "e_create");
    out->sprite = sm_create_global_dynamic_sprite(an, x, y, PLAYER, CENTERED);
    out->data = data;
    out->chunk = NULL;
    out->hand = NULL;

    return out;
}

struct animation *e_load_animations_from_config(ALLEGRO_CONFIG *cfg)
{
    int noanimations = u_atoi(al_get_config_value(cfg, "", "animations"));
    if(noanimations == 0)
        return NULL;
    struct animation *out = s_aligned_malloc(noanimations * sizeof(struct animation), 32, "e_load_animations_from_config");
    char an[4] = "an ";

    int i;
    for(i = 0; i < noanimations; i++)
    {
        an[2] = (char)(i + 48);
        out[i].width = u_atoi(al_get_config_value(cfg, an, "width"));
        out[i].height = u_atoi(al_get_config_value(cfg, an, "height"));
        out[i].x = u_atoi(al_get_config_value(cfg, an, "x"));
        out[i].y = u_atoi(al_get_config_value(cfg, an, "y"));
        out[i].spritecount = u_atoi(al_get_config_value(cfg, an, "spritecount"));
        out[i].ticks = u_atoi(al_get_config_value(cfg, an, "ticks"));
        out[i].offsetx = u_atoi(al_get_config_value(cfg, an, "offsetx"));
        out[i].offsety = u_atoi(al_get_config_value(cfg, an, "offsety"));
    }

    return out;
}

void e_load_stats_from_config(ALLEGRO_CONFIG *cfg, struct entity *e)
{
    e->speedx = 0;
    e->speedy = 0;
    e->maxspeed = u_atof(al_get_config_value(cfg, "stats", "maxspeed"));
    e->weight = u_atof(al_get_config_value(cfg, "stats", "weight"));
    e->accel = u_atof(al_get_config_value(cfg, "stats", "accel"));
    e->colrad = u_atof(al_get_config_value(cfg, "stats", "colrad"));
}

void e_destroy(struct entity *e)
{
    if(e->data)
        s_free(e->data, NULL);

    sm_destroy_sprite(e->sprite);

    s_free(e, NULL);
}