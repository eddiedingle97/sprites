#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "list.h"
#include "spritemanager.h"
#include "sprites.h"
#include "map.h"
#include "tilemanager.h"
#include "mapmanager.h"
#include "menu.h"
#include "menudriver.h"
#include "maker.h"
#include "mouse.h"
#include "keyboard.h"
#include "debug.h"
#include "colors.h"

static struct list *tilemenus;
static int currenttilemenuindex;
static struct tilemenu *currenttilemenu;
static struct tile *currenttile;
static const int topleftcoord[] = {HEIGHT / 2, HEIGHT / 2};
static const int TILESIZE = 32;
static struct menu *makermenu;
static struct menu *editmenu;
static char editmenuactive;
static char mode;
static struct list *deletedtiles;
static struct node *grabbedtilenode;
static struct tile *ghosttile;
static struct list *foregroundsprites;
enum MAKERMODES {PLACE = 1, EDIT = 2, REMOVE = 4, GRAB = 8, SWAP = 16};

void maker_main_menu_frame_handler(struct menu *m);
void maker_main_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll);
void maker_tile_menu_frame_handler(struct menu *m);
void maker_tile_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll);
void maker_edit_frame_handler(struct menu *m);
void *maker_set_edit_mode(void *a);
void *maker_set_remove_mode(void *a);
void *maker_toggle_current_tile_solid(void *a);
void *maker_toggle_current_tile_breakable(void *a);
void *maker_set_current_tile_durability(void *a);
void maker_set_current_tile(struct tile *tile);
void maker_add_edit_menu();
void maker_set_grab_mode(struct node *node);
void maker_null_current_tile();
void maker_undo_remove();
void maker_remove_edit_menu();
void maker_show_solid_tiles();

#include "tilemenu.c"
#include "makermenu.c"

void maker_init()
{
    tilemenus = list_create();
    deletedtiles = list_create();
    foregroundsprites = NULL;
    currenttilemenuindex = -1;
    currenttilemenu = NULL;
    currenttile = NULL;
    grabbedtilenode = NULL;
    mode = EDIT;
    ghosttile = s_malloc(sizeof(struct tile), "ghosttile: maker_init");
    memset(ghosttile, 0, sizeof(struct tile));
    
    struct list *makermenuitems = list_create();
    
    list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("edit tile"), maker_set_edit_mode));
    list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("remove tile"), maker_set_remove_mode));
    //list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("show solid tiles"), maker_show_solid_tiles));

    struct menuentrysub *image = s_malloc(sizeof(struct menuentrysub), "image: maker_init");
    image->title = "open tilemenu from image";
    struct list *images = s_get_file_list_from_dir("images");
    struct list *imagemenuitems = list_create();
    struct node *node;
    for(node = images->head; node != NULL; node = node->next)
        list_append(imagemenuitems, menu_create_menu_item(node->p, maker_load_image_function));
    list_destroy(images);
    image->menu = menu_create_default(imagemenuitems, al_load_ttf_font("./fonts/lcd.ttf", 15, 0), -WIDTH / 2 + 400, 400);
    list_append(makermenuitems, menu_create_menu_item(image, (void *(*)(void *))maker_open_sub_menu));

    struct menuentrysub *file = s_malloc(sizeof(struct menuentrysub), "file: maker_init");
    file->title = "open tilemenu from file";
    struct list *files = s_get_file_list_from_dir("tilemenus");
    struct list *filemenuitems = list_create();
    for(node = files->head; node != NULL; node = node->next)
        list_append(filemenuitems, menu_create_menu_item(node->p, maker_load_tile_menu_function));
    list_destroy(files);
    file->menu = menu_create_default(filemenuitems, al_load_ttf_font("./fonts/lcd.ttf", 15, 0), -WIDTH / 2 + 400, 400);
    list_append(makermenuitems, menu_create_menu_item(file, (void *(*)(void *))maker_open_sub_menu));

    makermenu = menu_create(makermenuitems, al_load_ttf_font("./fonts/lcd.ttf", 20, 0), -WIDTH / 2, HEIGHT / 2, maker_main_menu_frame_handler, maker_main_menu_select_handler);
    md_add_menu(makermenu);

    struct list *edititems = list_create();
    list_append(edititems, menu_create_menu_item(s_get_heap_string("solid"), maker_toggle_current_tile_solid));
    list_append(edititems, menu_create_menu_item(s_get_heap_string("breakable"), maker_toggle_current_tile_breakable));
    list_append(edititems, menu_create_menu_item(s_get_heap_string("set durability"), maker_set_current_tile_durability));
    editmenu = menu_create(edititems, al_load_ttf_font("./fonts/lcd.ttf", 20, 0), 0, 0, maker_edit_frame_handler, NULL);
    editmenuactive = 0;
}

void maker_destroy_tile(struct tile *tile)
{
    free(tile);
}

void maker_destroy_tile_menu(struct tilemenu *tm)
{
    md_remove_menu(tm);
    list_destroy_with_function(tm->tiles, (void (*)(void *))maker_destroy_tile);
    free(tm->tilemapfile);
    sm_destroy_sprite(tm->menu->frame);
    al_destroy_bitmap(tm->menu->select);
    al_destroy_font(tm->menu->font);
    md_remove_menu(tm->menu);
    free(tm->menu);
    free(tm);
}

void maker_destroy_maker_menu(struct menu *m)
{
    al_destroy_font(m->font);

    struct node *node = m->items->head;
    menu_destroy_menu_item(node->p);
    node = node->next;

    menu_destroy_menu_item(node->p);
    node = node->next;

    struct menuitem *mi = node->p;
    struct menuentrysub *mes = mi->entry;
    menu_destroy(mes->menu);
    md_remove_menu(mes->menu);
    free(mi);

    node = node->next;
    mi = node->p;
    mes = mi->entry;
    menu_destroy(mes->menu);
    md_remove_menu(mes->menu);
    free(mi);

    free(m);
    md_remove_menu(m);
}

void maker_destroy()
{
    list_destroy_with_function(tilemenus, (void (*)(void *))maker_destroy_tile_menu);
    md_remove_menu(makermenu);
    maker_destroy_maker_menu(makermenu);
    maker_remove_edit_menu();
    menu_destroy(editmenu);
    list_destroy_with_function(deletedtiles, (void(*)(void *))maker_destroy_tile);
    maker_destroy_tile(ghosttile);
    if(foregroundsprites)
        list_destroy_with_function(foregroundsprites,(void (*)(void *))sm_destroy_sprite);
}

void maker_remove_edit_menu()
{
    if(editmenuactive)
    {
        md_remove_menu(editmenu);
        editmenuactive = 0;
    }
}

void maker_add_edit_menu()
{
    if(!editmenuactive)
    {
        md_add_menu(editmenu);
        editmenuactive = 1;
    }
    editmenu->framehandler(editmenu);
}

void maker_actions()
{
    unsigned short action = mode | (mouse_get_one() << 5) | (mouse_get_single_one() << 6) | (mouse_get_single_two() << 7) | (md_menu_hover() ? 1 << 8 : 0) | (kb_get_undo() << 9);

    switch(action)
    {
        case 8: //grab mode
            mode = SWAP;
            break;

        case 16: //swap mode
            mode = EDIT;
            grabbedtilenode->p = currenttile;
            grabbedtilenode = NULL;
            maker_null_current_tile();
            break;

        case 97: //one & single one & place mode
            mm_update_tile(sm_rel_to_global_x(mouse_get_rel_x()), sm_rel_to_global_y(mouse_get_rel_y()), currenttile);
            break;
        
        case 129: //single two & place mode
            maker_set_edit_mode(NULL);
            //NO BREAK

        case 130: //single two & edit mode
            currenttile = mm_get_tile(sm_rel_to_global_x(mouse_get_rel_x()), sm_rel_to_global_y(mouse_get_rel_y()));
            editmenu->x = mouse_get_rel_x();
            editmenu->y = mouse_get_rel_y();
            maker_add_edit_menu();

            debug_printf("%d, %d, %d\n", currenttile->tilemap_x, currenttile->tilemap_y, currenttile->tilemap_z);
            break;

        case 264: //menuhover & grab mode
            mode = SWAP;
            break;

        case 272: //grab mode & menuhover
            mode = EDIT;
            grabbedtilenode->p = currenttile;
            grabbedtilenode = NULL;
            maker_null_current_tile();
            break;

        case 516: //undo & remove mode
            //NO BREAK

        case 772: //undo & menu hover
            maker_undo_remove();
            break;
    }
}

void maker_show_solid_tiles()
{
    if(foregroundsprites)
    {
        foregroundsprites = list_destroy_with_function(foregroundsprites,(void (*)(void *))sm_destroy_sprite);
        return;
    }
    struct map *map = mm_get_top_map();
    struct chunk **corners = mm_get_corners();
    
    foregroundsprites = list_create();
    struct chunk *chunkleft = corners[TOPLEFT];
    int y = 0;
    int r, c;

    while(chunkleft && chunkleft->index_y != corners[BOTTOMLEFT]->index_y + 1)
    {
        while(chunkleft && chunkleft->index_x != corners[TOPRIGHT]->index_x + 1)
        {
            for(r = 0; r < map->chunksize; r++)
            {
                for(c = 0; c < map->chunksize; c++)
                {
                    if(chunkleft->tiles[r][c].solid)
                    {
                        ALLEGRO_BITMAP *redbitmap = al_create_bitmap(map->tilesize, map->tilesize);
                        al_set_target_bitmap(redbitmap);
                        al_clear_to_color(RED);
                        struct sprite *sprite = sm_create_global_sprite(redbitmap, tm_get_tile_x(chunkleft->x, 16, c), tm_get_tile_y(chunkleft->y, 16, r), FOREGROUND, 0);
                        sm_add_sprite_to_layer(sprite);
                        list_append(foregroundsprites, sprite);
                    }
                }
            }
            y = chunkleft->index_y;
            chunkleft = map_get_chunk_from_index(map, chunkleft->index_x + 1, chunkleft->index_y);
        }
        if(chunkleft)
            chunkleft = map_get_chunk_from_index(map, corners[TOPLEFT]->index_x, chunkleft->index_y + 1);
        else
            chunkleft = map_get_chunk_from_index(map, corners[TOPLEFT]->index_x, y + 1);
    }
}

void maker_set_place_mode(struct tile *tile)
{
    maker_remove_edit_menu();
    maker_set_current_tile(tile);
    mode = PLACE;
}

void maker_undo_remove()
{
    struct tilemenu *tm = list_get(tilemenus, currenttilemenuindex);
    struct tile *tile = list_pop(deletedtiles);
    if(tile)
        list_append(tm->tiles, tile);
}

void maker_load_tile_menu_from_file(char *filepath)
{
    char buf[256];
    memset(buf, 0, 256);
    FILE *f = fopen(filepath, "r");
    if(!f)
    {
        perror("Error in maker_load_tile_menu_from_file");
        return;
    }

    struct tilemenu *tm = s_malloc(sizeof(struct tilemenu), "tm: maker_load_tile_menu_from_file");

    fgets(buf, 256, f);
    buf[strlen(buf) - 1] = '\0';
    tm->tilemapfile = s_get_heap_string(buf);

    fgets(buf, 256, f);
    tm->tilesize = atoi(buf);
    int z = tm_load_tile_map(tm->tilemapfile, tm->tilesize);
    tm->tiles = list_create();

    while(fgets(buf, 256, f) != NULL)
    {
        struct tile *tile = s_malloc(sizeof(struct tile), "tile: maker_load_tile_menu_from_file");
        tile->tilemap_x = atoi(strtok(buf, ","));
        tile->tilemap_y = atoi(strtok(NULL, ","));
        tile->tilemap_z = z;
        strtok(NULL, ",");
        tile->solid = atoi(strtok(NULL, ","));
        tile->breakable = atoi(strtok(NULL, ","));
        tile->damage = atoi(strtok(NULL, ","));
        list_append(tm->tiles, tile);
    }
    
    tm->menu = menu_create(tm->tiles, al_load_ttf_font("./fonts/lcd.ttf", 10, 0), topleftcoord[X], topleftcoord[Y], maker_tile_menu_frame_handler, maker_tile_menu_select_handler);
    fclose(f);
    list_append(tilemenus, tm);
}

int maker_save_tile_menus()
{
    struct node *n;
    for(n = tilemenus->head; n != NULL; n = n->next)
    {
        struct tilemenu *tm = (struct tilemenu *)n->p;
        struct node *node;
        struct tile *tile;
        int count;
        char buf[1024];
        FILE *f = fopen("./tilemenus/tilemenu1", "w");

        if(!f)
        {
            perror("error in maker_save_tile_menus");
            return -1;
        }

        fwrite(tm->tilemapfile, sizeof(char), strlen(tm->tilemapfile), f);
        count = sprintf(buf, "\n%d\n", tm->tilesize);
        fwrite(buf, sizeof(char), count, f);

        for(node = tm->tiles->head; node != NULL; node = node->next)
        {
            tile = (struct tile *)node->p;
            
            if((count = sprintf(buf, "%d,%d,%d,%d,%d,%d\n", tile->tilemap_x, tile->tilemap_y, tile->tilemap_z, tile->solid, tile->breakable, tile->damage)) < 0)
            {
                perror("Error in maker_save_tilemaps");
                return -2;
            }

            fwrite(buf, sizeof(char), count, f);
        }

        fclose(f);
    }

    return 0;
}

void maker_load_tile_menu_from_image(char *tilemapfile, int tilesize)
{
    struct tilemenu *tm = s_malloc(sizeof(struct tilemenu), "tm: maker_load_tile_menu_from_image");
    list_append(tilemenus, tm);
    
    int z = tm_load_tile_map(tilemapfile, tilesize);
    tm->tiles = list_create();

    ALLEGRO_BITMAP *tilemap = al_load_bitmap(s_get_full_path_with_dir("images", tilemapfile));

    int r, c;
    for(r = 0; r < al_get_bitmap_height(tilemap); r += tilesize)
    {
        for(c = 0; c < al_get_bitmap_width(tilemap); c += tilesize)
        {
            struct tile *tile = s_malloc(sizeof(struct tile), "tile: maker_load_tile_menu_from_image");
            tile->tilemap_x = c;
            tile->tilemap_y = r;
            tile->tilemap_z = z;
            tile->solid = 0;
            tile->breakable = 0;
            tile->damage = 0;
            list_append(tm->tiles, tile);
        }
    }

    al_destroy_bitmap(tilemap);
    tm->tilesize = tilesize;
    tm->tilemapfile = s_get_heap_string(tilemapfile);
    tm->menu = menu_create(tm->tiles, al_load_ttf_font("./fonts/lcd.ttf", 10, 0), topleftcoord[X], topleftcoord[Y], maker_tile_menu_frame_handler, maker_tile_menu_select_handler);
}

void maker_show_tile_menu(int next)
{
    struct tilemenu *tm = NULL;

    if(!tilemenus->size)
    {
        fprintf(stderr, "No tilemenus to show\n");
        return;
    }

    if(currenttilemenuindex == -1 && !currenttilemenu)
    {
        currenttilemenuindex = 0;
        tm = (struct tilemenu *)tilemenus->head->p;
    }

    else if(next < 0)
    {
        currenttilemenuindex = (currenttilemenuindex + 1) % tilemenus->size;
        tm = (struct tilemenu *)list_get(tilemenus, currenttilemenuindex);
        md_remove_menu(currenttilemenu->menu);
    }

    else
    {
        currenttilemenuindex = next % tilemenus->size;
        tm = (struct tilemenu *)list_get(tilemenus, currenttilemenuindex);
        md_remove_menu(currenttilemenu->menu);
    }

    md_add_menu(tm->menu);
    currenttilemenu = tm;
    list_destroy_with_function(deletedtiles, (void (*)(void *))maker_destroy_tile);
    deletedtiles = list_create();
}

void *maker_set_edit_mode(void *a)
{
    maker_null_current_tile();
    mode = EDIT;
    return NULL;
}

void *maker_set_remove_mode(void *a)
{
    maker_remove_edit_menu();
    maker_null_current_tile();
    mode = REMOVE;
    return NULL;
}

void maker_set_grab_mode(struct node *node)
{
    maker_set_current_tile(node->p);
    maker_remove_edit_menu();
    grabbedtilenode = node;
    mode = GRAB;// MODE
    node->p = ghosttile;
}

void *maker_toggle_current_tile_solid(void *a)
{
    currenttile->solid = !currenttile->solid;
    return NULL;
}

void *maker_toggle_current_tile_breakable(void *a)
{
    currenttile->breakable = !currenttile->breakable;
    return NULL;
}

void *maker_set_current_tile_durability(void *a)
{
    /*if(scanf("%3hhd", &currenttile->damage) < 1)
        currenttile->damage = 1;*/

    return NULL;
}

void maker_edit_frame_handler(struct menu *m)
{
    m->height = al_get_font_line_height(m->font) * (m->items->size + 2);
    m->width = 250;
    m->movable = 1;

    if(!m->frame)
    {
        ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, NOZOOM);
    }
    
    al_set_target_bitmap(m->frame->bitmap);
    al_clear_to_color(WHITE);

    if(currenttile)
    {
        int texty = al_get_font_line_height(m->font);
        struct node *node = m->items->head;
        char buf[32], concat[8];
        memset(buf, 0, 32);
        memset(concat, 0, 8);

        struct menuitem *mi = (struct menuitem *)node->p;
        memcpy(buf, mi->entry, strlen(mi->entry)); 
        sprintf(concat, ": %hhd", currenttile->solid);
        strcat(buf, concat);
        al_draw_text(m->font, BLACK, 10, texty, 0, buf);
        texty += al_get_font_line_height(m->font);
        node = node->next;
        memset(buf, 0, 32);
        memset(concat, 0, 8);

        mi = (struct menuitem *)node->p;
        memcpy(buf, mi->entry, strlen(mi->entry));
        sprintf(concat, ": %hhd", currenttile->breakable);
        strcat(buf, concat);
        al_draw_text(m->font, BLACK, 10, texty, 0, buf);
        texty += al_get_font_line_height(m->font);
        node = node->next;
        memset(buf, 0, 32);
        memset(concat, 0, 8);

        mi = (struct menuitem *)node->p;
        memcpy(buf, mi->entry, strlen(mi->entry));
        sprintf(concat, ": %hhd", currenttile->damage);
        strcat(buf, concat);
        al_draw_text(m->font, BLACK, 10, texty, 0, buf);
    }

    m->frame->x = m->x;
    m->frame->y = m->y;
}

void maker_set_current_tile(struct tile *tile)
{
    ALLEGRO_BITMAP *bit = tm_get_tile_bitmap(tile);
    mouse_set_bitmap(bit);
    currenttile = tile;
}

void maker_null_current_tile()
{
    maker_remove_edit_menu();
    mouse_destroy();
    currenttile = NULL;
}