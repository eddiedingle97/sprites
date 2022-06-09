#include <math.h>
#include "action.h"
#include "emath.h"
#include "sprites.h"
#include "entity.h"

void action_init_swing(struct entity *e, float angle, int ccw)
{
	float holddist = math_get_distance(e->hand->holdx, e->hand->holdy);
	e->hand->sprite->rot = angle + (ccw ? -1.0f : 1.0f) * M_PI_2;
	e->hand->sprite->y = math_sin(e->hand->sprite->rot) * (e->colrad + holddist) + e->sprite->y;
	e->hand->sprite->x = math_cos(e->hand->sprite->rot) * (e->colrad + holddist) + e->sprite->x;
	struct swingdata *sd = s_malloc(sizeof(struct swingdata), "action_init_swing: sd");
	struct action *a = s_malloc(sizeof(struct action), "action_init_swing: a");
	sd->angle = angle;
	sd->ccw = ccw;
	a->data = sd;
	a->done = 0;
	a->actionid = 0;
	e->actions = a;
	e->noactions++;
}

void action_swing(struct entity *e)
{
	struct swingdata *sd = e->actions->data;
	float holddist = math_get_distance(e->hand->holdx, e->hand->holdy);
	float r = holddist + e->colrad;
	r /= 6;
	if(sd->ccw)
	{
		if(e->hand->sprite->rot < sd->angle + M_PI_2)
		{
			e->hand->angvel += e->strength / (e->hand->weight * r * r);
		}
		else if(e->hand->sprite->rot > sd->angle + M_PI_2 && e->hand->angvel > 0)
		{
			e->hand->angvel -= e->strength / (e->hand->weight * r * r);
		}
		else
		{
			e->actions->done = 1;
			e->noactions--;
		}
	}
	else
	{
		if(e->hand->sprite->rot > sd->angle - M_PI_2)
		{
			e->hand->angvel -= e->strength / (e->hand->weight * r * r);
		}
		else if(e->hand->sprite->rot < sd->angle - M_PI_2 && e->hand->angvel < 0)
		{
			e->hand->angvel += e->strength / (e->hand->weight * r * r);
		}
		else
		{
			e->actions->done = 1;
			e->noactions--;
		}
	}
}

void action_destroy(struct entity *e)
{
	s_free(e->actions->data, NULL);
	s_free(e->actions, "Freed action");
	e->actions = NULL;
}