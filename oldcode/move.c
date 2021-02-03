#include <allegro5/allegro.h>
#include "list/list.h"
#include "move.h"
#include "colors.h"

const int BOUNDARY = 450;

struct list *layers; //create list of lists here, for layering

struct sprite sp;
struct sprite back;

//struct sprite *focus;

void init_sprite(int width, int height, int x, int y)
{
	sp.bitmap = al_create_bitmap(width, height);
	al_set_target_bitmap(sp.bitmap);
	al_clear_to_color(WHITE);
	sp.y = y - (height / 2);
	sp.x = x - (width / 2);

	int backx = (x << 1) + x;
	int backy = (y << 1) + y;

	back.bitmap = al_create_bitmap(backx, backy);
	al_set_target_bitmap(back.bitmap);
	al_lock_bitmap(back.bitmap, 0, 0);
	int i, j;
	for(i = 0; i < backx; i++)
	{
		for(j = 0; j < backy; j++)
		{
			if(j < backy / 3)
				al_put_pixel(i, j, GREEN);
			else if(j < 2 * backy / 3)
				al_put_pixel(i, j, BLUE);
			else 
				al_put_pixel(i, j, RED);
			if(j == 0 || j == backy - 1)
				al_put_pixel(i, j, YELLOW);
			if(i == 0 || i == backx - 1)
				al_put_pixel(i, j, YELLOW);
		}
	}
	al_unlock_bitmap(back.bitmap);
	back.x = -(x >> 1);
	back.y = -(y >> 1);
}

void draw_sprite(ALLEGRO_DISPLAY *display, unsigned char w, unsigned char a, unsigned char s, unsigned char d)
{
	if(!(sp.x > (al_get_bitmap_width(al_get_backbuffer(display))) - BOUNDARY && d - a == 1) && !(sp.x < BOUNDARY && d - a == -1))
		sp.x += (d - a) << 2;
	else
		back.x += (a - d) << 2;

	if(!(sp.y > (al_get_bitmap_height(al_get_backbuffer(display))) - BOUNDARY && s - w == 1) && !(sp.y < BOUNDARY && s - w == -1))
		sp.y += (s - w) << 2;
	else
		back.y += (w - s) << 2;
	
		
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BLACK);
	al_draw_bitmap(back.bitmap, back.x, back.y, 0);	
	al_draw_bitmap(sp.bitmap, sp.x, sp.y, 0);
}

void destroy_sprite()
{
	al_destroy_bitmap(sp.bitmap);
}
