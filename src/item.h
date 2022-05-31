#ifndef __ITEM_H__
#define __ITEM_H__
#include <allegro5/allegro.h>
#include "entity.h"

struct entity *item_create(ALLEGRO_BITMAP *bitmap, void *data);
ALLEGRO_BITMAP *item_get_bitmap_from_config(ALLEGRO_CONFIG *cfg);
void item_get_stats_from_config(ALLEGRO_CONFIG *cfg, struct entity *e);
void item_destroy(struct entity *e);

#endif