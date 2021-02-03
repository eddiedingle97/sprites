#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "list/list.h"
#include "menu.h"
#include "map.h"
#include "spritemanager.h"
#include "mouse.h"
#include "enums.h"

static struct list *menus;
static struct menu *grab;
static int grabx;
static int graby;
static struct menu *menuhover;

void md_init()
{
    menus = list_create();
    grab = NULL;
    grabx = 0;
    graby = 0;
    menuhover = 0;
}

void md_destroy()
{
    list_destroy_with_function(menus, (void (*)(void *))menu_destroy);
}

void md_add_menu(struct menu *m)
{
    list_push(menus, m);
    sm_add_sprite_to_layer(m->frame);
}

struct menu *md_remove_last_menu()
{
    if(!menus->size)
        return NULL;
    struct menu *out = (struct menu *)list_pop(menus);
    sm_remove_sprite_from_layer(out->frame);
    return out;
}

void md_remove_menu(struct menu *m)
{
    if(!menus->size)
        return;
    struct node *node;
    for(node = menus->head; node != NULL; node = node->next)
        if(node->p == m)
        {
            struct menu *menu = node->p;
            sm_remove_sprite_from_layer(menu->frame);
            list_delete_node(menus, node);
            break;
        }
}

struct menu *md_menu_hover()
{
    return menuhover;
}

int md_menu_tick()
{
    int x = mouse_get_rel_x();
    int y = mouse_get_rel_y();
    menuhover = NULL;
    if(grab && mouse_get_one() && grab->movable)
    {
        grab->x = x - grabx;
        grab->y = y - graby;
        grab->framehandler(grab);
    }

    struct node *node;
    struct menu *m;
    for(node = menus->head; node != NULL; node = node->next)
    {
        m = (struct menu *)node->p;

        int range[4];
        
        if(m->frame->type & CENTERED)
        {
            int hw = m->width / 2;
            int hh = m->height / 2;
            range[UP] = m->y + hh;
            range[DOWN] = m->y - hh;
            range[LEFT] = m->x - hw;
            range[RIGHT] = m->x + hw;
        }

        else
        {
            range[UP] = m->y;
            range[DOWN] = m->y - m->height;
            range[LEFT] = m->x;
            range[RIGHT] = m->x + m->width;
        }

        if(!menuhover && x > range[LEFT] && x < range[RIGHT] && y > range[DOWN] && y < range[UP])
        {
            m->selecthandler(m, x - range[LEFT], range[UP] - y, mouse_get_single_one(), mouse_get_single_two(), mouse_get_scroll());

            if(m->movable && (range[UP] - y) / al_get_font_line_height(m->font) == 0)
            {
                if(mouse_get_one())
                {
                    grab = m;
                    grabx = x - m->x;
                    graby = y - m->y;
                }
                else
                {
                    grab = NULL;
                    grabx = 0;
                    graby = 0;
                }
            }

            menuhover = m;
        }

        else
            m->framehandler(m);
    }

    return 0;
}