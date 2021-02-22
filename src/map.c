#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "map.h"
#include "sprites.h"
#include "map.h"
#include "debug.h"
#include "colors.h"

struct chunk *map_init_chunk(struct map *map, ALLEGRO_FILE *file, int x, int y);
struct chunk *map_create_test_chunk(int x, int y, int chunksize);
void map_create_test_chunk_list(struct map *map);
void map_create_chunks(struct map *map, ALLEGRO_FILE *file);

struct map *map_create(int chunksize, int tilesize, int width, int height)
{
    struct map *map = s_malloc(sizeof(struct map), "map_create");
    map->chunksize = chunksize;
    map->tilesize = tilesize;
    map->height = height;
    map->width = width;
    map->name = NULL;
    map_create_test_chunk_list(map);

    return map;
}

void map_create_test_chunk_list(struct map *map)
{
    map->chunks = s_malloc(sizeof(struct chunk **) * map->height, NULL);
    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk *) * map->width, NULL);
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
    chunk = s_malloc(sizeof(struct chunk), NULL);
    chunk->tiles = s_malloc(sizeof(struct tile *) * chunksize, NULL);
    chunk->x = x;
    chunk->y = y;

    int r, c;
    for(r = 0; r < chunksize; r++)
    {
        chunk->tiles[r] = s_malloc(sizeof(struct tile) * chunksize, NULL);

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

int map_save(struct map *map, char *mapname)
{
    char buf[1024];
    memset(buf, 0, 1024);
    strcat(buf, "maps/");
    strcat(buf, mapname);
    ALLEGRO_FILE *file = al_fopen(s_get_full_path_with_dir(buf, "mapfile"), "w");

    if(!file)
    {
        perror("Error in map_save");
        return -1;
    }

    int count;
    memset(buf, 0, 1024);
    if((count = sprintf(buf, "%d,%d,%d,%d\n", map->width, map->height, map->chunksize, map->tilesize)) < 0)
    {
        perror("Error in map_save");
        return -1;
    }
    al_fwrite(file, buf, count);

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
                    struct tile *tile = &map->chunks[r][c]->tiles[r2][c2];
                    count = sprintf(buf, "%d,%d,%d,%d,%d,%d\n", tile->tilemap_x, tile->tilemap_y, tile->tilemap_z, tile->solid, tile->breakable, tile->damage);
                    al_fwrite(file, buf, count);
                }
            }
        }
    }

    al_fclose(file);
    return 0;
}

struct chunk *map_init_chunk(struct map *map, ALLEGRO_FILE *file, int x, int y)
{
    struct chunk *chunk;
    chunk = s_malloc(sizeof(struct chunk), NULL);
    chunk->tiles = s_malloc(sizeof(struct tile *) * map->chunksize, NULL);
    chunk->x = x;
    chunk->y = y;
    chunk->id = NULL;

    char buf[32];
    int r, c;
    for(r = 0; r < map->chunksize; r++)
    {
        chunk->tiles[r] = s_malloc(sizeof(struct tile) * map->chunksize, NULL);
        for(c = 0; c < map->chunksize; c++)
        {
            if(!al_fgets(file, buf, 32))
            {
                debug_perror("Corrupted map file: not enough tiles\n");
                chunk->tiles[r][c].tilemap_x = 0;
                chunk->tiles[r][c].tilemap_y = 0;
                chunk->tiles[r][c].tilemap_z = 1;
                chunk->tiles[r][c].solid = 0;
                chunk->tiles[r][c].breakable = 0;
                chunk->tiles[r][c].damage = 0;
            }
            else
            {
                chunk->tiles[r][c].tilemap_x = atoi(strtok(buf, ","));
                chunk->tiles[r][c].tilemap_y = atoi(strtok(NULL, ","));
                chunk->tiles[r][c].tilemap_z = atoi(strtok(NULL, ","));
                chunk->tiles[r][c].solid = atoi(strtok(NULL, ","));
                chunk->tiles[r][c].breakable = atoi(strtok(NULL, ","));
                chunk->tiles[r][c].damage = atoi(strtok(NULL, ","));
            }
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

void map_create_chunks(struct map *map, ALLEGRO_FILE *file)
{
    map->chunks = s_malloc(sizeof(struct chunk **) * map->height, NULL);

    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk *) * map->width, NULL);

        for(c = 0; c < map->width; c++)
        {
            map->chunks[r][c] = map_init_chunk(map, file, x, y);
            map->chunks[r][c]->index_x = c;
            map->chunks[r][c]->index_y = r;
            x += gridsize;
        }
        y -= gridsize;
        x = smx;
    }
}

struct map *map_load(char *dir)
{
    struct map *map = s_malloc(sizeof(struct map), "map: map_load");

    map->name = s_get_heap_string(dir);

    char buf[32];
    memset(buf, 0, 32);
    strcat(buf, "maps/");
    strcat(buf, dir);

    ALLEGRO_FILE *mapfile = al_fopen(s_get_full_path_with_dir(buf, "mapfile"), "r");

    if(!mapfile)
    {
        free(map->name);
        free(map);
        al_fclose(mapfile);
        debug_perror("Error in map_load\n");
        return NULL;
    }

    memset(buf, 0, 32);
    if(!al_fgets(mapfile, buf, 32))
    {
        free(map->name);
        free(map);
        al_fclose(mapfile);
        debug_perror("Error in map_load\n");
        return NULL;
    }

    map->width = atoi(strtok(buf, ","));
    map->height = atoi(strtok(NULL, ","));
    map->chunksize = atoi(strtok(NULL, ","));
    map->tilesize = atoi(strtok(NULL, ","));

    map_create_chunks(map, mapfile);

    al_fclose(mapfile);

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
    {
        for(c = 0; c < map->width; c++)
        {
            map_destroy_chunk(map->chunks[r][c], map->chunksize);
        }
        free(map->chunks[r]);
    }
    free(map->chunks);

    if(map->name)
        free(map->name);

    free(map);
}