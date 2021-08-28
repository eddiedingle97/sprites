#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "list.h"
#include "spritemanager.h"
#include "map.h"
#include "tilemanager.h"
#include "debug.h"
#include "colors.h"

static struct list *chunks;
static struct tilemap **tilemaps;
static int tilemapssize, chunksize;
static const int HHEIGHT = HEIGHT / 2;
static const int HWIDTH = WIDTH / 2;
static const int TILESIZE = 16;
static int *matrix;
enum TILEMAPZ {EMPTY, ERROR, BLANK};

struct tilemap *tm_load_tile_map_from_file(char *tilemapfile, int tilesize);
int tm_add_tile_map(struct tilemap *tm);

void tm_init(int csize)
{
    chunks = list_create();
    tilemapssize = 0;
    tm_add_tile_map(tm_load_tile_map_from_file("defaulttile.bmp", TILESIZE));
    tm_add_tile_map(tm_load_tile_map_from_file("purplebitmap.bmp", TILESIZE));
    tm_add_tile_map(tm_load_tile_map_from_file("blanktile.png", TILESIZE));
    chunksize = csize;

    matrix = s_malloc(csize * sizeof(int), "tm_init: matrix");
    int i;
    for(i = 0; i < csize; i++)
        matrix[i] = i * TILESIZE;
}

void tm_destroy_tile_map(struct tilemap *tm)
{
    s_free(tm->tilemapfile, NULL);
    al_destroy_bitmap(tm->bitmap);
    s_free(tm, NULL);
}

void tm_destroy()
{
    list_destroy(chunks);
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        tm_destroy_tile_map(tilemaps[i]);
    }
    s_free(tilemaps, NULL);
    s_free(matrix, NULL);
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

void tm_save_tile_maps(char *dir)//creates a tilemapconfig based on the list's current state file given a directory
{
    char buf[512];
    memset(buf, 0, 512);
    strcat(buf, "maps/");
    strcat(buf, dir);
    ALLEGRO_FILE *tilemapconfig = al_fopen(s_get_full_path_with_dir(buf, "tilemapconfig"), "w");
    if(!tilemapconfig)
    {
        debug_perror("Error opening file in tm_save_tilemaps in dir %s\n", buf);
        return;
    }
    int i, count;

    struct tilemap *tm;
    for(i = 3; i < tilemapssize; i++)
    {
        memset(buf, 0, 512);
        tm = tilemaps[i];
        count = sprintf(buf, "%s,%d\n", tm->tilemapfile, tm->tilesize);
        if(al_fwrite(tilemapconfig, buf, count) != count)
            debug_perror("Error writing to file in tm_save_tilemaps");
    }

    al_fclose(tilemapconfig);
}

int tm_load_tile_maps(char *dir)//loads tilemaps from a tilemapconfig file given the directory it's in
{
    char buf[512];
    memset(buf, 0, 512);
    strcat(buf, "maps/");
    strcat(buf, dir);
    ALLEGRO_FILE *tilemapconfig = al_fopen(s_get_full_path_with_dir(buf, "tilemapconfig"), "r");
    if(!tilemapconfig)
    {
        debug_perror("Error opening tilemapconfig for map \"%s\"\n", dir);
        return -1;
    }

    char *comma;
    memset(buf, 0, 512);
    while(al_fgets(tilemapconfig, buf, 512))
    {
        comma = strchr(buf, ',');
        *comma = '\0';
        if(strchr(comma + 1, ','))
        {
            debug_perror("Error: tilemap image file name contains a comma\n");
            return -1;
        }

        tm_add_tile_map_to_list(buf, atoi(comma + 1));
        memset(buf, 0, 512);
    }
    al_fclose(tilemapconfig);
    return 0;
}

int tm_add_tile_map_to_list(char *tilemapfile, int tilesize)//takes filepath and tilesize, adds a tile map struct to the list if it does not exist
{
    int z = tm_get_tile_map_z(tilemapfile);
    if(z == 1)
        return tm_add_tile_map(tm_load_tile_map_from_file(tilemapfile, tilesize));

    return z;
}

struct tilemap *tm_load_tile_map_from_file(char *tilemapfile, int tilesize)//creates tilemap struct from filename and tilesize
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

    struct tilemap *out = s_malloc(sizeof(struct tilemap), "tm_load_tile_map from");
    out->bitmap = bitmap;
    out->tilemapfile = s_get_heap_string(tilemapfile);
    out->tilesize = tilesize;
    return out;
}

int tm_add_tile_map(struct tilemap *tm)//increases the list's size by 1 and adds the tilemap struct to the list
{
    if(!tm)
    {
        fprintf(stderr, "Received null pointer in tm_add_tilemap\n");
        return ERROR;
    }

    tilemaps = s_realloc(tilemaps, sizeof(struct tilemap *) * (tilemapssize + 1), "tm_add_tile_map");
    tilemaps[tilemapssize] = tm;
    return tilemapssize++;
}

int tm_get_tile_map_z(char *tilemapfile)//looks for filename in tilemap list
{
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        if(s_string_match(tilemapfile, tilemaps[i]->tilemapfile))
            return i;
    }

    return ERROR;
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

float tm_get_tile_x(int chunk_x, int tilesize, int column)
{
    //return column * tilesize + chunk_x;
    return matrix[column] + chunk_x;
}
//(((column * tilesize + chunk_x) - coord[X]) * zoom) + HWIDTH - tilesize / 2;
float tm_get_tile_y(int chunk_y, int tilesize, int row)
{
    //return -row * tilesize + chunk_y;
    return -matrix[row] + chunk_y;
}

float tm_get_x(float x, int bitmapwidth)
{
	return x + HWIDTH - bitmapwidth / 2;
}

float tm_get_y(float y, int bitmapheight)
{
	return HHEIGHT - y - bitmapheight / 2;
}

float tm_global_to_rel_x(float x)
{
	return (x - sm_get_coord(X)) * sm_get_zoom();
}

float tm_global_to_rel_y(float y)
{
	return (y - sm_get_coord(Y)) * sm_get_zoom();
}

void tm_draw_chunks(ALLEGRO_DISPLAY *display)
{
    al_set_target_bitmap(al_get_backbuffer(display));
    al_clear_to_color(BLACK);
    float zoom = sm_get_zoom();
    int newsize = TILESIZE * zoom;

    struct node *node;
    struct chunk *chunk;
    struct tilemap *tilemap;
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
                tilemap = tilemaps[tile->tilemap_z];
                al_draw_scaled_bitmap(tilemap->bitmap, tile->tilemap_x, tile->tilemap_y, tilemap->tilesize, tilemap->tilesize, tm_get_x(tm_global_to_rel_x(tm_get_tile_x(chunk->x, tilemap->tilesize, c)), 0), tm_get_y(tm_global_to_rel_y(tm_get_tile_y(chunk->y, tilemap->tilesize, r)), 0), newsize, newsize, 0);
            }
        }
    }
    al_hold_bitmap_drawing(0);
}