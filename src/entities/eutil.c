#include <stdio.h>
#include "entities.h"
#include "../sprites.h"
#include "../map.h"
#include "../emath.h"
#include "../entity.h"
#include "../debug.h"

float eu_lerp_check(struct map *map, float x1, float y1, float x2, float y2, unsigned char tilemask)//FIX: does a check for each pixel... maybe do raycast approach in the future
{
    float x = x1 - x2, y = y1 - y2;
    int n = math_sqrt(x * x + y * y), i = 0;
    struct tile *t = NULL;

    x /= n;
    y /= n;

    for(i = 0; i < n; i++)
    {
        x1 -= x;
        y1 -= y;
        t = map_get_tile_from_coordinate(map, x1, y1);
        if(t && t->type & tilemask)
            return 0;
    }

    return n;
}

struct pixcoord *eu_a_star(struct map *map, float startx, float starty, float endx, float endy, int *no, unsigned char typemask,
    float (*cost)(struct map *, struct coord *, struct coord *, void *data), void *data)//takes pixel coordinates
{
    struct pixcoord start, end;
    start.x = startx;
    start.y = starty;
    end.x = endx;
    end.y = endy;
    struct pixcoord *path = mu_a_star(map, &start, &end, no, typemask, cost, data);
	if(!path)
		return NULL;
    struct pixcoord *outpath = s_malloc(sizeof(struct pixcoord), NULL);
    int outpathsize = 1;
    struct pixcoord *base, *last;
    base = &path[*no - 1];
    outpath[0].x = path[*no - 1].x;
    outpath[0].y = path[*no - 1].y;
	last = &path[*no - 2];

    int i;
    for(i = *no - 2; i > 0; i--)//reduce number of coords in path, ensure more direct path
    {
        if(debug_get())
            map_test_color_tile(map, path[i].x, path[i].y);
        if(eu_lerp_check(map, base->x, base->y, path[i].x, path[i].y, typemask) != 0)
            last = &path[i];
        else
        {
            outpath = s_realloc(outpath, ++outpathsize * sizeof(struct pixcoord), NULL);
            outpath[outpathsize - 1].x = last->x;
            outpath[outpathsize - 1].y = last->y;
            base = last;
            last = &path[i];
        }
    }

    outpath = s_realloc(outpath, ++outpathsize * sizeof(struct pixcoord), NULL);
    outpath[outpathsize - 1].x = path[0].x;
    outpath[outpathsize - 1].y = path[0].y;

    s_free(path, NULL);
    *no = outpathsize;

    return outpath;
}

int eu_follow_path(struct map *map, struct entity *e, struct pixcoord *path, int pathsize, int *next, float *dx, float *dy)
{
    if(*next == pathsize)
        return 1;
    if(math_in_range(path[*next].x - map->tilesize / 8, e->sprite->x, path[*next].x + map->tilesize / 8) &&
       math_in_range(path[*next].y - map->tilesize / 8, e->sprite->y, path[*next].y + map->tilesize / 8))
    {
        *next = *next + 1;
        if(*next == pathsize)
        {
            return 1;
        }
    }

    *dx = path[*next].x - e->sprite->x;
    *dy = path[*next].y - e->sprite->y;
    return 0;
}