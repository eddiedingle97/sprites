#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "dungeon.h"

static void copy_shape(unsigned char *dest, unsigned char *src, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		dest[i] = src[i];
	}
}

void fill_shape(unsigned char *tiles, int size, int tile)
{
	int i;
	for(i = 0; i < size; i++)
		tiles[i] = tile;
}

static void invert_shape(struct area *a)
{
	int size = a->width * a->height;
	int i;
	for(i = 0; i < size; i++)
	{
		if(a->tiles[i] == OPEN)
			a->tiles[i] = WALL;

		else
			a->tiles[i] = OPEN;

	}
}

static void rotate_shape(struct area *a)
{
	int size = a->height * a->width;
	unsigned char *old = malloc(size);
	copy_shape(old, a->tiles, size);

	//printf("height: %d, width %d\n", a->height, a->width);

	int r, c;
	for(r = 0; r < a->height; r++)
	{
		for(c = 0; c < a->width; c++)
		{
			//printf("%c", old[r * a->width + c] + 48);
			a->tiles[c * a->height + (a->height - r - 1)] = old[r * a->width + c];
		}
		//printf("\n");
	}

	int temp = a->height;
	a->height = a->width;
	a->width = temp;

	free(old);
}

static void L_shape(struct area *a, double scale)
{
	int sheight = a->height * scale;
	int swidth = a->width * scale;

	int r, c;
	for(r = 0; r < a->height; r++)
	{
		for(c = 0; c < a->width; c++)
		{
			if(r < (a->height - sheight + 1) && c < swidth)
				a->tiles[c + r * a->width] = WALL;

			else if(r > (a->height - sheight))
				a->tiles[c + r * a->width] = WALL;
		}
	}
}

static void circle_shape(struct area *a, double scale, double scalex, double scaley, int filled)
{
	double x = (double)(a->width >> 1), y = (double)(a->height >> 1);	
	
	double rad = a->width < a->height ? (double)(a->width >> 1) : (double)(a->height >> 1);
	double rad2 = rad * rad * scale;
	
	//printf("%.2fr, %.2fx, %.2fy", rad, x, y);

	int r, c;
	for(r = 0; r < a->height; r++)
	{
		for(c = 0; c < a->width; c++)
		{
			//printf("%.2f ", (pow((c - x), 2.0) / scalex) + (pow((r + y), 2.0) / scaley));
			
			double left = (pow((c - x), 2.0) / scalex) + (pow((r - y), 2.0) / scaley); 

			if(filled && left < rad2)
				a->tiles[c + r * a->width] = WALL;

			if(!filled && (left > rad2 - rad && left < rad2 + rad))
				a->tiles[c + r * a->width] = WALL;
		}
	}	

}

static void rectangle_shape(struct area *a, double scale)
{
	double x = (double)(a->width >> 1), y = (double)(a->height >> 1);
	
	double height = y * scale, width = x * scale;

	//printf("%.2f, %.2f", x - width, x + width);

	int r, c;
	for(r = 0; r < a->height; r++)
	{
		for(c = 0; c < a->width; c++)
		{
			if(c >= x - width && c <= x + width && r >= y - height && r <= y + height)
				a->tiles[c + r * a->width] = WALL;
		}
	}
}

static void line_shape(struct area *a, double scale, int vertical)
{
	int width;
	int middle;

	if(vertical)
	{
		width = (int)(a->width * scale);
		middle = a->width >> 1;
		
		int r, c;
		for(c = middle - (width >> 1); c < middle + (width >> 1); c++)
		{
			for(r = 0; r < a->height; r++)
			{
				a->tiles[c + r * a->width] = WALL;
			}
		}
	}

	else
	{
		width = (int)(a->height * scale);
		middle = a->height >> 1;

		int r, c;
		for(r = middle - (width >> 1); r < middle + (width >> 1); r++)
		{
			for(c = 0; c < a->width; c++)
			{
				a->tiles[c + r * a->width] = WALL;
			}
		}
	}
}

void _shape(struct area *a, gsl_rng *r)
{
	fill_shape(a->tiles, a->height * a->width, OPEN);
	int roll = gsl_rng_uniform_int(r, CIRCLE_WEIGHT + RECTANGLE_WEIGHT + L_WEIGHT + OPEN_WEIGHT);
	double scale = .2 + (gsl_rng_uniform(r) * .6);
	int rotations = gsl_rng_uniform_int(r, 4);
	
	if(roll < L_WEIGHT)
	{
		//printf("L_shape %d\n", roll);
		L_shape(a, scale);
	}
	else if(roll < L_WEIGHT + CIRCLE_WEIGHT)
	{
		//printf("circle_shape %d \n", roll);
		circle_shape(a, scale, 4 * gsl_rng_uniform(r), gsl_rng_uniform(r), gsl_rng_uniform_int(r, 2));
	}
	else if(roll < L_WEIGHT + CIRCLE_WEIGHT + RECTANGLE_WEIGHT)
	{
		//printf("rectangle shape %d \n", roll);

		rectangle_shape(a, scale);
	}
	
	scale = .1 + (gsl_rng_uniform(r) * .3);
	int vertical = gsl_rng_uniform_int(r, 2);

	if(roll % 5 == 0)
		line_shape(a, scale, vertical);
	if(roll % 10 == 0)
		line_shape(a, scale, !vertical);

	if(roll % 8 == 0)
		invert_shape(a);

	int i;
	for(i = 0; i < rotations; i++)
		rotate_shape(a);
}

