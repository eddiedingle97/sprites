#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "../sprites.h"
#include "../spritemanager.h"
#include "../entity.h"
#include "../util.h"
#include "../item.h"
#include "items.h"

static ALLEGRO_BITMAP *swordbitmap = NULL;
static ALLEGRO_CONFIG *swordcfg = NULL;
static int noswords = 0;

static float offsetx = 10.5f;
static float offsety = 4.0f;

struct entity *sword_create()
{
	if(!swordcfg)
	{
		swordcfg = al_load_config_file(s_get_full_path_with_dir("config/items", "sword.cfg"));
		swordbitmap = item_get_bitmap_from_config(swordcfg);
	}
	
	struct entity *out = item_create(swordbitmap, NULL);
	item_get_stats_from_config(swordcfg, out);
	out->sprite->rotoffset = u_atof(al_get_config_value(swordcfg, "sprite", "rotoffset")) * M_PI;
	out->holdx = 0;
	out->holdy = -8.5;
	out->angvel = 0;
	out->rotx = 0;
	out->roty = 0;

	noswords++;

	return out;
}

void sword_behaviour(struct entity *e, float *dx, float *dy)
{
	
}

void sword_destroy(struct entity *e)
{
	if(--noswords == 0)
	{
		al_destroy_bitmap(swordbitmap);
		swordbitmap = NULL;
		al_destroy_config(swordcfg);
		swordcfg = NULL;
	}

	item_destroy(e);
}