#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
	struct fractal *newFractal = malloc(sizeof(struct fractal));
	if(!newFractal)
		return NULL;
	newFractal->name = name;
	newFractal->width = width;
	newFractal->height = height;
	newFractal->a = a;
	newFractal->b = b;
	newFractal->value = (double **) malloc(width*sizeof(double));
	if(!newFractal->value){
		free(newFractal);
		return NULL;	
	}
	int i;
	for(i = 0; i < width; i++){
		newFractal->value[i] = (double *) malloc(height*sizeof(double));
	}
    	return newFractal;
}

void fractal_free(struct fractal *f)
{
	int i;
	for(i = 0; i < f->width; i++){
		free(f->value[i]);
	}
	free(f->value);
   	free(f);
}	

const char *fractal_get_name(const struct fractal *f)
{
    return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return f->value[x][y];
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    return f->value[x][y] = val;
}

int fractal_get_width(const struct fractal *f)
{
    return f->width;
}

int fractal_get_height(const struct fractal *f)
{
    return f->height;
}

double fractal_get_a(const struct fractal *f)
{
    return f->a;
}

double fractal_get_b(const struct fractal *f)
{
    return f->b;
}
