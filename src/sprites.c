#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
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
#include "list.h"
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
static char *rootdir;
static char buf[512];

int main(int argc, char **argv)
{
	char opt, mode = 0, newmap = 0;
	int width = 0, height = 0;

	rootdir = al_get_current_directory();

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
	debug_init(debug);
	game_init(mode, newmap, width, height);
	debug_add_sprites();
	

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
	debug_printf("after game_destroy\n");

	mouse_destroy();
	debug_printf("after mouse_destroy\n");

	debug_destroy();
	debug_printf("after debug_destroy\n");

	al_uninstall_keyboard();
	debug_printf("after al_uninstall_keyboard\n");

	al_uninstall_mouse();
	debug_printf("after al_uninstall_mouse\n");

	al_shutdown_image_addon();
	debug_printf("after al_shutdown_image_addon\n");

	al_shutdown_ttf_addon();
	debug_printf("after al_shutdown_ttf_addon\n");

	al_shutdown_font_addon();
	debug_printf("after al_shutdown_font_addon\n");

	alobj_destroy(al);
	debug_printf("after alobj_destroy\n");
	debug_printf("%ld bytes allocated\n", bytes);

	al_free(rootdir);

	return 0;
}

void *s_malloc(int b, const char *msg)
{
	void *out = malloc(b);
	if(!out)
	{
		debug_perror("Error allocating %d bytes of memory, shutting down\n", b);
		exit(1);
	}
	if(debug)
	{
		if(msg)
			printf("%s: %d\n", msg, b);
		bytes += b;
	}

	return out;
}

void *s_realloc(void *ptr, int b, const char *msg)
{
	void *out = realloc(ptr, b);
	if(!out)
	{
		debug_perror("Error reallocating %d bytes memory, shutting down\n", b);
		exit(1);
	}
	if(debug)
	{
		if(msg)
			printf("%s: %d\n", msg, b);
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
		debug_perror("s_get_full_path argument too long, returning null\n");
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
		debug_perror("s_get_full_path_with_dir arguments too long, returning null\n");
		return NULL;
	}
	memcpy(buf, rootdir, strlen(rootdir));
	strcat(buf, "/");
	strcat(buf, dir);
	strcat(buf, "/");
	return strcat(buf, file);
}

struct list *s_get_file_list_from_dir(char *dir)
{
	DIR *dr = opendir(s_get_full_path(dir));
    struct dirent *de;
	struct list *out = list_create();

    if(dr)
    {
        while((de = readdir(dr)) != NULL)
        {
            if(*de->d_name != '.')
            {
				list_append(out, s_get_heap_string(de->d_name));
            }
        }
        closedir(dr);
    }

	return out;
}

int s_round(float f)
{
	int out = (int)f;
	float cmp = f - out;
	if(cmp < .5)
		return out;
	else
		return ++out;
}

int s_floor(float f)
{
	return (int)f;
}