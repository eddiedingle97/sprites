#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "list/list.h"
#include "spritemanager.h"
#include "map.h"
#include "tilemanager.h"
#include "debug.h"
#include "colors.h"

static struct list *chunks;
static struct tilemap **tilemaps;
static int tilemapssize, chunksize;
enum TILEMAPZ {EMPTY, ERROR};

struct tilemap *tm_load_tile_map_from_file(char *tilemapfile, int tilesize);
int tm_add_tile_map(struct tilemap *tm);

void tm_init(int csize)
{
    chunks = list_create();
    tilemapssize = 0;
    tm_add_tile_map(tm_load_tile_map_from_file("defaulttile.bmp", 16));
    tm_add_tile_map(tm_load_tile_map_from_file("purplebitmap.bmp", 16));
    chunksize = csize;
}

void tm_destroy_tile_map(struct tilemap *tm)
{
    free(tm->tilemapfile);
    al_destroy_bitmap(tm->bitmap);
    free(tm);
}

void tm_destroy()
{
    printf("here %d\n", tilemapssize);
    list_destroy(chunks);
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        printf("here %d %p\n", i, tilemaps[i]);
        tm_destroy_tile_map(tilemaps[i]);
    }
    free(tilemaps);
}

void tm_print_tile_maps()
{
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        struct tilemap *tm = tilemaps[i];
        printf("%s, %p, %d\n", tm->tilemapfile, tm->bitmap, tm->tilesize);
    }
}

int tm_load_tile_map(char *tilemapfile, int tilesize)
{
    int z = tm_get_tile_map_z(tilemapfile);
    if(z == 1)
        return tm_add_tile_map(tm_load_tile_map_from_file(tilemapfile, tilesize));

    return z;
}

struct tilemap *tm_load_tile_map_from_file(char *tilemapfile, int tilesize)
{
    int z = tm_get_tile_map_z(tilemapfile);
    if(z != ERROR)
        return tilemaps[z];

    ALLEGRO_BITMAP *bitmap = al_load_bitmap(s_get_full_path_with_dir("images", tilemapfile));
    if(!bitmap)
    {
        fprintf(stderr, "Tile map file \"%s\" failed to load\n", tilemapfile);
        return NULL;
    }

    struct tilemap *out = s_malloc(sizeof(struct tilemap), "tm_load_tile_map");
    out->bitmap = bitmap;
    out->tilemapfile = s_get_heap_string(tilemapfile);
    out->tilesize = tilesize;
    return out;
}

void tm_add_tile_map_list(struct list *l)
{
    struct node *node;
    struct tilemap *tm = NULL;
    char nomatch = 1;
    int i;
    for(node = l->head; node != NULL; node = node->next)
    {
        for(i = 0; i < tilemapssize; i++)
        {
            tm = node->p;
            if(strlen(tm->tilemapfile) == strlen(tilemaps[i]->tilemapfile) && !strcmp(tm->tilemapfile, tilemaps[i]->tilemapfile))
                nomatch = 0;
        }

        if(nomatch)
            tm_add_tile_map(tm);
        nomatch = 1;
    }
}

struct tilemap *tm_get_tile_map_for_tile(struct tile *tile)
{
    return tilemaps[tile->tilemap_z];
}

struct tilemap *tm_get_tile_map_from_z(int z)
{
    return tilemaps[z];
}

ALLEGRO_BITMAP *tm_get_tile_bitmap(struct tile *tile)
{
    return al_create_sub_bitmap(tilemaps[tile->tilemap_z]->bitmap, tile->tilemap_x, tile->tilemap_y, tilemaps[tile->tilemap_z]->tilesize, tilemaps[tile->tilemap_z]->tilesize);
}

int tm_add_tile_map(struct tilemap *tm)
{
    if(!tm)
    {
        fprintf(stderr, "Received null pointer in tm_add_tilemap\n");
        return ERROR;
    }

    tilemaps = s_realloc(tilemaps, sizeof(struct tilemap *), "tm_add_tile_map");
    tilemaps[tilemapssize] = tm;
    return tilemapssize++;
}

int tm_get_tile_map_z(char *tilemapfile)
{
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        if(strlen(tilemapfile) == strlen(tilemaps[i]->tilemapfile) && !strcmp(tilemapfile, tilemaps[i]->tilemapfile))
            return i;
    }

    return ERROR;
}

void tm_add_chunk(struct chunk *chunk)
{
    if(!chunk)
    {
        fprintf(stderr, "Received null pointer in tm_add_chunk\n");
        return;
    }
    if(chunk->id)
        return;
    list_append(chunks, chunk);
    chunk->id = chunks->tail;
}

void tm_remove_chunk(struct chunk *chunk)
{
    if(!chunk)
    {
        fprintf(stderr, "Received null pointer in tm_remove_chunk\n");
        return;
    }
    if(!chunk->id)
        return;
    list_delete_node(chunks, chunk->id);
    chunk->id = NULL;
}

int tm_get_chunk_count()
{
    return chunks->size;
}

int tm_get_tile_x(int chunk_x, int tilesize, int column)
{
    return column * tilesize + chunk_x;
}

int tm_get_tile_y(int chunk_y, int tilesize, int row)
{
    return -row * tilesize + chunk_y;
}

void tm_draw_tiles(ALLEGRO_DISPLAY *display)
{
    al_set_target_bitmap(al_get_backbuffer(display));
    al_clear_to_color(BLACK);
    float zoom = sm_get_zoom();
    int newsize = 16 * zoom;

    struct node *node;
    struct chunk *chunk;
    struct tile *tile;
    int r, c;
    al_hold_bitmap_drawing(1);
    for(node = chunks->head; node != NULL; node = node->next)
    {
        chunk = node->p;
        for(r = 0; r < chunksize; r++)
        {
            for(c = 0; c < chunksize; c++)
            {
                tile = &chunk->tiles[r][c];
                al_draw_scaled_bitmap(tilemaps[tile->tilemap_z]->bitmap, tile->tilemap_x, tile->tilemap_y, tilemaps[tile->tilemap_z]->tilesize, tilemaps[tile->tilemap_z]->tilesize, sm_get_x(sm_global_to_rel_x(tm_get_tile_x(chunk->x, tilemaps[tile->tilemap_z]->tilesize, c)), 0), sm_get_y(sm_global_to_rel_y(tm_get_tile_y(chunk->y, tilemaps[tile->tilemap_z]->tilesize, r)), 0), newsize, newsize, 0);
            }
        }
    }
    al_hold_bitmap_drawing(0);
}