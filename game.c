#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_ttf.h>
#include "list/list.h"
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
            tm_init(5);
            mm_init(map_create(5, 16, 200, 200));
            break;
        case REG:
            break;
        case MAKER:
            sm_init(0, 0);
            tm_init(5);
            mm_init(newmap ? map_create(5, 16, width, height) : map_load("./maps/map1.map"));
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
            break;
        case REG:
            break;
        case MAKER:
            maker_destroy();
            if(debug_get())
                printf("after maker_destroy\n");

            md_destroy();
            if(debug_get())
                printf("after md_destroy\n");
                
            mm_destroy();
            if(debug_get())
                printf("after mm_destroy\n");

            tm_destroy();
            if(debug_get())
                printf("after tm_destroy\n");

            sm_destroy();
            if(debug_get())
                printf("after sm_destroy\n");
            break;
    }
    
}

void game_tick(ALLEGRO_DISPLAY *display)
{
    switch(mode)
    {
        case NONE:
            game_get_actions();
            mm_update_chunks();
            tm_draw_tiles(display);
            sm_draw_sprites(display);
            break;

        case REG:
            break;

        case MAKER:
            game_get_actions();
            mm_update_chunks();
            tm_draw_tiles(display);
            sm_draw_sprites(display);
            break;
    }
}

void game_get_actions()
{
    unsigned char shift = 1, ctrl = 0, up = kb_get_up(), down = kb_get_down(), left = kb_get_left(), right = kb_get_right();
    char scroll = 0;

    scroll = mouse_get_scroll();

    if(kb_get_toggle_debug())
        tm_print_tile_maps();

    switch(mode)
    {
        case NONE:
            sm_move_coord(up, down, left, right);
            sm_set_zoom(scroll);

            if(mouse_get_single_one())
                mm_test_color_tile(sm_rel_to_global_x(mouse_get_rel_x()), sm_rel_to_global_y(mouse_get_rel_y()));
            break;

        case REG:
            break;

        case MAKER:
            md_menu_tick();
            sm_move_coord(up, down, left, right);
            sm_set_zoom(scroll);
            maker_actions();

            if(kb_get_mapsave())
                mm_save_map();

            if(kb_get_tile_menu_save())
                maker_show_solid_tiles();

            if(kb_get_gettext())
            {
                memset(buf, 0, 64);
                kb_get_text(buf, 64);
            }

            if(kb_get_next_tile_menu())
                maker_show_tile_menu(-1);
            
            break;
    }
}