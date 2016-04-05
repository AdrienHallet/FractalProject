#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "libfractal/fractal.h"
#include "handleArgument.h"

#define EXIT_SIGNAL "SIGNAl_THREAD_EXIT"

struct fractal *(*buffer)[];
struct arguments *args;
struct fractal *average_fractal;
int current_average;
int counter; //current buffered data
int thread_counter;
pthread_mutex_t buffer_access;
pthread_mutex_t thread_count;
pthread_mutex_t average_access;
sem_t full, empty;
const char *extension = ".bmp";
const char *filename;

void debug_print_buffer(int a){
	int i;
	printf("DEBUG PRINT BUFFER\n");
	printf("Current counter : %d\n", counter);
	printf("Pointer to buffer : %p\n", buffer);
	printf("Pointer to last buffed : %p\n", (*buffer)[counter]);
	for(i = 0; i <counter-a; i++){
		printf("Pointer to current buffed : %p\n", (*buffer)[i]);
		printf("At counter %d the name is : %s\n", i, (*buffer)[i]->name);
	}
}
//HELP FUNCTION - TO MOVE IN OTHER FILE
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
	char *name = strsep(&line, delim);
	int width = (int) atoi(strsep(&line, delim));
	int height = (int) atoi(strsep(&line, delim));
	double a = (double)atof(strsep(&line, delim));
	double b= (double) atof(strsep(&line, delim));
	struct fractal *newfractal = fractal_new(name, width, height, a, b);
	printf("P: Parsed fractal name = %s\n", fractal_get_name(newfractal));
	fflush(stdout);
	return newfractal;
}

int insert_fractal(struct fractal *fractal){
	/* When the buffer is not full add the item
	and increment the counter*/
	if(counter < args->maxThreads) {
		(*buffer)[counter] = fractal_new(fractal->name, 
			*fractal->width, *fractal->height, 
			*fractal->a, *fractal->b);
		fractal_free(fractal);
		counter++;
		return 0;
	}
	else { /* Error the buffer is full */
		return -1;
	}
}

int isExitSignal(struct fractal* signal){
	const char* current = fractal_get_name(signal);
	if(strcmp(current, EXIT_SIGNAL))
		return 0;
	else
		return 1;
}

int *send_signals(){
	printf("Send the signals\n");
	fflush(stdout);
	int thread;
	for(thread = 0; thread < args->maxThreads; thread++){
		printf("Waiting----------\n");
		fflush(stdout);
		sem_wait(&empty);
		printf("WAITED----------------\n");
		fflush(stdout);
		pthread_mutex_lock(&buffer_access);
		struct fractal *myfrac = fractal_new(EXIT_SIGNAL, 0, 0, 0, 0);
		if(insert_fractal(myfrac)) {
			fprintf(stderr, "Could not send stop signal to thread %d\n", thread);
		}
		pthread_mutex_unlock(&buffer_access);
		sem_post(&full);
	}
	return 0;
}

int *readFile(const char *filename){

	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int iteration = 0;
	fp = fopen(filename, "r");
	if (fp == NULL)
		return -1;
	while (read != -1) {
		iteration++;
		read = getline(&line, &len, fp);
		if(read>1 && line[0]!='#'){ //ignore empty lines and comments
			sem_wait(&empty);
			pthread_mutex_lock(&buffer_access);
			struct fractal *myfrac = fractal_parse(line);
			
			if(insert_fractal(myfrac)) {
				fprintf(stderr, "Producer report error condition\n");
			}
			pthread_mutex_unlock(&buffer_access);
			sem_post(&full);
		}
		
	}
	pthread_mutex_lock(&thread_count);
	thread_counter--;
	printf("THREAD OUT : %d\n", thread_counter);
	fflush(stdout);
	if(thread_counter==0)
		send_signals();
	pthread_mutex_unlock(&thread_count);
	fclose(fp);
        free(line);
	pthread_exit(1);
}

int remove_fractal(struct fractal **fractal) {
   /* When the buffer is not empty remove the item
      and decrement the counter */
   if(counter > 0) {
	if(fractal_get_name((*buffer)[(counter-1)])!=NULL)
	      *fractal = fractal_new(fractal_get_name((*buffer)[(counter-1)]),
				fractal_get_width((*buffer)[(counter-1)]),
				fractal_get_height((*buffer)[(counter-1)]),
				fractal_get_a((*buffer)[(counter-1)]),
				fractal_get_b((*buffer)[(counter-1)]));
      counter--;
      return 0;
   }
   else { /* Error buffer empty */
      return -1;
   }
}

void *compute()
{
	struct fractal **fractal;
	fractal = malloc(sizeof(struct fractal*));
	while(1) {
		printf("Will wait\n");
		sem_wait(&full);
		pthread_mutex_lock(&buffer_access);
		printf("Has waited\n");
		fflush(stdout);
		if(remove_fractal(fractal)) {
			fprintf(stderr, "Consumer report error condition\n");
			pthread_mutex_unlock(&buffer_access);
		}
		else if(isExitSignal(*fractal)){
			fractal_free(*fractal);
			free(fractal);
			pthread_mutex_unlock(&buffer_access);
			sem_post(&empty);
			pthread_exit(0);
		}
		else {
			printf("Treating %s\n", fractal_get_name(*fractal));
			fflush(stdout);
			pthread_mutex_unlock(&buffer_access);
			int w,h,x,y;
			w=fractal_get_width(*fractal);
			h=fractal_get_height(*fractal);
			char *filename = malloc(strlen(fractal_get_name(*fractal))+strlen(extension)+strlen(args->currentDirectory)+1);
			strcpy(filename, args->currentDirectory);
			strcat(filename, fractal_get_name(*fractal));
			strcat(filename, extension);
			double average;
			printf("Starting computations\n");
			fflush(stdout);
			for(x = 0; x < w; x++){
				for(y = 0; y < h; y++){
					fractal_compute_value(*fractal,x,y);
					average += fractal_get_value(*fractal, x, y);
				}

			}
			printf("Ended computations\n");
			average = average/(fractal_get_height(*fractal) * fractal_get_width(*fractal));
			printf("Will lock average\n");
			pthread_mutex_lock(&average_access);
			if(average>current_average){
				average_fractal = fractal_new(fractal_get_name(*fractal),
							 fractal_get_width(*fractal), 
							 fractal_get_height(*fractal), 
							 fractal_get_a(*fractal), 
							 fractal_get_b(*fractal));

			}
			pthread_mutex_unlock(&average_access);
			printf("Unlocked average\n");
			if(args->allImages){
				if(access(filename, F_OK) != -1){ //if file exists
					fprintf(stderr, "File %s already exists\n");
				}
				else{
					write_bitmap_sdl(*fractal, filename);
				}
				printf("C:Created %s\n", filename);
			}
			
			free(filename);
		}
		printf("Ended one computation\n");
		fractal_free(*fractal);
		/* signal empty */
		sem_post(&empty);
		
	}
	free(fractal);
	pthread_exit(0);
}

int initialize()
{
	thread_counter = 0;
	counter = 0;
	average_fractal = malloc(sizeof(struct fractal*));
	//struct fractal *fbuffer = malloc(maxThreads * sizeof(struct fractal));
	//buffer = &fbuffer;	
	
	printf("%d\n", args->maxThreads);
	buffer = malloc((int)args->maxThreads * sizeof(struct fractal));
	
	sem_init(&full, 0, 0);
	sem_init(&empty,0, (int)args->maxThreads);

	if (pthread_mutex_init(&buffer_access, NULL) != 0)
	{
		printf("\n buffer mutex init failed\n");
		return 0;
	}
	if (pthread_mutex_init(&thread_count, NULL) != 0)
	{
		printf("\n counter mutex init failed\n");
		return 0;
	}
	return 1;
}

int launch_threads()
{
	int i;
	current_average = 0;
	//producer
	pthread_t producer[(int)args->inputCount];
	thread_counter = args->inputCount;
	for(i = 0; i < args->inputCount; i++){
		if(pthread_create(&producer[i], NULL, readFile, args->inputFiles[i]) == -1)
			return EXIT_FAILURE;
	}
	
	//consumer
	pthread_t consumer[(int)args->maxThreads];
	for(i = 0; i < args->maxThreads; i++) {
		if(pthread_create(&consumer[i],NULL,compute,NULL) == -1)
			printf("FAILED THREAD\n");
	}

	for(i = 0; i < args->inputCount; i++) {
		if(pthread_join(producer[i], NULL))
			return EXIT_FAILURE;
	}
	for(i = 0; i < args->maxThreads; i++){
		if(pthread_join(consumer[i], NULL))
			return EXIT_FAILURE;
	}
	char *output_file = malloc(strlen(args->outputFile)+strlen(args->currentDirectory)+strlen(extension)+1);
	strcpy(output_file, args->currentDirectory);
	strcat(output_file, args->outputFile);
	strcat(output_file, extension);
	int x,y;
	int w = fractal_get_width(average_fractal);
	printf("%d\n", w);
	int h = fractal_get_height(average_fractal);
	printf("%d\n", h);
	double a = fractal_get_a(average_fractal);
	printf("%f\n", a);
	double b = fractal_get_b(average_fractal);
	printf("%f\n", b);
	printf("new fractal final\n");
	//free(average_fractal);
	printf("freed\n");
	//struct fractal *final = fractal_new("Final", 50, 50, -0.5, 0.4);
	printf("%d %d\n", w, h);
	for(x = 0; x < w; x++){
		for(y = 0; y < h; y++){
			fractal_compute_value(average_fractal,x,y);
		}

	}
	printf("Will crash ?\n");
	write_bitmap_sdl(average_fractal, output_file);
	printf("no \n");
	printf("freeing fractal\n");
	fractal_free(average_fractal);
	printf("freeing output\n");
	free(output_file);
	printf("freeing buffer\n");
	free(buffer);
	return 0;
}



int main(int argc, const char * argv[])
{
	printf("Execute now %s\n", argv[0]);

	if(handle_no_arguments(argc) < 0){
		return 0;	
	}
	args = parse_arguments(argc, argv);
	printf("%s\n", args->currentDirectory);	
	
	if(!initialize())
		return NULL;
	launch_threads();
	printf("Execution finished\n");
	return 0;
	
}
