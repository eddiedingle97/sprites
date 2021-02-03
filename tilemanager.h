#ifndef __TILEMANAGER_H__
#define __TILEMANAGER_H__

void tm_init(int csize, int tsize);
struct tilemap *tm_load_tile_map(char *tilemapfile, int tilesize);
int tm_add_tile_map(struct tilemap *tm);
int tm_get_tile_map_z(char *tilemapfile);
void tm_add_chunk(struct chunk *chunk);
void tm_remove_chunk(struct chunk *chunk);
int tm_get_chunk_count();
void tm_draw_tiles(ALLEGRO_DISPLAY *display);
struct tilemap *tm_get_tile_map_for_tile(struct tile *tile);
ALLEGRO_BITMAP *tm_get_tile_bitmap(struct tile *tile);
int tm_get_tile_x(int chunk_x, int tilesize, int column);
int tm_get_tile_y(int chunk_y, int tilesize, int row);

#endif