#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include "list.h"
#include "sprites.h"
#include "spritemanager.h"
#include "game.h"
#include "mouse.h"
#include "keyboard.h"
#include "map.h"
#include "tilemanager.h"
#include "mapmanager.h"
#include "menu.h"
#include "menudriver.h"
#include "maker.h"
#include "movementandcollision.h"
#include "entitymanager.h"
#include "debug.h"

void game_get_actions();
static char mode;
char buf[64];

void game_init(char gamemode, char newmap, int width, int height)
{
    mode = gamemode;
    switch(mode)
    {
        case NONE:
            sm_init(0, 0);
            newmap ? mm_init("new", 5, 16, width, height) : mm_init("map1");
            mc_init();
            em_init();
            break;
        case REG:
            break;
        case MAKER:
            sm_init(0, 0);
            newmap ? mm_init("new", 5, 16, width, height) : mm_init("map1");
            md_init();
            maker_init();
            break;
    }
}

void game_destroy()
{
    switch(mode)
    {
        case NONE:
            em_destroy();
            debug_printf("after em_destroy\n");

            mm_destroy();
            debug_printf("after mm_destroy\n");

            sm_destroy();
            debug_printf("after sm_destroy\n");

            break;
        case REG:
            break;
        case MAKER:

            maker_destroy();
            debug_printf("after maker_destroy\n");

            md_destroy();
            debug_printf("after md_destroy\n");

            mm_destroy();
            debug_printf("after mm_destroy\n");

            sm_destroy();
            debug_printf("after sm_destroy\n");
            break;
    }
    
}

void game_tick(ALLEGRO_DISPLAY *display)
{
    switch(mode)
    {
        case NONE:
            game_get_actions();
            em_tick();
            mm_update_chunks();
            tm_draw_chunks(display);
            sm_draw_sprites(display);
            break;

        case REG:
            break;

        case MAKER:
            game_get_actions();
            mm_update_chunks();
            tm_draw_chunks(display);
            sm_draw_sprites(display);
            break;
    }
}

void game_get_actions()
{
    float shift = 1, ctrl = 0, up = kb_get_up(), down = kb_get_down(), left = kb_get_left(), right = kb_get_right();
    char scroll = 0;
    float coef = .5;

    scroll = mouse_get_scroll();

    if(kb_get_toggle_debug())
        debug_toggle_sprites();

    if(kb_get_mapsave())
        mm_save_map("map1");

    switch(mode)
    {
        case NONE:
            sm_set_zoom(scroll);
            break;

        case REG:
            break;

        case MAKER:
            md_menu_tick();
            sm_set_zoom(scroll);
            maker_actions();
            mc_do_movement(NULL, up, down, left, right);

            if(kb_get_mapsave())
                mm_save_map("map1");

            if(kb_get_tile_menu_save())
                maker_show_solid_tiles();

            if(kb_get_gettext())
            {
                memset(buf, 0, 64);
                kb_get_text(buf, 64);
            }

            if(kb_get_single_key(MISC))
                maker_save_tile_menus();

            if(kb_get_next_tile_menu())
                maker_show_tile_menu(-1);
            
            break;
    }
}