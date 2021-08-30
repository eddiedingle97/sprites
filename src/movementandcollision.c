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

    debug_add_sprite(collisionbox);
    collision = 1;
}

int mc_do_entity_movement(struct sprite *sprite, float dx, float dy)
{
    struct tile *currenttile = mm_get_tile(sprite->x, sprite->y);
    if(collision && currenttile)
    {
        struct tile *nexttile = mm_get_tile(sprite->x + dx, sprite->y);
        if(nexttile && nexttile->solid)
        {
            dx = 0;
        }
        
        nexttile = mm_get_tile(sprite->x, sprite->y + dy);
        if(nexttile && nexttile->solid)
        {
            dy = 0;
        }
    }

    if(sprite)
    {
        printf("here dx: %.2f, dy: %.2f, %.2f, %.2f\n", dx, dy, sprite->x, sprite->y);
        sprite->x += dx;
        sprite->y += dy;
    }

    return dx || dy;
}

int mc_do_main_movement(struct sprite *sprite, float up, float down, float left, float right)
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