#include <allegro5/allegro.h>
#include <stdio.h>
#include "sprites.h"
#include "map.h"
#include "mapmanager.h"
#include "entitymanager.h"
#include "mapgenerator.h"
#include "tilelist.h"
#include "emath.h"
#include "graph.h"
#include "pqueue.h"
#include "dictionary.h"
#include "list.h"

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
    int enemies;
};

enum DIR {UP, DOWN, LEFT, RIGHT};
enum EXITENUM {TO, FROM};

void mg_destroy_rooms(struct room *rooms, int norooms);
int mg_collides(struct room *rooms, int i, struct room *room);
int mg_put_room_on_map(struct map *map, struct room *rooms);
int mg_connect_room_exits(struct map *map, struct room *rooms, struct graph *graph);
int mg_connect_rooms(struct map *map, struct graph *graph);
struct tile *mg_update_tile(struct map *map, float x, float y, struct tile *tile);
void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile);
void mg_create_simple_dungeon(struct map *map, int norooms);
void mg_create_classic_dungeon(struct map *map, int maxrooms);
struct coord *a_star(struct map *map, struct coord *start, struct coord *end, int *no, unsigned char typemask);
void delaunay_triangulation(struct graph *graph);
int inside_circumcircle(struct room *one, struct room *two, struct room *three, struct room *four);
int room_comp(struct room *one, struct room *two);
int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter);
struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor);
int mg_room_center_x(struct room *room);
int mg_room_center_y(struct room *room);

struct map *mg_create_map(int w, int h)
{
    struct map *out = map_create(5, 16, w, h);

    mg_create_classic_dungeon(out, 50);
    
    return out;
}

void mg_create_classic_dungeon(struct map *map, int maxrooms)
{
    math_seed(1643581891);

    int norooms = 1 + math_get_random(maxrooms);

    struct room *rooms = s_malloc(norooms * sizeof(struct room), "mg_create_classic_dungeon");
    memset(rooms, 0, norooms * sizeof(struct room));
    
    rooms[0].w = 7 + math_get_random(13);
    rooms[0].h = 7 + math_get_random(13);
    rooms[0].x = -3;
    rooms[0].y = 3;
    int leftx = -map->width * map->chunksize / 2, topy = map->height * map->chunksize / 2;
    int rightx = -leftx - 20, bottomy = -topy + 20;
    int maxradius = rightx < bottomy ? rightx - 10 : bottomy - 10;

    int i, tries = 0;
    int angle, distance;
    for(i = 1; i < norooms; i++)
    {
        if(tries == 10)
            break;
        angle = math_get_random(360);
        distance = 10 + math_get_random(maxradius);
        rooms[i].x = math_floor(distance * math_cos_d(angle));
        rooms[i].y = math_ceil(distance * math_sin_d(angle));
        rooms[i].w = 7 + math_get_random(13);
        rooms[i].h = 7 + math_get_random(13);
        rooms[i].enemies = 3 + math_get_random(5);

        if(mg_collides(rooms, i - 1, &rooms[i]))
        {
            //puts("here");
            i--;
            tries++;
            continue;
        }
        if(!math_in_range(leftx, rooms[i].x, rightx) || !math_in_range(bottomy, rooms[i].y, topy))
        {
            i--;
            tries++;
            continue;
        }

        tries = 0;
    }

    struct graph *graph = graph_create(0);
    math_mergesort(rooms, i, room_comp, sizeof(struct room));
    int j;
    for(j = 0; j < i; j++)
    {
        graph_add_vertex(graph, &rooms[j], NULL);
        mg_put_room_on_map(map, &rooms[j]);
    }

    delaunay_triangulation(graph);
    //puts(graph_is_connected(graph) ? "connected" : "unconnected");

    /*struct edge *e = graph->edges + 52;
    struct vertex *v = graph_get_vertex(graph, e->from), *n = graph_get_vertex(graph, e->to);
    printf("base edge %d %d, %d %d\n", mg_room_center_x(v->p), mg_room_center_y(v->p), mg_room_center_x(n->p), mg_room_center_y(n->p));*/
    
    mg_connect_rooms(map, graph);

    mg_destroy_rooms(rooms, norooms);
    graph_destroy(graph);
}

int room_comp(struct room *one, struct room *two)
{
    if(mg_room_center_x(one) == mg_room_center_x(two))
        return mg_room_center_y(one) - mg_room_center_y(two);
    return mg_room_center_x(one) - mg_room_center_x(two);
}

static void delaunay_triangulation_f(struct vertex *vertices, int size, struct graph *graph)
{
    if(size == 2)
    {
        graph_add_edge_v(graph, &vertices[0], &vertices[1], 0);
        return;
    }

    if(size == 3)
    {
        graph_add_edge_v(graph, &vertices[0], &vertices[1], 0);
        graph_add_edge_v(graph, &vertices[1], &vertices[2], 0);
        graph_add_edge_v(graph, &vertices[2], &vertices[0], 0);
        return;
    }

    int newsize = size / 2;
    int isodd = size & 1;

    delaunay_triangulation_f(vertices, newsize, graph);
    delaunay_triangulation_f(&vertices[newsize], isodd ? newsize + 1 : newsize, graph);

    //get base edge

    //puts("getting base edge");
    int i;
    struct vertex *lowl = NULL, *lowr = NULL;
    struct edge *inter = NULL;
    do
    {
        if(lowl && lowr)
        {
            if(graph_get_vertex(graph, inter->to) < &vertices[newsize] && graph_get_vertex(graph, inter->from) < &vertices[newsize])
                lowl = NULL;
            else
                lowr = NULL;
        }

        if(!lowl)
            for(i = 0; i < newsize; i++)
                if(!vertices[i].mark && (!lowl || ((struct room *)lowl->p)->y >= ((struct room *)vertices[i].p)->y))
                    lowl = &vertices[i];

        if(!lowr)
            for(i = newsize; i < size; i++)
                if(!vertices[i].mark && (!lowr || ((struct room *)lowr->p)->y > ((struct room *)vertices[i].p)->y))
                    lowr = &vertices[i];


        //((struct room *)lowl->p)->y > ((struct room *)vertices[i].p)->y)
        //((struct room *)lowr->p)->y > ((struct room *)vertices[i].p)->y)
        //printf("marking vertices %p %p\n", lowl, lowr);
        //printf("vertices y %d %d\n", ((struct room *)lowl->p)->y, ((struct room *)lowr->p)->y);
        /*printf("index of lowl %ld lowr %ld\n", lowl - graph->vertices, lowr - graph->vertices);
        printf("coords of lowl %d %d lowr %d %d\n", mg_room_center_x(lowl->p), mg_room_center_y(lowl->p), mg_room_center_x(lowr->p), mg_room_center_y(lowr->p));*/
        lowl->mark = 1;
        lowr->mark = 1;
    } while(intersect(graph, lowl, lowr, &inter));

    graph_add_edge_v(graph, lowl, lowr, 0);
    
    /*struct edge *prevedge = graph->edges + (graph->noedges - 1);
    printf("base edge %d %d %d\n", prevedge->from, prevedge->to, graph->noedges - 1);
    prevedge = graph->edges + (graph->noedges - 2);
    printf("base edge %d %d %d\n", prevedge->from, prevedge->to, graph->noedges - 2);
    printf("base edge %d %d, %d %d\n", mg_room_center_x(lowl->p), mg_room_center_y(lowl->p), mg_room_center_x(lowr->p), mg_room_center_y(lowr->p));*/
    for(i = 0; i < size; i++)
        vertices[i].mark = 0;

    //get candidates, loop

    
    struct vertex *rcandidate = get_candidate(graph, lowr, lowl), *lcandidate = get_candidate(graph, lowl, lowr);
    struct vertex *rbase = lowr, *lbase = lowl;
    while(rcandidate || lcandidate)
    {
        //printf("merging size: %d graph size: %d %p %p %p %p\n", size, graph->novertices, lcandidate, rcandidate, lbase, rbase);
        if(!rcandidate)
        {
            graph_add_edge_v(graph, lcandidate, rbase, 0);
            lbase = lcandidate;
        }
        else if(!lcandidate)
        {
            graph_add_edge_v(graph, rcandidate, lbase, 0);
            rbase = rcandidate;
        }
        else
        {
            if(inside_circumcircle(lcandidate->p, lbase->p, rbase->p, rcandidate->p))
            {
                graph_add_edge_v(graph, lbase, rcandidate, 0);
                rbase = rcandidate;
            }
            else
            {
                graph_add_edge_v(graph, rbase, lcandidate, 0);
                lbase = lcandidate;
            }
        }

        //printf("before get candidate %p %p\n", rbase, lbase);
        rcandidate = get_candidate(graph, rbase, lbase);
        lcandidate = get_candidate(graph, lbase, rbase);
        //printf("after get candidate %p %p \n", rcandidate, lcandidate);
    }
    //both candidates null
}

struct vertex *get_candidate(struct graph *graph, struct vertex *target, struct vertex *neighbor)
{
    struct vertex *out = NULL, *temp = NULL;
    int i, ccw, outedge;
    ccw = mg_room_center_x(target->p) < mg_room_center_x(neighbor->p);
    for(i = 0; i < target->noedges; i++)
    {
        temp = graph_get_next_vertex(graph, target, i);
        if(!temp)
            continue;
        if(temp == neighbor)
            continue;
        if(mg_room_center_y(temp->p) <= mg_room_center_y(target->p))//doesn't necessarily remove 180 degree angles / may remove valid candidates
            continue;
        if(!out)
        {
            out = temp;
            outedge = i;
            continue;
        }
        if(ccw)
        {   
            if(inside_circumcircle(out->p, target->p, neighbor->p, temp->p))//inside_circumcircle(out->p, target->p, neighbor->p, temp->p)
            {
                graph_remove_edge(graph, graph_get_edge(graph, target, outedge));
                out = temp;
                outedge = i;
            }
        }
        else
        {
            if(inside_circumcircle(neighbor->p, target->p, out->p, temp->p))//inside_circumcircle(neighbor->p, target->p, out->p, temp->p)
            {
                graph_remove_edge(graph, graph_get_edge(graph, target, outedge));
                out = temp;
                outedge = i;
            }
        }
    }
    return out;
}

int intersect(struct graph *graph, struct vertex *one, struct vertex *two, struct edge **inter)
{
    struct room *rone = one->p, *rtwo = two->p, *pone = NULL, *ptwo = NULL;
    struct vertex *vpone = NULL, *vptwo = NULL;
    *inter = NULL;
    int i;
    float m1 = (float)(mg_room_center_y(rone) - mg_room_center_y(rtwo)) / (float)(mg_room_center_x(rone) - mg_room_center_x(rtwo));
    float b1 = (float)mg_room_center_y(rone) - (m1 * mg_room_center_x(rone));
    for(i = 0; i < graph->noedges; i += 2)
    {
        vpone = graph_get_vertex(graph, graph->edges[i].to);
        vptwo = graph_get_vertex(graph, graph->edges[i].from);
        if(vpone && vptwo /*&& one != vpone && one != vptwo && two != vpone && two != vptwo*/)
        {
            pone = vpone->p;
            ptwo = vptwo->p;

            float m2 = (float)(mg_room_center_y(pone) - mg_room_center_y(ptwo)) / (float)(mg_room_center_x(pone) - mg_room_center_x(ptwo));
            if(m1 == m2)
                continue;
            float b2 = (float)mg_room_center_y(pone) - (m2 * mg_room_center_x(pone));
            float x = (b2 - b1) / (m1 - m2);
            if((math_in_range(mg_room_center_x(rone) + 1, x, mg_room_center_x(rtwo) - 1) || math_in_range(mg_room_center_x(rtwo) + 1, x, mg_room_center_x(rone) - 1)) && (math_in_range(mg_room_center_x(pone) + 1, x, mg_room_center_x(ptwo) - 1) || math_in_range(mg_room_center_x(ptwo) + 1, x, mg_room_center_x(pone) - 1)))
            {
                *inter = &graph->edges[i];
                return 1;
            }
        }
    }
    return 0;
}
//(math_in_range(mg_room_center_x(rone), x, mg_room_center_x(rtwo)) || math_in_range(mg_room_center_x(rtwo), x, mg_room_center_x(rone))) && (math_in_range(mg_room_center_x(pone), x, mg_room_center_x(ptwo)) || math_in_range(mg_room_center_x(ptwo), x, mg_room_center_x(pone)))
//(math_in_range(mg_room_center_x(rone) + 1, x, mg_room_center_x(rtwo) - 1) || math_in_range(mg_room_center_x(rtwo) + 1, x, mg_room_center_x(rone) - 1)) && (math_in_range(mg_room_center_x(pone) + 1, x, mg_room_center_x(ptwo) - 1) || math_in_range(mg_room_center_x(ptwo) + 1, x, mg_room_center_x(pone) - 1))
void delaunay_triangulation(struct graph *graph)
{
    if(graph->novertices > 1)
        delaunay_triangulation_f(graph->vertices, graph->novertices, graph);
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

    int i, tries = 0;
    struct coord to, from;
    for(i = 1; i < norooms; i++)
    {
        if(tries == 6)
        {
            tries = 0;
            i--;
        }
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
            tries++;
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
    mg_connect_room_exits(map, rooms, graph);

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

    for(r = 0; r < room->enemies; r++)
    {
        int x = (room->x + 1 + math_get_random(room->w - 3)) * 16;
        int y = (room->y - 1 - math_get_random(room->h - 3)) * 16;
        //printf("adding enemy %d %d %d %d %d %d %.2f %.2f\n", room->x, room->w, room->y, room->h, x, y, (float)x, (float)y);
        map_add_entity_to_chunk(map, em_create_enemy(x, y));
    }

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

    *oldtile = *tile;

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

int mg_connect_room_exits(struct map *map, struct room *rooms, struct graph *graph)
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
            struct coord *path = NULL;
            path = a_star(map, &one->exit[FROM], &two->exit[TO], &count, 0);

            if(path)
            {
                for(count--; count >= 0; count--)
                    mg_update_tile(map, path[count].x, path[count].y, floor);

                s_free(path, NULL);
            }
        }
    }

    return 1;
}

inline int mg_room_center_x(struct room *room)
{
    return room->x + room->w / 2;
}

inline int mg_room_center_y(struct room *room)
{
    return room->y - room->h / 2;
}

int mg_connect_rooms(struct map *map, struct graph *graph)
{
    int i, count, right = 0, up = 0;
    struct edge *e = NULL;
    struct vertex *v = NULL, *n = NULL;
    struct room *here = NULL, *there = NULL;
    struct coord to, from;
    struct tile *floor = mg_get_tile(FLOOR), *temp = NULL;
    for(i = 0; i < graph->noedges; i += 2)
    {
        e = &graph->edges[i];
        
        v = graph_get_vertex(graph, e->from);
        n = graph_get_vertex(graph, e->to);
        if(v && n)
        {
            here = v->p;
            there = n->p;

            from.x = mg_room_center_x(here);
            from.y = mg_room_center_y(here);
            to.x = mg_room_center_x(there);
            to.y = mg_room_center_y(there);
            //mg_update_tile(map, from.x, from.y, mg_get_tile(ERROR));
            //mg_update_tile(map, to.x, to.y, mg_get_tile(ERROR));

            struct coord *path = a_star(map, &from, &to, &count, 0);

            if(path)
            {
                for(count -= 2; count >= 0; count--)
                {
                    mg_update_tile(map, path[count].x, path[count].y, floor);
                    if(path[count].x - path[count + 1].x)
                    {
                        temp = mg_get_tile_from_coordinate(map, path[count].x, path[count].y + 1);
                        if(temp->tilemap_z != 3)
                            *temp = *mg_get_tile(TOPWALL);
                        temp = mg_get_tile_from_coordinate(map, path[count].x, path[count].y - 1);
                        if(temp->tilemap_z != 3)
                            *temp = *mg_get_tile(BOTTOMWALL);

                        if(up)
                        {
                            temp = mg_get_tile_from_coordinate(map, path[count + 1].x, path[count + 1].y - 1);
                            if(temp->tilemap_z != 3)
                                *temp = *mg_get_tile(BOTTOMWALL);
                            temp = mg_get_tile_from_coordinate(map, path[count + 1].x, path[count + 1].y + 1);
                            if(temp->tilemap_z != 3)
                                *temp = *mg_get_tile(TOPWALL);
                        }
                        right = 1;
                        up = 0;
                    }
                    else
                    {
                        temp = mg_get_tile_from_coordinate(map, path[count].x + 1, path[count].y);
                        if(temp->tilemap_z != 3)
                            *temp = *mg_get_tile(RIGHTWALL);
                        temp = mg_get_tile_from_coordinate(map, path[count].x - 1, path[count].y);
                        if(temp->tilemap_z != 3)
                            *temp = *mg_get_tile(LEFTWALL);

                        if(right)
                        {
                            temp = mg_get_tile_from_coordinate(map, path[count + 1].x - 1, path[count + 1].y);
                            if(temp->tilemap_z != 3)
                                *temp = *mg_get_tile(LEFTWALL);
                            temp = mg_get_tile_from_coordinate(map, path[count + 1].x + 1, path[count + 1].y);
                            if(temp->tilemap_z != 3)
                                *temp = *mg_get_tile(RIGHTWALL);
                        }
                        right = 0;
                        up = 1;
                    }
                }

                s_free(path, NULL);
            }
        }
    }
    graph_unmark(graph);
    return 1;
}

int compare_coords(struct coord *one, struct coord *two)
{
    if(one->x == two->x)
        return one->y - two->y;
    return one->x - two->x;
}

float euclid_dist(int x1, int y1, int x2, int y2)
{
    int xdiff = x1 - x2;
    int ydiff = y1 - y2;
    return math_sqrt(xdiff * xdiff + ydiff * ydiff);
}

float grid_heur(int x1, int y1, int x2, int y2)
{
    return math_abs(x1 - x2) + math_abs(y1 - y2);
}

struct coord *a_star(struct map *map, struct coord *start, struct coord *end, int *no, unsigned char typemask)
{
    struct tile *starttile = mg_get_tile_from_coordinate(map, start->x, start->y);
    struct tile *endtile = mg_get_tile_from_coordinate(map, end->x, end->y);
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
    pq_insert(frontier, 0, start);

    dict_add_entry(camefrom, start, start);
    dist = s_malloc(sizeof(float), NULL);
    *dist = 0;
    dict_add_entry(costsofar, start, dist);
    struct coord next;
    struct coord *out = NULL;

    while(frontier->noitems)
    {
        struct coord *cur = pq_pop(frontier);
        struct tile *currenttile = mg_get_tile_from_coordinate(map, cur->x, cur->y);
        if(!currenttile)
            continue;
        if(currenttile->type & typemask)
            continue;
        if(currenttile == endtile)
        {
            out = s_realloc(out, ++(*no) * sizeof(struct coord), NULL);
            out[0] = *end;
            struct coord *c = dict_get_entry(camefrom, end);
            while(c != start)
            {
                out = s_realloc(out, ++(*no) * sizeof(struct coord), NULL);
                out[*no - 1] = *c;
                c = dict_get_entry(camefrom, c);
            }

            out = s_realloc(out, ++(*no) * sizeof(struct coord), NULL);
            out[*no - 1] = *start;
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
                csf = s_malloc(sizeof(float), NULL);
                *csf = newcost;
                c = s_malloc(sizeof(struct coord), NULL);
                
                *c = next;
                dict_add_entry(costsofar, c, csf);
                dict_add_entry(camefrom, c, cur);
                
                pq_insert(frontier, newcost + grid_heur(c->x, c->y, end->x, end->y), c);
            }
            else if(curcost && newcost < *curcost)
            {
                *curcost = newcost;
                dict_add_entry(camefrom, &next, cur);

                pq_insert(frontier, newcost + grid_heur(c->x, c->y, end->x, end->y), c);
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

int inside_circumcircle(struct room *one, struct room *two, struct room *three, struct room *four)//one two and three must be in ccw order
{
    int x2 = mg_room_center_x(four);
    x2 *= x2;
    int y2 = mg_room_center_y(four);
    y2 *= y2;
    int a = mg_room_center_x(one) - mg_room_center_x(four);
    int b = mg_room_center_y(one) - mg_room_center_y(four);
    int c = (mg_room_center_x(one) * mg_room_center_x(one) - x2) + (mg_room_center_y(one) * mg_room_center_y(one) - y2);
    int d = mg_room_center_x(two) - mg_room_center_x(four);
    int e = mg_room_center_y(two) - mg_room_center_y(four);
    int f = (mg_room_center_x(two) * mg_room_center_x(two) - x2) + (mg_room_center_y(two) * mg_room_center_y(two) - y2);
    int g = mg_room_center_x(three) - mg_room_center_x(four);
    int h = mg_room_center_y(three) - mg_room_center_y(four);
    int i = (mg_room_center_x(three) * mg_room_center_x(three) - x2) + (mg_room_center_y(three) * mg_room_center_y(three) - y2);
    int orientation = a * e * i + b * f * g + c * d * h - c * e * g - f * h * a - i * b * d;
    return orientation > 0;
}