#include <stdio.h>
#include <allegro5/allegro.h>
#include "list.h"
#include "spritemanager.h"
#include "movementandcollision.h"
#include "map.h"
#include "mapmanager.h"
#include "debug.h"
#include "colors.h"

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

void mc_init()
{
    ALLEGRO_BITMAP *boxbitmap = al_create_bitmap(collisionboxsize, collisionboxsize);
    al_set_target_bitmap(boxbitmap);
    al_lock_bitmap(boxbitmap, 0, 0);
    int i, thick;
    for(thick = 0; thick < 1; thick++)
        for(i = 0; i < collisionboxsize; i++)
        {
            al_draw_pixel(i, thick, RED);
            al_draw_pixel(i, collisionboxsize - 1 - thick, RED);
            al_draw_pixel(thick, i, RED);
            al_draw_pixel(collisionboxsize - 1 - thick, i, RED);
        }
    al_unlock_bitmap(boxbitmap);
    collisionbox = sm_create_sprite(boxbitmap, 0, 0, PLAYER, CENTERED);

    if(debug_get())
        sm_add_sprite_to_layer(collisionbox);
    collision = 1;
}

int mc_do_movement(struct sprite *sprite, float up, float down, float left, float right)
{
    float xcoord = sm_get_coord(X), ycoord = sm_get_coord(Y), dx = right - left, dy = up - down;
    struct tile *currenttile = mm_get_tile(xcoord, ycoord);
    if(collision)
    {
        struct tile *nexttile = mm_get_tile(xcoord + dx, ycoord);
        if(nexttile->solid)
        {
            dx = 0;
        }
        
        nexttile = mm_get_tile(xcoord, ycoord + dy);
        if(nexttile->solid)
        {
            dy = 0;
        }
    }

    if(sprite)
    {
        sprite->x += dx;
        sprite->y += dy;
    }
    sm_move_coord(dx, dy);
    return dx || dy;
}