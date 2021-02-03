#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "list/list.h"
#include "spritemanager.h"
#include "tilemanager.h"
#include "sprites.h"
#include "map.h"
#include "colors.h"
#include "enums.h"

static ALLEGRO_BITMAP *test, *purplebitmap;

struct chunk *map_init_chunk(struct map *map, FILE *file, int x, int y);
struct chunk *map_create_test_chunk(int x, int y, int chunksize, int tilesize);
void map_create_test_chunk_list(struct map *map);
void map_create_chunks(struct map *map, FILE *file);

/*void map_init()
{
    test = al_create_bitmap(16, 16);
    al_set_target_bitmap(test);
    al_clear_to_color(WHITE);

    al_lock_bitmap(test, 0, 0);
    int r, c;
    for(r = 0; r < 16; r++)
    {
        for(c = 0; c < 16; c++)
        {
            if(r == 0 || r == 15)
                al_draw_pixel(r, c, BLACK);
            if(c == 0 || c == 15)
                al_draw_pixel(r, c, BLACK);
        }
    }
    al_unlock_bitmap(test);

    purplebitmap = al_create_bitmap(16, 16);
    al_set_target_bitmap(purplebitmap);
    al_lock_bitmap(purplebitmap, 0, 0);
    for(r = 0; r < 16; r++)
    {
        for(c = 0; c < 16; c++)
        {
            al_draw_pixel(r, c, PURPLE);
        }
    }
    al_unlock_bitmap(purplebitmap);
}

void map_destroy()
{
    al_destroy_bitmap(test);
    al_destroy_bitmap(purplebitmap);
}*/

struct map *map_create(int chunksize, int tilesize, int width, int height)
{
    struct map *map = s_malloc(sizeof(struct map), "map_create");
    map->chunksize = chunksize;
    map->tilesize = tilesize;
    map->height = height;
    map->width = width;
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
            map->chunks[r][c] = map_create_test_chunk(x, y, map->chunksize, map->tilesize);
            map->chunks[r][c]->index_x = c;
            map->chunks[r][c]->index_y = r;
            map->chunks[r][c]->id = NULL;
            x += gridsize;
        }
        y -= gridsize;
        x = smx;
    }
}

struct chunk *map_create_test_chunk(int x, int y, int chunksize, int tilesize)
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
            //chunk->tiles[r][c].sprite = sm_create_global_sprite(al_clone_bitmap(test), c * tilesize + x, -r * tilesize + y, BACKGROUND, 0);
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
    y = pixelheight / 2 - y;
    x = pixelwidth / 2 + x;

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

/*struct chunk *map_init_chunk_to_layer(ALLEGRO_BITMAP *bitmap, int x, int y, int chunksize, int tilesize)
{
    struct chunk *chunk;
    chunk = s_malloc(sizeof(chunk), "chunk: map_init_chunk_to_layer");
    chunk->tiles = s_malloc(sizeof(struct tile *) * chunksize, "chunk->tiles: map_init_chunk_to_layer");
    chunk->x = x;
    chunk->y = y;

    int r, c;
    for(r = 0; r < chunksize; r++)
    {
        chunk->tiles[r] = s_malloc(chunksize * sizeof(struct tile), "chunk->tiles[r]: map_init_chunk_to_layer");

        for(c = 0; c < chunksize; c++)
        {
            chunk->tiles[r][c].sprite = sm_create_global_sprite(bitmap, c * tilesize + x, r * tilesize + y, BACKGROUND, 0);
            sm_add_sprite_to_layer(chunk->tiles[r][c].sprite);
        }
    }

    return chunk;
}*/

int map_save(struct map *map, const char *filepath)
{
    printf("here\n");
    FILE *file = fopen(filepath, "w");

    if(!file)
    {
        perror("Error in map_save");
        return -1;        
    }

    int count;
    char buf[1024];
    if((count = sprintf(buf, "%d,%d,%d,%d\n", map->width, map->height, map->chunksize, map->tilesize)) < 0)
    {
        perror("Error in map_save");
        return -1;
    }
    fwrite(buf, sizeof(char), count, file);

    int i;
    if(map->tilemaps)
        for(i = 0; i < map->tilemaps->size; i++)
        {
            char *tilemapfile = ((struct tilemap *)list_get(map->tilemaps, i))->tilemapfile;
            memcpy(buf, tilemapfile, strlen(tilemapfile));
            fwrite(buf, sizeof(char), strlen(buf), file);
            putc('\n', file);
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

struct chunk *map_init_chunk(struct map *map, FILE *file, int x, int y)
{
    struct chunk *chunk;
    chunk = s_malloc(sizeof(chunk), "chunk: map_init_chunk");
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
            tile->tilemap_z = atoi(strtok(NULL, ","));
            tile->solid = atoi(strtok(NULL, ","));
            tile->breakable = atoi(strtok(NULL, ","));
            tile->damage = atoi(strtok(NULL, ","));

            /*if(tile->tilemap_z < 0)
            {
                if(tile->tilemap_z == -1)
                {
                    tile->sprite = sm_create_global_sprite(al_clone_bitmap(test), c * map->tilesize + x, -r * map->tilesize + y, BACKGROUND, 0);
                }
                else
                    tile->sprite = sm_create_global_sprite(al_clone_bitmap(purplebitmap), c * map->tilesize + x, -r * map->tilesize + y, BACKGROUND, 0);
            }
            else
            {
                ALLEGRO_BITMAP *tilemap = ((struct tilemap *)list_get(map->tilemaps, tile->tilemap_z))->bitmap;
                tile->sprite = sm_create_global_sprite(al_create_sub_bitmap(tilemap, tile->tilemap_x, tile->tilemap_y, map->tilesize, map->tilesize), c * map->tilesize + x, -r * map->tilesize + y, BACKGROUND, 0);
            }*/
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

void map_create_chunks(struct map *map, FILE *file)
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
            map->chunks[r][c] = map_init_chunk(map, file, x, y);
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
    memset(buf, 0, 1024);

    while(strcmp(fgets(buf, 1024, file), "\n"))
    {
        struct tilemap *tm = s_malloc(sizeof(struct tilemap), "tilemap: map_load");
        buf[strlen(buf) - 1] = '\0';
        tm->tilemapfile = s_get_heap_string(buf);
        memset(buf, 0, 1024);
        strcat(buf, "images/");
        tm->bitmap = al_load_bitmap(s_get_full_path(strcat(buf, tm->tilemapfile)));

        if(!tm->bitmap)
            fprintf(stderr, "Error loading tilemap");
        else
            list_append(map->tilemaps, tm);
            
        memset(buf, 0, 1024);
    }

    map_create_chunks(map, file);

    fclose(file);

    return map;
}

void map_destroy_chunk(struct chunk *chunk, int chunksize)
{
    int r, c;
    for(r = 0; r < chunksize; r++)
    {
        /*for(c = 0; c < chunksize; c++)
        {
            if(chunk->tiles[r][c].sprite->id == NULL)
                sm_destroy_sprite(chunk->tiles[r][c].sprite);
            else
                sm_destroy_sprite_from_layer(chunk->tiles[r][c].sprite);
        }*/
        free(chunk->tiles[r]);
    }
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
    }

    free(map);
}
