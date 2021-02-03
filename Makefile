ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_dialog-5)
#GSL := $(shell pkg-config --libs --cflags gsl)
LOADER := $(shell export LDLIBRARYPATH=$LDLIBRARYPATH:/usr/local/lib)
FLAGS = -Wall -g 
IGNORE = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-char-subscripts
SOURCES = sprites.c alobj.c spritemanager.c mouse.c list/list.c game.c map.c mapmanager.c debug.c maker.c menu.c menudriver.c keyboard.c tilemanager.c



sprites: sprites.c
	$(LOADER)
	gcc $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) 

all: sprites.c 
	$(LOADER)
	gcc $(SOURCES) -o sprites $(FLAGS) $(ALLEGRO)