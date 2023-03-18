#include <stdio.h>
#include <math.h>
#include "../entity.h"
#include "../keyboard.h"
#include "../mouse.h"
#include "../sprites.h"
#include "../spritemanager.h"
#include "../emath.h"
#include "../action.h"
#include "../debug.h"
#include "entities.h"

static struct animation *lizardanimations = NULL;
static ALLEGRO_CONFIG *lizardcfg = NULL;
static int nolizards = 0;

void lizard_behaviour(struct entity *e, int tick, float *dx, float *dy)
{
	struct lizarddata *ld = e->data;
    e->sprite->i = ld->idle;
}

struct entity *lizard_create()
{
    struct lizarddata *ld = s_malloc(sizeof(struct lizarddata), NULL);
    ld->idle = 1;

	if(!lizardcfg)
	{
		lizardcfg = al_load_config_file(s_get_full_path_with_dir("config/entities", "lizard.cfg"));
		lizardanimations = e_load_animations_from_config(lizardcfg);
	}

    struct entity *out = e_create(0, 0, lizardanimations, ld);
    out->strength = 10;
	
    e_load_stats_from_config(lizardcfg, out);
	out->health = 20;
	nolizards++;

    return out;
}

void lizard_destroy(struct entity *lizard)
{
    //s_free(lizard->sprite->an, NULL);//FIX: assumes theres only one
	if(--nolizards == 0)
	{
		s_free(lizardanimations, NULL);
		al_destroy_config(lizardcfg);
	}
	
	e_destroy(lizard);
}