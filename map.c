#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "list/list.h"
#include "spritemanager.h"
#include "map.h"
#include "tilemanager.h"
#include "sprites.h"
#include "map.h"
#include "colors.h"

static ALLEGRO_BITMAP *test, *purplebitmap;

struct chunk *map_init_chunk(struct map *map, FILE *file, int x, int y, int *table);
struct chunk *map_create_test_chunk(int x, int y, int chunksize);
void map_create_test_chunk_list(struct map *map);
void map_create_chunks(struct map *map, FILE *file, int *table);

struct map *map_create(int chunksize, int tilesize, int width, int height)
{
    struct map *map = s_malloc(sizeof(struct map), "map_create");
    map->chunksize = chunksize;
    map->tilesize = tilesize;
    map->height = height;
    map->width = width;
    map->tilemaps = list_create();
    map_create_test_chunk_list(map);

    return map;
}

void map_create_test_chunk_list(struct map *map)
{
    map->chunks = s_malloc(sizeof(struct chunk **) * map->height, "map->chunks: map_create_test_chunk_list");

    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk *) * map->width, "map->chunks[r]: map_create_test_chunk_list");
        for(c = 0; c < map->width; c++)
        {
            map->chunks[r][c] = map_create_test_chunk(x, y, map->chunksize);
            map->chunks[r][c]->index_x = c;
            map->chunks[r][c]->index_y = r;
            map->chunks[r][c]->id = NULL;
            x += gridsize;
        }
        y -= gridsize;
        x = smx;
    }
}

struct chunk *map_create_test_chunk(int x, int y, int chunksize)
{
    struct chunk *chunk;
    chunk = s_malloc(sizeof(struct chunk), "chunk: map_create_test_chunk");
    chunk->tiles = s_malloc(sizeof(struct tile *) * chunksize, "chunk->tiles: map_create_test_chunk");
    chunk->x = x;
    chunk->y = y;

    int r, c;
    for(r = 0; r < chunksize; r++)
    {
        chunk->tiles[r] = s_malloc(sizeof(struct tile) * chunksize, "chunk->tiles[r]: map_create_test_chunk");

        for(c = 0; c < chunksize; c++)
        {
            chunk->tiles[r][c].tilemap_x = 0;
            chunk->tiles[r][c].tilemap_y = 0;
            chunk->tiles[r][c].tilemap_z = 0;
            chunk->tiles[r][c].solid = 0;
            chunk->tiles[r][c].breakable = 0;
            chunk->tiles[r][c].damage = 0;
        }
    }

    return chunk;
}

struct chunk *map_get_chunk_from_coordinate(struct map *map, int x, int y)
{
    int chunkgrid = map->chunksize * map->tilesize;
    int pixelheight = map->height * chunkgrid;
    int pixelwidth = map->width * chunkgrid;
    y = pixelheight / 2 - y;//height in pixels
    x = pixelwidth / 2 + x;//width in pixels

    if(x < 0 || y < 0 || x >= pixelwidth || y >= pixelheight)
        return NULL;

    return map->chunks[y / chunkgrid][x / chunkgrid];
}

struct chunk *map_get_chunk_from_index(struct map *map, int x, int y)
{
    if(x < 0 || y < 0 || x >= map->width || y >= map->height)
        return NULL;

    return map->chunks[y][x];
}

int map_save(struct map *map, const char *filepath)
{
    FILE *file = fopen(filepath, "w");

    if(!file)
    {
        perror("Error in map_save");
        return -1;        
    }

    int count;
    char buf[1024];
    memset(buf, 0, 1024);
    if((count = sprintf(buf, "%d,%d,%d,%d\n", map->width, map->height, map->chunksize, map->tilesize)) < 0)
    {
        perror("Error in map_save");
        return -1;
    }
    fwrite(buf, sizeof(char), count, file);

    int i;
    if(map->tilemaps)
    {
        for(i = 0; i < map->tilemaps->size; i++)
        {
            memset(buf, 0, 1024);
            struct tilemap *tm = list_get(map->tilemaps, i);
            count = sprintf(buf, "%s\n", tm->tilemapfile);
            fwrite(buf, sizeof(char), strlen(buf), file);
        }
    }

    putc('\n', file);
    int r, c;
    for(r = 0; r < map->height; r++)
    {
        for(c = 0; c < map->width; c++)
        {
            int r2, c2;
            for(r2 = 0; r2 < map->chunksize; r2++)
            {
                for(c2 = 0; c2 < map->chunksize; c2++)
                {
                    struct tile tile = map->chunks[r][c]->tiles[r2][c2];
                    count = sprintf(buf, "%d,%d,%d,%d,%d,%d\n", tile.tilemap_x, tile.tilemap_y, tile.tilemap_z, tile.solid, tile.breakable, tile.damage);
                    fwrite(buf, sizeof(char), count, file);
                }
            }
        }
    }

    fclose(file);
    return 0;
}

struct chunk *map_init_chunk(struct map *map, FILE *file, int x, int y, int *table)
{
    struct chunk *chunk;
    chunk = s_malloc(sizeof(struct chunk), "chunk: map_init_chunk");
    chunk->tiles = s_malloc(sizeof(struct tile *) * map->chunksize, "chunk->tiles: map_init_chunk");
    chunk->x = x;
    chunk->y = y;
    chunk->id = NULL;

    char buf[32];

    int r, c;
    for(r = 0; r < map->chunksize; r++)
    {
        chunk->tiles[r] = s_malloc(sizeof(struct tile) * map->chunksize, "chunk->tiles[r]: map_init_chunk");
        for(c = 0; c < map->chunksize; c++)
        {
            struct tile *tile = &chunk->tiles[r][c];
            if(!fgets(buf, 32, file))
            {
                fprintf(stderr, "Corrupted map file: not enough tiles\n");
                exit(1);
            }
            
            tile->tilemap_x = atoi(strtok(buf, ","));
            tile->tilemap_y = atoi(strtok(NULL, ","));
            tile->tilemap_z = table[atoi(strtok(NULL, ","))];
            tile->solid = atoi(strtok(NULL, ","));
            tile->breakable = atoi(strtok(NULL, ","));
            tile->damage = atoi(strtok(NULL, ","));
        }
    }

    return chunk;
}

struct tile *map_get_tile_from_coordinate(struct map *map, int x, int y)
{
    struct chunk *chunk = map_get_chunk_from_coordinate(map, x, y);

    if(!chunk)
        return NULL;

    x = x - chunk->x;
    y = chunk->y - y;
    x /= map->tilesize;
    y /= map->tilesize;

    return &chunk->tiles[y][x];
}

void map_create_chunks(struct map *map, FILE *file, int *table)
{
    map->chunks = s_malloc(sizeof(struct chunk **) * map->height, "map->chunks: map_create_chunks");

    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk *) * map->width, "map->chunks[r]: map_create_chunks");
        for(c = 0; c < map->width; c++)
        {
            map->chunks[r][c] = map_init_chunk(map, file, x, y, table);
            map->chunks[r][c]->index_x = c;
            map->chunks[r][c]->index_y = r;
            x += gridsize;
        }
        y -= gridsize;
        x = smx;
    }
}

struct map *map_load(const char *mapfile)
{
    struct map *map = s_malloc(sizeof(struct map), "map: map_load");

    FILE *file = fopen(mapfile, "r");

    if(!file)
    {
        free(map);
        perror("Error in map_load");
        return NULL;
    }

    char buf[1024];
    fgets(buf, 1024, file);

    map->width = atoi(strtok(buf, ","));
    map->height = atoi(strtok(NULL, ","));
    map->chunksize = atoi(strtok(NULL, ","));
    map->tilesize = atoi(strtok(NULL, ","));

    map->tilemaps = list_create();
    int *table = s_malloc(2 * sizeof(int), "table: map_load");
    table[0] = 0;
    table[1] = 1;
    int size = 2;
    
    memset(buf, 0, 1024);

    while(strcmp(fgets(buf, 1024, file), "\n"))
    {
        buf[strlen(buf) - 1] = '\0';
        
        int z = tm_load_tile_map(buf, map->tilesize);
        int nomatch = 1;
        int i;
        for(i = 0; i < size; i++)
        {
            if(table[i] == z)
                nomatch = 0;
        }
        if(nomatch)
        {
            table = s_realloc(table, sizeof(int), "table: map_load");
            table[size++] = z;
            list_append(map->tilemaps, tm_get_tile_map_from_z(z));
        }
        
        memset(buf, 0, 1024);
    }

    map_create_chunks(map, file, table);

    free(table);

    fclose(file);

    return map;
}

void map_destroy_chunk(struct chunk *chunk, int chunksize)
{
    int r;
    for(r = 0; r < chunksize; r++)
        free(chunk->tiles[r]);

    free(chunk->tiles);
    free(chunk);
}

void map_destroy(struct map *map)
{
    if(map == NULL)
        return;

    int r, c;
    for(r = 0; r < map->height; r++)
        for(c = 0; c < map->width; c++)
            map_destroy_chunk(map->chunks[r][c], map->chunksize);

    free(map);
}
