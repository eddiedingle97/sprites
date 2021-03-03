#ifndef __MOVEMENTANDCOLLISION_H__
#define __MOVEMENTANDCOLLISION_H__

void mc_init();
int mc_do_movement(struct sprite *sprite, float up, float down, float left, float right);

#endif