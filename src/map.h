#ifndef __MAP_H__
#define __MAP_H__

#include "list.h"
#include "entity.h"
#include "graph.h"

struct tilemap
{
    char *tilemapfile;
    ALLEGRO_BITMAP *bitmap;
    int tilesize;
};

struct palette
{
    unsigned short tilemap_x;
    unsigned short tilemap_y;
    unsigned short tilemap_z;
    unsigned char type:3;
};

struct tile
{
    unsigned char id;
    unsigned char type;
    /*unsigned short tilemap_x:9;
    unsigned short tilemap_y:9;
    unsigned short tilemap_z:6;
    unsigned char type;
    unsigned char func;
    char damage;*/
};

struct chunk
{
    struct tile *tiles;
    unsigned short index_x;
    unsigned short index_y;
    unsigned short flags;
};

struct map
{
    char *name;
    struct chunk **chunks;
    struct node **entitylists;
    int chunksize;//in tiles
    int tilesize;//in pixels
    int width;//in chunks
    int height;//in chunks
    struct graph *graph;
    struct palette *palette;
};

struct map *map_create(int chunksize, int tilesize, int width, int height);
struct map *map_load(char *dir);
void map_destroy(struct map *map);
float map_get_chunk_y(struct map *map, struct chunk *chunk);
float map_get_chunk_x(struct map *map, struct chunk *chunk);
struct chunk *map_get_chunk_from_coordinate(struct map *map, float x, float y);
struct chunk *map_get_chunk_from_index(struct map *map, int x, int y);
struct tile *map_get_tile_from_coordinate(struct map *map, float x, float y);
void map_add_entity_to_chunk(struct map *map, struct entity *e);
void map_remove_entity_from_chunk(struct map *map, struct chunk *chunk, struct entity *e);
int map_save(struct map *map, char *dir);
void map_destroy_chunk(struct chunk *chunk);

enum TILETYPE {SOLID = 1, BREAKABLE = 2, EVENT = 4};

#endif