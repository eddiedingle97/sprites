#include <stdio.h>
#include <allegro5/allegro.h>
#include "entity.h"
#include "spritemanager.h"

#include "knight.c"

struct entity *knight;

void em_init()
{
    knight = create_knight();
    sm_add_sprite_to_layer(knight->sprite);
}

void em_tick()
{
    knight->behaviour(knight->sprite, knight->data);
}

void em_destroy()
{
    destroy_knight(knight);
}