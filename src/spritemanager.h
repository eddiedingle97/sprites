#ifndef __MOVE_H__
#define __MOVE_H__

struct animation
{
	unsigned short width;
    unsigned short height;
    unsigned short y;
    unsigned short x;
    unsigned char spritecount;
    unsigned char cycle;
    unsigned char ticks;
    int alflags;
};

struct sprite
{
	char type;
	char layer;
	char *name;
	float x;
	float y;
	struct node *id;
	ALLEGRO_BITMAP *bitmap;
	void (*draw)(struct sprite *sprite, int tick);
	struct animation *an;
	char i;
};

void sm_init(const int height, const int width);
struct sprite *sm_create_sprite(ALLEGRO_BITMAP *bit, float x, float y, int layer, int typeflags);
struct sprite *sm_create_global_sprite(ALLEGRO_BITMAP *bitmap, float x, float y, int layer, int typeflags);
struct sprite *sm_create_global_dynamic_sprite(ALLEGRO_BITMAP *bitmap, void (*draw)(struct sprite *sprite, int tick), struct animation *an, float x, float y, int layer, int typeflags);
void sm_update_sprite(struct sprite *sprite, ALLEGRO_BITMAP *bitmap, int x, int y);
void sm_add_sprite_to_layer(struct sprite *sprite);
void sm_draw_sprites(ALLEGRO_DISPLAY *display);
void sm_move_to_front(struct node *id);
void sm_destroy_sprite(struct sprite *sprite);
void sm_destroy_sprite_from_layer(struct sprite *sprite);
void sm_remove_sprite_from_layer(struct sprite *sprite);
void sm_destroy();
float sm_global_to_rel_x(float x);
float sm_global_to_rel_y(float y);
float sm_rel_to_global_x(float x);
float sm_rel_to_global_y(float y);
float sm_get_x(float x, int bitmapwidth);
float sm_get_y(float y, int bitmapheight);
void sm_set_zoom(int z);
float sm_get_zoom();
float sm_get_coord(int i);
int sm_get_sprite_count();
void sm_move_coord(float dx, float dy);

enum SPRITETYPE {LOCAL = 1, GLOBAL = 2, CENTERED = 4, NOZOOM = 8, DYNAMIC = 16};
enum LAYERENUM {TEST = 0, MENU = 1, FOREGROUND = 2, PLAYER = 3, SECOND = 4, BACKGROUND = 5};
enum COORDENUM {X = 0, Y = 1};

#define LAYERS 6

#endif