#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../movementandcollision.h"
#include "../spritemanager.h"

#include "../emath.h"

struct orcdata
{
    unsigned char idle;
    float speed;
    struct entity *target;
    ALLEGRO_BITMAP *bitmap;
};

void orc_behaviour(struct sprite *sprite, struct orcdata *data)
{
    struct animation *an = sprite->an;
    struct sprite *target = data->target->sprite;

    float dx = target->x - sprite->x, dy = target->y - sprite->y;
    float invdist = math_fast_inverse_sqrt(dx * dx + dy * dy);
    dx *= invdist * data->speed;
    dy *= invdist * data->speed;

    if(mc_do_entity_movement(sprite, dx, dy))
        data->idle = 0;
    else
        data->idle = 1;

    if(dx < 0)
        an->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(dx > 0)
        an->alflags = 0;

    sprite->i = data->idle;
}

struct entity *orc_create(ALLEGRO_BITMAP *spritesheet)
{
    struct orcdata *od = s_malloc(sizeof(struct orcdata), "kd: create_orc");
    struct animation *an = s_malloc(2 * sizeof(struct animation), "an: create_orc");
    an->width = 16;
    an->height = 32;
    an->x = 64;
    an->y = 432;
    an->spritecount = 4;
    an->cycle = 0;
    an->ticks = 6;
    an->alflags = 0;

    od->idle = 1;
    od->speed = .5;
    
    an[1].width = 16;
    an[1].height = 32;
    an[1].x = 0;
    an[1].y = 432;
    an[1].spritecount = 4;
    an[1].cycle = 0;
    an[1].ticks = 6;
    an[1].alflags = 0;

    struct entity *out = e_create(spritesheet, NULL, orc_behaviour, 0, 0, an, od);
    return out;
}

void orc_destroy(struct entity *orc)
{
    e_destroy(orc);
}