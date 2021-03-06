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
#include "mapmanager.h"
#include "menu.h"
#include "menudriver.h"
#include "maker.h"
#include "entitymanager.h"
#include "mapgenerator.h"
#include "levelgenerator.h"
#include "debug.h"

#include "graph.h"

void game_get_actions();
static char mode;
char buf[64];
static char *gt = NULL;

void game_init(char gamemode, char newmap, int width, int height)
{
    mode = gamemode;
    switch(mode)
    {
        case NONE:
            sm_init(al_load_bitmap(s_get_full_path_with_dir("images", "0x72_DungeonTilesetII_v1.3.png")), 0, 0);
            mm_init();
            em_init();
            lg_generate_level(newmap);

            break;
        case REG:
            break;
        case MAKER:
            sm_init(al_load_bitmap(s_get_full_path_with_dir("images", "0x72_DungeonTilesetII_v1.3.png")), 0, 0);
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
            lg_destroy_level();
            debug_printf("after lg_destroy_level\n");

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
            lg_tick();
            em_tick();
            mm_update_chunks();
            mm_draw_chunks(display);
            sm_draw_sprites(display);
            break;

        case REG:
            break;

        case MAKER:
            game_get_actions();
            mm_update_chunks();
            mm_draw_chunks(display);
            sm_draw_sprites(display);
            break;
    }
}

void game_get_actions()
{
    float shift = 1, ctrl = 0, up = kb_get_up(), down = kb_get_down(), left = kb_get_left(), right = kb_get_right();

    char scroll = mouse_get_scroll();

    if(kb_get_toggle_debug())
        debug_toggle_sprites();

    if(kb_get_gettext())
    {
        memset(buf, 0, 64);
        gt = kb_get_text(buf, 64);
    }
    if(gt && !*gt)
    {
        printf("%s\n", buf);
        gt = NULL;
    }

    /*if(mouse_get_single_one())
    {
        struct tile *tile = mm_get_tile_from_rel_coordinate(mouse_get_rel_x(), mouse_get_rel_y());
        tile->damage--;
    }*/

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
            sm_move_coord(up - down, right - left);

            if(kb_get_mapsave())
                mm_save_map("map1");

            if(kb_get_tile_menu_save())
                maker_show_solid_tiles();

            if(kb_get_single_key(MISC))
                maker_save_tile_menus();

            if(kb_get_next_tile_menu())
                maker_show_tile_menu(-1);
            
            break;
    }
}