#include <allegro5/allegro.h>
#include "sprites.h"
#include "entity.h"
#include "util.h"
#include "item.h"

struct entity *item_create(ALLEGRO_BITMAP *bitmap, void *data)
{
	struct entity *out = s_malloc(sizeof(struct entity), "item_create");
	out->sprite = sm_create_global_sprite(bitmap, 0, 0, PLAYER, CENTERED);
	out->chunk = NULL;
	out->data = data;
	out->holder = NULL;

	return out;
}

ALLEGRO_BITMAP *item_get_bitmap_from_config(ALLEGRO_CONFIG *cfg)
{
	int w = u_atoi(al_get_config_value(cfg, "sprite", "width"));
	int h = u_atoi(al_get_config_value(cfg, "sprite", "height"));
	int x = u_atoi(al_get_config_value(cfg, "sprite", "x"));
	int y = u_atoi(al_get_config_value(cfg, "sprite", "y"));
	return sm_get_sub_bitmap(x, y, w, h);
}

void item_get_stats_from_config(ALLEGRO_CONFIG *cfg, struct entity *e)
{
	e->speedx = 0;
	e->speedy = 0;
	e->damage = u_atof(al_get_config_value(cfg, "stats", "damage"));
	e->weight = u_atof(al_get_config_value(cfg, "stats", "weight"));
	e->colrad = u_atof(al_get_config_value(cfg, "stats", "colrad"));
}

void item_destroy(struct entity *e)
{
	sm_remove_sprite_from_layer(e->sprite);
	s_free(e->sprite, NULL);
	s_free(e, NULL);
}