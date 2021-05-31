#ifndef __DEBUG_H__
#define __DEBUG_H__

void debug_init(char d);
char debug_get();
void debug_add_sprite(struct sprite *sprite);
void debug_destroy();
void debug_tick(long time);
void debug_toggle_sprites();
void debug_printf(char *format, ...);
void debug_perror(char *format, ...);

#endif