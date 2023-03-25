#ifndef __PERLIN_H__
#define __PERLIN_H__

struct noisemap 
{
	int height;
	int width;
	int depth;
	unsigned char *noise;
};

char *perlin_noise_map(int seed, int width, int height, int no_z, int noiter, float scale, float centerweight);
void perlin_noise_iter(int seed, int width, int height, int no_z, int noiter, float scale, float centerweight, void *data, void (*func)(int, int, int, float, void *));
void perlin_init(int seed, int noiter);
float perlin_noise_sample(int width, int height, int x, int y, int noiter, float scale, float centerweight);
void perlin_done();

#endif