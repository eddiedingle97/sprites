#include <stdio.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "emath.h"
#include "spritemanager.h"
#include "entity.h"
#include "list.h"
#include "entitymanager.h"
#include "map.h"
#include "mapmanager.h"
#include "debug.h"
#include "colors.h"

#include "entities/entities.h"

void em_do_movement(struct map *map, struct entity *e, float *dx, float *dy);

static struct entity *knight;
static ALLEGRO_BITMAP *spritesheet;
static struct entity *(**create)(ALLEGRO_BITMAP *);
static void (**destroy)(struct entity *);
static void (**behaviour)(struct entity *, int *, int *);
static int registeredentities;

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

void em_init()
{
    spritesheet = al_load_bitmap(s_get_full_path_with_dir("images", "DungeonAllEntities.png"));
    if(!spritesheet)
        debug_perror("Spritesheet failed to load in em_init\n");

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
    create = NULL;
    behaviour = NULL;
    destroy = NULL;
    registeredentities = 0;
}

int em_register_entity(struct entity *(*c)(ALLEGRO_BITMAP *), void (*b)(struct entity *, float *, float *), void (*d)(struct entity *))
{
    create = s_realloc(create, ++registeredentities * sizeof(void (*)()), NULL);
    create[registeredentities - 1] = c;
    behaviour = s_realloc(behaviour, registeredentities * sizeof(void (*)()), NULL);
    behaviour[registeredentities - 1] = b;
    destroy = s_realloc(destroy, registeredentities * sizeof(void (*)()), NULL);
    destroy[registeredentities - 1] = d;
    return registeredentities - 1;
}

struct entity *em_create_entity(unsigned char id, float x, float y)
{
    if(id >= registeredentities)
        return NULL;
    struct entity *out = create[id](spritesheet);
    out->id = id;
    out->sprite->x = x;
    out->sprite->y = y;
    return out;
}

int em_add_entity_to_chunk(struct map *map, struct entity *e)
{
    if(!e)
        return 1;
    if(!e->chunk && map)
        map_add_entity_to_chunk(map, e);
    else
        return 1;
    return 1;
}

int em_remove_entity_from_chunk(struct map *map, struct entity *e)
{
    if(!e)
        return 1;
    if(!e->chunk)
        return 0;
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
    return &map->chunks[r][c];
}

void em_tick()
{
    struct list *maps = mm_get_map_list();
    struct map *map = NULL;
    struct node *mlnode = NULL;
    for(mlnode = maps->head; mlnode; mlnode = mlnode->next)
    {
        map = mlnode->p;
        if(map)
        {
            struct node *node, *next;
            struct entity *e;
            
            int r, c;
            float dx, dy;
            for(r = 0; r < map->height; r++)
                for(c = 0; c < map->width; c++)
                {
                    node = map->chunks[r][c].ehead;
                    for(; node; node = next)//FOR EACH ENTITY
                    {
                        next = node->next;
                        dx = 0;
                        dy = 0;
                        e = node->p;
                        behaviour[e->id](e, &dx, &dy);
                        
                        em_do_movement(map, e, &dx, &dy);
                        if(e->id == 0)
                            sm_move_coord(dx, dy);
                        
                        mm_call_tile_functions(map, e);
                    }
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
            em_remove_entity_from_chunk(map, e);
            em_add_entity_to_chunk(map, e);
        }
    }

    else
    {
        *dx = 0;
        *dy = 0;
    }
}

void em_destroy()
{
    struct list *maps = mm_get_map_list();
    struct entity *e;
    int r, c;
    struct node *node, *mlnode;
    struct map *map;
    for(mlnode = maps->head; mlnode; mlnode = mlnode->next)
    {
        map = mlnode->p;
        for(r = 0; r < map->height; r++)
            for(c = 0; c < map->width; c++)
                for(node = map->chunks[r][c].ehead; node; node = node->next)
                    {
                        e = node->p;
                        destroy[e->id](e);
                    }
    }

    s_free(create, NULL);
    s_free(behaviour, NULL);
    s_free(destroy, NULL);
    al_destroy_bitmap(spritesheet);
}