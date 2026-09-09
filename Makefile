ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5 allegro_primitives-5)
#GSL := $(shell pkg-config --libs --cflags gsl)
#LOADER := $(shell export LD_LIBRARY_PATH=/usr/lib64)
OTHERS = -lm -D ALMALLOC -D HT_IMPLEMENTATION
FLAGS = -Wall -g
IGNORE = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function -Wno-incompatible-pointer-types
SOURCES = $(wildcard src/*.c) $(wildcard src/entities/*.c) $(wildcard src/items/*.c)
CC = gcc

delaunaydebug:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) $(OTHERS) -D DEBUG -D DELAUNAYDEBUG

debug:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) $(OTHERS) -D DEBUG

sprites:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(IGNORE) $(ALLEGRO) $(OTHERS)

all:
	$(CC) $(SOURCES) -o sprites $(FLAGS) $(ALLEGRO) $(OTHERS)
