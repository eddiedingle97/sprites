#include <allegro5/allegro.h>
#include <stdio.h>
#include <math.h>
#include "sprites.h"
#include "list/list.h"
#include "spritemanager.h"
#include "colors.h"
#include "enums.h"
#include "debug.h"

#define addlayer list_append(layers, list_create())

static struct list *layers;

static const int HHEIGHT = HEIGHT / 2;
static const int HWIDTH = WIDTH / 2;
static const float MAXZOOM = 4;
static const float MINZOOM = 1;

static int nosprites = 0;
static int coord[2];
static char move[LAYERS][2];
static float zoom = MAXZOOM;
static float zoominc = .1;

void sm_init(int x, int y)
{
	layers = list_create();

	coord[X] = x;
	coord[Y] = y;

	int i;
	for(i = 0; i < LAYERS; i++)
	{
		addlayer;
		move[i][0] = 0;
		move[i][1] = 0;
	}
}

void sm_add_sprite_to_layer(struct sprite *sprite)
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
	struct list *l = (struct list *)list_get(layers, layer);
	list_append(l, (void *)sprite);
	sprite->id = l->tail;
	nosprites++;
}

void sm_draw_sprites(ALLEGRO_DISPLAY *display)
{
	al_set_target_bitmap(al_get_backbuffer(display));

	int i, j;
	struct list *layer;
	struct node *node;
	struct sprite *sprite;

	/*layer = list_get(layers, layers->size - 1);
	node = layer->head;
	
	for(j = 0; j < layer->size; j++)
	{
		sprite = (struct sprite *)node->p;
		int w = al_get_bitmap_width(sprite->bitmap);
		int h = al_get_bitmap_height(sprite->bitmap);
		int neww = w * zoom;
		int newh = h * zoom;

		switch(sprite->type)
		{
			case 1://LOCAL
				al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sprite->x * zoom, 0), sm_get_y(sprite->y * zoom, 0), neww, newh, 0);
				break;
			
			case 2://GLOBAL
				al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), neww, newh, 0);
				break;

			case 3://LOCAL + GLOBAL
				if(!sprite->name)
					fprintf(stderr, "Error: sprite cannot be both local and global\n");
				else
					fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
				break;

			case 4:
				if(!sprite->name)
					fprintf(stderr, "Error: sprite must be either local or global\n");
				else
					fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
				break;

			case 5://LOCAL + CENTERED
				al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sprite->x * zoom, w), sm_get_y(sprite->y * zoom, h), neww, newh, 0);
				break;

			case 6://GLOBAL + CENTERED
				al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), neww), sm_get_y(sm_global_to_rel_y(sprite->y), newh), neww, newh, 0);
				break;
			
			case 7://LOCAL + GLOBAL + CENTERED
				if(!sprite->name)
					fprintf(stderr, "Error: sprite cannot be both local and global\n");
				else
					fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
				break;
		}

		node = node->next;
	}*/

	for(i = layers->size - 1; i >= 0; i--)
	{
		layer = list_get(layers, i);
		node = layer->head;
		//printf("layer size: %d\n", layer->size);
		for(j = 0; j < layer->size; j++)
		{
			sprite = (struct sprite *)node->p;
			/*sprite->x += move[i][X];
			sprite->y += move[i][Y];*/
			int w = al_get_bitmap_width(sprite->bitmap);
			int h = al_get_bitmap_height(sprite->bitmap);
			int neww = w * zoom;
			int newh = h * zoom;

			switch(sprite->type)
			{
				case 1://LOCAL
					al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sprite->x * zoom, 0), sm_get_y(sprite->y * zoom, 0), neww, newh, 0);
					break;
				
				case 2://GLOBAL
					al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), neww, newh, 0);
					break;

				case 3://LOCAL + GLOBAL
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 4:
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;

				case 5://LOCAL + CENTERED
					al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sprite->x * zoom, w), sm_get_y(sprite->y * zoom, h), neww, newh, 0);
					break;

				case 6://GLOBAL + CENTERED
					al_draw_scaled_bitmap(sprite->bitmap, 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), w), sm_get_y(sm_global_to_rel_y(sprite->y), h), neww, newh, 0);
					break;
				
				case 7://LOCAL + GLOBAL + CENTERED
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 8://NOZOOM
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;

				case 9://LOCAL + NOZOOM
					
					al_draw_bitmap(sprite->bitmap, sm_get_x(sprite->x, 0), sm_get_y(sprite->y, 0), 0);
					break;

				case 10://GLOBAL + NOZOOM
					al_draw_bitmap(sprite->bitmap, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), 0);
					break;

				case 11://LOCAL + GLOBAL + NOZOOM
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 12://CENTERED + NOZOOM
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;

				case 13://LOCAL + CENTERED + NOZOOM
					al_draw_bitmap(sprite->bitmap, sm_get_x(sprite->x, w), sm_get_y(sprite->y, h), 0);
					break;

				case 14://GLOBAL + CENTERED + NOZOOM
					al_draw_bitmap(sprite->bitmap, sm_get_x(sm_global_to_rel_x(sprite->x), w), sm_get_y(sm_global_to_rel_y(sprite->y), h), 0);
					break;

				case 15://LOCAL + GLOBAL + CENTERED + NOZOOM
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 16://TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;

				case 17://LOCAL + TRANSPARENT
					al_draw_tinted_scaled_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), 0, 0, w, h, sm_get_x(sprite->x * zoom, 0), sm_get_y(sprite->y * zoom, 0), neww, newh, 0);
					break;

				case 18://GLOBAL + TRANSPARENT
					if(sprite->name)
						printf("%s\n", sprite->name);
					al_draw_tinted_scaled_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), neww, newh, 0);
					break;

				case 19://LOCAL + GLOBAL + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;
				
				case 20://CENTERED + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;
				
				case 21://LOCAL + CENTERED + TRANSPARENT
					al_draw_tinted_scaled_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), 0, 0, w, h, sm_get_x(sprite->x * zoom, w), sm_get_y(sprite->y * zoom, h), neww, newh, 0);
					break;
				
				case 22://GLOBAL + CENTERED + TRANSPARENT
					al_draw_tinted_scaled_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), 0, 0, w, h, sm_get_x(sm_global_to_rel_x(sprite->x), w), sm_get_y(sm_global_to_rel_y(sprite->y), h), neww, newh, 0);
					break;
				
				case 23://LOCAL + GLOBAL + CENTERED + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 24://NOZOOM + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;
				
				case 25://LOCAL + NOZOOM + TRANSPARENT
					al_draw_tinted_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), sm_get_x(sprite->x, 0), sm_get_y(sprite->y * zoom, 0), 0);
					break;

				case 26://GLOBAL + NOZOOM + TRANSPARENT
					al_draw_tinted_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), sm_get_x(sm_global_to_rel_x(sprite->x), 0), sm_get_y(sm_global_to_rel_y(sprite->y), 0), 0);
					break;

				case 27://LOCAL + GLOBAL + NOZOOM + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;

				case 28://CENTERED + NOZOOM + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite must be either local or global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" must be either local or global\n", sprite->name);
					break;
				
				case 29://LOCAL + CENTERED + NOZOOM + TRANSPARENT
					al_draw_tinted_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), sm_get_x(sprite->x, w), sm_get_y(sprite->y * zoom, h), 0);
					break;

				case 30://GLOBAL + CENTERED + NOZOOM + TRANSPARENT
					al_draw_tinted_bitmap(sprite->bitmap, al_map_rgba_f(1, 1, 1, .5), sm_get_x(sm_global_to_rel_x(sprite->x), w), sm_get_y(sm_global_to_rel_y(sprite->y), h), 0);
					break;

				case 31://LOCAL + GLOBAL + CENTERED + NOZOOM + TRANSPARENT
					if(!sprite->name)
						fprintf(stderr, "Error: sprite cannot be both local and global\n");
					else
						fprintf(stderr, "Error: sprite \"%s\" cannot be both local and global\n", sprite->name);
					break;
			}

			node = node->next;
		}
		move[i][X] = 0;
		move[i][Y] = 0;
	}

	/*layer = list_get(layers, 0);
	node = layer->head;
	for(j = 0; j < layer->size; j++)
	{
		sprite = (struct sprite *)node->p;
		al_draw_bitmap(sprite->bitmap, sm_get_x(sprite->x, al_get_bitmap_width(sprite->bitmap)), sm_get_y(sprite->y, al_get_bitmap_height(sprite->bitmap)), 0);

		node = node->next;
	}*/
}

int sm_get_x(int x, int bitmapwidth)
{
	return x + HWIDTH - bitmapwidth / 2;
}

int sm_get_y(int y, int bitmapheight)
{
	return HHEIGHT - y - bitmapheight / 2;
}

int sm_global_to_rel_x(int x)
{
	return x * zoom - coord[X] * zoom;
}

int sm_global_to_rel_y(int y)
{
	return y * zoom - coord[Y] * zoom;
}

int sm_rel_to_global_x(int x)
{
	return x / zoom + coord[X];
}

int sm_rel_to_global_y(int y)
{
	return y / zoom + coord[Y];
}

void sm_move_coord(unsigned char up, unsigned char down, unsigned char left, unsigned char right)
{
	coord[X] += right - left;
	coord[Y] += up - down;
}

int sm_get_coord(int i)
{
	return coord[i];
}

void sm_set_zoom(int z)
{
	zoom += zoominc * z;
	if(zoom > MAXZOOM)
		zoom = MAXZOOM;
	else if(zoom < MINZOOM)
		zoom = MINZOOM;
}

float sm_get_zoom()
{
	return zoom;
}

struct sprite *sm_create_global_sprite(ALLEGRO_BITMAP *bitmap, int x, int y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_global_sprite");

	out->name = NULL;
	out->type = GLOBAL | typeflags;
	out->bitmap = bitmap;
	out->x = x;
	out->y = y;
	out->id = NULL;
	out->layer = layer;

	return out;
}

struct sprite *sm_create_sprite(ALLEGRO_BITMAP *bitmap, int x, int y, int layer, int typeflags)
{
	struct sprite *out = s_malloc(sizeof(struct sprite), "sm_create_sprite");

	out->name = NULL;
	out->type = LOCAL | typeflags;
	out->bitmap = bitmap;
	out->x = x;
	out->y = y;
	out->id = NULL;
	out->layer = layer;

	return out;
}

void sm_update_sprite(struct sprite *sprite, ALLEGRO_BITMAP *bitmap, int x, int y)
{
	al_destroy_bitmap(sprite->bitmap);
	sprite->bitmap = bitmap;
	sprite->x = x;
	sprite->y = y;
}

void sm_destroy_sprite_from_layer(struct sprite *sprite)
{
	sm_destroy_sprite(sprite);
}

void sm_remove_sprite_from_layer(struct sprite *sprite)
{
	if(!sprite->id)
		return;
	list_delete_node((struct list *)list_get(layers, sprite->layer), sprite->id);
	sprite->id = NULL;
	nosprites--;
}

void sm_destroy_sprite(struct sprite *sprite)
{
	if(!sprite)
		return;

	if(sprite->id)
		sm_remove_sprite_from_layer(sprite);
	else
		nosprites--;

	al_destroy_bitmap(sprite->bitmap);

	free(sprite);
}

int sm_get_sprite_count()
{
	return nosprites;
}

void sm_destroy()
{
	int i, j;
	struct node *layer = layers->head;
	for(i = 0; i < layers->size; i++)
	{
		struct list *l = (struct list *)layer->p;
		struct node *node = l->head;
		for(j = 0; j < l->size; j++)
		{
			sm_destroy_sprite((struct sprite *)node->p);
			node = node->next;
		}
		list_destroy(l);
		layer = layer->next;
	}
	list_destroy(layers);
}