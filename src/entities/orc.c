#include <stdlib.h>
#include <allegro5/allegro.h>
#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../spritemanager.h"
#include "../map.h"
#include "../mapmanager.h"
#include "../emath.h"
#include "../util.h"
#include "entities.h"

enum ORCSTATE {IDLE, AGGRO, SEEK};

static struct animation *orcanimations = NULL;
static ALLEGRO_CONFIG *orccfg = NULL;
static int noorcs = 0;

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
        t = mm_get_tile(x2, y2);//does not work with multiple maps, change this
        if(t && t->type & tilemask)
            return 0;
    }

    return n;
}

void orc_behaviour(struct entity *entity, float *dx, float *dy)
{
    struct sprite *sprite = entity->sprite;
    struct orcdata *data = entity->data;
    struct sprite *target = data->target->sprite;
    float dist;

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
            }

            break;
    }

    if(*dx < 0)
        sprite->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(*dx > 0)
        sprite->alflags = 0;

    sprite->i = data->state == IDLE;
}

struct entity *orc_create()
{
    struct orcdata *od = s_malloc(sizeof(struct orcdata), NULL);//orc_get_stats();
    od->x = 0;
    od->y = 0;
    od->state = IDLE;
    struct animation *an;
    if(!orccfg)
    {
        orccfg = al_load_config_file(s_get_full_path_with_dir("config/entities", "orc.cfg"));
        an = e_load_animations_from_config(orccfg);

        orcanimations = an;
    }
    else
        an = orcanimations;

    struct entity *out = e_create(0, 0, an, od);
    e_load_stats_from_config(orccfg, out);
    noorcs++;
    return out;
}

void orc_destroy(struct entity *orc)
{
    if(--noorcs == 0)
    {
        s_free(orcanimations, NULL);
        orcanimations = NULL;
        al_destroy_config(orccfg);
        orccfg = NULL;
    }

    e_destroy(orc);
}