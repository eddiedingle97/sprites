#ifndef __MENUDRIVER_H__
#define __MENUDRIVER_H__

void md_init();
void md_destroy();
void md_add_menu(struct menu *m);
struct menu *md_remove_last_menu();
void md_remove_menu();
int md_menu_tick();
struct menu *md_menu_hover();
char md_has_menu(struct menu *m);

#endif