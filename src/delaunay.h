#ifndef __DELAUNAY_H__
#define __DELAUNAY_H__
#include "graph.h"
#include <allegro5/allegro.h>

struct delaunaydata
{
	struct graph *graph;
	int(*get_x)(void *); 
	int(*get_y)(void *);
	ALLEGRO_MUTEX *mutex;
	ALLEGRO_COND *cond;
};

void delaunay_triangulation(struct graph *, int(*)(void *), int(*)(void *));
void delaunay_triangulation_debug(ALLEGRO_THREAD *thread, struct delaunaydata *data);

#endif