ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_dialog-5)
#GSL := $(shell pkg-config --libs --cflags gsl)
#LOADER := $(shell export LD_LIBRARY_PATH=/usr/lib64)
FLAGS = -Wall -O0 -march=native
IGNORE = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-char-subscripts -Wno-unused-function -Wno-incompatible-pointer-types
SOURCES = src/sprites.c src/alobj.c src/spritemanager.c src/mouse.c src/list.c src/game.c src/map.c src/mapmanager.c src/debug.c src/maker.c src/menu.c src/menudriver.c src/keyboard.c src/tilemanager.c src/movementandcollision.c src/entity.c src/entitymanager.c



sprites: 
	gcc $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) 

all:
	gcc $(SOURCES) -o sprites $(FLAGS) $(ALLEGRO)