#ifndef __ACTION_H__
#define __ACTION_H__
#include "entity.h"

struct action
{
	unsigned short actionid;
	unsigned short done;
	void *data;
};

struct swingdata
{
	float angle;
	int ccw;
};

void action_init_swing(struct entity *e, float angle, int ccw);
void action_swing(struct entity *e);


void action_destroy(struct entity *e);

#endif