#include <allegro5/allegro.h>
#include <stdio.h>
#include <math.h>
#include "sprites.h"
#include "list.h"
#include "spritemanager.h"
#include "map.h"
#include "mapmanager.h"
#include "entity.h"
#include "colors.h"
#include "debug.h"

static struct list *layers;
static ALLEGRO_BITMAP *spritesheet;

static const int HHEIGHT = HEIGHT / 2;
static const int HWIDTH = WIDTH / 2;
static const float MAXZOOM = 4;
static const float MINZOOM = 1;

static int nosprites = 0;
static const int TICKRESET = 256;
static int tick = 0;
static float coord[2];
static float zoom = MAXZOOM;
static const float ZOOMINC = .125;
static int deferreddrawthisframe[LAYERS];
static ALLEGRO_BITMAP *deferredlayers[LAYERS];

void sm_init(ALLEGRO_BITMAP *ss, int x, int y)
{
	spritesheet = ss;
	layers = list_create();

	coord[X] = x;
	coord[Y] = y;

	int i;
	for(i = 0; i < LAYERS; i++)
	{
		deferredlayers[i] = al_create_bitmap(WIDTH, HEIGHT);
		deferreddrawthisframe[i] = 0;
		list_append(layers, list_create());
	}
}

void sm_add_sprite_to_layer(struct sprite *sprite)
{
	if(!sprite->node)
	{
		int layer = sprite->layer;
		if(layer < 0 || layer >= LAYERS)
		{
			if(sprite->name)
				fprintf(stderr, "sprite %s layer value is invalid", sprite->name);
			else
				fprintf(stderr, "sprite layer value is invalid");
			return;
		}
		struct list *l = list_get(layers, layer);
		list_append(l, sprite);
		sprite->node = l->tail;
		nosprites++;
	}
}

void sm_error_local_and_global(struct sprite *sprite)
{
	if(!sprite->name)
		fprintf(stderr, "Error: sprite cannot be both local and global\n");
	else
		fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
	sm_remove_sprite_from_layer(sprite);
	sprite->layer = LIMBO;
	sm_add_sprite_to_layer(sprite);
}

void sm_error_local_nor_global(struct sprite *sprite)
{
	if(!sprite->name)
		fprintf(stderr, "Error: sprite must be either local or global\n");
	else
		fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
	sm_remove_sprite_from_layer(sprite);
	sprite->layer = LIMBO;
	sm_add_sprite_to_layer(sprite);
}

void sm_draw_sprites(ALLEGRO_DISPLAY *display)
{
	al_set_target_bitmap(al_get_backbuffer(display));

	tick = (tick + 1) & ~TICKRESET;

	int i, j;
	struct list *layer;
	struct node *node;
	struct sprite *sprite;
	al_hold_bitmap_drawing(1);
	for(i = SECOND; i >= 0; i--)
	{
		layer = list_get(layers, i);
		node = layer->head;
		if(deferreddrawthisframe[i])
			al_draw_bitmap(deferredlayers[i], 0, 0, 0);
		for(j = 0; j < layer->size; j++)
		{
			sprite = node->p;

			sm_default_draw(sprite);

			node = node->next;
		}
	}
	al_hold_bitmap_drawing(0);

	for(i = SECOND; i >= 0; i--)
	{
		if(deferreddrawthisframe[i])
		{
			al_set_target_bitmap(deferredlayers[i]);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			deferreddrawthisframe[i] = 0;
		}
	}
}

void sm_deferred_draw(struct sprite *sprite)
{
	if(sprite->layer < LIMBO)
	{
		al_set_target_bitmap(deferredlayers[sprite->layer]);
		
		sm_default_draw(sprite);
		deferreddrawthisframe[sprite->layer] = 1;
	}
}

void sm_default_draw(struct sprite *sprite)
{
	float w = 0;
	float h = 0;
	float neww = 0;
	float newh = 0;
	struct animation *an = NULL;

	if(sprite->type & DYNAMIC)
	{
		an = &sprite->an[sprite->i];
    	int t = tick % an->ticks;
    	if(t == 0)
        	sprite->cycle++;
    
    	neww = an->width * zoom;
		newh = an->height * zoom;
    	sprite->cycle = sprite->cycle % an->spritecount;
	}
	else
	{
		w = al_get_bitmap_width(sprite->bitmap);
		h = al_get_bitmap_height(sprite->bitmap);
		neww = w * zoom;
		newh = h * zoom;
	}

	switch(sprite->type)
	{
		case 0:
			sm_error_local_nor_global(sprite);
			break;

		case 1://LOCAL
			al_draw_scaled_rotated_bitmap(sprite->bitmap, 0, 0, sm_get_x(sprite->x * zoom, 0), sm_get_y(sprite->y * zoom, 0), zoom, zoom, -sprite->rot - sprite->rotoffset, 0);
			break;
		
		case 2://GLOBAL
			al_draw_scaled_rotated_bitmap(sprite->bitmap, 0, 0, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), zoom, zoom, -sprite->rot - sprite->rotoffset, 0);
			break;

		case 3://LOCAL + GLOBAL
			sm_error_local_and_global(sprite);
			break;

		case 4:
			sm_error_local_nor_global(sprite);
			break;

		case 5://LOCAL + CENTERED
			al_draw_scaled_rotated_bitmap(sprite->bitmap, w / 2, h / 2, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), zoom, zoom, -sprite->rot - sprite->rotoffset, 0);
			break;

		case 6://GLOBAL + CENTERED
			al_draw_scaled_rotated_bitmap(sprite->bitmap, w / 2, h / 2, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), zoom, zoom, -sprite->rot - sprite->rotoffset, 0);
			break;
		
		case 7://LOCAL + GLOBAL + CENTERED
			sm_error_local_and_global(sprite);
			break;

		case 8://NOZOOM
			sm_error_local_nor_global(sprite);
			break;

		case 9://LOCAL + NOZOOM
			al_draw_rotated_bitmap(sprite->bitmap, 0, 0, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), -sprite->rot, 0);
			break;

		case 10://GLOBAL + NOZOOM
			al_draw_rotated_bitmap(sprite->bitmap, 0, 0, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), -sprite->rot, 0);
			break;

		case 11://LOCAL + GLOBAL + NOZOOM
			sm_error_local_and_global(sprite);
			break;

		case 12://CENTERED + NOZOOM
			sm_error_local_nor_global(sprite);
			break;

		case 13://LOCAL + CENTERED + NOZOOM
			al_draw_rotated_bitmap(sprite->bitmap, w / 2, h / 2, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), -sprite->rot, 0);
			break;

		case 14://GLOBAL + CENTERED + NOZOOM
			al_draw_rotated_bitmap(sprite->bitmap, w / 2, h / 2, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), -sprite->rot, 0);
			break;

		case 15://LOCAL + GLOBAL + CENTERED + NOZOOM
			sm_error_local_and_global(sprite);
			break;
		/*
			DYNAMIC SPRITE DRAWING BELOW
		*/
		case 16:
			sm_error_local_nor_global(sprite);
			break;

		case 17://LOCAL
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), neww, newh, sprite->alflags);
			break;
		
		case 18://GLOBAL
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), neww, newh, sprite->alflags);
			break;

		case 19://LOCAL + GLOBAL
			sm_error_local_and_global(sprite);
			break;

		case 20:
			sm_error_local_nor_global(sprite);
			break;

		case 21://LOCAL + CENTERED
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sprite->x, neww), sm_get_y(sprite->y, newh), neww, newh, sprite->alflags);
			break;

		case 22://GLOBAL + CENTERED
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sm_global_to_rel_x(sprite->x + an->offsetx), neww), sm_get_y(sm_global_to_rel_y(sprite->y + an->offsety), newh), neww, newh, sprite->alflags);
			break;
		
		case 23://LOCAL + GLOBAL + CENTERED
			sm_error_local_and_global(sprite);
			break;

		case 24://NOZOOM
			sm_error_local_nor_global(sprite);
			break;

		case 25://LOCAL + NOZOOM
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), an->width, an->height, sprite->alflags);
			break;

		case 26://GLOBAL + NOZOOM
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), an->width, an->height, sprite->alflags);
			break;

		case 27://LOCAL + GLOBAL + NOZOOM
			sm_error_local_and_global(sprite);
			break;

		case 28://CENTERED + NOZOOM
			sm_error_local_nor_global(sprite);
			break;

		case 29://LOCAL + CENTERED + NOZOOM
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sprite->x, an->width), sm_get_y(sprite->y, an->height), an->width, an->height, sprite->alflags);
			break;

		case 30://GLOBAL + CENTERED + NOZOOM
			al_draw_scaled_bitmap(spritesheet, an->x + sprite->cycle * an->width, an->y, an->width, an->height, sm_get_x(sm_global_to_rel_x(sprite->x), an->width), sm_get_y(sm_global_to_rel_y(sprite->y), an->height), an->width, an->height, sprite->alflags);
			break;

		case 31://LOCAL + GLOBAL + CENTERED + NOZOOM
			sm_error_local_and_global(sprite);
			break;
	}
}

float sm_get_x(float x, float bitmapwidth)
{
	return x + HWIDTH - bitmapwidth / 2;
}

float sm_get_y(float y, float bitmapheight)
{
	return HHEIGHT - y - bitmapheight / 2;
}

float sm_global_to_rel_x(float x)
{
	return (x - coord[X]) * zoom;
}

float sm_global_to_rel_y(float y)
{
	return (y - coord[Y]) * zoom;
}

float sm_rel_to_global_x(float x)
{
	return x / zoom + coord[X];
}

float sm_rel_to_global_y(float y)
{
	return y / zoom + coord[Y];
}

void sm_move_coord(float dx, float dy)
{
	coord[X] += dx;
	coord[Y] += dy;
}

void sm_set_coord(float x, float y)
{
	coord[X] = x;
	coord[Y] = y;
}

float sm_get_coord(int i)
{
	return coord[i];
}

void sm_set_zoom(int z)
{
	zoom += ZOOMINC * z;
	if(!debug_get())
	{
		if(zoom > MAXZOOM)
			zoom = MAXZOOM;
		else if(zoom < MINZOOM)
			zoom = MINZOOM;
	}
}

float sm_get_zoom()
{
	return zoom;
}

struct sprite *sm_create_global_static_sprite(ALLEGRO_BITMAP *bitmap, float x, float y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_global_static_sprite");

	out->bitmap = bitmap;
	out->name = NULL;
	out->type = GLOBAL | typeflags;
	out->x = x;
	out->y = y;
	out->layer = layer;
	out->node = NULL;
	out->rot = 0;
	out->rotoffset = 0;

	return out;
}

struct sprite *sm_create_global_dynamic_sprite(struct animation *an, float x, float y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_global_dynamic_sprite");

	out->name = NULL;
	out->type = GLOBAL | DYNAMIC | typeflags;
	out->x = x;
	out->y = y;
	out->layer = layer;
	out->an = an;
	out->i = 0;
	out->node = NULL;
	out->cycle = 0;
	out->alflags = 0;

	return out;
}

struct sprite *sm_create_global_sprite(ALLEGRO_BITMAP *bitmap, float x, float y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_global_sprite");

	out->name = NULL;
	out->type = GLOBAL | typeflags;
	out->bitmap = bitmap;
	out->x = x;
	out->y = y;
	out->node = NULL;
	out->rot = 0;
	out->rotoffset = 0;
	out->layer = layer;

	return out;
}

struct sprite *sm_create_sprite(ALLEGRO_BITMAP *bitmap, float x, float y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_sprite");

	out->name = NULL;
	out->type = LOCAL | typeflags;
	out->bitmap = bitmap;
	out->x = x;
	out->y = y;
	out->node = NULL;
	out->rot = 0;
	out->rotoffset = 0;
	out->layer = layer;

	return out;
}

ALLEGRO_BITMAP *sm_get_sub_bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	return al_create_sub_bitmap(spritesheet, x, y, width, height);
}

void sm_destroy_sprite_from_layer(struct sprite *sprite)
{
	sm_destroy_sprite(sprite);
}

void sm_remove_sprite_from_layer(struct sprite *sprite)
{
	if(!sprite->node || !layers)
		return;
	list_delete_node(list_get(layers, sprite->layer), sprite->node);
	sprite->node = NULL;
	nosprites--;
}

void sm_destroy_sprite(struct sprite *sprite)
{
	if(!sprite)
		return;

	if(!(sprite->type & DYNAMIC))
	{
		al_destroy_bitmap(sprite->bitmap);
		sm_remove_sprite_from_layer(sprite);
	}
	
	if(sprite->name)
	{
		debug_printf("Free sprite %s\n", sprite->name);
		s_free(sprite->name, NULL);
	}

	s_free(sprite, NULL);
}

int sm_get_sprite_count()
{
	return nosprites;
}

void sm_destroy()
{
	int i;
	struct node *layer = layers->head;
	for(i = 0; i < layers->size; i++)
	{
		struct list *l = layer->p;
		list_destroy(l);
		layer->p = NULL;
		layer = layer->next;
		al_destroy_bitmap(deferredlayers[i]);
	}
	list_destroy(layers);
	layers = NULL;
	al_destroy_bitmap(spritesheet);
}