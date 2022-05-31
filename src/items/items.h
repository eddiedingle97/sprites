#ifndef __ITEMS_H__
#define __ITEMS_H__

struct entity *sword_create();
void sword_behaviour(struct entity *e, float *dx, float *dy);
void sword_destroy(struct entity *e);

#endif