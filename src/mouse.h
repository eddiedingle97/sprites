#ifndef __MOUSE_H__
#define __MOUSE_H__

void mouse_init(ALLEGRO_MOUSE_STATE *mousestate);
void mouse_draw(ALLEGRO_DISPLAY *display);
void mouse_set_bitmap(ALLEGRO_BITMAP *bitmap);
int mouse_get_scroll();
int mouse_get_rel_x();
int mouse_get_rel_y();
char mouse_get_single_one();
char mouse_get_one();
char mouse_get_single_two();
char mouse_get_two();
char mouse_get_three();
void mouse_destroy();

#endif
