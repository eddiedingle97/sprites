#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "keyboard.h"

static struct
{
    unsigned char hist:7;
    unsigned char key:1;
} keys[ALLEGRO_KEY_MAX];

enum keypositions {UP, LEFT, DOWN, RIGHT, PAUSE, SHIFT, MAPSAVE, UNDO, TILEMENUSAVE,
GETTEXT, ENTER, NEXTTILEMENU, TOGGLEDEBUG};
static char keyconfig[] = {ALLEGRO_KEY_W, ALLEGRO_KEY_A, ALLEGRO_KEY_S, ALLEGRO_KEY_D, ALLEGRO_KEY_ESCAPE, ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_M, ALLEGRO_KEY_Z, ALLEGRO_KEY_N,
ALLEGRO_KEY_T, ALLEGRO_KEY_ENTER, ALLEGRO_KEY_TAB, ALLEGRO_KEY_SLASH};
static char gettext = 0;
static char *textfield = NULL;
static int textfieldsize = 0;

void kb_init()
{
    memset(keys, 0, sizeof(keys));
}

void kb_update(ALLEGRO_EVENT *event)
{
    switch(event->type)
    {
        case ALLEGRO_EVENT_KEY_DOWN:
            switch(event->keyboard.keycode)
            {
                case ALLEGRO_KEY_ENTER:
                    //NO BREAK
                case ALLEGRO_KEY_ESCAPE:
                    gettext = 0;
                    textfield = 0;
                    break;
            }
            keys[event->keyboard.keycode].key = 1 ^ gettext;
            keys[event->keyboard.keycode].hist |= 1 ^ gettext;

            break;

        case ALLEGRO_EVENT_KEY_UP:
            keys[event->keyboard.keycode].key = 0;
            break;

        case ALLEGRO_EVENT_KEY_CHAR:
            if(gettext && strlen(textfield) < textfieldsize)
            {
                const char *c = al_keycode_to_name(event->keyboard.keycode);
                if(strlen(c) == 1)
                    strcat(textfield, c);
            }
            break;
    }
}

void kb_get_text(char *buf, int size)
{
    gettext = 1;
    textfield = buf;
    textfieldsize = size;
}

void kb_tick()
{
    int i;
    for(i = 0; i < ALLEGRO_KEY_MAX; i++)
    {
        keys[i].hist <<= 1;
    }
}

char kb_get_key(unsigned char k)
{
    return keys[keyconfig[k]].key;
}

char kb_get_single_key(unsigned char k)
{
    return keys[keyconfig[k]].key && (keys[keyconfig[k]].hist & 1);
}

char kb_get_shifted_single_key(unsigned char k)
{
    return kb_get_single_key(k) && kb_get_key(SHIFT);
}

char kb_get_up()
{
    return kb_get_key(UP);
}

char kb_get_left()
{
    return kb_get_key(LEFT);
}

char kb_get_down()
{
    return kb_get_key(DOWN);
}

char kb_get_right()
{
    return kb_get_key(RIGHT);
}

char kb_get_pause()
{
    return kb_get_key(PAUSE);
}

char kb_get_shift()
{
    return kb_get_key(SHIFT);
}

char kb_get_mapsave()
{
    return kb_get_single_key(MAPSAVE);
}

char kb_get_undo()
{
    return kb_get_single_key(UNDO);
}

char kb_get_gettext()
{
    return kb_get_single_key(GETTEXT);
}

char kb_get_tile_menu_save()
{
    return kb_get_single_key(TILEMENUSAVE);
}

char kb_get_enter()
{
    return kb_get_single_key(ENTER);
}

char kb_get_next_tile_menu()
{
    return kb_get_single_key(NEXTTILEMENU);
}

char kb_get_toggle_debug()
{
    return kb_get_single_key(TOGGLEDEBUG);
}