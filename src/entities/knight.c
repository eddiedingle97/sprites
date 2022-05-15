#include "../entity.h"
#include "../keyboard.h"
#include "../sprites.h"
#include "../spritemanager.h"

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

void knight_behaviour(struct entity *e, float *dx, float *dy)
{
    struct sprite *sprite = e->sprite;
    struct knightdata *data = e->data;
    struct animation *an = sprite->an;
    float up = kb_get_up() * data->speed, down = kb_get_down() * data->speed, left = kb_get_left() * data->speed, right = kb_get_right() * data->speed;
    if(left && !right)
        sprite->alflags |= ALLEGRO_FLIP_HORIZONTAL;

    else if(right && !left)
        sprite->alflags = 0;

    *dx = right - left;
    *dy = up - down;

    if(*dx == 0 && *dy == 0)
        data->idle = 1;
    else
        data->idle = 0;

    sprite->i = data->idle;
}

struct entity *knight_create(ALLEGRO_BITMAP *spritesheet)
{
    struct knightdata *kd = s_malloc(sizeof(struct knightdata), "kd: create_knight");
    struct animation *an = s_malloc(2 * sizeof(struct animation), "an: create_knight");
    an[0].width = 16;
    an[0].height = 32;
    an[0].x = 64;
    an[0].y = 64;
    an[0].spritecount = 4;
    an[0].ticks = 6;
    an[0].offsetx = 0;
    an[0].offsety = 8;
    
    kd->idle = 1;
    kd->speed = 1;
    //kd->bitmap = spritesheet;//al_load_bitmap(s_get_full_path_with_dir("images", "DungeonPlayersprites.png"));
    
    an[1].width = 16;
    an[1].height = 32;
    an[1].x = 0;
    an[1].y = 64;
    an[1].spritecount = 4;
    an[1].ticks = 6;
    an[1].offsetx = 0;
    an[1].offsety = 8;

    struct entity *out = e_create(spritesheet, NULL, knight_behaviour, 0, 0, an, kd);
    out->destroy = 0;
    return out;
}

void knight_destroy(struct entity *knight)
{
    s_free(knight->sprite->an, NULL);
    e_destroy(knight);
}