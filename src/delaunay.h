#ifndef __DELAUNAY_H__
#define __DELAUNAY_H__
#include "graph.h"

void delaunay_triangulation(struct graph *, int(*)(void *), int(*)(void *));

#endif