#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "list/list.h"
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
#include "enums.h"

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

void maker_main_menu_frame_handler(struct menu *m);
void maker_main_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll);
void *maker_load_tile_menu_function(void *file);
void maker_frame_handler(struct menu *m);
void maker_select_handler(struct menu *m, int x, int y, char one, char two, char scroll);
void maker_edit_frame_handler(struct menu *m);
void *maker_set_edit_mode(void *a);
void *maker_set_remove_mode(void *a);
void *maker_toggle_current_tile_solid(void *a);
void *maker_toggle_current_tile_breakable(void *a);
void *maker_set_current_tile_durability(void *a);
void maker_set_current_tile(struct tile *tile);
void maker_null_current_tile();
void maker_undo_remove();
void maker_remove_edit_menu();
void maker_show_solid_tiles();

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
    ghosttile = s_malloc(sizeof(struct tile), "maker_init");
    
    struct list *makermenuitems = list_create();
    
    list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("edit tile"), maker_set_edit_mode));
    list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("remove tile"), maker_set_remove_mode));
    //list_append(makermenuitems, menu_create_menu_item(s_get_heap_string("show solid tiles"), maker_show_solid_tiles));
    
    DIR *dr = opendir(s_get_full_path("tilemenus"));
    struct dirent *de;
    char buf[128];

    if(dr)
    {
        while((de = readdir(dr)) != NULL)
        {
            if(*de->d_name != '.')
            {
                strcat(buf, "open ");
                list_append(makermenuitems, menu_create_menu_item(s_get_heap_string(strcat(buf, de->d_name)), maker_load_tile_menu_function));
                memset(buf, 0, sizeof(char) * 128);
            }
        }
        closedir(dr);
    }

    makermenu = menu_create(makermenuitems, al_load_ttf_font("./fonts/lcd.ttf", 20, 0), -WIDTH / 2, HEIGHT / 2, maker_main_menu_frame_handler, maker_main_menu_select_handler);
    md_add_menu(makermenu);

    struct list *edititems = list_create();
    list_append(edititems, menu_create_menu_item(s_get_heap_string("solid"), maker_toggle_current_tile_solid));
    list_append(edititems, menu_create_menu_item(s_get_heap_string("breakable"), maker_toggle_current_tile_breakable));
    list_append(edititems, menu_create_menu_item(s_get_heap_string("set durability"), maker_set_current_tile_durability));
    editmenu = menu_create(edititems, al_load_ttf_font("./fonts/lcd.ttf", 20, 0), 0, 0, maker_edit_frame_handler, NULL);
    editmenuactive = 0;
}

void *maker_load_tile_menu_function(void *file)
{
    char buf[128];
    memset(buf, 0, 128 * sizeof(char));
    strcat(buf, "tilemenus/");
    strcat(buf, file);
    maker_load_tile_menu_from_file(s_get_full_path(buf));
    return NULL;
}

void maker_destroy_tile(struct tile *tile)
{
    free(tile);
}

void maker_destroy_tilemenu(struct tilemenu *tm)
{
    md_remove_menu(tm);
    list_destroy_with_function(tm->tiles, (void (*)(void *))maker_destroy_tile);
    al_destroy_bitmap(tm->tilem);
    free(tm->tilemapfile);
    sm_destroy_sprite(tm->menu->frame);
    al_destroy_bitmap(tm->menu->select);
    al_destroy_font(tm->menu->font);
    md_remove_menu(tm->menu);
    free(tm->menu);
    free(tm);
}

void maker_destroy()
{
    list_destroy_with_function(tilemenus, (void (*)(void *))maker_destroy_tilemenu);
    md_remove_menu(makermenu);
    menu_destroy(makermenu);
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
            editmenu->x = mouse_get_rel_x() + editmenu->width / 2;
            editmenu->y = mouse_get_rel_y() - editmenu->height / 2;
            maker_add_edit_menu();

            if(debug_get())
                printf("%d, %d, %d\n", currenttile->tilemap_x, currenttile->tilemap_y, currenttile->tilemap_z);
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
                        struct sprite *sprite = sm_create_global_sprite(redbitmap, tm_get_tile_x(chunkleft->x, 16, c), tm_get_tile_y(chunkleft->y, 16, y), FOREGROUND, TRANSPARENT);
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

char maker_get_tile_z(char *tilemapfile)
{
    struct map *map = mm_get_top_map();
    int i;
    for(i = 0; i < map->tilemaps->size; i++)
    {
        struct tilemap *tm = list_get(map->tilemaps, i);
        if(strlen(tm->tilemapfile) == strlen(tilemapfile) && !strcmp(tm->tilemapfile, tilemapfile))
            return i;
    }
    
    struct tilemap *new = s_malloc(sizeof(struct tilemap), "maker_get_tile_z");
    new->tilemapfile = s_get_heap_string(tilemapfile);
    char buf[256];
    memset(buf, 0, 256);
    strcat(buf, "tilemenus/");
    new->bitmap = al_load_bitmap(s_get_full_path(strcat(buf, new->tilemapfile)));
    return list_append(map->tilemaps, new);
}

void maker_load_tile_menu_from_file(char *filepath)
{
    char buf[256];
    FILE *f = fopen(filepath, "r");
    if(!f)
    {
        perror("Error in maker_load_tile_menu_from_file");
        return;
    }

    struct tilemenu *tm = s_malloc(sizeof(struct tilemenu), "tm: maker_load_tile_menu_from_file");

    fgets(buf, 256, f);
    tm->tilemapfile = s_get_heap_string(buf);
    tm->tilemapfile[strlen(tm->tilemapfile) - 1] = '\0';
    ALLEGRO_BITMAP *tilemap = al_load_bitmap(s_get_full_path_with_dir("images", tm->tilemapfile));

    if(!tilemap)
    {
        fprintf(stderr, "tilemap file failed to load\n");
        return;
    }

    fgets(buf, 256, f);
    tm->tilesize = atoi(buf);
    int z = tm_add_tile_map(tm_load_tile_map(tm->tilemapfile));
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
        ALLEGRO_BITMAP *subbitmap = al_create_sub_bitmap(tilemap, tile->tilemap_x, tile->tilemap_y, tm->tilesize, tm->tilesize);
        list_append(tm->tiles, tile);
    }
    
    tm->tilem = tilemap;
    tm->menu = menu_create(tm->tiles, al_load_ttf_font("./fonts/lcd.ttf", 10, 0), topleftcoord[X], topleftcoord[Y], maker_frame_handler, maker_select_handler);
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

    printf("here\n");
    return 0;
}

void maker_load_tile_menu_from_image(char *tilemapfile, int tilesize)
{
    ALLEGRO_BITMAP *tilemap = al_load_bitmap(s_get_full_path_with_dir("images", tilemapfile));

    if(!tilemap)
    {
        fprintf(stderr, "tilemenu file failed to load\n");
        return;
    }

    struct tilemenu *tm = s_malloc(sizeof(struct tilemenu), "tm: maker_load_tile_menu_from_image");
    list_append(tilemenus, tm);
    
    int z = tm_add_tile_map(tm_load_tile_map(tilemapfile, tilesize));
    tm->tiles = list_create();

    int r, c;
    for(r = 0; r < al_get_bitmap_height(tilemap); r += tilesize)
    {
        for(c = 0; c < al_get_bitmap_width(tilemap); c += tilesize)
        {
            ALLEGRO_BITMAP *subbitmap = al_create_sub_bitmap(tilemap, c, r, tilesize, tilesize);
            struct tile *tile = s_malloc(sizeof(struct tile), "tile: maker_load_tile_menu_from_file");
            tile->tilemap_x = c;
            tile->tilemap_y = r;
            tile->tilemap_z = z;
            tile->solid = 0;
            tile->breakable = 0;
            tile->damage = 0;
            list_append(tm->tiles, tile);
        }
    }
    
    tm->tilem = tilemap;
    tm->tilesize = tilesize;
    tm->tilemapfile = s_get_heap_string(tilemapfile);
    tm->menu = menu_create(tm->tiles, al_load_ttf_font("./fonts/lcd.ttf", 10, 0), topleftcoord[X], topleftcoord[Y], maker_frame_handler, maker_select_handler);
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
        currenttilemenuindex = currenttilemenuindex + 1 % tilemenus->size;
        tm = (struct tilemenu *)list_get(tilemenus, currenttilemenuindex);
        md_remove_menu(currenttilemenu->menu);
    }

    else
    {
        currenttilemenuindex = next % tilemenus->size;
        tm = (struct tilemenu *)list_get(tilemenus, currenttilemenuindex);
        md_remove_menu(currenttilemenu->menu);
    }

    if(!tm->menu)
        tm->menu = menu_create(tm->tiles, al_load_ttf_font("./fonts/lcd.ttf", 10, 0), topleftcoord[X], topleftcoord[Y], maker_frame_handler, maker_select_handler);

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

void maker_frame_handler(struct menu *m)
{
    int w, h;
    w = WIDTH / 2 - abs(topleftcoord[X]);
    h = HEIGHT;
    m->width = w - 10;
    m->height = h - 10;
    m->x = topleftcoord[X] + w / 2;
    m->y = topleftcoord[Y] - h / 2;
    m->movable = 0;

    struct tilemap *tm = tm_get_tile_map_for_tile(m->items->head->p);

    ALLEGRO_BITMAP *f = al_create_bitmap(m->width, m->height);
    al_set_target_bitmap(f);
    al_clear_to_color(WHITE);

    int r, c = 0, ncolumns;
    ncolumns = (m->width - 10) / TILESIZE;
    for(r = 0; ncolumns * r < m->items->size; r++)
    {
        for(c = 0; c < ncolumns && ncolumns * r + c < m->items->size; c++)
        {
            struct tile *t = (struct tile *)list_get(m->items, ncolumns * r + c);
            al_draw_scaled_bitmap(tm->bitmap, t->tilemap_x, t->tilemap_y, tm->tilesize, tm->tilesize, 5 + c * TILESIZE, 5 + r * TILESIZE, TILESIZE, TILESIZE, 0);
            //al_draw_scaled_bitmap(t->sprite->bitmap, 0, 0, al_get_bitmap_width(t->sprite->bitmap), al_get_bitmap_height(t->sprite->bitmap), 5 + c * TILESIZE, 5 + r * TILESIZE, al_get_bitmap_width(t->sprite->bitmap) * 2, al_get_bitmap_height(t->sprite->bitmap) * 2, 0);
        }
    }

    if(!m->frame)
        m->frame = sm_create_sprite(f, m->x, m->y, MENU, CENTERED | NOZOOM);
    else
        sm_update_sprite(m->frame, f, m->x, m->y);

    if(!m->select)
    {
        ALLEGRO_BITMAP *s = al_create_bitmap(TILESIZE, TILESIZE);
        al_set_target_bitmap(s);
        al_lock_bitmap(s, 0, 0);

        int thick = 2;
        for(r = 0; r < TILESIZE; r++)
        {
            for(c = 0; c < TILESIZE; c++)
            {
                if(r < thick)
                    al_draw_pixel(c, r, BLACK);
                else if(r > TILESIZE - 1 - thick)
                    al_draw_pixel(c, r, BLACK);
                else if(c < thick)
                    al_draw_pixel(c, r, BLACK);
                else if(c > TILESIZE - 1 - thick)
                    al_draw_pixel(c, r, BLACK);
            }
        }
        al_unlock_bitmap(s);
        m->select = s;
    }
}

void maker_select_handler(struct menu *m, int x, int y, char one, char two, char scroll)
{
    m->framehandler(m);
    y -= 5;
    x -= 5;

    int ncolumns = (m->width - 10) / TILESIZE;

    y /= TILESIZE;
    x /= TILESIZE;

    al_set_target_bitmap(m->frame->bitmap);
    //al_draw_scaled_bitmap(m->select, 0, 0, al_get_bitmap_width(m->select), al_get_bitmap_height(m->select), x * TILESIZE + 5, y * TILESIZE + 5, al_get_bitmap_width(m->select) * 2, al_get_bitmap_height(m->select) * 2, 0);
    al_draw_bitmap(m->select, x * TILESIZE + 5, y * TILESIZE + 5, 0);
    struct node *node = list_get_node(m->items, y * ncolumns + x);

    if(node)
    {
        if(mode == SWAP && node != grabbedtilenode)
        {
            list_swap_node(m->items, grabbedtilenode, node);
        }

        if(mode == SWAP && node == grabbedtilenode)
        {
            grabbedtilenode->p = currenttile;
            grabbedtilenode = NULL;
            mode = PLACE;
        }

        else if(one)
        {
                switch(mode)
                {
                    case 0:
                        break;
                    case PLACE:
                    case EDIT: 
                        maker_set_grab_mode(node);
                        break;
                    case REMOVE: 
                        list_push(deletedtiles, node->p);
                        list_delete_node(m->items, node);
                        break;
                }
        }

        else if(two)
        {
            switch(mode)
            {
                case 0:
                    break;
                case PLACE:
                    maker_set_edit_mode(NULL);
                case EDIT:
                    currenttile = node->p;
                    editmenu->x = m->x + (x - m->width / 2);
                    editmenu->y = m->y - y;
                    maker_add_edit_menu();
                    break;
                case REMOVE:
                    break;
            }
        }
    }
}

void maker_edit_frame_handler(struct menu *m)
{
    m->height = al_get_font_line_height(m->font) * (m->items->size + 2);
    m->width = 250;
    m->movable = 1;

    ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
    al_set_target_bitmap(frame);
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

    if(!m->frame)
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, CENTERED | NOZOOM);
    else
        sm_update_sprite(m->frame, frame, m->x, m->y);
}

void maker_main_menu_frame_handler(struct menu *m)
{
    m->height = al_get_font_line_height(m->font) * (m->items->size + 2);
    m->width = 400;
    m->movable = 0;

    ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
    al_set_target_bitmap(frame);
    al_clear_to_color(WHITE);

    int i, texty = al_get_font_line_height(m->font);
    struct node *node = m->items->head;
    for(i = 0; i < m->items->size; i++)
    {
        struct menuitem *mi = (struct menuitem *)node->p;
        al_draw_text(m->font, BLACK, 10, texty, 0, mi->entry);
        texty += al_get_font_line_height(m->font);
        node = node->next;
    }
    
    if(!m->frame)
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, NOZOOM);
    else
    {
        sm_update_sprite(m->frame, frame, m->x, m->y);
    }
}

void maker_main_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll)
{
    if(!m->select)
    {
        m->select = al_create_bitmap(180, al_get_font_line_height(m->font));
        al_set_target_bitmap(m->select);
        al_lock_bitmap(m->select, 0, 0);
        int r, c;
        for(r = 0; r < al_get_bitmap_height(m->select); r++)
            for(c = 0; c < al_get_bitmap_width(m->select); c++)
            {
                if(c == 0 || c == al_get_bitmap_width(m->select) - 1)
                    al_draw_pixel(c, r, BLACK);

                if(r == 0 || r == al_get_bitmap_height(m->select) - 1)
                    al_draw_pixel(c, r, BLACK);
            }
        al_unlock_bitmap(m->select);
    }

    int item = y / al_get_font_line_height(m->font);
    m->framehandler(m);

    al_set_target_bitmap(m->frame->bitmap);
    if(item > 0 && item < m->items->size + 1)
        al_draw_bitmap(m->select, 10, item * al_get_font_line_height(m->font), 0);

    item--;
    struct menuitem *mi = list_get(m->items, item);
    if(mi && item > -1 && one)
    {
        if(!strncmp(mi->entry, "open ", 5 * sizeof(char)))
        {
            mi->func(mi->entry + 5 * sizeof(char));
            menu_destroy_menu_item(list_delete(m->items, item));
        }
        else
            mi->func(NULL);
    } 
}

void maker_set_current_tile(struct tile *tile)
{
    ALLEGRO_BITMAP *bit = tm_get_tile_bitmap(tile);
    mouse_set_bitmap(bit);
    currenttile = tile;
}

void maker_null_current_tile()
{
    mouse_destroy();
    currenttile = NULL;
}