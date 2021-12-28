#ifndef __MENU_H__
#define __MENU_H__
#include <allegro5/allegro.h>

struct menu 
{
    int width;
    int height;
    int x;
    int y;
    char movable;
    struct sprite *frame;
    ALLEGRO_BITMAP *select;
    struct list *items;
    ALLEGRO_FONT *font;
    void (*framehandler)(struct menu *);
    void (*selecthandler)(struct menu *, int, int, char, char, char);
};

struct menuitem
{
    void *entry;
    void *(*func)(void *);
};

struct menu *menu_create(struct list *itemlist, ALLEGRO_FONT *font, int x, int y, void (*framehandler)(struct menu *), void (*selecthandler)(struct menu *, int, int, char, char, char));
struct menu *menu_create_default(struct list *itemlist, ALLEGRO_FONT *font, int x, int y);
void menu_destroy(struct menu *m);
void menu_append_menu_item(struct menu *m, struct menuitem *mi);
void menu_default_initializer(struct menu *m);
void menu_default_destructor(struct menu *m);
void menu_default_frame_handler(struct menu *m);
void menu_default_select_handler(struct menu *m, int x, int y, char one, char two, char scroll);
struct menuitem *menu_create_menu_item(void *entry, void *(*func)(void *));
void menu_destroy_menu_item(struct menuitem *mi);

#endif