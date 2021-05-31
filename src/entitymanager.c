#include <stdio.h>
#include <allegro5/allegro.h>
#include "entity.h"
#include "spritemanager.h"
#include "debug.h"

#include "knight.c"

struct entity *knight;
ALLEGRO_BITMAP *spritesheet;

void em_init()
{
    spritesheet = al_load_bitmap(s_get_full_path_with_dir("images", "DungeonAllEntities.png"));
    if(!spritesheet)
        debug_perror("Spritesheet failed to load in em_init\n");
    knight = knight_create(spritesheet);
    sm_add_sprite_to_layer(knight->sprite);
}

void em_tick()
{
    knight->behaviour(knight->sprite, knight->data);
}

void em_destroy()
{
    knight_destroy(knight);
    al_destroy_bitmap(spritesheet);
}