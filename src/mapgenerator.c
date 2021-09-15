#include <allegro5/allegro.h>
#include <stdio.h>
#include "sprites.h"
#include "map.h"
#include "mapmanager.h"
#include "mapgenerator.h"
#include "tilelist.h"
#include "emath.h"
#include "graph.h"

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
    struct coord *exits;
};

int mg_collides(struct room *rooms, int i, int x, int y);
int mg_put_room_on_map(struct map *map, struct room *rooms, int gap);
int mg_connect_rooms(struct map *map, struct room *rooms, struct graph *graph);
struct tile *mg_update_tile(struct map * map, float x, float y, struct tile *tile);
void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile);
void mg_create_simple_dungeon(struct map *map, int norooms);

struct map *mg_create_map(int w, int h)
{
    struct map *out = map_create(5, 16, w, h);

    math_seed(0);

    int norooms = math_get_random(10);

    mg_create_simple_dungeon(out, norooms);
    
    return out;
}

void mg_create_simple_dungeon(struct map *map, int norooms)
{
    struct room *rooms = s_malloc(norooms * sizeof(struct room), "mg_create_map");
    struct graph *graph = graph_create(DIRECTED);

    rooms[0].w = 10;
    rooms[0].h = 10;
    rooms[0].x = 0;
    rooms[0].y = 0;
    mg_put_room_on_map(map, rooms, 0);
    graph_add_vertex(graph, rooms, NULL);

    int i;
    for(i = 1; i < norooms; i++)
    {
        rooms[i].w = 10;
        rooms[i].h = 10;
        int up;
        int right;
        switch(math_get_random(3))
        {
            case 0:
                up = 1;
                right = 0;
                break;
            case 1:
                up = 0;
                right = 1;
                break;
            case 2:
                up = -1;
                right = 0;
                break;
            case 3:
                up = 0;
                right = -1;
                break;
        }

        if(mg_collides(rooms, i - 1, rooms[i - 1].x + right * rooms[i].w, rooms[i - 1].y + up * rooms[i].h))
        {
            i--;
            continue;
        }

        rooms[i].y = rooms[i - 1].y + up * rooms[i].h;
        rooms[i].x = rooms[i - 1].x + right * rooms[i].w;
        //printf("%d %d\n", rooms[i].x, rooms[i].y);
        mg_put_room_on_map(map, &rooms[i], 0);
        graph_add_vertex(graph, &rooms[i], NULL);
        graph_add_edge(graph, i - 1, i, 1);
    }

    mg_connect_rooms(map, rooms, graph);

    s_free(rooms, NULL);
    graph_destroy(graph);
}

int mg_put_room_on_map(struct map *map, struct room *room, int gap)
{
    int r, c;
    int z = 3;
    int startx = room->x + gap - room->w / 2, starty = room->y + gap + room->h / 2;
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

    return 1;
}

int mg_collides(struct room *rooms, int i, int x, int y)
{
    for(; i >= 0; i--)
    {

        if(rooms[i].x == x && rooms[i].y == y)
            return 1;
    }

    return 0;
}

struct tile *mg_update_tile(struct map * map, float x, float y, struct tile *tile)
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

int mg_connect_rooms(struct map *map, struct room *rooms, struct graph *graph)
{
    int i, j;
    struct vertex *v, *n;
    //printf("graph edges %d graph vertices: %d\n", graph->noedges, graph->novertices);
    for(i = 0; i < graph->novertices; i++)
    {
        v = &graph->vertices[i];
        for(j = 0; j < v->noedges; j++)
        {
            n = graph_get_next_vertex(graph, v, j);
            struct room *one = v->p, *two = n->p;
            int up = two->y - one->y;
            int right = two->x - one->x;

            /*printf("one: %d %d, two: %d %d\n", one->x, one->y, two->x, two->y);
            printf("up: %d, right: %d\n", up, right);*/

            if(right)
            {
                if(right > 0)
                {
                    mg_update_tile(map, two->x - two->w / 2, one->y, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x - two->w / 2, one->y + 1, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x - two->w / 2 - 1, one->y, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x - two->w / 2 - 1, one->y + 1, mg_get_tile(FLOOR));
                }
                else
                {
                    mg_update_tile(map, one->x - one->w / 2, two->y, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x - one->w / 2, two->y + 1, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x - one->w / 2 - 1, two->y, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x - one->w / 2 - 1, two->y + 1, mg_get_tile(FLOOR));
                }
            }

            else
            {
                if(up > 0)
                {
                    mg_update_tile(map, one->x, one->y + one->h / 2, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x - 1, one->y + one->h / 2, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x, one->y + one->h / 2 + 1, mg_get_tile(FLOOR));
                    mg_update_tile(map, one->x - 1, one->y + one->h / 2 + 1, mg_get_tile(FLOOR));
                }
                else
                {
                    mg_update_tile(map, two->x, two->y + two->h / 2, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x - 1, two->y + two->h / 2, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x, two->y + two->h / 2 + 1, mg_get_tile(FLOOR));
                    mg_update_tile(map, two->x - 1, two->y + two->h / 2 + 1, mg_get_tile(FLOOR));
                }
            }
        }
    }

    return 1;
}

void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile)
{
    int r = y1 - y2, c;

    for(; r >= 0; r--)
    {
        for(c = x2 - x1; c >=0; c--)
        {
            mg_update_tile(map, x1 + c, y1 - r, tile);
        }
    } 
}