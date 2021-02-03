#ifndef __ENUMS_H__
#define __ENUMS_H__

enum DIR {UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3};
enum DIR2 {TOPLEFT = 0, TOPRIGHT = 1, BOTTOMLEFT = 2, BOTTOMRIGHT = 3};
enum MODES {NONE = 0, REG = 1, MAKER = 2};
enum LAYERENUM {TEST = 0, MENU = 1, FOREGROUND = 2, PLAYER = 3, SECOND = 4, BACKGROUND = 5};
enum COORDENUM {X = 0, Y = 1};
enum SPRITETYPE {LOCAL = 1, GLOBAL = 2, CENTERED = 4, NOZOOM = 8, TRANSPARENT = 16};
enum MAKERMODES {PLACE = 1, EDIT = 2, REMOVE = 4, GRAB = 8, SWAP = 16};

#endif