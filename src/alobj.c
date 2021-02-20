#include <allegro5/allegro.h>
#include <stdio.h>
#include "alobj.h"

struct alobj *alobj_create(int width, int height, double timer)
{
	if(width < 1 || height < 1)
		return NULL;

	al_set_app_name("sprites :)");
	al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR);

	struct alobj *obj = malloc(sizeof(struct alobj));
	obj->display = al_create_display(width, height);
	al_set_window_position(obj->display, 0, 0);

	if(!obj->display)
	{	
		fprintf(stderr, "failed to create display\n");
		free(obj);

		return NULL;
	}

	obj->event_queue = al_create_event_queue();

	if(!obj->event_queue)
	{
		fprintf(stderr, "failed to create event queue\n");
		al_destroy_display(obj->display);
		free(obj);

		return NULL;
	}

	obj->timer = al_create_timer(timer);

	if(!obj->timer)
	{
		fprintf(stderr, "failed to create timer\n");
		al_destroy_display(obj->display);
		al_destroy_event_queue(obj->event_queue);
		free(obj);

		return NULL;
	}

	return obj;
}

void alobj_destroy(struct alobj *obj)
{
	al_destroy_display(obj->display);
	al_destroy_event_queue(obj->event_queue);
	al_destroy_timer(obj->timer);
	free(obj);
}	
