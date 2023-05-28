#include <stdio.h>
#include "entities.h"
#include "../sprites.h"
#include "../map.h"
#include "../emath.h"
#include "../entity.h"

float eu_lerp_check(struct map *map, float x1, float y1, float x2, float y2, unsigned char tilemask)
{
    float x = x1 - x2, y = y1 - y2;
    int n = math_sqrt(x * x + y * y), i = 0;
    struct tile *t = NULL;

    x /= n;
    y /= n;

    for(i = 0; i < n; i++)
    {
        x2 += x;
        y2 += y;
        t = map_get_tile_from_coordinate(map, x2, y2);
        if(t && t->type & tilemask)
            return 0;
    }

    return n;
}

struct pixcoord *eu_a_star(struct map *map, float startx, float starty, float endx, float endy, int *no, unsigned char typemask)//takes pixel coordinates
{
    struct coord start, end;
    start.x = startx / map->tilesize;
    start.y = starty / map->tilesize;
    end.x = endx / map->tilesize;
    end.y = endy / map->tilesize;
    struct coord *path = a_star(map, &start, &end, no, typemask);
	if(!path)
		return NULL;
    struct pixcoord *outpath = s_malloc(sizeof(struct pixcoord), NULL);
    int outpathsize = 1;
    struct coord *base, *last;
    base = &path[*no - 1];
    outpath[0].x = path[*no - 1].x * map->tilesize + map->tilesize / 2;
    outpath[0].y = path[*no - 1].y * map->tilesize - map->tilesize / 2;
	last = &path[*no - 2];

    int i;
    for(i = *no - 2; i > 0; i--)//reduce number of coords in path, ensure more direct path
    {
        if(eu_lerp_check(map, base->x * map->tilesize, base->y * map->tilesize, path[i].x * map->tilesize, path[i].y * map->tilesize, typemask) != 0)
            last = &path[i];
        else
        {
            outpath = s_realloc(outpath, ++outpathsize * sizeof(struct pixcoord), NULL);
            outpath[outpathsize - 1].x = last->x * map->tilesize + map->tilesize / 2;
            outpath[outpathsize - 1].y = last->y * map->tilesize - map->tilesize / 2;
            base = last;
        }
    }

    outpath = s_realloc(outpath, ++outpathsize * sizeof(struct pixcoord), NULL);
    outpath[outpathsize - 1].x = path[0].x * map->tilesize + map->tilesize / 2;
    outpath[outpathsize - 1].y = path[0].y * map->tilesize - map->tilesize / 2;

	for(i = 0; i < outpathsize; i++)
	{
		printf("%.2f %.2f\n", outpath[i].x, outpath[i].y);
	}

    s_free(path, NULL);
    *no = outpathsize;

    return outpath;
}