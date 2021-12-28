#ifndef __GAME_H__
#define __GAME_H__
#include <allegro5/allegro.h>

void game_init(char gamemode, char newmap, int width, int height);
void game_destroy();
void game_tick(ALLEGRO_DISPLAY *display);

#define MOVE 1

enum GAMEMODES {NONE, REG, MAKER};

#endif