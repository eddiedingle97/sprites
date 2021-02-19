#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "list/list.h"
#include "spritemanager.h"
#include "sprites.h"
#include "mouse.h"
#include "colors.h"

static int debug = 0;
static struct sprite *frame;
static ALLEGRO_FONT *font;
static struct sprite *dbinfo;
static char buf[32];

void debug_init(char d)
{
    debug = d;

    if(d)
    {
        int boxsize = HEIGHT;
        ALLEGRO_BITMAP *box = al_create_bitmap(boxsize, boxsize);
        al_set_target_bitmap(box);
        al_lock_bitmap(box, 0, 0);
        int i, thick;
        for(thick = 0; thick < 3; thick++)
            for(i = 0; i < boxsize; i++)
            {
                al_draw_pixel(i, thick, RED);
                al_draw_pixel(i, boxsize - 1 - thick, RED);
                al_draw_pixel(thick, i, RED);
                al_draw_pixel(boxsize - 1 - thick, i, RED);
            }
        al_unlock_bitmap(box);
        frame = sm_create_sprite(box, 0, 0, TEST, CENTERED | NOZOOM);
        sm_add_sprite_to_layer(frame);

        font = al_load_ttf_font("fonts/lcd.ttf", 10, 0);

        ALLEGRO_BITMAP *infobox = al_create_bitmap(150, 100);
        al_set_target_bitmap(infobox);
        al_clear_to_color(WHITE);

        dbinfo = sm_create_sprite(infobox, -WIDTH / 2, HEIGHT / 2, TEST, NOZOOM);
        sm_add_sprite_to_layer(dbinfo);
    }
}

void debug_toggle_sprites()
{
    if(dbinfo->id)
    {
        sm_remove_sprite_from_layer(dbinfo);
        sm_remove_sprite_from_layer(frame);
    }
    else
    {
        sm_add_sprite_to_layer(dbinfo);
        sm_add_sprite_to_layer(frame);
    }
}

void debug_printf(char *format, ...)
{
    if(debug)
    {
        va_list vl;
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}

void debug_print_error(char *format, ...)
{
    if(debug)
    {
        if(!errno)
        {
            va_list vl;
            va_start(vl, format);
            vfprintf(stderr, format, vl);
            va_end(vl);
        }

        else
        {
            perror(format);
        }
    }
}

void debug_tick(long time)
{
    if(debug)
    {
        al_set_target_bitmap(dbinfo->bitmap);
        al_clear_to_color(WHITE);
        memset(buf, 0, 32);
        sprintf(buf, "FPS ratio: %.2f", (time / (float)CLOCKS_PER_SEC) * 60.0);
        al_draw_text(font, BLACK, 5, 5, 0, buf);
        memset(buf, 0, 32);
        sprintf(buf, "Frame time %ld", time);
        al_draw_text(font, BLACK, 5, 5 + al_get_font_line_height(font), 0, buf);
        memset(buf, 0, 32);
        sprintf(buf, "Sprites drawn: %d", sm_get_sprite_count());
        al_draw_text(font, BLACK, 5, 5 + 2 * al_get_font_line_height(font), 0, buf);
        memset(buf, 0, 32);
        sprintf(buf, "Mouse GX: %d", sm_rel_to_global_x(mouse_get_rel_x()));
        al_draw_text(font, BLACK, 5, 5 + 3 * al_get_font_line_height(font), 0, buf);
        memset(buf, 0, 32);
        sprintf(buf, "Mouse GY: %d", sm_rel_to_global_y(mouse_get_rel_y()));
        al_draw_text(font, BLACK, 5, 5 + 4 * al_get_font_line_height(font), 0, buf);
    }
}

char debug_get()
{
    return debug;
}

void debug_destroy()
{
    if(debug)
    {
        al_destroy_font(font);
    }
}