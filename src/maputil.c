#include <stdio.h>
#include "sprites.h"
#include "map.h"
#include "mapgenerator.h"
#include "pqueue.h"
#include "dictionary.h"
#include "emath.h"

enum DIR {UP, DOWN, LEFT, RIGHT};

static struct map *curmap = NULL;

float mu_grid_heur(struct map *map, struct coord *cur, struct coord *end)
{
    return math_abs(cur->x - end->x) + math_abs(cur->y - end->y);
}

static int compare_coords(struct coord *one, struct coord *two)
{
    if(one->x == two->x)
        return one->y - two->y;
    return one->x - two->x;
}

//change this to use grid coordinates
struct pixcoord *mu_a_star(struct map *map, struct pixcoord *start, struct pixcoord *end, int *no, unsigned char typemask, 
	float (*cost)(struct map *, struct coord *, struct coord *, void *), void *data)
{
	curmap = map;
    struct tile *starttile = map_get_tile_from_coordinate(map, start->x, start->y);
    struct tile *endtile = map_get_tile_from_coordinate(map, end->x, end->y);
    if(!starttile || !endtile)
        return NULL;
    if(starttile->type & typemask || endtile->type & typemask)
        return NULL;

    struct pq *frontier = pq_create(10);
    struct dict *camefrom = dict_create(compare_coords);
    struct dict *costsofar = dict_create(compare_coords);
    float *dist = NULL;
    int i;
    *no = 0;
    struct coord *gridend = s_malloc(sizeof(struct coord), NULL);
    gridend->x = end->x / 16;
    gridend->y = end->y / 16;

    struct coord *gridstart = s_malloc(sizeof(struct coord), NULL);
    gridstart->x = start->x / 16;
    gridstart->y = start->y / 16;
    pq_insert(frontier, 0, gridstart);

    dict_add_entry(camefrom, gridstart, gridstart);
    dist = s_malloc(sizeof(float), NULL);
    *dist = 0;
    dict_add_entry(costsofar, gridstart, dist);
    struct coord next;
    struct pixcoord *out = NULL;

    while(frontier->noitems)
    {
        struct coord *cur = pq_pop(frontier);
        struct tile *currenttile = map_get_tile_from_coordinate(map, cur->x * (float)map->tilesize, cur->y * (float)map->tilesize);
        if(!currenttile)
            continue;
        if(currenttile->type & typemask)
            continue;
        if(currenttile == endtile)
        {
            out = s_realloc(out, ++(*no) * sizeof(struct pixcoord), NULL);
            out[0].x = end->x;
            out[0].y = end->y;
            struct coord *c = dict_get_entry(camefrom, cur);
            while(c != gridstart)
            {
                out = s_realloc(out, ++(*no) * sizeof(struct pixcoord), NULL);
                out[*no - 1].x = c->x * (float)map->tilesize;
                out[*no - 1].y = c->y * (float)map->tilesize;
                c = dict_get_entry(camefrom, c);
            }

            out = s_realloc(out, ++(*no) * sizeof(struct pixcoord), NULL);
            out[*no - 1].x = start->x;
            out[*no - 1].y = start->y;
            break;
        }

        for(i = 0; i < 4; i++)
        {   
            switch(i)
            {
                case UP:
                    next.x = cur->x;
                    next.y = cur->y + 1;
                    break;

                case DOWN:
                    next.x = cur->x;
                    next.y = cur->y - 1;
                    break;

                case LEFT:
                    next.x = cur->x - 1;
                    next.y = cur->y;
                    break;

                case RIGHT:
                    next.x = cur->x + 1;
                    next.y = cur->y;
                    break;
            }

            float *csf = dict_get_entry(costsofar, cur);//csf -> cost so far
            float newcost = *csf + 1;
            struct coord *c = dict_get_entry(camefrom, &next);
            float *curcost = dict_get_entry(costsofar, &next);
            if(!c)
            {
                csf = s_malloc(sizeof(float), NULL);
                *csf = newcost;
                c = s_malloc(sizeof(struct coord), NULL);
                *c = next;
                
                dict_add_entry(costsofar, c, csf);
                dict_add_entry(camefrom, c, cur);
                
                pq_insert(frontier, newcost + cost(map, c, gridend, data), c);
            }
            else if(curcost && newcost < *curcost)
            {
                *curcost = newcost;
                dict_add_entry(camefrom, &next, cur);

                pq_insert(frontier, newcost + cost(map, c, gridend, data), c);
            }
        }
    }

    for(i = 0; i < costsofar->size; i++)
    {
        s_free(costsofar->keys[i], NULL);
        s_free(costsofar->p[i], NULL);
    }
    dict_destroy(camefrom);
    dict_destroy(costsofar);
    pq_destroy(frontier);
    return out;
}
