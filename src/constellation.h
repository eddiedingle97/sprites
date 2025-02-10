#ifndef __CONSTELLATION_H__
#define __CONSTELLATION_H__
#include "spritemanager.h"

struct sky
{
	struct sprite *canvas;
	int *starx;
	int *stary;
	char *starflags;
	int nostars;
};

struct sky *cons_create_sky();
void cons_gen_sky(struct sky *sky);
void cons_destroy_sky(struct sky *sky);

#endif