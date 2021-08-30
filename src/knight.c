#include "entity.h"
#include "keyboard.h"
#include "sprites.h"
#include "movementandcollision.h"
#include "spritemanager.h"

struct knightdata
{
    unsigned char idle;
    float speed;
    ALLEGRO_BITMAP *bitmap;
};

/*void knight_draw(float x, float y, float zoom, struct knightdata *data, int tick)
{
    tick = tick % data->ticks;
    if(!tick)
        data->cycle++;
    
    float neww = data->width * zoom, newh = data->height * zoom;
    data->cycle = data->cycle % data->spritecount;

    al_draw_scaled_bitmap(data->bitmap, data->x + data->cycle * data->width + (!data->idle * data->spritecount * data->width), data->y, data->width, data->height, sm_get_x(x, neww), sm_get_y(y, newh * 3 / 2), neww, newh, data->alflags);
}*/

void knight_behaviour(struct sprite *sprite, struct knightdata *data)
{
    struct animation *an = sprite->an;
    float up = kb_get_up() * data->speed, down = kb_get_down() * data->speed, left = kb_get_left() * data->speed, right = kb_get_right() * data->speed;
    if(left && !right)
        an->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(right && !left)
        an->alflags = 0;

    if(mc_do_main_movement(sprite, up, down, left, right))
        data->idle = 0;
    else
        data->idle = 1;

    sprite->i = data->idle;
}

struct entity *knight_create(ALLEGRO_BITMAP *spritesheet)
{
    struct knightdata *kd = s_malloc(sizeof(struct knightdata), "kd: create_knight");
    struct animation *an = s_malloc(2 * sizeof(struct animation), "an: create_knight");
    an->width = 16;
    an->height = 32;
    an->x = 64;
    an->y = 64;
    an->spritecount = 4;
    an->cycle = 0;
    an->ticks = 6;
    an->alflags = 0;
    kd->idle = 1;
    kd->speed = 1;
    //kd->bitmap = spritesheet;//al_load_bitmap(s_get_full_path_with_dir("images", "DungeonPlayersprites.png"));
    
    an[1].width = 16;
    an[1].height = 32;
    an[1].x = 0;
    an[1].y = 64;
    an[1].spritecount = 4;
    an[1].cycle = 0;
    an[1].ticks = 6;
    an[1].alflags = 0;

    struct entity *out = e_create(spritesheet, NULL, knight_behaviour, 0, 0, an, kd);
    return out;
}

void knight_destroy(struct entity *knight)
{
    e_destroy(knight);
}