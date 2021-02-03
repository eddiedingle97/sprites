#ifndef __MAKER_H__
#define __MAKER_H__

struct tilemenu
{
    ALLEGRO_BITMAP *tilem;
    struct list *tiles;
    struct menu *menu;
    char *tilemapfile;
    int tilesize;
};

void maker_init();
void maker_destroy();
void maker_load_tile_menu_from_image(char *tilemapfile, int tilesize);
void maker_load_tile_menu_from_file(char *filepath);
void maker_actions();
int maker_save_tile_menus();
void maker_show_tile_menu(int next);
void maker_show_solid_tiles();

#endif