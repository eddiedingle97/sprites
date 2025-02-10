#include "sprites.h"
#include "constellation.h"
#include "spritemanager.h"
#include "emath.h"
#include "colors.h"

struct sky *cons_create_sky()
{
	struct sky *out = s_malloc(sizeof(struct sky), NULL);
	out->canvas = sm_create_sprite(al_create_bitmap(2 * WIDTH, 2 * HEIGHT), 0, 0, FOREGROUND, CENTERED | NOZOOM);
	out->starx = NULL;
	out->stary = NULL;
	out->starflags = NULL;
	out->nostars = 0;

	return out;
}

void cons_gen_sky(struct sky *sky)
{
	sky->nostars = 100;
	sky->starx = s_malloc(sizeof(int) * sky->nostars, NULL);
	sky->stary = s_malloc(sizeof(int) * sky->nostars, NULL);
	sky->starflags = s_malloc(sizeof(char) * sky->nostars, NULL);
	
	math_seed(0);
	int i;
	al_set_target_bitmap(sky->canvas->bitmap);
	al_clear_to_color(BLACK);
	for(i = 0; i < sky->nostars; i++)
	{
		sky->starx[i] = math_rand() % WIDTH;
		sky->stary[i] = math_rand() % HEIGHT;
		sky->starflags[i] = 0;

		al_put_pixel(sky->starx[i] + WIDTH / 2, sky->stary[i] + HEIGHT / 2, WHITE);
	}
}

void cons_destroy_sky(struct sky *sky)
{
	sm_destroy_sprite(sky->canvas);
	s_free(sky->starx, NULL);
	s_free(sky->stary, NULL);
	s_free(sky->starflags, NULL);
	s_free(sky, NULL);
}