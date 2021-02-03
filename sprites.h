#ifndef __SPRITES_H__
#define __SPRITES_H__

#define HEIGHT 900
#define WIDTH 1600

void *s_malloc(int bytes, const char *msg);
void *s_realloc(void *ptr, int b, const char *msg);
char *s_get_heap_string(const char *str);
char *s_get_root_dir();
char *s_get_full_path(char *str);
char *s_get_full_path_with_dir(char *dir, char *file);

#endif