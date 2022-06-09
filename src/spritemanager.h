#ifndef __MOVE_H__
#define __MOVE_H__
#include <allegro5/allegro.h>

struct animation
{
	unsigned char width;
    unsigned char height;
    unsigned short y;
    unsigned short x;
	char offsetx;
	char offsety;
    unsigned char spritecount;
    unsigned char ticks;
};

struct sprite
{
	char type;
	unsigned char layer;
	float x;
	float y;
	struct node *node;
	union
	{
		struct//static 16 bytes
		{
			ALLEGRO_BITMAP *bitmap;
			float rot;
			float rotoffset;
		};
		struct//dynamic 14 bytes
		{
			struct animation *an;
			unsigned char i;
			unsigned char cycle;
			int alflags;
		};
	};
	char *name;
};

void sm_init(ALLEGRO_BITMAP *spritesheet, const int height, const int width);
struct sprite *sm_create_sprite(ALLEGRO_BITMAP *bit, float x, float y, int layer, int typeflags);
struct sprite *sm_create_global_sprite(ALLEGRO_BITMAP *bitmap, float x, float y, int layer, int typeflags);
struct sprite *sm_create_global_dynamic_sprite(struct animation *an, float x, float y, int layer, int typeflags);
ALLEGRO_BITMAP *sm_get_sub_bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
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
float sm_get_x(float x, float bitmapwidth);
float sm_get_y(float y, float bitmapheight);
void sm_set_zoom(int z);
float sm_get_zoom();
float sm_get_coord(int i);
int sm_get_sprite_count();
void sm_move_coord(float dx, float dy);
void sm_set_coord(float x, float y);
void sm_default_draw(struct sprite *sprite);
void sm_deferred_draw(struct sprite *sprite);

enum SPRITETYPE {LOCAL = 1, GLOBAL = 2, CENTERED = 4, NOZOOM = 8, DYNAMIC = 16};
enum LAYERENUM {TEST, MENU, FOREGROUND, PLAYER, SECOND, BACKGROUND, LIMBO};
enum COORDENUM {X, Y};

#define LAYERS 7

#endif