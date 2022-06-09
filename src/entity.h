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
            float strength;
            float health;
            struct entity *hand;
            struct action *actions;
            unsigned char noactions;
        };
        struct//item 32 bytes
        {
            float rotx;
            float roty;
            float holdx;
            float holdy;
            float damage;
            float angvel;
            struct entity *holder;
        };
    };
    
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