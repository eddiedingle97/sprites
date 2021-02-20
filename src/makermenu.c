struct menuentrysub
{
    char *title;
    struct menu *menu;
};

void maker_main_menu_frame_handler(struct menu *m)
{
    m->height = al_get_font_line_height(m->font) * (m->items->size + 2);
    m->width = 400;
    m->movable = 0;
    if(!m->frame)
    {
        ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, NOZOOM);
    }

    al_set_target_bitmap(m->frame->bitmap);
    al_clear_to_color(WHITE);

    int i, texty = al_get_font_line_height(m->font);
    struct node *node = m->items->head;
    for(i = 0; i < 2; i++)
    {
        struct menuitem *mi = (struct menuitem *)node->p;
        al_draw_text(m->font, BLACK, 10, texty, 0, mi->entry);
        texty += al_get_font_line_height(m->font);
        node = node->next;
    }

    struct menuitem *mi = node->p;
    struct menuentrysub *mes = mi->entry;
    al_draw_text(m->font, BLACK, 10, texty, 0, mes->title);
    texty += al_get_font_line_height(m->font);
    node = node->next;

    mi = node->p;
    mes = mi->entry;
    al_draw_text(m->font, BLACK, 10, texty, 0, mes->title);
}

void maker_main_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll)
{
    if(!m->select)
    {
        m->select = al_create_bitmap(380, al_get_font_line_height(m->font));
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
    if(mi && item > -1 && two)
        mi->func(mi->entry);

    else if(mi && item > -1 && one)
        mi->func(mi->entry);
}

void *maker_load_tile_menu_function(void *file)
{
    maker_load_tile_menu_from_file(s_get_full_path_with_dir("tilemenus", file));
    return NULL;
}

void *maker_load_image_function(void *file)
{
    maker_load_tile_menu_from_image(file, 16);
    return NULL;
}

void *maker_open_sub_menu(struct menuentrysub *mes)
{
    if(!md_has_menu(mes->menu))
        md_add_menu(mes->menu);
    else
        md_remove_menu(mes->menu);
    return NULL;
}