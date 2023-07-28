#include <allegro5/allegro.h>
#include <stdio.h>
#include <math.h>
#include "sprites.h"
#include "map.h"
#include "mapmanager.h"
#include "entitymanager.h"
#include "mapgenerator.h"
#include "tilepalette.h"
#include "emath.h"
#include "graph.h"
#include "pqueue.h"
#include "dictionary.h"
#include "delaunay.h"
#include "list.h"
#include "debug.h"
#include "perlin.h"

enum DIR {UP, DOWN, LEFT, RIGHT};
enum EXITENUM {TO, FROM};

void mg_destroy_rooms(struct room *rooms, int norooms);
int mg_collides(struct room *rooms, int i, struct room *room);
int mg_put_room_on_map(struct map *map, struct room *rooms);
int mg_connect_room_exits(struct map *map, struct room *rooms, struct graph *graph);
int mg_connect_rooms(struct map *map, struct graph *graph, int width);
int mg_connect_coords(struct map *map, struct graph *graph, int width);
void mg_add_path(struct map *map, struct coord *path, int count, int width);
struct tile *mg_update_tile(struct map *map, float x, float y, struct tile *tile);
struct tile *mg_get_tile(struct map *map, float x, float y);
struct tile *mg_get_tile_from_coordinate(struct map *map, float x, float y);
void mg_fill_area(struct map *map, int x1, int y1, int x2, int y2, struct tile *tile);
void mg_create_simple_dungeon(struct map *map, int norooms);
void mg_create_classic_dungeon(struct map *map, int maxrooms);
void mg_create_hallway_dungeon(struct map *map, int maxrooms);
void mg_create_recursive_dungeon(struct map *map, int maxrooms);
void mg_create_island(struct map *map);
int room_comp(struct room *one, struct room *two);

struct map *mg_create_map(int w, int h)
{
    tp_change_palette(s_get_full_path_with_dir("config/tilemaps", "dungeontiles.cfg"), PT_DUNGEONTILES);

    struct map *out = map_create(5, 16, w, h);

    out->palette = tp_get_palette_copy();//s_realloc(out->palettes, ++out->nopalettes * sizeof(struct palette *), "mg_create_map");
    //out->palettes[out->nopalettes - 1] = 

    mg_create_recursive_dungeon(out, 50);
    //mg_create_hallway_dungeon(out, 50);

    tp_destroy();

    return out;
}

struct map *mg_create_island_map(int w, int h)
{
    tp_change_palette(s_get_full_path_with_dir("config/tilemaps", "islandtiles.cfg"), PT_ISLANDTILES);

    struct map *out = map_create(5, 16, w, h);

    out->palette = tp_get_palette_copy();//s_realloc(out->palettes, ++out->nopalettes * sizeof(struct palette *), "mg_create_island_map");
    //out->palettes[out->nopalettes - 1] = 

    mg_create_island(out);

    tp_destroy();

    return out;
}

void mg_per_chunk_perlin(struct map *map)
{
    perlin_init(0, 3);

    int r, c, tr, tc;
    float noise;
    struct chunk *chunk;
    struct tile *tile, *tptile;
    int x, y, count = 0;//to be fed to perlin_noise_sample
    int water, sand, grass, dirt;
    for(r = 0; r < map->height; r++)
    {
        for(c = 0; c < map->width; c++)//FOR EACH CHUNK
        {
            chunk = &map->chunks[r][c];
            x = c * map->chunksize;
            y = r * map->chunksize;
            water = 0;
            sand = 0;
            grass = 0;
            dirt = 0;
            for(tr = 0; tr < map->chunksize; tr++)
            {
                for(tc = 0; tc < map->chunksize; tc++)//FOR EACH TILE IN THE CHUNK
                {
                    tile = &chunk->tiles[tr * map->chunksize + tc];
                    noise = perlin_noise_sample(map->width * map->chunksize, map->height * map->chunksize, x + tc, y + tr, 3, .002/*FIX: adjust this with map size*/, .6);
                    count++;
                    if(noise < .45)
                    {
                        *tile = *tp_get_tile(IT_WATER);
                        water++;
                    }
                    else if(noise < .48)
                    {
                        *tile = *tp_get_tile(IT_SAND);
                        sand++;
                    }
                    else
                    {
                        *tile = *tp_get_tile(IT_GRASS);
                        grass++;
                    }
                }
            }
            //FOR EACH CHUNK
            //enum CHUNKFLAGS {MG_OCEAN = 1, MG_SHORE = 2, MG_MAINLAND = 4, MG_SMALLISLAND = 8, MG_HABITABLE = 16, MG_HASWATER = 32};
            float notiles = map->chunksize * map->chunksize;
            float per = grass / notiles;
            if(per > .6f)
                chunk->flags |= MG_MAINLAND;
            per = water / notiles;
            if(per == 1.0f)
                chunk->flags |= MG_OCEAN;
            if(per > 0.0f)
                chunk->flags |= MG_HASWATER;
            per = sand / notiles;
            if(per > 0.0f)
                chunk->flags |= MG_SHORE;
        }
    }

    perlin_done();
}

int mg_contiguous(struct map *map, struct chunk *one, struct chunk *two)
{
    //check if two chunks are contiguous... more annoying to solve than expected. custom a* needed?
    //FIX: make a* take a function pointer instead of mask for collisions, will help constrain path further
    int xdiff = one->index_x - two->index_x;
    int ydiff = one->index_y - two->index_y;
    
    return 0;
}

int mg_update_areas(struct map *map, struct chunk *chunk, struct chunk **areas, int noareas)
{
    /*int i, j, expand = 0;
    for(i = 0; i < 2 * noareas; i += 2)
    {
        if(chunk->index_x < areas[i]->index_x || chunk->index_x > areas[i + 1]->index_x)
        {
            if(chunk->index_x - areas[i]->index_x == -1)//direct left of area
            {
                //int sidelength = areas[i]->index_y - areas[i + 1]->index_y;
                expand = 1;
                for(j = areas[i]->index_y; j < areas[i + i]->index_y; j++)
                {
                    if(map->chunks[j][areas[i]->index_x - 1].flag & MG_OCEAN)
                    {
                        expand = 0;
                        break;
                    }
                }
                if(expand)
                {

                }
            }
            else if(chunk->index_x - areas[i + 1]->)
        }

        else if(chunk->index_y < areas[i]->index_y || chunk->index_y > areas[i + 1]->index_y);


    }*/
}

int mg_island_chunk_post_check(struct map *map, struct chunk *chunk, int depth)//bread first search essentially
{
    if(chunk->flags & MG_CHECKED)
        return 0;
    long count = 0;
    struct list *queue; 
    if(!(chunk->flags & MG_OCEAN))
    {
        chunk->flags |= MG_CHECKED;
        queue = list_create();
        list_queue(queue, chunk);
    }
    else
    {
        chunk->flags |= MG_CHECKED;
        return 0;
    }
    
    struct chunk *ochunk;
    
    struct chunk **areas = s_malloc(2 * sizeof(struct chunk *), NULL);
    areas[0] = chunk;
    areas[1] = chunk;
    int noareas = 1;

    while(queue->size > 0)
    {
        chunk = list_dequeue(queue);
        
        count++;

        mg_update_areas(map, chunk, areas, noareas);

        ochunk = map_get_chunk_from_index(map, chunk->index_x + 1, chunk->index_y);
        if(ochunk && !(ochunk->flags & MG_OCEAN) && !(ochunk->flags & MG_CHECKED))
        {
            ochunk->flags |= MG_CHECKED;
            list_queue(queue, ochunk);
        }

        ochunk = map_get_chunk_from_index(map, chunk->index_x - 1, chunk->index_y);
        if(ochunk && !(ochunk->flags & MG_OCEAN) && !(ochunk->flags & MG_CHECKED))
        {
            ochunk->flags |= MG_CHECKED;
            list_queue(queue, ochunk);
        }

        ochunk = map_get_chunk_from_index(map, chunk->index_x, chunk->index_y + 1);
        if(ochunk && !(ochunk->flags & MG_OCEAN) && !(ochunk->flags & MG_CHECKED))
        {
            ochunk->flags |= MG_CHECKED;
            list_queue(queue, ochunk);
        }

        ochunk = map_get_chunk_from_index(map, chunk->index_x, chunk->index_y - 1);
        if(ochunk && !(ochunk->flags & MG_OCEAN) && !(ochunk->flags & MG_CHECKED))
        {
            ochunk->flags |= MG_CHECKED;
            list_queue(queue, ochunk);
        }
    }

    list_destroy(queue);
    s_free(areas, NULL);
    
    return count;
}

void mg_create_island(struct map *map)
{
    math_seed(1679340287);//1679171525

    mg_per_chunk_perlin(map);

    //perlin_noise_iter(0, map->width * map->chunksize, map->height * map->chunksize, 1, 3, .002/*FIX: adjust this with map size*/, .6, &iid, mg_island_map_iter);

    int r, c, t, done = 0, count = 0, discrete = 0;
    struct chunk *chunk;
    while(!done)
    {
        for(r = 0; r < map->height; r++)
        {
            for(c = 0; c < map->width; c++)
            {
                chunk = &map->chunks[r][c];
                count = mg_island_chunk_post_check(map, chunk, 0);
                if(count > 0)
                {
                    printf("here %d\n", count);
                    discrete++;
                }
            }
        }
        printf("discrete %d\n", discrete);
        
        done++;
    }

    /*for(r = 0; r < map->height; r++)//replace with some kind of cellular automata / game of life approach, think that would be best
    {
        for(c = 0; c < map->width; c++)
        {
            chunk = &map->chunks[r][c];
            if(chunk->flags & MG_MAINLAND)
            {
                for(t = 0; t < map->chunksize * map->chunksize; t++)
                {
                    chunk->tiles[t] = *tp_get_tile(IT_DIRT);
                }
                int i, j, checkdone = 0;
                for(i = -1; i < 1 && !checkdone; i++)
                {
                    for(j = -1; j < 1 && !checkdone; j++)
                    {
                        if(map->chunks[r + i][c + j].flags & MG_SHORE)
                        {
                            chunk->flags |= MG_HABITABLE;
                            for(t = 0; t < map->chunksize * map->chunksize; t++)
                            {
                                chunk->tiles[t] = *tp_get_tile(IT_DIRT);
                            }
                            checkdone = 1;
                        }
                    }
                }
            }
        }
    }*/
    struct tile *tile;
    char buf[512];
    int nobytes;
    memset(buf, 0, 512);
    if(debug_get())
    {
        ALLEGRO_BITMAP *bmap = al_create_bitmap(map->width, map->height);
        struct sprite *mapsprite = sm_create_sprite(bmap, WIDTH / 2 - map->width, HEIGHT / 2, TEST, NOZOOM);
        al_lock_bitmap(bmap, 0, ALLEGRO_LOCK_WRITEONLY);
        al_set_target_bitmap(bmap);
        ALLEGRO_FILE *f = al_fopen("map.ppm", "w");
        nobytes = snprintf(buf, 511, "P3\n%d %d\n%d\n", 500, 500, 255);
        al_fwrite(f, buf, nobytes);
        for(r = 0; r < map->height; r++)
        {
            for(c = 0; c < map->width; c++)
            {
                chunk = &map->chunks[r][c];
                if(chunk->flags & MG_HABITABLE)
                {
                    al_fwrite(f, "102 57 49\n", 8);
                    al_put_pixel(c, r, al_map_rgb(102, 57, 49));
                }
                else if(chunk->flags & MG_MAINLAND)
                {
                    al_fwrite(f, "0 255 0\n", 8);
                    al_put_pixel(c, r, al_map_rgb(0, 255, 0));
                }
                else if(chunk->flags & MG_OCEAN)
                {
                    al_fwrite(f, "0 0 255\n", 8);
                    al_put_pixel(c, r, al_map_rgb(0, 0, 255));
                }
                else
                {
                    al_fwrite(f, "255 204 102\n", 12);
                    al_put_pixel(c, r, al_map_rgb(255, 204, 102));
                }
            }
        }
        al_unlock_bitmap(bmap);
        al_fclose(f);
        debug_add_sprite(mapsprite);
    }
}

struct turtle
{
    int x;
    int y;
    int dir;
    int valid;
};

int compare_coords(struct coord *one, struct coord *two)
{
    if(one->x == two->x)
        return one->y - two->y;
    return one->x - two->x;
}

int coord_get_x(struct coord *coord)
{
    return coord->x;
}

int coord_get_y(struct coord *coord)
{
    return coord->y;
}

void mg_create_recursive_dungeon(struct map *map, int maxrooms)//not a recursive function, just starts with large rooms and reiterates with smaller room sizes
{
    math_seed(0);//1686504579//1688432932//1688434343//1688504818//1690048624//1690219895

    struct room *rooms = NULL;//s_malloc(maxrooms * sizeof(struct room));
    int norooms = 0;
    struct coord *nodes = NULL;//FIX: this leaks
    int nonodes = 0;

    int wbound = map->width * map->chunksize / 2;
    int hbound = map->height * map->chunksize / 2;
    int largestroomwidth = 50;//maybe set these with config
    int largestroomheight = 50;
    int minlargerooms = wbound * hbound / (largestroomwidth * largestroomheight);

    printf("%d %d %d\n", largestroomwidth, largestroomheight, minlargerooms);
    struct graph *graph = graph_create(0);

    int i, j, retries;
    for(i = 1; i <= 1; i++)
    {
        retries = 0;
        int roomiter = (minlargerooms + math_get_random(minlargerooms)) * i;
        for(j = 0; j < roomiter; j++)
        {
            rooms = s_realloc(rooms, ++norooms * sizeof(struct room), NULL);
            struct room *room = &rooms[norooms - 1];
            
            room->w = (60 + math_get_random(40)) * largestroomwidth  / 100 / i;
            room->h = (60 + math_get_random(40)) * largestroomheight / 100 / i;
            room->x = (math_get_random(1) ? 1 : -1) * math_get_random((wbound - largestroomwidth / 2 - 1)) - largestroomwidth / 2;
            room->y = (math_get_random(1) ? 1 : -1) * math_get_random((hbound - largestroomheight / 2 - 1)) + largestroomheight / 2;
            if(mg_collides(rooms, norooms - 2, room))
            {
                j--;
                norooms--;
                retries++;
                continue;
            }
            if(retries > 100)
                break;

            nodes = s_realloc(nodes, ++nonodes * sizeof(struct coord), NULL);
            struct coord *node = &nodes[nonodes - 1];

            char *dirstr;
            int dir = math_rand() % 4;
            switch(dir)
            {
                case UP:
                    node->x = mg_room_center_x(room);
                    node->y = room->y + 3;
                    dirstr = "up";
                    break;
                case DOWN:
                    node->x = mg_room_center_x(room);
                    node->y = room->y - room->h - 3;
                    dirstr = "down";
                    break;
                case LEFT:
                    node->x = room->x - 3;
                    node->y = mg_room_center_y(room);
                    dirstr = "left";
                    break;
                case RIGHT:
                    node->x = room->x + room->w + 3;
                    node->y = mg_room_center_y(room);
                    dirstr = "right";
                    break;
            }
        }
    }

    math_mergesort(nodes, nonodes, compare_coords, sizeof(struct coord));
    printf("no coords %d\n", nonodes);
    for(i = 0; i < nonodes; i++)
    {
        graph_add_vertex(graph, &nodes[i]);
        //printf("%d %d\n", nodes[i].x, nodes[i].y);
        map_test_color_tile(map, nodes[i].x * (float)map->tilesize, nodes[i].y * (float)map->tilesize);
    }
    for(i = 0; i < norooms; i++)
    {
        mg_put_room_on_map(map, &rooms[i]);
    }
    printf("%d %d %d %d\n", retries, norooms, nonodes, graph->novertices);

    puts("before d");
    delaunay_triangulation(graph, coord_get_x, coord_get_y);
    puts("after d");

    if(graph_is_connected(graph))
        puts("graph is connected");
    else
        puts("graph is disconnected");

    /*struct edge **mst = graph_mst(graph);//set of edges is directional i.e. no two edges u->v and v->u
    struct edge *hall = NULL;
    struct vertex *u = NULL, *v = NULL;
    struct coord *path = NULL, start, end;
    int pathsize;
    if(mst)
    {
        for(i = 0; i < graph->novertices - 1; i++)
        {
            hall = mst[i];
            u = graph_get_vertex(graph, hall->from);
            v = graph_get_vertex(graph, hall->to);
            start = *(struct coord *)u->p;
            end = *(struct coord *)v->p;
            path = mg_a_star(map, &start, &end, &pathsize, 0);
            mg_add_path(map, path, pathsize, 4);
        }
        s_free(mst, NULL);
    }
    else
    {
        puts("graph not connected");
        //mg_connect_coords(map, graph, 4);
    }*/

    map->graph = graph;
    map->rooms = rooms;
    //s_free(nodes, NULL);//FIX: remove this later, free in map_destroy
}

#define DISABLEWALLS 1
void mg_add_path(struct map *map, struct coord *path, int count, int width)
{
    struct tile *temp = NULL;
    struct tile floor = *tp_get_tile(DT_FLOOR);
    int i, j, last;

    if(path)
    {
        last = count - 1;
        for(count -= 2; count >= 0; count--)
        {
            if(path[count].x - path[count + 1].x != 0)//path went right or left, add tiles above and below
            {
                temp = mg_get_tile_from_coordinate(map, path[count].x, path[count].y - width / 2 - 1);
                if(!DISABLEWALLS && temp && temp->id == DT_EMPTY)
                    *temp = *tp_get_tile(DT_BOTTOMWALL);

                for(j = -width / 2; j <= width / 2; j++)
                    mg_update_tile(map, path[count].x, path[count].y + j, &floor);

                temp = mg_get_tile_from_coordinate(map, path[count].x, path[count].y + j);
                if(!DISABLEWALLS && temp && temp->id == DT_EMPTY)
                    *temp = *tp_get_tile(DT_TOPWALL);
                

                /*if(up)
                {
                    temp = mg_get_tile_from_coordinate(map, path[count + 1].x, path[count + 1].y - 1);
                    if(temp->id != DT_EMPTY)
                        *temp = *tp_get_tile(DT_BOTTOMWALL);
                    temp = mg_get_tile_from_coordinate(map, path[count + 1].x, path[count + 1].y + 1);
                    if(temp->id != DT_EMPTY)
                        *temp = *tp_get_tile(DT_TOPWALL);
                }*/
                /*right = 1;
                up = 0;*/
            }
            else//path went up or down, add tiles left and right
            {
                temp = mg_get_tile_from_coordinate(map, path[count].x - width / 2 - 1, path[count].y);
                if(!DISABLEWALLS && temp && temp->id == DT_EMPTY)
                    *temp = *tp_get_tile(DT_LEFTWALL);

                for(j = -width / 2; j <= width / 2; j++)
                    mg_update_tile(map, path[count].x + j, path[count].y, &floor);

                temp = mg_get_tile_from_coordinate(map, path[count].x + j, path[count].y);
                if(!DISABLEWALLS && temp && temp->id == DT_EMPTY)
                    *temp = *tp_get_tile(DT_RIGHTWALL);

                /*if(right)
                {
                    temp = mg_get_tile_from_coordinate(map, path[count + 1].x - 1, path[count + 1].y);
                    if(temp->id != DT_EMPTY)
                        *temp = *tp_get_tile(DT_LEFTWALL);
                    temp = mg_get_tile_from_coordinate(map, path[count + 1].x + 1, path[count + 1].y);
                    if(temp->id != DT_EMPTY)
                        *temp = *tp_get_tile(DT_RIGHTWALL);
                }*/
                /*right = 0;
                up = 1;*/
            }
        }
        if(debug_get())
        {
            temp = mg_get_tile_from_coordinate(map, path[last].x, path[last].y);
            *temp = *tp_get_tile(DT_ERROR);
            temp = mg_get_tile_from_coordinate(map, path[0].x, path[0].y);
            *temp = *tp_get_tile(DT_ERROR);
        }
        s_free(path, NULL);
    }
}

void mg_create_hallway_dungeon(struct map *map, int maxrooms)//WIP...
{
    math_seed(0);//1643581891//1649730122//1649739262//1649800390//1650157911

    struct room *halls = NULL;
    struct room *rooms = NULL;
    int nohalls = 0;
    int noturtles = 1;
    struct turtle *turtles = s_malloc(noturtles * sizeof(struct turtle), NULL);

    turtles[0].x = 0;
    turtles[0].y = 0;
    turtles[0].dir = 0;
    turtles[0].valid = 1;
    int i;
    for(i = 1; i < noturtles; i++)
    {
        turtles[i].x = 0;
        turtles[i].y = 0;
        turtles[i].dir = 0;
        turtles[i].valid = 1;
    }

    int dist, avdist = 50, hallhwidth = 2, noiter = 5, j;
    for(j = 0; j < noiter; j++)
    {
        for(i = 0; i < noturtles; i++)
        {
            if(!turtles[i].valid)
                continue;

            dist = (math_get_random(avdist) + math_get_random(avdist) + math_get_random(avdist)) / 3;
            if(math_get_random(1))//are we turning?
            {
                turtles[i].dir += math_get_random(1) ? 1 : -1;//turn up to 90 degrees
                if(turtles[i].dir == -1)
                    turtles[i].dir = 3;
                else if (turtles[i].dir == 4)
                    turtles[i].dir = 0;
            }
            struct room *newhall = NULL;

            switch(turtles[i].dir)
            {
                case 0://right
                    if(turtles[i].x + dist > map->width * map->chunksize / 2 || turtles[i].y + hallhwidth > map->height * map->chunksize / 2)
                    {
                        turtles[i].valid = 0;
                        continue;
                    }
                    halls = s_realloc(halls, ++nohalls * sizeof(struct room), NULL);
                    newhall = &halls[nohalls - 1];
                    newhall->x = turtles[i].x;
                    newhall->y = turtles[i].y + hallhwidth;
                    newhall->w = dist;
                    newhall->h = 2 * hallhwidth;
                    turtles[i].x += dist;
                    break;
                case 1://up
                    if(turtles[i].x - hallhwidth > map->width * map->chunksize / 2 || turtles[i].y + dist > map->height * map->chunksize / 2)
                    {
                        turtles[i].valid = 0;
                        continue;
                    }
                    halls = s_realloc(halls, ++nohalls * sizeof(struct room), NULL);
                    newhall = &halls[nohalls - 1];
                    newhall->x = turtles[i].x - hallhwidth;
                    newhall->y = turtles[i].y + dist;
                    newhall->w = 2 * hallhwidth;
                    newhall->h = dist;
                    turtles[i].y += dist;
                    break;
                case 2://left
                    if(turtles[i].x - dist < -map->width * map->chunksize / 2 || turtles[i].y + hallhwidth > map->height * map->chunksize / 2)
                    {
                        turtles[i].valid = 0;
                        continue;
                    }
                    halls = s_realloc(halls, ++nohalls * sizeof(struct room), NULL);
                    newhall = &halls[nohalls - 1];
                    newhall->x = turtles[i].x - dist;
                    newhall->y = turtles[i].y + hallhwidth;
                    newhall->w = dist;
                    newhall->h = 2 * hallhwidth;
                    turtles[i].x -= dist;
                    break;
                case 3://down
                    if(turtles[i].x - hallhwidth < -map->width * map->chunksize / 2 || turtles[i].y - dist > -map->height * map->chunksize / 2)
                    {
                        turtles[i].valid = 0;
                        continue;
                    }
                    halls = s_realloc(halls, ++nohalls * sizeof(struct room), NULL);
                    newhall = &halls[nohalls - 1];
                    newhall->x = turtles[i].x - hallhwidth;
                    newhall->y = turtles[i].y;
                    newhall->w = 2 * hallhwidth;
                    newhall->h = dist;
                    turtles[i].y -= dist;
                    break;
            }
        }
    }

    s_free(turtles, NULL);

    for(i = 0; i < nohalls; i++)
    {
        mg_put_room_on_map(map, &halls[i]);
    }
    s_free(halls, NULL);
    /*struct room *rooms = s_malloc(norooms * sizeof(struct room), "mg_create_classic_dungeon");
    memset(rooms, 0, norooms * sizeof(struct room));*/
    
    /*rooms[0].w = 7 + math_get_random(13);
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
    }*/

    /*struct graph *graph = graph_create(0);
    struct tile *t = map_get_tile_from_coordinate(map, (rooms[0].x + rooms[0].w - 3.0f) * 16.0f, (rooms[0].y - rooms[0].h + 3.0f) * 16.0f);
    printf("%.2f %.2f\n", ((float)rooms[0].x + rooms[0].w - 3.0f) * 16.0f, ((float)rooms[0].y - rooms[0].h + 3.0f) * 16.0f);
    printf("%d %d\n", rooms[0].x + rooms[0].w - 3, rooms[0].y - rooms[0].h + 3);
    math_mergesort(rooms, i, room_comp, sizeof(struct room));
    int j;
    for(j = 0; j < i; j++)
    {
        graph_add_vertex(graph, &rooms[j]);
        mg_put_room_on_map(map, &rooms[j]);
        
    }*/

    /*for(j = 0; j < i; j++)
        debug_printf("%d %d\n", mg_room_center_x(&rooms[j]), mg_room_center_y(&rooms[j]));*/

    //delaunay_triangulation(graph, mg_room_center_x, mg_room_center_y);
    
    //mg_connect_rooms(map, graph);

    //map->graph = graph;
}

void mg_create_classic_dungeon(struct map *map, int maxrooms)
{
    math_seed(1676594803);//1643581891//1649730122//1649739262//1649800390//1650157911//

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
    /*struct tile *t = map_get_tile_from_coordinate(map, (rooms[0].x + rooms[0].w - 3.0f) * 16.0f, (rooms[0].y - rooms[0].h + 3.0f) * 16.0f);
    printf("%.2f %.2f\n", ((float)rooms[0].x + rooms[0].w - 3.0f) * 16.0f, ((float)rooms[0].y - rooms[0].h + 3.0f) * 16.0f);
    printf("%d %d\n", rooms[0].x + rooms[0].w - 3, rooms[0].y - rooms[0].h + 3);*/
    math_mergesort(rooms, i, room_comp, sizeof(struct room));
    int j;
    for(j = 0; j < i; j++)
    {
        graph_add_vertex(graph, &rooms[j]);
        mg_put_room_on_map(map, &rooms[j]);
    }

    /*for(j = 0; j < i; j++)
        debug_printf("%d %d\n", mg_room_center_x(&rooms[j]), mg_room_center_y(&rooms[j]));*/

    delaunay_triangulation(graph, mg_room_center_x, mg_room_center_y);
    
    mg_connect_rooms(map, graph, 1);

    map->graph = graph;
    map->rooms = rooms;
}

int room_comp(struct room *one, struct room *two)
{
    if(mg_room_center_x(one) == mg_room_center_x(two))
        return mg_room_center_y(two) - mg_room_center_y(one);
    return mg_room_center_x(one) - mg_room_center_x(two);
}

int mg_room_center_x(struct room *room)
{
    return room->x + room->w / 2;
}

int mg_room_center_y(struct room *room)
{
    return room->y - room->h / 2;
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
    graph_add_vertex(graph, rooms);

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

        graph_add_vertex(graph, &rooms[i]);
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
    int startx = room->x, starty = room->y;
    struct tile *tile;

    for(c = 1; c < room->w - 1; c++)
    {
        tile = mg_update_tile(map, startx + c, starty, tp_get_tile(DT_TOPWALL));
        tile = mg_update_tile(map, startx + c, starty - room->h + 1, tp_get_tile(DT_BOTTOMWALL));
    }

    for(r = 1; r < room->h - 1; r++)
    {
        tile = mg_update_tile(map, startx, starty - r, tp_get_tile(DT_LEFTWALL));
        tile = mg_update_tile(map, startx + room->w - 1, starty - r, tp_get_tile(DT_RIGHTWALL));
    }

    mg_fill_area(map, startx + 1, starty - 1, startx + room->w - 2, starty - room->h + 2, tp_get_tile(DT_FLOOR));

    for(r = 0; r < room->noexits; r++)
        mg_update_tile(map, room->exit[r].x, room->exit[r].y, tp_get_tile(DT_ERROR));

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

    struct tile *oldtile = map_get_tile_from_coordinate(map, x * 16.0f, y * 16.0f);
    if(!oldtile)
        return NULL;

    oldtile->id = tile->id;
    oldtile->type = tile->type;

    return oldtile;
}

struct tile *mg_get_tile(struct map *map, float x, float y)
{
    struct tile *tile = map_get_tile_from_coordinate(map, x * map->tilesize, y * map->tilesize);
    return tile;
}

struct chunk *mg_get_chunk(struct map *map, float x, float y)
{
    return NULL;
}

struct tile *mg_get_tile_from_coordinate(struct map *map, float x, float y)
{
    return map_get_tile_from_coordinate(map, x * map->tilesize, y * map->tilesize);
}

int mg_connect_room_exits(struct map *map, struct room *rooms, struct graph *graph)
{
    int i, j;
    struct vertex *v, *n;
    struct tile *floor = tp_get_tile(DT_FLOOR);
    for(i = 0; i < graph->novertices; i++)
    {
        v = &graph->vertices[i];
        for(j = 0; j < v->noedges; j++)
        {
            n = graph_get_next_vertex(graph, v, j);
            struct room *one = v->p, *two = n->p;

            int count;
            struct coord *path = NULL;
            path = mg_a_star(map, &one->exit[FROM], &two->exit[TO], &count, 0);

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

int mg_connect_rooms(struct map *map, struct graph *graph, int width)
{
    int i, j, count, right = 0, up = 0;
    struct edge *e = NULL;
    struct vertex *v = NULL, *n = NULL;
    struct room *here = NULL, *there = NULL;
    struct coord to, from;
    struct tile floor = *tp_get_tile(DT_FLOOR), *temp = NULL;
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
            //mg_update_tile(map, from.x, from.y, tp_get_tile(ERROR));
            //mg_update_tile(map, to.x, to.y, tp_get_tile(ERROR));

            struct coord *path = mg_a_star(map, &from, &to, &count, 0);
            //a star is pointless here, just being lazy, tremendous performance hit
            //just find a path with a right angle, same outcome

            mg_add_path(map, path, count, width);
            
        }
    }
    graph_unmark(graph);
    return 1;
}

int mg_connect_coords(struct map *map, struct graph *graph, int width)
{
    int i, j, count, right = 0, up = 0;
    struct edge *e = NULL;
    struct vertex *v = NULL, *n = NULL;
    struct room *here = NULL, *there = NULL;
    struct coord to, from;
    struct tile floor = *tp_get_tile(DT_FLOOR), *temp = NULL;
    int last;
    for(i = 0; i < graph->noedges; i += 2)
    {
        e = &graph->edges[i];
        
        v = graph_get_vertex(graph, e->from);
        n = graph_get_vertex(graph, e->to);
        if(v && n)
        {
            here = v->p;
            there = n->p;

            from.x = here->x;
            from.y = here->y;
            to.x = there->x;
            to.y = there->y;
            //mg_update_tile(map, from.x, from.y, tp_get_tile(ERROR));
            //mg_update_tile(map, to.x, to.y, tp_get_tile(ERROR));

            struct coord *path = mg_a_star(map, &from, &to, &count, 0);
            //a star is pointless here, just being lazy, tremendous performance hit
            //just find a path with a right angle, same outcome

            mg_add_path(map, path, count, width);
        }
    }
    graph_unmark(graph);
    return 1;
}

/*int compare_coords(struct coord *one, struct coord *two)
{
    if(one->x == two->x)
        return one->y - two->y;
    return one->x - two->x;
}*/

float euclid_dist(int x1, int y1, int x2, int y2)
{
    int xdiff = x1 - x2;
    int ydiff = y1 - y2;
    return math_sqrt(xdiff * xdiff + ydiff * ydiff);
}

static float grid_heur(int x1, int y1, int x2, int y2)
{
    return math_abs(x1 - x2) + math_abs(y1 - y2);
}

struct coord *mg_a_star(struct map *map, struct coord *start, struct coord *end, int *no, unsigned char typemask)//uses grid coordinates
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