#include <stdio.h>
#include <string.h>
#include <allegro5/allegro.h>
#include "keyboard.h"

static char buf[512];

void c_command()
{
    memset(buf, 0, 512);
    kb_get_text(buf, 512);
    printf("%s\n", buf);
}