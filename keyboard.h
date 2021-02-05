#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void kb_init();
void kb_update(ALLEGRO_EVENT *event);
void kb_get_text(char *buf, int size);
void kb_tick();
char kb_get_up();
char kb_get_left();
char kb_get_down();
char kb_get_right();
char kb_get_pause();
char kb_get_shift();
char kb_get_mapsave();
char kb_get_undo();
char kb_get_tile_menu_save();
char kb_get_gettext();
char kb_get_enter();
char kb_get_next_tile_menu();
char kb_get_toggle_debug();

#endif