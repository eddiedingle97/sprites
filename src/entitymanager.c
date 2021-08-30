#include <stdio.h>
#include <allegro5/allegro.h>
#include "spritemanager.h"
#include "entity.h"

#include "debug.h"

#include "knight.c"
#include "orc.c"

struct entity *knight;
struct entity *orc;
ALLEGRO_BITMAP *spritesheet;

void em_init()
{
    spritesheet = al_load_bitmap(s_get_full_path_with_dir("images", "DungeonAllEntities.png"));
    if(!spritesheet)
        debug_perror("Spritesheet failed to load in em_init\n");
    knight = knight_create(spritesheet);
    orc = orc_create(spritesheet);
    struct orcdata *data = orc->data;
    data->target = knight;
    sm_add_sprite_to_layer(knight->sprite);
    sm_add_sprite_to_layer(orc->sprite);
}

void em_tick()
{
    knight->behaviour(knight->sprite, knight->data);
    orc->behaviour(orc->sprite, orc->data);
}

void em_destroy()
{
    knight_destroy(knight);
    orc_destroy(orc);
    al_destroy_bitmap(spritesheet);
}