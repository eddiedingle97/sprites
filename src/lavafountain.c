#include <allegro5/allegro.h>
#include "sprites.h"
#include "spritemanager.h"
#include "entity.h"


struct lavafountaindata
{
    unsigned short width;
    unsigned short height;
    unsigned short y;
    unsigned short x;
    unsigned char spritecount;
    unsigned char cycle;
    unsigned char ticks;
    ALLEGRO_BITMAP *bitmap;
};

void lf_draw(float x, float y, float zoom, struct lavafountaindata *data, int tick)
{
    tick = tick % data->ticks;
    if(!tick)
        data->cycle++;
    
    float neww = data->width * zoom, newh = data->height * zoom;
    data->cycle = data->cycle % data->spritecount;

    al_draw_scaled_bitmap(data->bitmap, data->x + data->cycle * data->width, data->y, data->width, data->height, x, y, sm_get_x(x, 0), sm_get_y(y, 0), neww, newh, 0);
}

struct entity *lf_create(ALLEGRO_BITMAP *bitmap)
{
    struct lavafountaindata *lfd = s_malloc(sizeof(struct lavafountaindata));
    lfd->width = 16;
    lfd->height = 32;
    lfd->y = 800;
    lfd->x = 0;
    lfd->spritecount = 3;
    lfd->cycle = 0;
    lfd->ticks = 8;
    lfd->bitmap = bitmap;
    struct entity *out = e_create(lf_draw, NULL, 0, 0, lfd);

    return out;
}

void lf_destroy(struct entity *e)
{
    e_destroy(e);
}