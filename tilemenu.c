void maker_tile_menu_frame_handler(struct menu *m)
{
    int w, h;
    w = WIDTH / 2 - abs(topleftcoord[X]);
    h = HEIGHT;
    m->width = w - 10;
    m->height = h - 10;
    m->x = topleftcoord[X] + w / 2;
    m->y = topleftcoord[Y] - h / 2;
    m->movable = 0;
    struct tilemap *tm = tm_get_tile_map_for_tile(m->items->head->p);

    if(!m->frame)
    {
        ALLEGRO_BITMAP *frame = al_create_bitmap(m->width, m->height);
        m->frame = sm_create_sprite(frame, m->x, m->y, MENU, CENTERED | NOZOOM);
    }
    al_set_target_bitmap(m->frame->bitmap);
    al_clear_to_color(WHITE);

    int r, c = 0, ncolumns;
    ncolumns = (m->width - 10) / TILESIZE;
    for(r = 0; ncolumns * r < m->items->size; r++)
    {
        for(c = 0; c < ncolumns && ncolumns * r + c < m->items->size; c++)
        {
            struct tile *t = (struct tile *)list_get(m->items, ncolumns * r + c);
            al_draw_scaled_bitmap(tm->bitmap, t->tilemap_x, t->tilemap_y, tm->tilesize, tm->tilesize, 5 + c * TILESIZE, 5 + r * TILESIZE, TILESIZE, TILESIZE, 0);
            //al_draw_scaled_bitmap(t->sprite->bitmap, 0, 0, al_get_bitmap_width(t->sprite->bitmap), al_get_bitmap_height(t->sprite->bitmap), 5 + c * TILESIZE, 5 + r * TILESIZE, al_get_bitmap_width(t->sprite->bitmap) * 2, al_get_bitmap_height(t->sprite->bitmap) * 2, 0);
        }
    }
}

void maker_tile_menu_select_handler(struct menu *m, int x, int y, char one, char two, char scroll)
{
    if(!m->select)
    {
        ALLEGRO_BITMAP *s = al_create_bitmap(TILESIZE, TILESIZE);
        al_set_target_bitmap(s);
        al_lock_bitmap(s, 0, 0);

        int r, c;
        int thick = 2;
        for(r = 0; r < TILESIZE; r++)
        {
            for(c = 0; c < TILESIZE; c++)
            {
                if(r < thick)
                    al_draw_pixel(c, r, BLACK);
                else if(r > TILESIZE - 1 - thick)
                    al_draw_pixel(c, r, BLACK);
                else if(c < thick)
                    al_draw_pixel(c, r, BLACK);
                else if(c > TILESIZE - 1 - thick)
                    al_draw_pixel(c, r, BLACK);
            }
        }
        al_unlock_bitmap(s);
        m->select = s;
    }

    m->framehandler(m);
    y -= 5;
    x -= 5;

    int ncolumns = (m->width - 10) / TILESIZE;

    y /= TILESIZE;
    x /= TILESIZE;

    al_set_target_bitmap(m->frame->bitmap);
    //al_draw_scaled_bitmap(m->select, 0, 0, al_get_bitmap_width(m->select), al_get_bitmap_height(m->select), x * TILESIZE + 5, y * TILESIZE + 5, al_get_bitmap_width(m->select) * 2, al_get_bitmap_height(m->select) * 2, 0);
    al_draw_bitmap(m->select, x * TILESIZE + 5, y * TILESIZE + 5, 0);
    struct node *node = list_get_node(m->items, y * ncolumns + x);

    if(node)
    {
        if(mode == SWAP && node != grabbedtilenode)
        {
            list_swap_node(m->items, grabbedtilenode, node);
        }

        if(mode == SWAP && node == grabbedtilenode)
        {
            grabbedtilenode->p = currenttile;
            grabbedtilenode = NULL;
            mode = PLACE;
        }

        else if(one)
        {
                switch(mode)
                {
                    case 0:
                        break;
                    case PLACE:
                    case EDIT: 
                        maker_set_grab_mode(node);
                        break;
                    case REMOVE: 
                        list_push(deletedtiles, node->p);
                        list_delete_node(m->items, node);
                        break;
                }
        }

        else if(two)
        {
            switch(mode)
            {
                case 0:
                    break;
                case PLACE:
                    maker_set_edit_mode(NULL);
                case EDIT:
                    currenttile = node->p;
                    editmenu->x = m->x + (x - m->width / 2);
                    editmenu->y = m->y - y;
                    maker_add_edit_menu();
                    break;
                case REMOVE:
                    break;
            }
        }
    }
}