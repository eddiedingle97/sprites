#ifndef __ENTITY_H__
#define __ENTITY_H__

/*struct spritesheetdata
{
    unsigned short width;
    unsigned short height;
    unsigned short y;
    unsigned short x;
    unsigned char spritecount;
};*/

struct entity
{
    struct sprite *sprite;
    void *data;
    void (*behaviour)(struct sprite *sprite, void *data);
};

struct animation
{
    char spritecount;
    char cycle;
    char ticks;
    short x;
    short y;
};

struct entity *e_create(void (*draw)(float x, float y, float zoom, void *data, int tick), void (*behaviour)(struct sprite *sprite, void *data), float x, float y, void *data);
void e_destroy(struct entity *e);

#endif