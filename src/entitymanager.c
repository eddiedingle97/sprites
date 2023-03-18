#include <stdio.h>
#include <math.h>
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
#include "dictionary.h"
#include "debug.h"
#include "colors.h"

#include "entities/entities.h"

enum EM_COMPONENTS {ITEM = 1, HOLDABLE = 2, CANHOLD = 4, CIRCULARHITBOX = 8};

#define ISITEM(e) (components[e->id] & ITEM)
#define ISCIRCULAR(e) (components[e->id] & CIRCULARHITBOX)

void em_do_movement(struct map *map, int curcolumn, int currow, struct entity *e, float *dx, float *dy);
void em_do_speed(struct entity *e, float dx, float dy);
void em_do_collide(struct entity *one, struct entity *two, float dist);
int em_do_square_circle_collision(struct entity *square, struct entity *circle, float *dx, float *dy, int squaremove);

static struct entity *knight;
static struct entity *(**create)();
static void (**destroy)(struct entity *);
static void (**behaviour)(struct entity *, int, float *, float *);
static char *components;
static void (**actiontable)(struct entity *);
static int registeredentities;
static int tick = 0;

static char collision;
static const int collisionboxsize = 16;
static struct sprite *collisionbox;

static float frictionconstant = .2f;
static float collisionpushconstant = .1f;
static float knockbackconstant = 5;

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

int em_register_entity(struct entity *(*c)(), void (*b)(struct entity *, int, float *, float *), void (*d)(struct entity *), char comps)
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
    int i;
    for(i = 1; i < (1<<7); i <<= 1)
        switch(components[id] & i)
        {
            case 0:
                break;
            case ITEM:
                out->rotx = x;
                out->roty = y;
                break;
            case HOLDABLE:
                break;
            case CANHOLD:
                break;
        }
    return out;
}

int em_add_entity_to_map(struct map *map, struct entity *e)
{
    if(!e)
        return 0;
    if(map)
        map_add_entity_to_chunk(map, e);
    else
        return 0;
    return 1;
}

int em_remove_entity_from_map(struct map *map, struct chunk *chunk, struct entity *e)
{
    if(!e)
        return 0;
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
    tick++ % FPS;
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
                        /*
                            OPT: get tile and get chunk functions are called multiple times in this loop for each entity
                                cache should help a lot with this, not a big deal
                        */ 

                        next = node->next;
                        dx = 0;
                        dy = 0;
                        e = node->p;
                        behaviour[e->id](e, tick, &dx, &dy);

                        if(!ISITEM(e))
                        {
                            if(e->health < 0 && e->id != 0)//don't kill player for now
                            {
                                if(e->id == 0)
                                    puts("killing player");
                                em_remove_entity_from_map(map, curchunk, e);
                                sm_remove_sprite_from_layer(e->sprite);
                                if(e->hand)
                                    e->hand->holder = NULL;
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
                        
                        void (*tileevent)(struct map *, struct entity *) = mm_get_tile_event(map, e);
                        if(tileevent)
                            tileevent(map, e);

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

    if(!*dx && !*dy && !ISITEM(e))
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
                    /*
                        OPT: merge all of this into one function with em_do_collide, maybe use switch statement
                    */
                    if(ISCIRCULAR(e) && ISCIRCULAR(othere))
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
                    else if(!ISCIRCULAR(e) && ISCIRCULAR(othere))//FIX: skip collision with holder / held items
                    {
                        //skip check if it will obviously not collide
                        int largest = (e->width > e->height) ? e->width / 2 : e->height / 2;
                        if(othere->sprite->x + othere->colrad < e->sprite->x + *dx - largest
                            || othere->sprite->x - othere->colrad > e->sprite->x + *dx + largest)
                            continue;
                        if(othere->sprite->y + othere->colrad < e->sprite->y + *dy - largest
                            || othere->sprite->y - othere->colrad > e->sprite->y + *dy + largest)
                            continue;

                        em_do_square_circle_collision(e, othere, dx, dy, 1);
                    }
                    else if(ISCIRCULAR(e) && !ISCIRCULAR(othere))//FIX: skip collision with holder / held items
                    {
                        
                        //skip check if it will obviously not collide
                        int largest = (othere->width > othere->height) ? othere->width / 2 : othere->height / 2;
                        if(e->sprite->x + e->colrad < othere->sprite->x + *dx - largest
                            || e->sprite->x - e->colrad > othere->sprite->x + *dx + largest)
                            continue;
                        if(e->sprite->y + e->colrad < othere->sprite->y + *dy - largest
                            || e->sprite->y - e->colrad > othere->sprite->y + *dy + largest)
                            continue;
                        
                        if(em_do_square_circle_collision(othere, e, dx, dy, 0) && !e->hand && ISITEM(othere) && !othere->holder)
                        {
                            e->hand = othere;
                            othere->holder = e;
                        }
                    }
                    else if(!ISCIRCULAR(e) && !ISCIRCULAR(othere))//TODO: square-square collision
                    {
                        
                    }
                }
            }
        }
    }

    if(!collide)
    {
        e->sprite->x += *dx;
        e->sprite->y += *dy;
        if(!ISITEM(e) && e->hand)
        {   
            e->hand->rotx = e->sprite->x;
            e->hand->roty = e->sprite->y;
            e->hand->sprite->x += *dx;
            e->hand->sprite->y += *dy;
        }
        
        else if(ISITEM(e))
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

int em_do_square_circle_collision(struct entity *square, struct entity *circle, float *dx, float *dy, int squaremove)//SAT check https://www.sevenson.com.au/programming/sat/
{
    float corners[4][2];
    float squarex, squarey, circlex, circley, newrot;//square is usually a rectangle, oops
    if(squaremove)
    {
        squarex = square->sprite->x + *dx;
        squarey = square->sprite->y + *dy;
    }
    else
    {
        squarex = square->sprite->x;
        squarey = square->sprite->y;
    }
    if(ISITEM(square))
    {
        if(squaremove)
            newrot = square->sprite->rot + square->sprite->rotoffset + square->angvel;
        else
            newrot = square->sprite->rot + square->sprite->rotoffset;

        float st = math_sin(newrot);
        float ct = math_cos(newrot);//compiler didn't do this automatically... why? more variables?

        squarex += square->offsetx * ct - square->offsety * st;
        squarey += square->offsetx * st + square->offsety * ct;

        //topleft
        corners[0][X] = squarex - square->width * ct / 2 - square->height * st / 2;//OPT: this will be recomputed every time, probably bad, compiler can do a lot of optimization here though
        corners[0][Y] = squarey - square->width * st / 2 + square->height * ct / 2;//in clockwise order
        //topright
        corners[1][X] = squarex + square->width * ct / 2 - square->height * st / 2;
        corners[1][Y] = squarey + square->width * st / 2 + square->height * ct / 2;
        //bottomright
        corners[2][X] = squarex + square->width * ct / 2 + square->height * st / 2;
        corners[2][Y] = squarey + square->width * st / 2 - square->height * ct / 2;
        //bottomleft
        corners[3][X] = squarex - square->width * ct / 2 + square->height * st / 2;
        corners[3][Y] = squarey - square->width * st / 2 - square->height * ct / 2;
        //printf("width %hhd height %hhd offsetx %hhd offsety %hhd\n", square->width, square->height, square->offsetx, square->offsety);

        //printf("%.2f %.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n", math_cos(newrot), math_sin(newrot), newrot, squarex, squarey, corners[0][X], corners[0][Y], corners[1][X], corners[1][Y], corners[2][X], corners[2][Y], corners[3][X], corners[3][Y]);
    }
    else
    {
        squarex += square->offsetx;
        squarey += square->offsety;

        corners[0][X] = squarex - square->width / 2;//OPT: this will be recomputed every time, probably bad, compiler can do a lot of optimization here though
        corners[0][Y] = squarey + square->height / 2;//in clockwise order
        corners[1][X] = squarex + square->width / 2;
        corners[1][Y] = squarey + square->height / 2;
        corners[2][X] = squarex + square->width / 2;
        corners[2][Y] = squarey - square->height / 2;
        corners[3][X] = squarex - square->width / 2;
        corners[3][Y] = squarey - square->height / 2;
        //printf("%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n", corners[0][X], corners[0][Y], corners[1][X], corners[1][Y], corners[2][X], corners[2][Y], corners[3][X], corners[3][Y]);
    }
    if(!squaremove)
    {
        circlex = circle->sprite->x + *dx;
        circley = circle->sprite->y + *dy;
    }
    else
    {
        circlex = circle->sprite->x;
        circley = circle->sprite->y;
    }

    
    float mindist = 1000000;
    int i, closest = 0;
    for(i = 0; i < 4; i++)//OPT: could unroll here
    {
        float dist = math_get_distance(corners[i][X] - circlex, corners[i][Y] - circley);
        if(dist < circle->colrad)
        {
            em_do_collide(square, circle, circle->colrad - dist);
            //printf("did fast collide\n");
            return 1;
        }
        
        if(dist < mindist)
        {
            mindist = dist;
            closest = i;
        }
    }
    
    float axis[2];
    axis[X] = corners[closest][X] - circlex;
    axis[Y] = corners[closest][Y] - circley;
    float dist = math_get_distance(axis[X], axis[Y]);//should never return 0
    axis[X] /= dist;
    axis[Y] /= dist;

    float min = 0, max = 0;

    for(i = 0; i < 4; i++)//OPT: should unroll
    {
        float proj = corners[i][X] * axis[X] + corners[i][Y] * axis[Y];
        //printf("proj %.2f\n", proj);
        if(i == 0)
        {
            min = proj;
            max = proj;
        }
        else if(proj > max)
            max = proj;
        else if(proj < min)
            min = proj;
    }

    float offset = (circlex - squarex) * axis[X] + (circley - squarey) * axis[Y];
    float circcenter = circlex * axis[X] + circley * axis[Y];
    float rmin_cmax = min - offset - (circcenter + circle->colrad);
    float rmax_cmin = circcenter - circle->colrad - (max - offset);
    if(rmin_cmax > 0 || rmax_cmin > 0)
    {
        //printf("no collide slow\n%.2f %.2f %.2f %.2f %.2f %.2f\n", circcenter - circle->colrad, circcenter + circle->colrad, min + offset, max + offset, min + offset - (circcenter + circle->colrad), circcenter - circle->colrad - (max + offset));
        return 1;
    }

    /*printf("did slow collide 1\n");
    printf("%.2f %.2f %.2f %.2f\n", circcenter - circle->colrad, circcenter + circle->colrad, min + offset, max + offset);*/
    /*if(circle->hand && components[square->id] & ISITEM && square->holder && circle->hand == square)
    {
        printf("did slow collide 1\n");
        printf("cmin %.2f, cmax %.2f, rmin %.2f, rmax %.2f\n", circcenter - circle->colrad, circcenter + circle->colrad, min + offset, max + offset);
        printf("%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n%.2f %.2f\n", circlex, circley, corners[0][X], corners[0][Y], corners[1][X], corners[1][Y], corners[2][X], corners[2][Y], corners[3][X], corners[3][Y], axis[X], axis[Y]);
    }*/
    em_do_collide(square, circle, rmin_cmax > rmax_cmin ? rmax_cmin : rmin_cmax);

    return 0;
}

//enum EM_COMPONENTS {ISITEM = 1, HOLDABLE = 2, CANHOLD = 4, CIRCULARHITBOX = 8};

void em_do_collide(struct entity *one, struct entity *two, float dist)
{
    if(!ISITEM(one) && !ISITEM(two))
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
    else if(ISITEM(one) && !ISITEM(two))
    {
        if(one->holder && one->holder->actions)//FIX: generalize damage for speed, accel, or maybe not idk
        {
            float r = math_get_distance(one->holdx, one->holdy) + one->holder->colrad;
            float speedx = one->holder->speedx + one->speedx + one->angvel * math_sin(one->sprite->rot + one->angvel) * r;
            float speedy = one->holder->speedy + one->speedy + one->angvel * math_cos(one->sprite->rot + one->angvel) * r;
            
            two->speedx -= speedx * one->weight * knockbackconstant / (one->weight + two->weight);
            two->speedy -= speedy * one->weight * knockbackconstant / (one->weight + two->weight);
        }
    }
    else if(!ISITEM(one) && ISITEM(two))
    {
        if(!one->hand && !two->holder)
        {
            one->hand = two;
            two->holder = one;
        }
    }
    else if(ISITEM(one) && ISITEM(two));
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
                        printf("destroying %d\n", e->id);
                        destroy[e->id](e);
                    }
    }

    s_free(create, NULL);
    s_free(behaviour, NULL);
    s_free(destroy, NULL);
    s_free(components, NULL);
    s_free(actiontable, NULL);
}