#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "list.h"
#include "sprites.h"
#include "map.h"
#include "spritemanager.h"
#include "tilemanager.h"
#include "mapmanager.h"
#include "debug.h"
#include "colors.h"

static struct chunk *corners[4];
static struct list *maps;
static const int DISTANCE = HEIGHT / 2;

void mm_add_chunk_to_layer(struct chunk *chunk, int chunksize);
void mm_remove_chunk_from_layer(struct chunk *chunk, int chunksize);

enum DIR {UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3};

void mm_init(char *mapdir, ...)
{
    maps = list_create();
    struct map *map;
    if(s_string_match(mapdir, "new"))
    {
        va_list vl;
        va_start(vl, mapdir);
        int chunksize = va_arg(vl, int);
        int tilesize = va_arg(vl, int);
        int width = va_arg(vl, int);
        int height = va_arg(vl, int);
        map = map_create(chunksize, tilesize, width, height);
        va_end(vl);
    }
    else
        map = map_load(mapdir);
        
    if(!map)
    {
        debug_perror("Failed to create/open map\n");
        exit(1);
    }
    tm_init(map->chunksize);
    tm_load_tile_maps(mapdir);
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
    list_destroy_with_function(maps, (void (*)(void *))map_destroy);
    tm_destroy();
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

    chunk->tiles[s_floor(y)][s_floor(x)].tilemap_z = 1;
}

struct chunk *mm_get_chunk_from_rel_coordinate(float x, float y)
{
    return map_get_chunk_from_coordinate(list_get(maps, 0), sm_rel_to_global_x(x), sm_rel_to_global_y(y));
}

void mm_add_chunk_to_layer(struct chunk *chunk, int chunksize)
{
    tm_add_chunk(chunk);
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

    tm_save_tile_maps(mapname);
}

void mm_load_map(char *mapname)
{
    char *dir = s_get_heap_string(s_get_full_path(mapname));

    list_append(maps, map_load(dir));

    tm_load_tile_maps(dir);

    free(dir);
}

struct chunk **mm_get_corners()
{
    return corners;
}

struct tile *mm_get_tile(float x, float y)
{
    return map_get_tile_from_coordinate(mm_get_top_map(), x, y);
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

void mm_remove_chunk_from_layer(struct chunk *chunk, int chunksize)
{
    tm_remove_chunk(chunk);
}