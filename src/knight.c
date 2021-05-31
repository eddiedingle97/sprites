#include "entity.h"
#include "keyboard.h"
#include "sprites.h"
#include "movementandcollision.h"
#include "spritemanager.h"

struct knightdata
{
    unsigned short width;
    unsigned short height;
    unsigned short y;
    unsigned short x;
    unsigned char spritecount;
    unsigned char cycle;
    unsigned char idle;
    unsigned char ticks;
    int alflags;
    float speed;
    ALLEGRO_BITMAP *bitmap;
};

void knight_draw(float x, float y, float zoom, struct knightdata *data, int tick)
{
    tick = tick % data->ticks;
    if(!tick)
        data->cycle++;
    
    float neww = data->width * zoom, newh = data->height * zoom;
    data->cycle = data->cycle % data->spritecount;

    al_draw_scaled_bitmap(data->bitmap, data->x + data->cycle * data->width + (!data->idle * data->spritecount * data->width), data->y, data->width, data->height, sm_get_x(x, neww), sm_get_y(y, newh * 3 / 2), neww, newh, data->alflags);
}

void knight_behaviour(struct sprite *sprite, struct knightdata *data)
{
    float up = kb_get_up() * data->speed, down = kb_get_down() * data->speed, left = kb_get_left() * data->speed, right = kb_get_right() * data->speed;
    if(left && !right)
        data->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(right && !left)
        data->alflags = 0;

    if(mc_do_movement(sprite, up, down, left, right))
        data->idle = 0;
    else
        data->idle = 1;
}

struct entity *knight_create(ALLEGRO_BITMAP *spritesheet)
{
    struct knightdata *kd = s_malloc(sizeof(struct knightdata), "create_knight");
    kd->width = 16;
    kd->height = 32;
    kd->x = 0;
    kd->y = 64;
    kd->spritecount = 4;
    kd->cycle = 0;
    kd->idle = 1;
    kd->ticks = 6;
    kd->speed = 1;
    kd->bitmap = spritesheet;//al_load_bitmap(s_get_full_path_with_dir("images", "DungeonPlayersprites.png"));
    kd->alflags = 0;
    struct entity *out = e_create(knight_draw, knight_behaviour, 0, 0, kd);
    return out;
}

void knight_destroy(struct entity *knight)
{
    e_destroy(knight);
}