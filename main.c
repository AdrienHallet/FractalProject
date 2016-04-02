#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "libfractal/fractal.h"
#include "errorHandling.h"

struct fractal *(*buffer)[];
int counter; //current buffered data
pthread_mutex_t buffer_access;
int maxthreads;
sem_t full, empty;

int hasNextData;

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
	double a = (double)atof(strsep(&line, delim));
	double b= (double) atof(strsep(&line, delim));
	struct fractal *newfractal = fractal_new(name, width, height, a, b);
	return newfractal;
	
}

int insert_fractal(struct fractal *fractal){
	/* When the buffer is not full add the item
	and increment the counter*/
	if(counter < maxthreads) {
		(*buffer)[counter] = fractal;
		printf("Saved %s in %d\n", (*buffer)[counter]->name, counter);
		printf("%s, %d %d %f %f\n", (*buffer)[counter]->name, (*buffer)[counter]->width, (*buffer)[counter]->height, (*buffer)[counter]->a, (*buffer)[counter]->b);
		fflush(stdout);
		counter++;
		return 0;
	}
	else { /* Error the buffer is full */
		return -1;
	}
}

int *readFile(const char *filename){
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
			sem_wait(&empty);
			struct fractal *myfrac = fractal_parse(line);
			iteration++;
			pthread_mutex_lock(&buffer_access);
			if(insert_fractal(myfrac)) {
				fprintf(stderr, " Producer report error condition\n");
			}
			/* release the mutex lock */
			pthread_mutex_unlock(&buffer_access);
			sem_post(&full);
		}
	}
	hasNextData=0;
	fclose(fp);
        free(line);
	pthread_exit(iteration);
}

int remove_fractal(struct fractal *fractal) {
   /* When the buffer is not empty remove the item
      and decrement the counter */
   if(counter > 0) {
      fractal = (*buffer)[(counter-1)];
      counter--;
      return 0;
   }
   else { /* Error buffer empty */
      return -1;
   }
}

/* Consumer Thread */
void *compute(void *param) {
   struct fractal *fractal;
	sem_wait(&full);
   while(hasNextData && counter>0) {
	printf("Will consume %d\n", counter);

      pthread_mutex_lock(&buffer_access);
      if(remove_fractal(fractal)) {
         fprintf(stderr, "Consumer report error condition\n");
      }
      else {
	int w,h,x,y;
	w=(*buffer)[(counter)]->width;
	h=(*buffer)[(counter)]->height;
	printf("%d %d\n", w, h);
	for(x = 0; x < w; x++){
		for(y = 0; y < h; y++){
			fractal_compute_value((*buffer)[(counter)],x,y);
		}
	
	}
	printf("computation ended\n");
	write_bitmap_sdl((*buffer)[(counter)], "Image.bmp");
      }
      /* release the mutex lock */
      pthread_mutex_unlock(&buffer_access);
      /* signal empty */
      sem_post(&empty);
   }
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
	maxthreads = 10;
	counter = 0;
	hasNextData = 1;
	struct fractal *fractalBuffer[maxthreads];
	int i;
	for(i = 0; i < maxthreads; i++){
		fractalBuffer[i] = malloc(sizeof(struct fractal));
	}
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
	if (pthread_mutex_init(&buffer_access, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

	buffer = &fractalBuffer;
	const char *filename = "inputfile1";
	pthread_t thread;
	
	if(pthread_create(&thread, NULL, readFile, filename) == -1)
		return EXIT_FAILURE;
	for(i = 0; i < 1; i++) { //REPLACE 2 BY MAXTHREADS WHEN TEST CLEARED
		pthread_t thread;
		pthread_create(&thread,NULL,compute,NULL);
		if(pthread_join(thread, NULL))
			return EXIT_FAILURE;
	}


	if(pthread_join(thread, NULL))
		return EXIT_FAILURE;
	free((*fractalBuffer));
	return 0;
}
