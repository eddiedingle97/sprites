#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "map.h"
#include "list.h"
#include "sprites.h"
#include "map.h"
#include "spritemanager.h"
#include "debug.h"
#include "colors.h"
#include "emath.h"

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
    map->graph = NULL;
    map->palette = NULL;
    map->entitylists = s_malloc(sizeof(struct node *) * height * width, "map_create: map->entitylists");
    int r, c;
    for(r = 0; r < height; r++)
    {
        for(c = 0; c < width; c++)
        {
            map->entitylists[c + r * width] = NULL;
        }
    }
    map_create_test_chunk_list(map);

    return map;
}

void map_create_test_chunk_list(struct map *map)
{
    map->chunks = s_malloc(sizeof(struct chunk *) * map->height, NULL);
    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk) * map->width, NULL);
        for(c = 0; c < map->width; c++)
        {
            //map->chunks[r][c] = map_create_test_chunk(x, y, map->chunksize);
            struct chunk *chunk = &map->chunks[r][c];

            chunk->tiles = s_malloc(sizeof(struct tile) * map->chunksize * map->chunksize, NULL);

            int r2, c2;
            for(r2 = 0; r2 < map->chunksize; r2++)
            {
                for(c2 = 0; c2 < map->chunksize; c2++)
                {
                    chunk->tiles[c2 + r2 * map->chunksize].id = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                    /*chunk->tiles[c2 + r2 * map->chunksize].tilemap_x = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].tilemap_y = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].tilemap_z = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].func = 0;
                    chunk->tiles[c2 + r2 * map->chunksize].damage = 0;*/
                }
            }

            chunk->index_x = c;
            chunk->index_y = r;
            chunk->flags = 0;
            x += gridsize;
        }
        y -= gridsize;
        x = smx;
    }
}

void map_add_entity_to_chunk(struct map *map, struct entity *e)
{
    struct chunk *chunk = map_get_chunk_from_coordinate(map, e->sprite->x, e->sprite->y);
    if(!chunk)
    {
        debug_printf("entity is not on a chunk, not adding\n");
        return;
    }

    struct node *newhead, *node = map->entitylists[chunk->index_x + chunk->index_y * map->width];
    newhead = s_malloc(sizeof(struct node), NULL);
    newhead->prev = NULL;
    newhead->p = e;
    newhead->next = node;
    if(node)
        node->prev = newhead;

    map->entitylists[chunk->index_x + chunk->index_y * map->width] = newhead;
}

void map_remove_entity_from_chunk(struct map *map, struct chunk *chunk, struct entity *e)
{
    if(!chunk)
    {
        debug_printf("entity not on map, not removing\n");
        return;
    }
    
    struct node *node, *head = map->entitylists[chunk->index_x + chunk->index_y * map->width];
    for(node = head; node; node = node->next)
        if(node->p == e)
            break;

    if(!node)
    {
        debug_printf("entity not found in chunk, not removing\n");
        return;
    }
        
    if(node->next && node->prev)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        s_free(node, "mrefc middle");
    }
    else if(node->next && !node->prev)
    {
        head = head->next;
        s_free(head->prev, "mrefc head");
        head->prev = NULL;
    }
    else if(!node->next && node->prev)
    {
        node->prev->next = NULL;
        s_free(node, "mrefc tail");
    }
    else if(!node->next && !node->prev)
    {
        s_free(head, "mrefc single");
        head = NULL;
    }

    /*if(e->id == 0)
    {
        printf("%hd\n", chunk->flags);
    }*/

    map->entitylists[chunk->index_x + chunk->index_y * map->width] = head;
}

float map_get_chunk_x(struct map *map, struct chunk *chunk)//x of top left corner
{
    return map->tilesize * map->chunksize * chunk->index_x - map->tilesize * map->width * map->chunksize / 2;
}

float map_get_chunk_y(struct map *map, struct chunk *chunk)//y of top left corner
{
    return -map->tilesize * map->chunksize * chunk->index_y + map->tilesize * map->height * map->chunksize / 2;
}

struct chunk *map_get_chunk_from_coordinate(struct map *map, float x, float y)
{
    int chunkgrid = map->chunksize * map->tilesize;
    int pixelheight = map->height * chunkgrid;//height in pixels
    int pixelwidth = map->width * chunkgrid;//width in pixels
    y = pixelheight / 2 - y;
    x = pixelwidth / 2 + x;

    if(x < 0 || y < 0 || x >= pixelwidth || y >= pixelheight)
        return NULL;

    return &map->chunks[(int)y / chunkgrid][(int)x / chunkgrid];
}

struct chunk *map_get_chunk_from_index(struct map *map, int x, int y)
{
    if(x < 0 || y < 0 || x >= map->width || y >= map->height)
        return NULL;

    return &map->chunks[y][x];
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
                    struct tile *tile = &map->chunks[r][c].tiles[c2 + r2 * map->chunksize];
                    /*count = sprintf(buf, "%d,%d,%d,%d,%d\n", tile->tilemap_x, tile->tilemap_y, tile->tilemap_z, tile->type, tile->damage);
                    al_fwrite(file, buf, count);*/
                }
            }
        }
    }

    al_fclose(file);
    return 0;
}

struct tile *map_get_tile_from_coordinate(struct map *map, float x, float y)
{
    struct chunk *chunk = map_get_chunk_from_coordinate(map, x, y);

    if(!chunk)
        return NULL;

    x = x - map_get_chunk_x(map, chunk);
    y = map_get_chunk_y(map, chunk) - y;
    x /= map->tilesize;
    y /= map->tilesize;

    //printf("%.2f %.2f\n", x, y);
    
    return &chunk->tiles[(int)x + (int)y * map->chunksize];
}

void map_create_chunks(struct map *map, ALLEGRO_FILE *file)
{
    map->chunks = s_malloc(sizeof(struct chunk *) * map->height, NULL);

    int gridsize = map->tilesize * map->chunksize;
    int x = -(gridsize * map->width / 2);
    int smx = x;
    int y = (gridsize * map->height / 2);

    int r, c;
    for(r = 0; r < map->height; r++)
    {
        map->chunks[r] = s_malloc(sizeof(struct chunk) * map->width, NULL);

        for(c = 0; c < map->width; c++)
        {
            struct chunk *chunk = &map->chunks[r][c];
            chunk->tiles = s_malloc(sizeof(struct tile) * map->chunksize * map->chunksize, NULL);
            /*chunk->x = x;
            chunk->y = y;*/

            char buf[32];
            int r2, c2;
            for(r2 = 0; r2 < map->chunksize; r2++)
            {
                //chunk->tiles[r] = s_malloc(sizeof(struct tile) * map->chunksize, NULL);
                for(c2 = 0; c2 < map->chunksize; c2++)
                {
                    if(!al_fgets(file, buf, 32))
                    {
                        debug_perror("Corrupted map file: not enough tiles\n");
                        chunk->tiles[c2 + r2 * map->chunksize].id = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                        /*chunk->tiles[c2 + r2 * map->chunksize].tilemap_x = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].tilemap_y = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].tilemap_z = 1;
                        chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].func = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].damage = 0;*/
                    }
                    else
                    {
                        chunk->tiles[c2 + r2 * map->chunksize].id = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                        /*chunk->tiles[c2 + r2 * map->chunksize].tilemap_x = atoi(strtok(buf, ","));
                        chunk->tiles[c2 + r2 * map->chunksize].tilemap_y = atoi(strtok(NULL, ","));
                        chunk->tiles[c2 + r2 * map->chunksize].tilemap_z = atoi(strtok(NULL, ","));
                        chunk->tiles[c2 + r2 * map->chunksize].type = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].func = 0;
                        chunk->tiles[c2 + r2 * map->chunksize].damage = atoi(strtok(NULL, ","));*/
                    }
                }
            }

            chunk->index_x = c;
            chunk->index_y = r;
            chunk->flags = 0;
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
        s_free(map->name, NULL);
        s_free(map, NULL);
        al_fclose(mapfile);
        debug_perror("Error in map_load\n");
        return NULL;
    }

    memset(buf, 0, 32);
    if(!al_fgets(mapfile, buf, 32))
    {
        s_free(map->name, NULL);
        s_free(map, NULL);
        al_fclose(mapfile);
        debug_perror("Error in map_load\n");
        return NULL;
    }

    map->width = atoi(strtok(buf, ","));
    map->height = atoi(strtok(NULL, ","));
    map->chunksize = atoi(strtok(NULL, ","));
    map->tilesize = atoi(strtok(NULL, ","));
    map->graph = NULL;
    map->entitylists = NULL;

    map_create_chunks(map, mapfile);

    al_fclose(mapfile);

    return map;
}

void map_destroy_chunk(struct chunk *chunk)
{
    s_free(chunk->tiles, NULL);
}

void map_destroy(struct map *map)
{
    if(map == NULL)
        return;

    int r, c;
    struct node *node, *next;
    for(r = 0; r < map->height; r++)
    {
        for(c = 0; c < map->width; c++)
        {
            map_destroy_chunk(&map->chunks[r][c]);
            node = map->entitylists[c + r * map->width];
            for(; node; node = next)
            {
                next = node->next;
                s_free(node, NULL);
            }
        }
        s_free(map->chunks[r], NULL);
    }
    s_free(map->chunks, NULL);
    s_free(map->entitylists, NULL);

    if(map->name)
        s_free(map->name, NULL);

    if(map->graph)
    {
        s_free(map->graph->vertices[0].p, NULL);//BAD HACK, NEEDS FIX
        graph_destroy(map->graph);
    }

    s_free(map->palette, NULL);

    s_free(map, "freeing map");
}