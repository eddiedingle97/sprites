#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "sprites.h"
#include "list/list.h"
#include "spritemanager.h"
#include "menu.h"
#include "colors.h"

struct menu *menu_create(struct list *itemlist, ALLEGRO_FONT *font, int x, int y, void (*framehandler)(struct menu *), void (*selecthandler)(struct menu *, int, int, char, char, char))
{
    struct menu *m = s_malloc(sizeof(struct menu), "menu_create");
    m->framehandler = menu_default_frame_handler;
    m->selecthandler = menu_default_select_handler;
    m->items = itemlist;
    m->x = x;
    m->y = y;
    m->font = font;
    m->select = NULL;
    m->frame = NULL;

    if(framehandler)
        m->framehandler = framehandler;

    if(selecthandler)
        m->selecthandler = selecthandler;

    return m;
}

struct menu *menu_create_default(struct list *itemlist, ALLEGRO_FONT *font, int x, int y)
{
    struct menu *m = s_malloc(sizeof(struct menu), "menu_create");
    m->framehandler = menu_default_frame_handler;
    m->selecthandler = menu_default_select_handler;
    m->items = itemlist;
    m->x = x;
    m->y = y;
    m->font = font;

    return m;
}

void menu_append_menu_item(struct menu *m, struct menuitem *mi)
{
    list_append(m->items, mi);
    m->framehandler(m);
}

void menu_destroy(struct menu *m)
{
    al_destroy_font(m->font);
    al_destroy_bitmap(m->select);
    list_destroy_with_function(m->items, (void (*)(void *))menu_destroy_menu_item);
    sm_destroy_sprite(m->frame);
    free(m);
}

struct menuitem *menu_create_menu_item(void *entry, void *(*func)(void *))
{
    struct menuitem *mi = s_malloc(sizeof(struct menuitem), "menu_create_menu_item");
    mi->entry = entry;
    mi->func = func;
    return mi;
}

void menu_destroy_menu_item(struct menuitem *mi)
{
    free(mi->entry);
    free(mi);
}

void menu_default_frame_handler(struct menu *m)
{
    m->movable = 1;
    m->height = al_get_font_line_height(m->font) * (m->items->size + 2);
    m->width = 200;
    if(!m->frame)
    {
        ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, CENTERED | NOZOOM);
    }

    al_set_target_bitmap(m->frame->bitmap);
    al_clear_to_color(WHITE);

    int i, texty = al_get_font_line_height(m->font);
    struct node *node = m->items->head;
    for(i = 0; i < m->items->size; i++)
    {
        struct menuitem *mi = (struct menuitem *)node->p;
        al_draw_text(m->font, BLACK, 10, texty, 0, mi->entry);
        texty += al_get_font_line_height(m->font);
        node = node->next;
    }
}

void menu_default_select_handler(struct menu *m, int x, int y, char one, char two, char scroll)
{
    if(!m->select)
    {
        m->select = al_create_bitmap(180, al_get_font_line_height(m->font));
        al_set_target_bitmap(m->select);
        al_lock_bitmap(m->select, 0, 0);
        int r, c;
        for(r = 0; r < al_get_bitmap_height(m->select); r++)
            for(c = 0; c < al_get_bitmap_width(m->select); c++)
            {
                if(c == 0 || c == al_get_bitmap_width(m->select) - 1)
                    al_draw_pixel(c, r, BLACK);

                if(r == 0 || r == al_get_bitmap_height(m->select) - 1)
                    al_draw_pixel(c, r, BLACK);
            }
        al_unlock_bitmap(m->select);
    }

    int item = y / al_get_font_line_height(m->font);
    m->framehandler(m);

    al_set_target_bitmap(m->frame->bitmap);
    if(item > 0 && item < m->items->size + 1)
        al_draw_bitmap(m->select, 10, item * al_get_font_line_height(m->font), 0);

    item--;
    struct menuitem *mi = list_get(m->items, item);
    if(mi && item > -1 && one)
        mi->func(mi->entry);
}