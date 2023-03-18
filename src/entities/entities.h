#ifndef __ENTITIES_H__
#define __ENTITIES_H__

struct knightdata
{
    unsigned char idle;
};

void knight_behaviour(struct entity *e, int tick, float *dx, float *dy);
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

void orc_behaviour(struct entity *entity, int tick, float *dx, float *dy);
struct entity *orc_create();
void orc_destroy(struct entity *orc);

struct lizarddata
{
    unsigned char idle;
};

void lizard_behaviour(struct entity *entity, int tick, float *dx, float *dy);
struct entity *lizard_create();
void lizard_destroy(struct entity *lizard);

#endif