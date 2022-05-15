#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../spritemanager.h"
#include "../map.h"
#include "../mapmanager.h"
#include "../emath.h"

enum ORCSTATE {IDLE, AGGRO, SEEK};

static struct animation *orcanimations = NULL;
static int noorcs = 0;

struct orcdata
{
    unsigned char state;
    float x;
    float y;
    float speed;
    struct entity *target;
    ALLEGRO_BITMAP *bitmap;
};

float lerp_check(float x1, float y1, float x2, float y2, unsigned char tilemask)
{
    float x = x1 - x2, y = y1 - y2;
    int n = math_sqrt(x * x + y * y), i = 0;
    struct tile *t = NULL;

    x /= n;
    y /= n;

    for(i = 0; i < n; i++)
    {
        x2 += x;
        y2 += y;
        t = mm_get_tile(x2, y2);
        if(t && t->type & tilemask)
            return 0;
    }

    return n;
}

void orc_behaviour(struct entity *entity, float *dx, float *dy)
{
    struct sprite *sprite = entity->sprite;
    struct orcdata *data = entity->data;
    struct animation *an = sprite->an;
    struct sprite *target = data->target->sprite;
    float dist;

    if(!entity->sprite->id)
        dist = 0;
    else
        dist = lerp_check(sprite->x, sprite->y, target->x, target->y, SOLID);
    switch(data->state)
    {
        case IDLE:
            if(dist != 0)
                data->state = AGGRO;
            else
                break;
        case AGGRO:
            if(dist != 0)
            {
                *dx = target->x - sprite->x, *dy = target->y - sprite->y;
                *dx = *dx / dist * data->speed;
                *dy = *dy / dist * data->speed;
                data->x = target->x;
                data->y = target->y;
                break;
            }
            else
            {
                data->state = SEEK;
            }
        case SEEK:
            if(dist != 0)
            {
                *dx = target->x - sprite->x, *dy = target->y - sprite->y;
                *dx = *dx / dist * data->speed;
                *dy = *dy / dist * data->speed;
                data->x = target->x;
                data->y = target->y;
                data->state = AGGRO;
            }

            else if(math_in_range(sprite->x - 1, data->x, sprite->x + 1) && math_in_range(sprite->y - 1, data->y, sprite->y + 1))
            {
                data->state = IDLE;
            }
                
            else
            {
                *dx = data->x - sprite->x, *dy = data->y - sprite->y;
                dist = math_sqrt(*dx * *dx + *dy * *dy);
                *dx = *dx / dist * data->speed;
                *dy = *dy / dist * data->speed;
            }

            break;
    }

    if(*dx < 0)
        sprite->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(*dx > 0)
        sprite->alflags = 0;

    sprite->i = data->state == IDLE;
}

struct entity *orc_create(ALLEGRO_BITMAP *spritesheet)
{
    struct orcdata *od = s_malloc(sizeof(struct orcdata), "od: create_orc");
    struct animation *an;
    if(!orcanimations)
    {
        an = s_malloc(2 * sizeof(struct animation), "an: create_orc");
        an[0].width = 16;
        an[0].height = 32;
        an[0].x = 64;
        an[0].y = 432;
        an[0].spritecount = 4;
        an[0].ticks = 6;
        an[0].offsetx = 0;
        an[0].offsety = 4;

        an[1].width = 16;
        an[1].height = 32;
        an[1].x = 0;
        an[1].y = 432;
        an[1].spritecount = 4;
        an[1].ticks = 6;
        an[1].offsetx = 0;
        an[1].offsety = 4;

        orcanimations = an;
    }
    else
        an = orcanimations;

    od->speed = .875;
    od->state = IDLE;
    od->x = 0;
    od->y = 0;

    struct entity *out = e_create(spritesheet, NULL, orc_behaviour, 0, 0, an, od);
    out->destroy = 1;
    noorcs++;
    return out;
}

void orc_destroy(struct entity *orc)
{
    if(--noorcs == 0)
        s_free(orc->sprite->an, NULL);
    
    e_destroy(orc);
}