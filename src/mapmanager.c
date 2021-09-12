#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "list.h"
#include "sprites.h"
#include "map.h"
#include "spritemanager.h"
#include "mapmanager.h"
#include "mapgenerator.h"
#include "debug.h"
#include "colors.h"
#include "emath.h"

static struct chunk *corners[4];
static struct list *maps;
static const int DISTANCE = HEIGHT / 2;

static struct list *chunks;
static struct tilemap **tilemaps;
static int tilemapssize, chunksize;
static const int HHEIGHT = HEIGHT / 2;
static const int HWIDTH = WIDTH / 2;
static const int TILESIZE = 16;
static int *matrix;
enum TILEMAPZ {EMPTY, ERROR, BLANK};

void mm_add_chunk_to_layer(struct chunk *chunk, int chunksize);
void mm_remove_chunk_from_layer(struct chunk *chunk, int chunksize);
struct tilemap *mm_load_tile_map_from_file(char *tilemapfile, int tilesize);
int mm_add_tile_map(struct tilemap *tm);
void mm_destroy_tile_map(struct tilemap *tm);

enum DIR {UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3};

void mm_init(char *mapdir, ...)
{
    maps = list_create();
    struct map *map;// = mg_create_map(30, 30);
    if(s_string_match(mapdir, "new"))
    {
        va_list vl;
        va_start(vl, mapdir);
        int chunksize = va_arg(vl, int);
        int tilesize = va_arg(vl, int);
        int width = va_arg(vl, int);
        int height = va_arg(vl, int);
        map = mg_create_map(30, 30);//map_create(chunksize, tilesize, width, height);
        va_end(vl);
    }
    else
        map = map_load(mapdir);
        
    if(!map)
    {
        debug_perror("Failed to create/open map\n");
        exit(1);
    }

    chunks = list_create();
    tilemapssize = 0;
    mm_add_tile_map_to_list("defaulttile.bmp", TILESIZE);
    mm_add_tile_map_to_list("purplebitmap.bmp", TILESIZE);
    mm_add_tile_map_to_list("blanktile.png", TILESIZE);
    mm_add_tile_map_to_list("DungeonTilesetIItiles.png", 16);
    chunksize = map->chunksize;

    matrix = s_malloc(map->chunksize * sizeof(int), "mm_init: matrix");
    int i;
    for(i = 0; i < map->chunksize; i++)
        matrix[i] = i * TILESIZE;

    mm_load_tile_maps(mapdir);
    list_append(maps, map);

    int distance = DISTANCE / sm_get_zoom();

    int range[4];

    range[LEFT]  = sm_get_coord(X) - distance;
    range[RIGHT] = sm_get_coord(X) + distance;
    range[DOWN]  = sm_get_coord(Y) - distance;
    range[UP]    = sm_get_coord(Y) + distance;

    corners[TOPLEFT] = map_get_chunk_from_coordinate(map, range[LEFT], range[UP]);
    corners[TOPRIGHT] = map_get_chunk_from_coordinate(map, range[RIGHT], range[UP]);
    corners[BOTTOMLEFT] = map_get_chunk_from_coordinate(map, range[LEFT], range[DOWN]);
    corners[BOTTOMRIGHT] = map_get_chunk_from_coordinate(map, range[RIGHT], range[DOWN]);
    
    if(!corners[TOPLEFT])
        corners[TOPLEFT] = map->chunks[0][0];

    if(!corners[TOPRIGHT])
        corners[TOPRIGHT] = map->chunks[0][map->width - 1];

    if(!corners[BOTTOMLEFT])
        corners[BOTTOMLEFT] = map->chunks[map->height - 1][0];

    if(!corners[BOTTOMRIGHT])
        corners[BOTTOMRIGHT] = map->chunks[map->height - 1][map->width - 1];

    int r, c;
    for(r = corners[TOPLEFT]->index_y; r <= corners[BOTTOMLEFT]->index_y; r++)
    {
        for(c = corners[TOPLEFT]->index_x; c <= corners[TOPRIGHT]->index_x; c++)
        {
            mm_add_chunk_to_layer(map->chunks[r][c], map->chunksize);
        }
    }
}

void mm_destroy()
{
    list_destroy_with_function(maps, map_destroy);
    list_destroy(chunks);
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        mm_destroy_tile_map(tilemaps[i]);
    }
    s_free(tilemaps, NULL);
    s_free(matrix, NULL);
}

struct map *mm_get_top_map()
{
    return list_get(maps, 0);
}

void mm_test_color_chunk(struct chunk *chunk)
{  
    int r, c;
    for(r = 0; r < 5; r++)
        for(c = 0; c < 5; c++)
            chunk->tiles[r][c].tilemap_z = 1;
}

void mm_test_color_tile(float x, float y)
{
    struct map *map = list_get(maps, 0);
    struct chunk *chunk = map_get_chunk_from_coordinate(map, x, y);

    if(!chunk)
        return;

    x = x - chunk->x;
    y = chunk->y - y;
    x /= map->tilesize;
    y /= map->tilesize;

    chunk->tiles[math_floor(y)][math_floor(x)].tilemap_z = 1;
}

struct chunk *mm_get_chunk_from_rel_coordinate(float x, float y)
{
    return map_get_chunk_from_coordinate(list_get(maps, 0), sm_rel_to_global_x(x), sm_rel_to_global_y(y));
}

void mm_add_chunk_to_layer(struct chunk *chunk, int chunksize)
{
    if(!chunk)
    {
        fprintf(stderr, "Received null pointer in mm_add_chunk\n");
        return;
    }
    if(chunk->id)
        return;
    list_append(chunks, chunk);
    chunk->id = chunks->tail;
}

void mm_remove_chunk_from_layer(struct chunk *chunk, int chunksize)
{
    if(!chunk)
    {
        fprintf(stderr, "Received null pointer in mm_remove_chunk\n");
        return;
    }
    if(!chunk->id)
        return;
    list_delete_node(chunks, chunk->id);
    chunk->id = NULL;
}

void mm_update_chunks()
{
    struct map *map = list_get(maps, 0);
    int chunkgrid = map->chunksize * map->tilesize;
    int distance = DISTANCE / sm_get_zoom();

    int range[4];

    range[RIGHT] = (sm_get_coord(X) + distance - corners[TOPRIGHT]->x) / chunkgrid;
    range[LEFT] = (sm_get_coord(X) - distance - corners[TOPLEFT]->x - chunkgrid) / chunkgrid;
    range[UP] = (sm_get_coord(Y) + distance - corners[TOPLEFT]->y + chunkgrid) / chunkgrid;
    range[DOWN] = (sm_get_coord(Y) - distance - corners[BOTTOMLEFT]->y) / chunkgrid;

    if(!(range[RIGHT] || range[LEFT] || range[UP] || range[DOWN]))
        return;

    /*
        This code checks to see if there are chunks to load or unload on the sides of what is currently loaded
    */

    int r, c;
    for(c = corners[TOPRIGHT]->index_x + 1; c <= corners[TOPRIGHT]->index_x + range[RIGHT] && c < map->width; c++)//adds chunks to the right
        for(r = corners[BOTTOMRIGHT]->index_y; r >= corners[TOPRIGHT]->index_y; r--)
            mm_add_chunk_to_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPRIGHT] = map_get_chunk_from_index(map, c - 1, corners[TOPRIGHT]->index_y);
    corners[BOTTOMRIGHT] = map_get_chunk_from_index(map, c - 1, corners[BOTTOMRIGHT]->index_y);

    for(c = corners[TOPRIGHT]->index_x; c > corners[TOPRIGHT]->index_x + range[RIGHT] && c - 1 > -1; c--)//takes away chunks from the right
        for(r = corners[BOTTOMRIGHT]->index_y; r >= corners[TOPRIGHT]->index_y; r--)
            mm_remove_chunk_from_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPRIGHT] = map_get_chunk_from_index(map, c, corners[TOPRIGHT]->index_y);
    corners[BOTTOMRIGHT] = map_get_chunk_from_index(map, c, corners[BOTTOMRIGHT]->index_y);


    for(c = corners[TOPLEFT]->index_x - 1; c >= corners[TOPLEFT]->index_x + range[LEFT] && c > -1; c--)//adds chunks to the left
        for(r = corners[BOTTOMLEFT]->index_y; r >= corners[TOPLEFT]->index_y; r--)
            mm_add_chunk_to_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPLEFT] = map_get_chunk_from_index(map, c + 1, corners[TOPLEFT]->index_y);
    corners[BOTTOMLEFT] = map_get_chunk_from_index(map, c + 1, corners[BOTTOMLEFT]->index_y);

    for(c = corners[TOPLEFT]->index_x; c < corners[TOPLEFT]->index_x + range[LEFT] && c + 1 < map->width; c++)//takes away chunks from the left
        for(r = corners[BOTTOMLEFT]->index_y; r >= corners[TOPLEFT]->index_y; r--)
            mm_remove_chunk_from_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPLEFT] = map_get_chunk_from_index(map, c, corners[TOPLEFT]->index_y);
    corners[BOTTOMLEFT] = map_get_chunk_from_index(map, c, corners[BOTTOMLEFT]->index_y);

    for(r = corners[TOPLEFT]->index_y - 1; r >= corners[TOPLEFT]->index_y - range[UP] && r > -1; r--)//adds chunks on top
        for(c = corners[TOPLEFT]->index_x; c <= corners[TOPRIGHT]->index_x; c++)
            mm_add_chunk_to_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPLEFT] = map_get_chunk_from_index(map, corners[TOPLEFT]->index_x, r + 1);
    corners[TOPRIGHT] = map_get_chunk_from_index(map, corners[TOPRIGHT]->index_x, r + 1);

    for(r = corners[TOPLEFT]->index_y; r < corners[TOPLEFT]->index_y - range[UP] && r + 1 < map->height; r++)//takes away top chunks
        for(c = corners[TOPLEFT]->index_x; c <= corners[TOPRIGHT]->index_x; c++)
            mm_remove_chunk_from_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[TOPLEFT] = map_get_chunk_from_index(map, corners[TOPLEFT]->index_x, r);
    corners[TOPRIGHT] = map_get_chunk_from_index(map, corners[TOPRIGHT]->index_x, r);

    for(r = corners[BOTTOMLEFT]->index_y + 1; r <= corners[BOTTOMLEFT]->index_y - range[DOWN] && r < map->height; r++)//adds chunks to bottom
        for(c = corners[BOTTOMLEFT]->index_x; c <= corners[BOTTOMRIGHT]->index_x; c++)
            mm_add_chunk_to_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[BOTTOMLEFT] = map_get_chunk_from_index(map, corners[BOTTOMLEFT]->index_x, r - 1);
    corners[BOTTOMRIGHT] = map_get_chunk_from_index(map, corners[BOTTOMRIGHT]->index_x, r - 1);

    for(r = corners[BOTTOMLEFT]->index_y; r > corners[BOTTOMLEFT]->index_y - range[DOWN] && r - 1 > -1; r--)//takes away bottom chunks
        for(c = corners[BOTTOMLEFT]->index_x; c <= corners[BOTTOMRIGHT]->index_x; c++)
            mm_remove_chunk_from_layer(map_get_chunk_from_index(map, c, r), map->chunksize);

    corners[BOTTOMLEFT] = map_get_chunk_from_index(map, corners[BOTTOMLEFT]->index_x, r);
    corners[BOTTOMRIGHT] = map_get_chunk_from_index(map, corners[BOTTOMRIGHT]->index_x, r);
}

void mm_save_map(char *mapname)
{
    if(!al_make_directory(s_get_full_path_with_dir("maps", mapname)))
    {
        debug_perror("Error creating directory %s\n", s_get_full_path_with_dir("maps", mapname));
        return;
    }

    map_save(list_get(maps, 0), mapname);

    mm_save_tile_maps(mapname);

    debug_printf("Map saved\n");
}

void mm_load_map(char *mapname)
{
    char *dir = s_get_heap_string(s_get_full_path(mapname));

    list_append(maps, map_load(dir));

    mm_load_tile_maps(dir);

    s_free(dir, NULL);
}

struct chunk **mm_get_corners()
{
    return corners;
}

struct tile *mm_get_tile(float x, float y)
{
    return map_get_tile_from_coordinate(mm_get_top_map(), x, y);
}

struct tile *mm_get_tile_from_rel_coordinate(float x, float y)
{
    return map_get_tile_from_coordinate(mm_get_top_map(), sm_rel_to_global_x(x), sm_rel_to_global_y(y));
}

struct tile *mm_update_tile(float x, float y, struct tile *tile)
{
    if(!tile)
        return NULL;

    struct tile *oldtile = mm_get_tile(x, y);
    if(!oldtile)
        return NULL;

    oldtile->tilemap_x = tile->tilemap_x;
    oldtile->tilemap_y = tile->tilemap_y;
    oldtile->tilemap_z = tile->tilemap_z;
    oldtile->solid = tile->solid;
    oldtile->breakable = tile->breakable;
    oldtile->damage = tile->damage;

    return oldtile;
}

void mm_print_tile_maps()
{
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        struct tilemap *tm = tilemaps[i];
        printf("%s, %p, %d\n", tm->tilemapfile, tm->bitmap, tm->tilesize);
    }
}

void mm_save_tile_maps(char *dir)//creates a tilemapconfig based on the list's current state file given a directory
{
    char buf[512];
    memset(buf, 0, 512);
    strcat(buf, "maps/");
    strcat(buf, dir);
    ALLEGRO_FILE *tilemapconfig = al_fopen(s_get_full_path_with_dir(buf, "tilemapconfig"), "w");
    if(!tilemapconfig)
    {
        debug_perror("Error opening file in mm_save_tilemaps in dir %s\n", buf);
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
            debug_perror("Error writing to file in mm_save_tilemaps");
    }

    al_fclose(tilemapconfig);
}

int mm_load_tile_maps(char *dir)//loads tilemaps from a tilemapconfig file given the directory it's in
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

        mm_add_tile_map_to_list(buf, atoi(comma + 1));
        memset(buf, 0, 512);
    }
    al_fclose(tilemapconfig);
    return 0;
}

void mm_destroy_tile_map(struct tilemap *tm)
{
    s_free(tm->tilemapfile, NULL);
    al_destroy_bitmap(tm->bitmap);
    s_free(tm, NULL);
}

int mm_add_tile_map_to_list(char *tilemapfile, int tilesize)//takes filepath and tilesize, adds a tile map struct to the list if it does not exist
{
    int z = mm_get_tile_map_z(tilemapfile);
    if(z == 1)
        return mm_add_tile_map(mm_load_tile_map_from_file(tilemapfile, tilesize));

    return z;
}

struct tilemap *mm_load_tile_map_from_file(char *tilemapfile, int tilesize)//creates tilemap struct from filename and tilesize
{
    int z = mm_get_tile_map_z(tilemapfile);
    if(z != ERROR)
        return tilemaps[z];

    ALLEGRO_BITMAP *bitmap = al_load_bitmap(s_get_full_path_with_dir("images", tilemapfile));
    if(!bitmap)
    {
        fprintf(stderr, "Tile map file \"%s\" failed to load\n", tilemapfile);
        return NULL;
    }

    struct tilemap *out = s_malloc(sizeof(struct tilemap), "mm_load_tile_map from");
    out->bitmap = bitmap;
    out->tilemapfile = s_get_heap_string(tilemapfile);
    out->tilesize = tilesize;
    return out;
}

int mm_add_tile_map(struct tilemap *tm)//increases the list's size by 1 and adds the tilemap struct to the list
{
    if(!tm)
    {
        fprintf(stderr, "Received null pointer in mm_add_tilemap\n");
        return ERROR;
    }

    tilemaps = s_realloc(tilemaps, sizeof(struct tilemap *) * (tilemapssize + 1), "mm_add_tile_map");
    tilemaps[tilemapssize] = tm;
    return tilemapssize++;
}

int mm_get_tile_map_z(char *tilemapfile)//looks for filename in tilemap list
{
    int i;
    for(i = 0; i < tilemapssize; i++)
    {
        if(s_string_match(tilemapfile, tilemaps[i]->tilemapfile))
            return i;
    }

    return ERROR;
}

struct tilemap *mm_get_tile_map_for_tile(struct tile *tile)
{
    return tilemaps[tile->tilemap_z];
}

struct tilemap *mm_get_tile_map_from_z(int z)
{
    return tilemaps[z];
}

ALLEGRO_BITMAP *mm_get_tile_bitmap(struct tile *tile)
{
    return al_create_sub_bitmap(tilemaps[tile->tilemap_z]->bitmap, tile->tilemap_x, tile->tilemap_y, tilemaps[tile->tilemap_z]->tilesize, tilemaps[tile->tilemap_z]->tilesize);
}

int mm_get_chunk_count()
{
    return chunks->size;
}

float mm_get_tile_x(int chunk_x, int tilesize, int column)
{
    //return column * tilesize + chunk_x;
    return matrix[column] + chunk_x;
}
//(((column * tilesize + chunk_x) - coord[X]) * zoom) + HWIDTH - tilesize / 2;
float mm_get_tile_y(int chunk_y, int tilesize, int row)
{
    //return -row * tilesize + chunk_y;
    return -matrix[row] + chunk_y;
}

float mm_get_x(float x, int bitmapwidth)
{
	return x + HWIDTH - bitmapwidth / 2;
}

float mm_get_y(float y, int bitmapheight)
{
	return HHEIGHT - y - bitmapheight / 2;
}

float mm_global_to_rel_x(float x)
{
	return (x - sm_get_coord(X)) * sm_get_zoom();
}

float mm_global_to_rel_y(float y)
{
	return (y - sm_get_coord(Y)) * sm_get_zoom();
}

void mm_draw_chunks(ALLEGRO_DISPLAY *display)
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
                /*if(tile->damage < 0 && tile->breakable)
                {
                    printf("damage: %d, breakable: %d\n", tile->damage, tile->breakable);
                    tile->tilemap_x = 0;
                    tile->tilemap_y = 0;
                    tile->tilemap_z = BLANK;
                    tile->damage = 10;
                }*/
                tilemap = tilemaps[tile->tilemap_z];
                
                al_draw_scaled_bitmap(tilemap->bitmap, tile->tilemap_x, tile->tilemap_y, tilemap->tilesize, tilemap->tilesize, mm_get_x(mm_global_to_rel_x(mm_get_tile_x(chunk->x, tilemap->tilesize, c)), 0), mm_get_y(mm_global_to_rel_y(mm_get_tile_y(chunk->y, tilemap->tilesize, r)), 0), newsize, newsize, 0);
            }
        }
    }
    al_hold_bitmap_drawing(0);
}