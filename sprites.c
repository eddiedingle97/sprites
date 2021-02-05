#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "sprites.h"
#include "keyboard.h"
#include "alobj.h"
#include "colors.h"
#include "list/list.h"
#include "spritemanager.h"
#include "mouse.h"
#include "game.h"
#include "map.h"
#include "mapmanager.h"
#include "debug.h"
#include "menu.h"

static const float FPS = 1/60.0;
static char debug;
static unsigned long bytes = 0;
static char rootdir[512];
static char buf[512];

int main(int argc, char **argv)
{
	char opt, mode = 0, newmap = 0;
	int width = 0, height = 0;

	printf("%ld\n", sizeof(struct tile));

	memset(rootdir, 0, 512);
	getcwd(rootdir, 512);

	while((opt = getopt(argc, argv, "dmn:")) != -1)
        switch(opt)
        {
            case 'd':
                debug = 1;
                break;

			case 'm':
				mode = 2;
				break;

			case 'n':
				newmap = 1;
				width = atoi(strtok(optarg, ","));
				height = atoi(strtok(NULL, ","));
				break;
        }

	if(!al_init())
	{
		fprintf(stderr, "al_init() failed\n");
		exit(1);
	}

	struct alobj *al = alobj_create(WIDTH, HEIGHT, FPS);

	if(!al)
		exit(1);

	if(!al_install_keyboard())
	{
		fprintf(stderr, "keyboard install failure\n");
		alobj_destroy(al);
		exit(1);
	}

	if(!al_install_mouse())
	{
		fprintf(stderr, "mouse install failure\n");
		alobj_destroy(al);
		al_uninstall_keyboard();
		exit(1);
	}
	
	if(!al_init_image_addon())
	{
		fprintf(stderr, "init image add on failed\n");
		alobj_destroy(al);
		al_uninstall_keyboard();
		al_uninstall_mouse();
		exit(1);
	}

	if(!al_init_font_addon())
	{
		fprintf(stderr, "init font add on failed\n");
		alobj_destroy(al);
		al_uninstall_keyboard();
		al_uninstall_mouse();
		al_shutdown_image_addon();
		exit(1);
	}

	if(!al_init_ttf_addon())
	{
		fprintf(stderr, "init ttf add on failed\n");
		alobj_destroy(al);
		al_uninstall_keyboard();
		al_uninstall_mouse();
		al_shutdown_image_addon();
		al_shutdown_font_addon();
		exit(1);
	}

	al_register_event_source(al->event_queue, al_get_display_event_source(al->display));
	al_register_event_source(al->event_queue, al_get_timer_event_source(al->timer));
	al_register_event_source(al->event_queue, al_get_keyboard_event_source());
	al_register_event_source(al->event_queue, al_get_mouse_event_source());

	al_set_target_backbuffer(al->display);
	al_set_target_bitmap(al_get_backbuffer(al->display));
	
	al_clear_to_color(BLACK);
	al_flip_display();

	al_set_mouse_xy(al->display, WIDTH / 2, HEIGHT / 2);

	ALLEGRO_EVENT ev;
	ALLEGRO_MOUSE_STATE mousestate;

	char done = 0, redraw = 0;

	kb_init();
	mouse_init(&mousestate);
	game_init(mode, newmap, width, height);
	debug_init(debug);

	clock_t start, stop;

	al_start_timer(al->timer);

	while(!done)
	{
		al_wait_for_event(al->event_queue, &ev);
		
		if(ev.type == ALLEGRO_EVENT_TIMER)
			redraw = 1;

		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			done = 1;

		al_get_mouse_state(&mousestate);

		kb_update(&ev);

		if(kb_get_pause())
			done = 1;

		if(redraw && al_is_event_queue_empty(al->event_queue))
		{
			start = clock();

			game_tick(al->display);
			mouse_draw(al->display);
			kb_tick();

			al_flip_display();
			redraw = 0;

			stop = clock();
			debug_tick(stop - start);
		}
	}

	game_destroy();
	if(debug)
		printf("after game_destroy\n");

	mouse_destroy();
	if(debug)
		printf("after mouse_destroy\n");

	debug_destroy();
	if(debug)
		printf("after debug\n");

	al_uninstall_keyboard();
	if(debug)
		printf("after keyboard\n");

	al_uninstall_mouse();
	if(debug)
		printf("after mouse\n");

	al_shutdown_image_addon();
	if(debug)
		printf("after image_addon\n");

	al_shutdown_ttf_addon();
	if(debug)
		printf("after ttf_addon\n");

	al_shutdown_font_addon();
	if(debug)
		printf("after font_addon\n");

	alobj_destroy(al);
	if(debug)
	{
		printf("after alobj_destroy\n");
		printf("%ld bytes allocated\n", bytes);
	}

	return 0;
}

void *s_malloc(int b, const char *msg)
{
	void *out = malloc(b);
	if(!out)
	{
		fprintf(stderr, "Error allocating memory, shutting down\n");
		exit(1);
	}
	if(debug && msg)
	{
		puts(msg);
		bytes += b;
	}

	return out;
}

void *s_realloc(void *ptr, int b, const char *msg)
{
	void *out = realloc(ptr, b);
	if(!out)
	{
		fprintf(stderr, "Error reallocating memory, shutting down\n");
		exit(1);
	}
	if(debug && msg)
	{
		puts(msg);
		bytes += b;
	}
	
	return out;
}

char *s_get_heap_string(const char *str)
{
    return memcpy(s_malloc(sizeof(char) * (strlen(str) + 1), "s_get_heap_string"), str, strlen(str) + 1);
}

char s_string_match(char *one, char *two)
{
	return strlen(one) == strlen(two) && !strcmp(one, two);
}

char *s_get_root_dir()
{
	return rootdir;
}

char *s_get_full_path(char *file)
{
	memset(buf, 0, 512);
	if(strlen(rootdir) + strlen(file) > 510)
	{
		fprintf(stderr, "s_get_full_path argument too long, returning null\n");
		return NULL;
	}
	memcpy(buf, rootdir, strlen(rootdir));
	strcat(buf, "/");
	return strcat(buf, file);
}

char *s_get_full_path_with_dir(char *dir, char *file)
{
	memset(buf, 0, 512);
	if(strlen(rootdir) + strlen(dir) + strlen(file) > 509)
	{
		fprintf(stderr, "s_get_full_path_with_dir arguments too long, returning null\n");
		return NULL;
	}
	memcpy(buf, rootdir, strlen(rootdir));
	strcat(buf, "/");
	strcat(buf, dir);
	strcat(buf, "/");
	return strcat(buf, file);
}