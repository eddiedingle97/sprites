#ifndef __LEVELGENERATOR_H__
#define __LEVELGENERATOR_H__
#include "dictionary.h"

void lg_generate_level(char newmap);
void lg_destroy_level();
struct dict *lg_get_warp_table();

#endif