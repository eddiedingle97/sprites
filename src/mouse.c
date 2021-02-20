#include <allegro5/allegro.h>
#include <stdio.h>
#include "mouse.h"
#include "sprites.h"
#include "colors.h"
#include "list.h"
#include "spritemanager.h"

static ALLEGRO_BITMAP *mo;
static ALLEGRO_MOUSE_STATE *state;
static unsigned char mousewidth, mouseheight;
static char scrollval = 0;
static char oneprev = 0;
static char twoprev = 0;

void mouse_init(ALLEGRO_MOUSE_STATE *mousestate)
{
	state = mousestate;
	mo = NULL;
	mousewidth = 0;
	mouseheight = 0;
}

void mouse_init_with_image(ALLEGRO_MOUSE_STATE *mousestate, const char *filepath)
{
	state = mousestate;
	mo = al_load_bitmap(filepath);
	if(!mo)
		fprintf(stderr, "load bitmap from file failed in mouse.c\n");
	
	mousewidth = al_get_bitmap_width(mo);
	mouseheight = al_get_bitmap_height(mo);
}

void mouse_set_bitmap(ALLEGRO_BITMAP *bitmap)
{
	if(mo)
		al_destroy_bitmap(mo);

	mo = bitmap;
	mousewidth = al_get_bitmap_width(mo);
	mouseheight = al_get_bitmap_height(mo);
}

int mouse_get_scroll()
{
	int temp = scrollval;
	scrollval = state->z;
	return state->z - temp;
}

int mouse_get_rel_x()
{
	return state->x - WIDTH / 2;
}

int mouse_get_rel_y()
{
	return -state->y + HEIGHT / 2;
}

char mouse_get_single_one()
{
	return state->buttons & 1 && !oneprev;
}

char mouse_get_one()
{
	return state->buttons & 1;
}

char mouse_get_single_two()
{
	return state->buttons & 2 && !twoprev;
}

char mouse_get_two()
{
	return state->buttons & 2;
}

char mouse_get_three()
{
	return state->buttons & 4;
}

void mouse_draw(ALLEGRO_DISPLAY *display)
{
	if(mo)
	{
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_bitmap(mo, state->x - (mousewidth / 2), state->y - (mouseheight / 2), 0);
	}
	
	oneprev = state->buttons & 1;
	twoprev = state->buttons & 2;
}

void mouse_destroy()
{
	if(mo)
		al_destroy_bitmap(mo);
	mo = NULL;
}