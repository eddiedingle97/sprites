#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <allegro5/allegro.h>
#include "spritemanager.h"

struct entity
{
    struct sprite *sprite;
    void *data;
    struct chunk *chunk;
    union
    {
        struct//pc
        {
            float accel;
            struct entity *hand;
        };
        struct//item
        {
            float damage;
            struct entity *holder;
        };
    };
    float maxspeed;
    float weight;
    float speedx;
    float speedy;
    float colrad;
    unsigned char id;
};

struct entity *e_create(float x, float y, struct animation *an, void *data);
void e_destroy(struct entity *e);
struct animation *e_load_animations_from_config(ALLEGRO_CONFIG *cfg);
void e_load_stats_from_config(ALLEGRO_CONFIG *cfg, struct entity *e);

#endif