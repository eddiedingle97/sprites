#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "../sprites.h"
#include "../spritemanager.h"
#include "../entity.h"
#include "../util.h"
#include "../item.h"
#include "items.h"

static ALLEGRO_BITMAP *knifebitmap = NULL;
static ALLEGRO_CONFIG *knifecfg = NULL;
static int noknifes = 0;

struct entity *knife_create()
{
	if(!knifecfg)
	{
		knifecfg = al_load_config_file(s_get_full_path_with_dir("config/items", "knife.cfg"));
		knifebitmap = item_get_bitmap_from_config(knifecfg);
	}
	
	struct entity *out = item_create(knifebitmap, NULL);
	item_get_stats_from_config(knifecfg, out);
	out->sprite->rotoffset = u_atof(al_get_config_value(knifecfg, "sprite", "rotoffset")) * M_PI;
	out->holdx = 0;
	out->holdy = 5.5;
	out->angvel = 0;
	out->rotx = 0;
	out->roty = 0;

	out->width = (unsigned char)u_atoi(al_get_config_value(knifecfg, "stats", "hitboxwidth"));//move to e_get_stats or whatever
	out->height = (unsigned char)u_atoi(al_get_config_value(knifecfg, "stats", "hitboxheight"));
	out->offsetx = (char)u_atoi(al_get_config_value(knifecfg, "stats", "hitboxoffsetx"));
	out->offsety = (char)u_atoi(al_get_config_value(knifecfg, "stats", "hitboxoffsety"));

	noknifes++;

	return out;
}

void knife_behaviour(struct entity *e, float *dx, float *dy)
{
	
}

void knife_destroy(struct entity *e)
{
	if(--noknifes == 0)
	{
		al_destroy_bitmap(knifebitmap);
		knifebitmap = NULL;
		al_destroy_config(knifecfg);
		knifecfg = NULL;
	}

	item_destroy(e);
}