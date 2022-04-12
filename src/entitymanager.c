#include <stdio.h>
#include <allegro5/allegro.h>
#include "spritemanager.h"
#include "entity.h"
#include "list.h"
#include "entitymanager.h"
#include "map.h"
#include "mapmanager.h"
#include "debug.h"
#include "colors.h"

#include "entities/knight.c"
#include "entities/orc.c"

void em_do_movement(struct map *map, struct entity *e, float *dx, float *dy);

static struct entity *knight;
static ALLEGRO_BITMAP *spritesheet;
static struct list *entities;

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

void em_init()
{
    spritesheet = al_load_bitmap(s_get_full_path_with_dir("images", "DungeonAllEntities.png"));
    if(!spritesheet)
        debug_perror("Spritesheet failed to load in em_init\n");
    knight = knight_create(spritesheet);
    sm_add_sprite_to_layer(knight->sprite);
    entities = list_create();
    list_append(entities, knight);

    ALLEGRO_BITMAP *boxbitmap = al_create_bitmap(collisionboxsize, collisionboxsize);
    al_set_target_bitmap(boxbitmap);
    al_lock_bitmap(boxbitmap, 0, 0);
    int i, thick;
    for(thick = 0; thick < 1; thick++)
        for(i = 0; i < collisionboxsize; i++)
        {
            al_draw_pixel(i, thick, RED);
            al_draw_pixel(i, collisionboxsize - 1 - thick, RED);
            al_draw_pixel(thick, i, RED);
            al_draw_pixel(collisionboxsize - 1 - thick, i, RED);
        }
    al_unlock_bitmap(boxbitmap);
    collisionbox = sm_create_sprite(boxbitmap, 0, 0, PLAYER, CENTERED);

    debug_add_sprite(collisionbox);
    collision = 1;
}

struct entity *em_create_enemy(float x, float y)
{
    struct entity *new = orc_create(spritesheet);
    struct orcdata *od = new->data;
    list_append(entities, new);
    od->target = knight;
    sm_add_sprite_to_layer(new->sprite);
    new->sprite->x = x;
    new->sprite->y = y;
    return new;
}

int em_add_entity_to_chunk(struct entity *e)
{
    if(!e)
        return 1;
    struct map *map = mm_get_top_map();
    if(!e->chunk && map)
        map_add_entity_to_chunk(map, e);
    else
        return 1;
    return 1;
}

int em_remove_entity_from_chunk(struct entity *e)
{
    if(!e)
        return 1;
    if(!e->chunk)
        return 0;
    struct map *map = mm_get_top_map();
    if(map)
        map_remove_entity_from_chunk(map, e);
    else
        return 0;
    return 1;
}

struct chunk *em_get_chunk(struct map *map, int r, int c)
{
    if(r < 0 || r >= map->height)
        return NULL;
    if(c < 0 || c >= map->width)
        return NULL;
    return &map->chunks[r][c][0];
}

void em_tick()
{
    struct map *map = mm_get_top_map();
    if(map)
    {
        struct node *node, *next;
        struct entity *e;
        int i, times = entities->size;
        for(i = 0; i < times; i++)
        {
            e = list_get(entities, 0);
            if(em_add_entity_to_chunk(e))
            {
                list_delete(entities, 0);
            }
        }
        
        int r, c;
        float dx, dy;
        for(r = 0; r < map->height; r++)
            for(c = 0; c < map->width; c++)
            {
                node = map->chunks[r][c][0].ehead;
                for(; node; node = next)
                {
                    next = node->next;
                    dx = 0;
                    dy = 0;
                    e = node->p;
                    e->behaviour(e, &dx, &dy);
                    
                    em_do_movement(map, e, &dx, &dy);
                    if(e == knight)
                        sm_move_coord(dx, dy);
                }
            }
    }
}

void em_do_movement(struct map *map, struct entity *e, float *dx, float *dy)
{
    int i, j, collide = 0;
    float ox, oy;
    struct chunk *chunk = NULL;
    struct node *node = NULL;
    struct entity *othere = NULL;

    struct tile *currenttile = map_get_tile_from_coordinate(map, e->sprite->x, e->sprite->y);
    if(currenttile)
    {
        struct tile *nexttile = map_get_tile_from_coordinate(map, e->sprite->x + *dx, e->sprite->y);
        if(nexttile && nexttile->type & SOLID)
        {
            *dx = 0;
        }
        
        nexttile = map_get_tile_from_coordinate(map, e->sprite->x, e->sprite->y + *dy);
        if(nexttile && nexttile->type & SOLID)
        {
            *dy = 0;
        }
    }

    if(!*dx && !*dy)
        collide = 1;

    for(i = -1; i < 2 && !collide; i++)
    {
        for(j = -1; j < 2 && !collide; j++)
        {
            chunk = em_get_chunk(map, e->chunk->index_y + i, e->chunk->index_x + j);
            if(!chunk)
                continue;
            node = chunk->ehead;

            for(; node; node = node->next)
            {
                othere = node->p;
                if(e != othere)
                {
                    ox = e->sprite->x + *dx - othere->sprite->x;
                    oy = e->sprite->y + *dy - othere->sprite->y;
                    if(math_sqrt(ox * ox + oy * oy) < 12.0f)
                    {
                        collide = 1;
                        break;
                    }
                }
            }
        }
    }

    if(!collide)
    {
        e->sprite->x += *dx;
        e->sprite->y += *dy;
        if(e->chunk != map_get_chunk_from_coordinate(map, e->sprite->x, e->sprite->y))
        {
            em_remove_entity_from_chunk(e);
            em_add_entity_to_chunk(e);
        }
        //add entities switching chunks here
    }

    else
    {
        *dx = 0;
        *dy = 0;
    }
}

void em_destroy()
{
    struct map *map = mm_get_top_map();
    int r, c;
    struct node *node;
    for(r = 0; r < map->height; r++)
        for(c = 0; c < map->width; c++)
            for(node = map->chunks[r][c][0].ehead; node; node = node->next)
                e_destroy(node->p);

    list_destroy_with_function(entities, e_destroy);
    al_destroy_bitmap(spritesheet);
}