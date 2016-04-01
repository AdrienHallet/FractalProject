#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "libfractal/fractal.h"
#include "errorHandling.h"

struct arg_readStruct{
	const char *filename;
	void *buffer;
};

char *strsep(char **stringp, const char *delim){
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
	    return (NULL);
	for (tok = s;;) {
	    c = *s++;
	    spanp = delim;
	    do {
		    if ((sc = *spanp++) == c) {
			    if (c == 0)
				    s = NULL;
			    else
				    s[-1] = 0;
			    *stringp = s;
			    return (tok);
		    }
	    } while (sc != 0);
	}
	/* NOTREACHED */
}

struct fractal *fractal_parse(char *line){
	const char *delim = " ";
	char **endptr;
	long value;
	char *name = strsep(&line, delim);

	int width = (int) strtol(strsep(&line, delim), endptr, 10);
	int height = (int) strtol(strsep(&line, delim), endptr, 10);
	double a = (double) strtol(strsep(&line, delim), endptr, 10);
	double b= (double) strtol(strsep(&line, delim), endptr, 10);
	struct fractal *newfractal = fractal_new(name, width, height, a, b);

	
}

int *readFile(const char *filename, struct fractal *(buffer)[]){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int *iteration;
	fp = fopen(filename, "r");
	if (fp == NULL)
	return -1;
	while ((read = getline(&line, &len, fp)) != -1) {
		if(read>1 && line[0]!='#'){ //ignore empty lines and comments
		struct fractal *myfrac = fractal_parse(line);
		iteration++;
		}
	}
	free(line);
	free(fp);
	pthread_exit(iteration);
}

int main(int argc, const char * argv[])
{
/*
	struct fractal *myfrac = fractal_new("Test 1", 800, 800, -0.8, 0.0);
	int w,h,x,y;
	for(x = 0; x < 800; x++){
		for(y = 0; y < 800; y++){
			fractal_compute_value(myfrac,x,y);
		}
		
	}
	write_bitmap_sdl(myfrac, "Image.bmp");

    	return 0;
*/
	
	if(handle_no_arguments(argc) < 0){
		return 0;	
	}
	int maxThreads = 10;
	struct fractal *fractalBuffer[maxThreads];
	int i;
	for(i = 0; i < maxThreads; i++){
		fractalBuffer[i] = malloc(sizeof(struct fractal));
		fractalBuffer[i] = NULL;
	}
	struct fractal *(*buffer)[] = &fractalBuffer;
	const char *filename = "inputfile1";
	pthread_t thread;
	struct arg_readStruct args;
	args.filename = filename;
	args.buffer = (void *)buffer;
	if(pthread_create(&thread, NULL, readFile, (void *)&args) == -1)
		return EXIT_FAILURE;
	printf("Thread created\n");
	int *iteration;
	if(pthread_join(thread, iteration))
		return EXIT_FAILURE;
	

	for(i = 0; i < maxThreads; i++){
		free(fractalBuffer[i]);
	}
	return 0;
}
