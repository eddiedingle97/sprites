#ifndef __TILEMANAGER_H__
#define __TILEMANAGER_H__

void tm_init(int csize);
void tm_destroy();
int tm_add_tile_map_to_list(char *tilemapfile, int tilesize);
int tm_get_tile_map_z(char *tilemapfile);
void tm_add_chunk(struct chunk *chunk);
void tm_remove_chunk(struct chunk *chunk);
int tm_get_chunk_count();
void tm_draw_chunks(ALLEGRO_DISPLAY *display);
struct tilemap *tm_get_tile_map_for_tile(struct tile *tile);
ALLEGRO_BITMAP *tm_get_tile_bitmap(struct tile *tile);
float tm_get_tile_x(int chunk_x, int tilesize, int column);
float tm_get_tile_y(int chunk_y, int tilesize, int row);
void tm_print_tile_maps();
struct tilemap *tm_get_tile_map_from_z(int z);
void tm_save_tile_maps(char *dir);
int tm_load_tile_maps(char *dir);

#endif