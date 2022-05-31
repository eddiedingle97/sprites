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
void em_do_speed(struct entity *e, float dx, float dy);

static struct entity *knight;
static ALLEGRO_BITMAP *spritesheet;
static struct entity *(**create)();
static void (**destroy)(struct entity *);
static void (**behaviour)(struct entity *, float *, float *);
static char *isitem;
static int registeredentities;

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

void em_init()
{
    //spritesheet = al_load_bitmap(s_get_full_path_with_dir("images", "0x72_DungeonTilesetII_v1.3.png"));
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
    isitem = NULL;
    registeredentities = 0;
}

int em_register_entity(struct entity *(*c)(), void (*b)(struct entity *, float *, float *), void (*d)(struct entity *), char i)
{
    create = s_realloc(create, ++registeredentities * sizeof(void (*)()), NULL);
    create[registeredentities - 1] = c;
    behaviour = s_realloc(behaviour, registeredentities * sizeof(void (*)()), NULL);
    behaviour[registeredentities - 1] = b;
    destroy = s_realloc(destroy, registeredentities * sizeof(void (*)()), NULL);
    destroy[registeredentities - 1] = d;
    isitem = s_realloc(isitem, registeredentities * sizeof(char), NULL);
    isitem[registeredentities - 1] = i;
    return registeredentities - 1;
}

struct entity *em_create_entity(unsigned char id, float x, float y)
{
    if(id < 0 || id >= registeredentities)
        return NULL;
    struct entity *out = create[id]();
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
    struct map *map = NULL, *topmap = mm_get_top_map();
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
            int draw = 0;
            for(r = 0; r < map->height; r++)
                for(c = 0; c < map->width; c++)
                {
                    draw = map == topmap && mm_is_chunk_loaded(c, r);
                    node = map->chunks[r][c].ehead;
                    for(; node; node = next)//FOR EACH ENTITY
                    {
                        next = node->next;
                        dx = 0;
                        dy = 0;
                        e = node->p;
                        behaviour[e->id](e, &dx, &dy);

                        em_do_speed(e, dx, dy);
                        dx = e->speedx;
                        dy = e->speedy;

                        em_do_movement(map, e, &dx, &dy);
                        
                        if(e->id == 0)
                            sm_move_coord(dx, dy);
                        
                        mm_call_tile_functions(map, e);

                        if(draw)
                            sm_add_sprite_to_layer(e->sprite);
                        else
                            sm_remove_sprite_from_layer(e->sprite);

                        /*if(draw)
                        {
                            if(!isitem[e->id])
                            {
                                sm_deferred_draw(e->sprite);
                                if(e->hand)
                                {
                                    sm_deferred_draw(e->hand->sprite);
                                }
                            }
                            else if(isitem[e->id] && !e->holder)
                                sm_deferred_draw(e->sprite);
                        }*/
                    }
                }
        }
    }
}

void em_do_speed(struct entity *e, float dx, float dy)
{
    if(dx == 0.0f && dy == 0.0f)
        return;
    float invdist = math_get_inverse_distance(dx, dy);
    dx = dx * invdist * e->accel;
    dy = dy * invdist * e->accel;
    
    e->speedx = e->speedx + dx;
    e->speedy = e->speedy + dy;
    invdist = math_get_inverse_distance(e->speedx, e->speedy);
    if(e->maxspeed * invdist < 1.0f)
    {
        e->speedx = e->speedx * invdist * e->maxspeed;
        e->speedy = e->speedy * invdist * e->maxspeed;
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

    if(!*dx && !*dy && !isitem[e->id])
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
                    if(math_sqrt(ox * ox + oy * oy) < e->colrad + othere->colrad)
                    {
                        if((isitem[e->id] && e->holder) || (isitem[othere->id] && othere->holder))
                            collide = 0;
                        else
                        {
                            collide = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    if(!collide)
    {
        e->sprite->x += *dx;
        e->sprite->y += *dy;
        if(!isitem[e->id] && e->hand)
        {
            e->hand->sprite->x += *dx;
            e->hand->sprite->y += *dy;
        }

        e->speedx -= e->speedx * .2f / (e->weight / 180.0f);
        e->speedy -= e->speedy * .2f / (e->weight / 180.0f);

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

void em_do_collide(struct entity *one, struct entity *two)
{

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
    s_free(isitem, NULL);
    //al_destroy_bitmap(spritesheet);
}