ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_dialog-5)
#GSL := $(shell pkg-config --libs --cflags gsl)
#LOADER := $(shell export LD_LIBRARY_PATH=/usr/lib64)
OTHERS = -lm -lmimalloc
FLAGS = -Wall -Og
IGNORE = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-incompatible-pointer-types
SOURCES = $(wildcard src/*.c) 
CC = gcc

sprites:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) $(OTHERS)

all:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(ALLEGRO) $(OTHERS)