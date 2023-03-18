#ifndef __ITEMS_H__
#define __ITEMS_H__

struct entity *sword_create();
void sword_behaviour(struct entity *e, float *dx, float *dy);
void sword_destroy(struct entity *e);

struct entity *rusty_sword_create();
void rusty_sword_behaviour(struct entity *e, float *dx, float *dy);
void rusty_sword_destroy(struct entity *e);

struct entity *knife_create();
void knife_behaviour(struct entity *e, float *dx, float *dy);
void knife_destroy(struct entity *e);

#endif