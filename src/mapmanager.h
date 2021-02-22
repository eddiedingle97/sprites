#ifndef __MAPMANAGER_H__
#define __MAPMANAGER_H__

void mm_init(char *mapdir, ...);
void mm_destroy();
void mm_update_chunks();
void mm_test_color_chunk(struct chunk *chunk);
void mm_test_color_tile(int x, int y);
void mm_save_map(char *mapname);
struct map *mm_get_top_map();
struct chunk **mm_get_corners();
struct chunk *mm_get_chunk_from_rel_coordinate(int x, int y);
struct tile *mm_update_tile(int x, int y, struct tile *tile);
struct tile *mm_get_tile(int x, int y);

enum DIR2 {TOPLEFT = 0, TOPRIGHT = 1, BOTTOMLEFT = 2, BOTTOMRIGHT = 3};

#endif