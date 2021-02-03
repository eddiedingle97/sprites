ALLEGRO := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_image-5)
GSL := $(shell pkg-config --libs --cflags gsl)
GSLLOADER := $(shell export LDLIBRARYPATH=LDLIBRARYPATH:/usr/local/lib)
FLAGS = -Wall -Og


driver: driver.c
	$(GSLLOADER)
	gcc driver.c dungeon.c shape.c ../list/list.c -o driver $(FLAGS) $(ALLEGRO) $(GSL) 
