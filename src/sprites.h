#ifndef __SPRITES_H__
#define __SPRITES_H__

#define HEIGHT 900
#define WIDTH 1600

void *s_malloc(int bytes, const char *msg);
void *s_realloc(void *ptr, int b, const char *msg);
void s_free(void *ptr, const char *msg);
char *s_get_heap_string(const char *str);
char  s_string_match(char *one, char *two);
char *s_get_root_dir();
char *s_get_full_path(char *str);
char *s_get_full_path_with_dir(char *dir, char *file);
struct list *s_get_file_list_from_dir(char *dir);
int s_round(float f);
int s_floor(float f);

#endif