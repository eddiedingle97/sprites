#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "../sprites.h"
#include "../map.h"
#include "../spritemanager.h"
#include "../entity.h"
#include "../util.h"
#include "../item.h"

static struct animation *potionanimations = NULL;
static ALLEGRO_CONFIG *potioncfg = NULL;
static int nopotions = 0;

void potion_behaviour(struct map *map, struct entity *entity, float *dx, float *dy)
{
    puts("hello!!");
    entity->health = -1;
}

struct entity *potion_create()
{
    struct animation *an;
    if(!potioncfg)
    {
        potioncfg = al_load_config_file(s_get_full_path_with_dir("config/entities", "potion.cfg"));
        an = e_load_animations_from_config(potioncfg);

        potionanimations = an;
    }
    else
        an = potionanimations;

    struct entity *out = e_create(0, 0, an, NULL);
    e_load_stats_from_config(potioncfg, out);
    nopotions++;
    return out;
}

void potion_destroy(struct entity *potion)
{
    if(--nopotions == 0)
    {
        s_free(potionanimations, NULL);
        potionanimations = NULL;
        al_destroy_config(potioncfg);
        potioncfg = NULL;
    }

    e_destroy(potion);
}
