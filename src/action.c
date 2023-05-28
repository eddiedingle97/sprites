#include <math.h>
#include <stdio.h>
#include "action.h"
#include "emath.h"
#include "sprites.h"
#include "entity.h"
#include "entitymanager.h"

void action_init_throw(struct entity *e, struct entity *throwe, float dx, float dy, float speed)//maybe do more with this later... quick and simple for now
{
	float speednorm = math_get_distance(dx, dy);
	throwe->speedx = dx * speed / speednorm;
	throwe->speedy = dy * speed / speednorm;
	throwe->sprite->x = e->sprite->x + 4 * throwe->speedx;
	throwe->sprite->y = e->sprite->y + 4 * throwe->speedy;
	throwe->flags |= AIRBORNE;
	throwe->z = 5;
	throwe->speedz = 5;
}

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
	sd->ticks = 15;//FIX: FPS dependent
	sd->tickcount = 0;
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
	if(sd->tickcount < sd->ticks)
	{
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
	else
	{
		if(e->hand->angvel == 0)
		{
			e->actions->done = 1;
			e->noactions--;
		}
	}

	sd->tickcount++;
}

void action_destroy(struct entity *e)
{
	s_free(e->actions->data, NULL);
	s_free(e->actions, "Freed action");
	e->actions = NULL;
}