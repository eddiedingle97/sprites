#include <allegro5/allegro.h>
#include <stdio.h>
#include "sprites.h"
#include "map.h"
#include "mapmanager.h"
#include "mapgenerator.h"
#include "tilelist.h"
#include "emath.h"
#include "graph.h"
#include "pqueue.h"
#include "dictionary.h"

struct coord
{
    int x;
    int y;
};

struct room
{
    int w;
    int h;
    int x;
    int y;
    struct coord *exit;
    int noexits;
};

enum DIR {UP, DOWN, LEFT, RIGHT};
enum EXITENUM {TO, FROM};

void mg_destroy_rooms(struct room *rooms, int norooms);
int mg_collides(struct room *rooms, int i, struct room *room);
int mg_put_room_on_map(struct map *map, struct room *rooms);
int mg_connect_rooms(struct map *map, struct room *rooms, struct graph *graph);
struct tile *mg_update_tile(struct map *map, float x, float y, struct tile *tile);
void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile);
void mg_create_simple_dungeon(struct map *map, int norooms);
struct tile **a_star(struct map *map, struct coord *start, struct coord *end, int *no);

struct map *mg_create_map(int w, int h)
{
    struct map *out = map_create(5, 16, w, h);

    mg_create_simple_dungeon(out, 10);
    
    return out;
}

void mg_create_simple_dungeon(struct map *map, int maxrooms)
{
    math_seed(0);

    int norooms = 1 + math_get_random(maxrooms);

    struct room *rooms = s_malloc(norooms * sizeof(struct room), "mg_create_simple_dungeon");
    memset(rooms, 0, norooms * sizeof(struct room));
    struct graph *graph = graph_create(DIRECTED);

    rooms[0].exit = s_malloc(2 * sizeof(struct coord), "rooms[0].exit: mg_create_simple_dungeon");
    rooms[0].noexits = 2;
    rooms[0].w = 10;
    rooms[0].h = 10;
    rooms[0].x = -5;
    rooms[0].y = 5;
    mg_put_room_on_map(map, rooms);
    graph_add_vertex(graph, rooms, NULL);

    int i;
    struct coord to, from;
    for(i = 1; i < norooms; i++)
    {
        rooms[i].w = 10;
        rooms[i].h = 10;
        rooms[i].exit = NULL; 
        rooms[i].noexits = 2;

        switch(math_get_random(3))
        {
            case 0://next room up
                rooms[i].y = rooms[i - 1].y + rooms[i].h;
                rooms[i].x = rooms[i - 1].x;
                from.y = rooms[i - 1].y;
                from.x = rooms[i - 1].x + rooms[i - 1].w / 2;
                to.y = rooms[i].y - rooms[i].h + 1;
                to.x = rooms[i].x + rooms[i].w / 2;
                break;
            case 1://next room down
                rooms[i].y = rooms[i - 1].y - rooms[i - 1].h;
                rooms[i].x = rooms[i - 1].x;
                from.y = rooms[i - 1].y - rooms[i - 1].h + 1;
                from.x = rooms[i - 1].x + rooms[i - 1].w / 2;
                to.y = rooms[i].y;
                to.x = rooms[i].x + rooms[i].w / 2;
                break;
            case 2://next room right
                rooms[i].x = rooms[i - 1].x + rooms[i - 1].w;
                rooms[i].y = rooms[i - 1].y;
                from.y = rooms[i - 1].y - rooms[i - 1].h / 2;
                from.x = rooms[i - 1].x + rooms[i - 1].w - 1;
                to.y = rooms[i].y - rooms[i].h / 2;
                to.x = rooms[i].x;
                break;
            case 3://next room left
                rooms[i].x = rooms[i - 1].x - rooms[i].w;
                rooms[i].y = rooms[i - 1].y;
                from.y = rooms[i - 1].y - rooms[i - 1].h / 2;
                from.x = rooms[i - 1].x;
                to.y = rooms[i].y - rooms[i].h / 2;
                to.x = rooms[i].x + rooms[i].w - 1;
                break;
        }
        
        if(mg_collides(rooms, i - 1, &rooms[i]))
        {
            i--;
            continue;
        }
        
        rooms[i].exit = s_malloc(2 * sizeof(struct coord), "rooms[i].exit: mg_create_simple_dungeon");
        rooms[i - 1].exit[FROM] = from;
        rooms[i].exit[TO] = to;

        graph_add_vertex(graph, &rooms[i], NULL);
        graph_add_edge(graph, i - 1, i, 1);
    }

    for(i = 0; i < norooms; i++)
        mg_put_room_on_map(map, &rooms[i]);
    mg_connect_rooms(map, rooms, graph);

    mg_destroy_rooms(rooms, norooms);
    graph_destroy(graph);
}

void mg_destroy_rooms(struct room *rooms, int norooms)
{
    int i;
    for(i = 0; i < norooms; i++)
        s_free(rooms[i].exit, NULL);

    s_free(rooms, NULL);
}

int mg_put_room_on_map(struct map *map, struct room *room)
{
    int r, c;
    int z = 3;
    int startx = room->x, starty = room->y;
    struct tile *tile;

    for(c = 1; c < room->w - 1; c++)
    {
        tile = mg_update_tile(map, startx + c, starty, mg_get_tile(TOPWALL));
        tile->tilemap_z = z;
        tile = mg_update_tile(map, startx + c, starty - room->h + 1, mg_get_tile(BOTTOMWALL));
        tile->tilemap_z = z;
    }

    for(r = 1; r < room->h - 1; r++)
    {
        tile = mg_update_tile(map, startx, starty - r, mg_get_tile(LEFTWALL));
        tile->tilemap_z = z;
        tile = mg_update_tile(map, startx + room->w - 1, starty - r, mg_get_tile(RIGHTWALL));
        tile->tilemap_z = z;
    }

    mg_fill_area(map, startx + 1, starty - 1, startx + room->w - 2, starty - room->h + 2, mg_get_tile(FLOOR));

    for(r = 0; r < room->noexits; r++)
        mg_update_tile(map, room->exit[r].x, room->exit[r].y, mg_get_tile(ERROR));

    return 1;
}

int mg_collides(struct room *rooms, int i, struct room *room)
{
    int x1 = room->x, x2 = room->x + room->w - 1, y1 = room->y, y2 = room->y - room->h + 1;
    int collides = 0;
    for(; i >= 0; i--)
    {
        if(rooms[i].x > x2 || rooms[i].x + rooms[i].w - 1 < x1);

        else if(rooms[i].y < y2 || rooms[i].y - rooms[i].h + 1 > y1);

        else
            collides = 1;
    }

    return collides;
}

struct tile *mg_update_tile(struct map *map, float x, float y, struct tile *tile)
{
    if(!tile)
        return NULL;

    struct tile *oldtile = map_get_tile_from_coordinate(map, x * 16, y * 16);
    if(!oldtile)
        return NULL;

    oldtile->tilemap_x = tile->tilemap_x;
    oldtile->tilemap_y = tile->tilemap_y;
    oldtile->tilemap_z = tile->tilemap_z;
    oldtile->solid = tile->solid;
    oldtile->breakable = tile->breakable;
    oldtile->damage = tile->damage;

    return oldtile;
}

struct chunk *mg_get_chunk(struct map *map, float x, float y)
{
    return NULL;
}

struct tile *mg_get_tile_from_coordinate(struct map *map, float x, float y)
{
    return map_get_tile_from_coordinate(map, x * 16, y * 16);
}

int mg_connect_rooms(struct map *map, struct room *rooms, struct graph *graph)
{
    int i, j;
    struct vertex *v, *n;
    struct tile *floor = mg_get_tile(FLOOR);
    for(i = 0; i < graph->novertices; i++)
    {
        v = &graph->vertices[i];
        for(j = 0; j < v->noedges; j++)
        {
            n = graph_get_next_vertex(graph, v, j);
            struct room *one = v->p, *two = n->p;

            int count;
            struct tile **path = a_star(map, &one->exit[FROM], &two->exit[TO], &count);

            if(path)
            {
                for(count--; count >= 0; count--)
                    *path[count] = *floor;

                s_free(path, NULL);
            }
        }
    }

    return 1;
}

int compare_coords(struct coord *one, struct coord *two)
{
    if(one->x == two->x)
        return one->y - two->y;
    return one->x - two->x;
}

struct tile **a_star(struct map *map, struct coord *start, struct coord *end, int *no)
{
    struct tile *starttile = mg_get_tile_from_coordinate(map, start->x, start->y);
    struct tile *endtile = mg_get_tile_from_coordinate(map, end->x, end->y);
    if(!starttile || !endtile)
        return NULL;
    if(starttile->solid || endtile->solid)
        return NULL;

    struct pq *frontier = pq_create(10);
    struct dict *camefrom = dict_create(compare_coords);
    struct dict *costsofar = dict_create(compare_coords);
    float *dist = NULL;
    int i;
    *no = 0;
    pq_insert(frontier, 0, start);

    dict_add_entry(camefrom, start, start);
    dist = s_malloc(sizeof(float), "dist: a_star");
    *dist = 0;
    dict_add_entry(costsofar, start, dist);
    struct coord next;
    struct tile **out = NULL;

    while(frontier->noitems)
    {
        struct coord *cur = pq_pop(frontier);
        struct tile *currenttile = mg_get_tile_from_coordinate(map, cur->x, cur->y);
        if(!currenttile)
            continue;
        if(currenttile->solid)
            continue;
        if(currenttile == endtile)
        {
            out = s_realloc(out, ++(*no) * sizeof(struct tile *), "out: a_star");
            out[0] = mg_get_tile_from_coordinate(map, end->x, end->y);
            struct coord *c = dict_get_entry(camefrom, end);
            while(c != start)
            {
                out = s_realloc(out, ++(*no) * sizeof(struct tile *), "out: a_star");
                out[*no - 1] = mg_get_tile_from_coordinate(map, c->x, c->y);
                c = dict_get_entry(camefrom, c);
            }

            out = s_realloc(out, ++(*no) * sizeof(struct tile *), "out: a_star");
            out[*no - 1] = mg_get_tile_from_coordinate(map, start->x, start->y);
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

            float *csf = dict_get_entry(costsofar, cur);
            
            float newcost = *csf + 1;
            struct coord *c = dict_get_entry(camefrom, &next);
            float *curcost = dict_get_entry(costsofar, &next);
            if(!c)
            {
                csf = s_malloc(sizeof(float), "csf: a_star");
                *csf = newcost;
                c = s_malloc(sizeof(struct coord), "c: a_star");
                
                *c = next;
                dict_add_entry(costsofar, c, csf);
                dict_add_entry(camefrom, c, cur);
                
                pq_insert(frontier, newcost + math_abs(c->x - end->x) + math_abs(c->y - end->y), c);
            }
            else if(curcost && newcost < *curcost)
            {
                *curcost = newcost;
                dict_add_entry(camefrom, &next, cur);

                pq_insert(frontier, newcost + math_abs(c->x - end->x) + math_abs(c->y - end->y), c);
            }
        }
    }

    for(i = 0; i < costsofar->size; i++)
    {
        if(costsofar->keys[i] != start && costsofar->keys[i] != end)
            s_free(costsofar->keys[i], NULL);

        s_free(costsofar->p[i], NULL);
    }

    dict_destroy(camefrom);
    dict_destroy(costsofar);
    pq_destroy(frontier);
    return out;
}

void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile)
{
    int r = y1 - y2, c;

    for(; r >= 0; r--)
    {
        for(c = x2 - x1; c >= 0; c--)
        {
            mg_update_tile(map, x1 + c, y1 - r, tile);
        }
    } 
}