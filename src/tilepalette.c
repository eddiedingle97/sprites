#include <allegro5/allegro.h>
#include <stdio.h>
#include "sprites.h"
#include "tilepalette.h"
#include "mapmanager.h"
#include "map.h"
#include "util.h"

char curtype = 0;
struct palette *palette = NULL;
struct tile tile;
int notiles = 0;

void tp_change_palette(char *configfile, int type)
{
    s_free(palette, NULL);
    notiles = 0;
    ALLEGRO_CONFIG *cfg = al_load_config_file(configfile);
    int tilesize = u_atoi(al_get_config_value(cfg, "", "tilesize"));
    unsigned short z = mm_add_tile_map_to_list(al_get_config_value(cfg, "", "spritesheet"), tilesize);
    tilesize = tilesize ? tilesize : 1;//set to 1 if not found in config file

    //printf("tile map z in tpcp %d\n", z);

    int i;
    switch(type)
    {
        case PT_DUNGEONTILES:
            for(i = 0; i < DT_SIZE; i++)
            {
                palette = s_realloc(palette, ++notiles * sizeof(struct palette), NULL);
                palette[i].tilemap_x = tilesize * u_atoi(al_get_config_value(cfg, DT_NAMES[i], "x"));
                palette[i].tilemap_y = tilesize * u_atoi(al_get_config_value(cfg, DT_NAMES[i], "y"));
                palette[i].tilemap_z = (short)z;
                palette[i].type = DT_TILETYPE[i];
            }
            break;
        case PT_ISLANDTILES:
            for(i = 0; i < IT_SIZE; i++)
            {
                palette = s_realloc(palette, ++notiles * sizeof(struct palette), NULL);
                palette[i].tilemap_x = tilesize * u_atoi(al_get_config_value(cfg, IT_NAMES[i], "x"));
                palette[i].tilemap_y = tilesize * u_atoi(al_get_config_value(cfg, IT_NAMES[i], "y"));
                palette[i].tilemap_z = (short)z;
                palette[i].type = IT_TILETYPE[i];
            }
            break;
    }

    palette = s_realloc(palette, ++notiles * sizeof(struct palette), NULL);
    palette[notiles - 1].tilemap_x = 0;
    palette[notiles - 1].tilemap_y = 0;
    palette[notiles - 1].tilemap_z = 1;
    palette[notiles - 1].type = 0;

    curtype = type;
    al_destroy_config(cfg);
}

struct palette *tp_get_palette_copy()
{
    struct palette *out = s_malloc(notiles * sizeof(struct palette), "tp_get_palette_copy");
    int i;
    for(i = 0; i < notiles; i++)
    {
        out[i].tilemap_x = palette[i].tilemap_x;
        out[i].tilemap_y = palette[i].tilemap_y;
        out[i].tilemap_z = palette[i].tilemap_z;
    }        

    return out;
}

/*struct palette *tp_create_custom_palette(int *values, int novalues)
{
}*/

void tp_destroy()
{
    s_free(palette, NULL);
}

struct tile *tp_get_tile(int tileid)
{
    if(!palette)
        return NULL;

    if(tileid < 0 || tileid >= notiles)
    {
        switch(curtype)
        {
            case PT_DUNGEONTILES:
                tile.id = DT_ERROR;
                tile.type = DT_TILETYPE[DT_ERROR];
                break;
            case PT_ISLANDTILES:
                tile.id = IT_ERROR;
                tile.type = IT_TILETYPE[IT_ERROR];
                break;
        }
    }

    switch(curtype)
    {
        case PT_DUNGEONTILES:
            tile.id = tileid;
            tile.type = DT_TILETYPE[tileid];
            break;
        case PT_ISLANDTILES:
            tile.id = tileid;
            tile.type = IT_TILETYPE[tileid];
            break;
    }
    return &tile;
}
