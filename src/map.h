#ifndef __MAP_H__
#define __MAP_H__

#include "list.h"
#include "entity.h"

struct tilemap
{
    char *tilemapfile;
    ALLEGRO_BITMAP *bitmap;
    int tilesize;
};

struct tile
{
    unsigned short tilemap_x:9;
    unsigned short tilemap_y:9;
    unsigned short tilemap_z:6;
    unsigned char type;
    char damage;
};

struct chunk
{
    struct tile **tiles;
    struct node *ehead;
    float x;
    float y;
    int index_x;
    int index_y;
};

struct map
{
    char *name;
    struct chunk ***chunks;
    int chunksize;
    int tilesize;
    int width;
    int height;
};

struct map *map_create(int chunksize, int tilesize, int width, int height);
struct map *map_load(char *dir);
void map_destroy(struct map *map);
struct chunk *map_get_chunk_from_coordinate(struct map *map, float x, float y);
struct chunk *map_get_chunk_from_index(struct map *map, int x, int y);
struct tile *map_get_tile_from_coordinate(struct map *map, float x, float y);
void map_add_entity_to_chunk(struct map *map, struct entity *e);
void map_remove_entity_from_chunk(struct map *map, struct entity *e);
int map_save(struct map *map, char *dir);
void map_destroy_chunk(struct chunk *chunk, int chunksize);

enum TILETYPE {SOLID = 1, BREAKABLE = 2};

#endif