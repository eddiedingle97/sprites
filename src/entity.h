#ifndef __ENTITY_H__
#define __ENTITY_H__
#include <allegro5/allegro.h>
#include "spritemanager.h"

struct entity
{
    struct sprite *sprite;//probably pull most of this out for ECS in the future, have a single id for each entity
    void *data;
    float weight;
    float speedx;
    float speedy;
    float z;
    float speedz;
    unsigned char id;//specific to an entity type
    unsigned char flags;
    union//hit box data
    {
        struct//circular hit box
        {
            float colrad;
        };
        struct //rectangular hit box
        {
            unsigned char width;
            unsigned char height;
            char offsetx;//from center
            char offsety;//from center
        };
    };
    union//mutually exclusive stats, future entity types might not allocate this portion for data savings
    {
        struct//pc 29 bytes
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
            float holdx;//FIX: put this in the <item>.c file
            float holdy;//FIX: put this in the <item>.c file
            float damage;
            float angvel;
            struct entity *holder;
        };
    };
};

struct pixcoord
{
    float x;
    float y;
};

struct entity *e_create(float x, float y, struct animation *an, void *data);
void e_destroy(struct entity *e);
struct animation *e_load_animations_from_config(ALLEGRO_CONFIG *cfg);
void e_load_stats_from_config(ALLEGRO_CONFIG *cfg, struct entity *e);

#endif