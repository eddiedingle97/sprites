#ifndef __GAME_H__
#define __GAME_H__

void game_init(char gamemode, char newmap, int width, int height);
void game_destroy();
void game_tick(ALLEGRO_DISPLAY *display);

#define MOVE 1

#endif