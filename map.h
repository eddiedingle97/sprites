#ifndef __MAP_H__
#define __MAP_H__

struct tilemap
{
    char *tilemapfile;
    ALLEGRO_BITMAP *bitmap;
    int tilesize;
};

struct tile
{
    unsigned short tilemap_x;
    unsigned short tilemap_y;
    unsigned short tilemap_z;
    unsigned char solid:1;
    unsigned char breakable:1;
    char damage;
};

struct chunk
{
    struct tile **tiles;
    struct node *id;
    float x;
    float y;
    int index_x;
    int index_y;
};

struct map
{
    char *name;
    char *mapfile;
    struct list *tilemaps;
    struct chunk ***chunks;
    int chunksize;
    int tilesize;
    int width;
    int height;
};

struct map *map_create(int chunksize, int tilesize, int width, int height);
struct map *map_load(const char *mapfile);
void map_destroy(struct map *map);
struct chunk *map_get_chunk_from_coordinate(struct map *map, int x, int y);
struct chunk *map_get_chunk_from_index(struct map *map, int x, int y);
struct tile *map_get_tile_from_coordinate(struct map *map, int x, int y);
int map_save(struct map *map, const char *filepath);
void map_destroy_chunk(struct chunk *chunk, int chunksize);


#endif