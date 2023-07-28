#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include "../entity.h"
#include "../spritemanager.h"
#include "../mapgenerator.h"
#include "../map.h"

float eu_lerp_check(struct map *map, float x1, float y1, float x2, float y2, unsigned char tilemask);
struct pixcoord *eu_a_star(struct map *map, float startx, float starty, float endx, float endy, int *no, unsigned char typemask,
    float (*cost)(struct map *, struct coord *, struct coord *, void *data), void *data);
int eu_follow_path(struct map *map, struct entity *e, struct pixcoord *path, int pathsize, int *next, float *dx, float *dy);

struct knightdata
{
    unsigned char idle;
};

void knight_behaviour(struct map *map, struct entity *e, float *dx, float *dy);
struct entity *knight_create();
void knight_destroy(struct entity *knight);

struct orcdata
{
    unsigned char state;
    unsigned char cooldown;
    float x;
    float y;
    struct entity *target;
};

void orc_behaviour(struct map *map, struct entity *entity, float *dx, float *dy);
struct entity *orc_create();
void orc_destroy(struct entity *orc);

struct lizarddata
{
    unsigned char idle;
};

void lizard_behaviour(struct map *map, struct entity *entity, float *dx, float *dy);
struct entity *lizard_create();
void lizard_destroy(struct entity *lizard);

struct necrodata
{
    unsigned char state;
    unsigned char cooldown;
    struct room *escapeplan;
    struct pixcoord *escaperoute;
    int escaperoutesize;
    int nexttile;
    int nospawned;
    int maxspawned;
    struct entity *target;
};

void necro_behaviour(struct map *map, struct entity *entity, float *dx, float *dy);
struct entity *necro_create();
void necro_destroy(struct entity *orc);

void potion_behaviour(struct map *map, struct entity *entity, float *dx, float *dy);
struct entity *potion_create();
void potion_destroy(struct entity *orc);

#endif