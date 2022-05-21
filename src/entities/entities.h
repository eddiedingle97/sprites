#ifndef __ENTITIES_H__
#define __ENTITIES_H__

struct knightdata
{
    unsigned char idle;
    float speed;
    ALLEGRO_BITMAP *bitmap;
};

void knight_behaviour(struct entity *e, float *dx, float *dy);
struct entity *knight_create(ALLEGRO_BITMAP *spritesheet);
void knight_destroy(struct entity *knight);

struct orcdata
{
    unsigned char state;
    float x;
    float y;
    float speed;
    struct entity *target;
};

void orc_behaviour(struct entity *entity, float *dx, float *dy);
struct entity *orc_create(ALLEGRO_BITMAP *spritesheet);
void orc_destroy(struct entity *orc);

#endif