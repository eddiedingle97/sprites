#include <stdio.h>
#include <allegro5/allegro.h>
#include "list.h"
#include "debug.h"

struct list *threads = NULL;

void tm_init()
{
	threads = list_create();
}

void tm_destroy()
{
	list_destroy(threads);
}

int tm_queue_thread(void *(*proc)(ALLEGRO_THREAD *thread, void *arg), void *arg)
{
	ALLEGRO_THREAD *newthread = al_create_thread(proc, arg);
	if(!newthread)
	{
		if(debug_get())
			debug_perror("failed to create thread");

		return -1;
	}

	list_append(threads, newthread);
	al_start_thread(newthread);

	return threads->size - 1;
}