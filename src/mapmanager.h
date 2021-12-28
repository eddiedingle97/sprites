#ifndef __MAPMANAGER_H__
#define __MAPMANAGER_H__
#include "map.h"

void mm_init(char *mapdir, ...);
void mm_destroy();
void mm_update_chunks();
void mm_test_color_chunk(struct chunk *chunk);
void mm_test_color_tile(float x, float y);
void mm_save_map(char *mapname);
struct map *mm_get_top_map();
struct chunk **mm_get_corners();
struct chunk *mm_get_chunk_from_rel_coordinate(float x, float y);
struct tile *mm_update_tile(float x, float y, struct tile *tile);
struct tile *mm_get_tile(float x, float y);
struct tile *mm_get_tile_from_rel_coordinate(float x, float y);

int mm_add_tile_map_to_list(char *tilemapfile, int tilesize);
int mm_get_tile_map_z(char *tilemapfile);
int mm_get_chunk_count();
void mm_draw_chunks(ALLEGRO_DISPLAY *display);
struct tilemap *mm_get_tile_map_for_tile(struct tile *tile);
ALLEGRO_BITMAP *mm_get_tile_bitmap(struct tile *tile);
float mm_get_tile_x(int chunk_x, int tilesize, int column);
float mm_get_tile_y(int chunk_y, int tilesize, int row);
void mm_print_tile_maps();
struct tilemap *mm_get_tile_map_from_z(int z);
void mm_save_tile_maps(char *dir);
int mm_load_tile_maps(char *dir);

enum DIR2 {TOPLEFT = 0, TOPRIGHT = 1, BOTTOMLEFT = 2, BOTTOMRIGHT = 3};

#endif