#include <stdio.h>
#include <allegro5/allegro.h>
#include "sprites.h"
#include "emath.h"
#include "spritemanager.h"
#include "entity.h"
#include "list.h"
#include "entitymanager.h"
#include "map.h"
#include "action.h"
#include "mapmanager.h"
#include "debug.h"
#include "colors.h"

#include "entities/entities.h"

enum EM_COMPONENTS {ISITEM = 1};

void em_do_movement(struct map *map, int curcolumn, int currow, struct entity *e, float *dx, float *dy);
void em_do_speed(struct entity *e, float dx, float dy);
void em_do_collide(struct entity *one, struct entity *two, float dist);

static struct entity *knight;
static struct entity *(**create)();
static void (**destroy)(struct entity *);
static void (**behaviour)(struct entity *, float *, float *);
static char *components;
static void (**actiontable)(struct entity *);
static int registeredentities;

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

static float frictionconstant = .2f;
static float collisionpushconstant = .1f;
static float knockbackconstant = 15;

void em_init()
{
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
    components = NULL;
    actiontable = s_malloc(sizeof(void (*)(struct entity *)) * 1, NULL);
    actiontable[0] = action_swing;
    registeredentities = 0;
}

int em_register_entity(struct entity *(*c)(), void (*b)(struct entity *, float *, float *), void (*d)(struct entity *), char comps)
{
    create = s_realloc(create, ++registeredentities * sizeof(void (*)()), NULL);
    create[registeredentities - 1] = c;
    behaviour = s_realloc(behaviour, registeredentities * sizeof(void (*)()), NULL);
    behaviour[registeredentities - 1] = b;
    destroy = s_realloc(destroy, registeredentities * sizeof(void (*)()), NULL);
    destroy[registeredentities - 1] = d;
    components = s_realloc(components, registeredentities * sizeof(char), NULL);
    components[registeredentities - 1] = comps;
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
    if(components[id] & ISITEM)
    {
        out->rotx = x;
        out->roty = y;
    }
    return out;
}

int em_add_entity_to_map(struct map *map, struct entity *e)
{
    if(!e)
        return 1;
    if(map)
        map_add_entity_to_chunk(map, e);
    else
        return 1;
    return 1;
}

int em_remove_entity_from_map(struct map *map, struct chunk *chunk, struct entity *e)
{
    if(!e)
        return 1;
    if(map)
        map_remove_entity_from_chunk(map, chunk, e);
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
    struct chunk *curchunk = NULL;
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
                    draw = mm_is_chunk_loaded(c, r) && map == topmap;
                    node = map->entitylists[c + r * map->width];
                    curchunk = &map->chunks[r][c];

                    for(; node; node = next)//FOR EACH ENTITY
                    {
                        next = node->next;
                        dx = 0;
                        dy = 0;
                        e = node->p;
                        behaviour[e->id](e, &dx, &dy);

                        if(!(components[e->id] & ISITEM))
                        {
                            if(e->health <= 0)
                            {
                                em_remove_entity_from_map(map, curchunk, e);
                                sm_remove_sprite_from_layer(e->sprite);
                                destroy[e->id](e);
                                continue;
                            }

                            if(e->actions)
                            {
                                actiontable[e->actions->actionid](e);
                                if(e->actions->done)
                                    action_destroy(e);
                            }
                        }

                        em_do_speed(e, dx, dy);

                        em_do_movement(map, r, c, e, &dx, &dy);

                        if(e->id == 0)
                        {
                            sm_set_coord(e->sprite->x, e->sprite->y);
                        }
                        
                        //mm_call_tile_functions(map, e); DO EVENT TILE HANDLING

                        if(draw)
                            sm_add_sprite_to_layer(e->sprite);
                        else
                            sm_remove_sprite_from_layer(e->sprite);
                        
                    }
                }
        }
    }
}

void em_do_speed(struct entity *e, float dx, float dy)
{
    if(dx == 0.0f && dy == 0.0f)
        return;
    float dist = math_get_distance(dx, dy);
    dx = dx * e->accel / dist;
    dy = dy * e->accel / dist;
    
    e->speedx = e->speedx + dx;
    e->speedy = e->speedy + dy;
}

void em_do_movement(struct map *map, int currow, int curcolumn, struct entity *e, float *dx, float *dy)
{
    int i, j, collide = 0;
    float ox, oy;
    struct node *node = NULL;
    struct chunk *chunk = NULL;
    struct entity *othere = NULL;
    *dx = e->speedx;
    *dy = e->speedy;

    /*if(e->id == 1)
    {
        //struct orcdata *data = e->data;
        printf("%.2f %.2f %.2f\n", e->speedx, e->speedy, *dx);
    }*/

    struct tile *currenttile = map_get_tile_from_coordinate(map, e->sprite->x, e->sprite->y);
    if(currenttile)
    {
        struct tile *nexttile = map_get_tile_from_coordinate(map, e->sprite->x + *dx, e->sprite->y);
        if(nexttile && nexttile->type & SOLID)
        {
            *dx = 0;
            e->speedx = 0;
        }
        
        nexttile = map_get_tile_from_coordinate(map, e->sprite->x, e->sprite->y + *dy);
        if(nexttile && nexttile->type & SOLID)
        {
            *dy = 0;
            e->speedy = 0;
        }
    }

    if(!*dx && !*dy && !(components[e->id] & ISITEM))
        collide = 1;

    for(i = -1; i < 2 && !collide; i++)//-1, 0, 1
    {
        for(j = -1; j < 2 && !collide; j++)//-1, 0, 1
        {
            int x = curcolumn + i, y = currow + j;
            if(x < 0 || x >= map->width || y < 0 || y >= map->height)
                break;
            node = map->entitylists[curcolumn + i + (currow + j) * map->width];

            for(; node; node = node->next)
            {
                othere = node->p;
                if(e != othere)//FIX: comparing pointers... probably bad
                {
                    ox = e->sprite->x + *dx - othere->sprite->x;
                    oy = e->sprite->y + *dy - othere->sprite->y;
                    float dist = math_get_distance(ox, oy);
                    float coldist = e->colrad + othere->colrad;
                    if(dist < coldist)
                    {
                        em_do_collide(e, othere, coldist - dist);
                    }
                }
            }
        }
    }

    if(!collide)
    {
        e->sprite->x += *dx;
        e->sprite->y += *dy;
        if(!(components[e->id] & ISITEM) && e->hand)
        {   
            e->hand->rotx = e->sprite->x;
            e->hand->roty = e->sprite->y;
            e->hand->sprite->x += *dx;
            e->hand->sprite->y += *dy;
        }
        
        else if(components[e->id] & ISITEM)
        {
            float r;
            if(e->holder)
                r = math_get_distance(e->holdx, e->holdy) + e->holder->colrad;
            else
                r = math_get_distance(e->rotx - e->sprite->x, e->roty - e->sprite->y);
            e->sprite->rot += e->angvel;
            e->rotx += *dx;
            e->roty += *dy;
            e->sprite->x = e->rotx + math_cos(e->sprite->rot) * r;
            e->sprite->y = e->roty + math_sin(e->sprite->rot) * r;

            if(e->holder)
            {
                float r = math_get_distance(e->holdx, e->holdy) + e->holder->colrad;
                float pushangle = e->sprite->rot - e->angvel / 2;
                e->holder->speedx += math_cos(pushangle) * math_abs(e->angvel) * r / 6 * e->weight / (e->holder->weight + e->weight);
                e->holder->speedy += math_sin(pushangle) * math_abs(e->angvel) * r / 6 * e->weight / (e->holder->weight + e->weight);
            }

            e->angvel -= e->angvel * frictionconstant;
        }

        e->speedx -= e->speedx * frictionconstant;
        e->speedy -= e->speedy * frictionconstant;

        chunk = map_get_chunk_from_coordinate(map, e->sprite->x, e->sprite->y);

        if(chunk->index_x != curcolumn || chunk->index_y != currow)
        {
            em_remove_entity_from_map(map, &map->chunks[currow][curcolumn], e);
            em_add_entity_to_map(map, e);
        }
    }

    else
    {
        *dx = 0;
        *dy = 0;
    }
}

void em_do_collide(struct entity *one, struct entity *two, float dist)
{
    if(!(components[one->id] & ISITEM) && !(components[two->id] & ISITEM))
    {
        float dist = math_get_distance(one->sprite->x - two->sprite->x, one->sprite->y - two->sprite->y);
        if(dist == 0)
            dist = .001;
        float pushx = dist * collisionpushconstant * (one->accel > math_abs(one->speedx) ? one->accel : math_abs(one->speedx)) * (one->sprite->x - two->sprite->x) / dist;
        float pushy = dist * collisionpushconstant * (one->accel > math_abs(one->speedy) ? one->accel : math_abs(one->speedy)) * (one->sprite->y - two->sprite->y) / dist;

        one->speedx += pushx * two->weight / (two->weight + one->weight);
        one->speedy += pushy * two->weight / (two->weight + one->weight);
        two->speedx -= pushx * one->weight / (two->weight + one->weight);
        two->speedy -= pushy * one->weight / (two->weight + one->weight);
    }
    else if((components[one->id] & ISITEM) && !(components[two->id] & ISITEM))
    {
        if(one->holder && one->holder->actions)
        {
            float r = math_get_distance(one->holdx, one->holdy) + one->holder->colrad;
            float speedx = one->holder->speedx + one->speedx + one->angvel * math_cos(one->sprite->rot) * r;
            float speedy = one->holder->speedy + one->speedy + one->angvel * math_sin(one->sprite->rot) * r;
            
            two->speedx -= speedx * (one->weight * knockbackconstant) / (one->weight + two->weight);
            two->speedy -= speedy * (one->weight * knockbackconstant) / (one->weight + two->weight);
            two->health -= one->damage;
        }
    }
    else if(!(components[one->id] & ISITEM) && (components[two->id] & ISITEM))
    {
        if(!two->holder)
        {
            one->hand = two;
            two->holder = one;
        }
    }
    else if((components[one->id] & ISITEM) && (components[two->id] & ISITEM));
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
                for(node = map->entitylists[c + r * map->width]; node; node = node->next)
                    {
                        e = node->p;
                        destroy[e->id](e);
                    }
    }

    s_free(create, NULL);
    s_free(behaviour, NULL);
    s_free(destroy, NULL);
    s_free(components, NULL);
    s_free(actiontable, NULL);
}