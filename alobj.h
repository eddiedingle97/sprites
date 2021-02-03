#include <allegro5/allegro.h>

#ifndef __ALOBJ_H__
#define __ALOBJ_H__

struct alobj
{
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;
};

struct alobj * alobj_create(int width, int height, double timer);
void alobj_destroy(struct alobj *obj);

#endif
