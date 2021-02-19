ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_dialog-5)
#GSL := $(shell pkg-config --libs --cflags gsl)
#LOADER := $(shell export LD_LIBRARY_PATH=/usr/lib64)
FLAGS = -Wall -O2 -march=native
IGNORE = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-char-subscripts -Wno-unused-function
SOURCES = sprites.c alobj.c spritemanager.c mouse.c list/list.c game.c map.c mapmanager.c debug.c maker.c menu.c menudriver.c keyboard.c tilemanager.c



sprites: sprites.c
	gcc $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) 

all: sprites.c 
	gcc $(SOURCES) -o sprites $(FLAGS) $(ALLEGRO)