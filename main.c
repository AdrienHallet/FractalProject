#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "libfractal/fractal.h"
#include "handleArgument.h"

#define EXIT_SIGNAL "SIGNAl_THREAD_EXIT"

struct fractal *(*buffer)[]; //Buffer, contains fractals pointers, size of max threads
struct fractal *average_fractal; //Pointer to final result
struct arguments *args; //arg structure, contains data for current computation

double* current_average;
int counter; //current buffered data
int thread_counter;

/*Mutexes*/
pthread_mutex_t buffer_access;
pthread_mutex_t thread_count;
pthread_mutex_t average_access;
/*Semaphores*/
sem_t full, empty;

const char *extension = ".bmp"; //generated image extension

/* debug_print_buffer : DEBUG, prints the current buffer
** 
** Prints the current buffer data on standard output, name and pointers.
** @a : number of fractals not to print (starting buffer's end)
*/
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
/* strsep : standard strsep implementation
** 
**
*/
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
	fflush(stdout);
	int thread;
	for(thread = 0; thread < args->maxThreads; thread++){
		sem_wait(&empty);
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

void *readFile(void *filename){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int iteration = 0;
	fp = fopen((char *)filename, "r");
	if (fp == NULL)
		pthread_exit(NULL);
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
	fflush(stdout);
	if(thread_counter==0)
		send_signals();
	pthread_mutex_unlock(&thread_count);
	fclose(fp);
        free(line);
	pthread_exit(NULL);
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

void *one_computation(struct fractal* fractal, double this_average){
	int w,h,x,y;
	w=fractal_get_width(fractal);
	h=fractal_get_height(fractal);
	char *filename = malloc(strlen(fractal_get_name(fractal))+strlen(extension)+strlen(args->currentDirectory)+1);
	strcpy(filename, args->currentDirectory);
	strcat(filename, fractal_get_name(fractal));
	strcat(filename, extension);
	double average =0.0;
	fflush(stdout);
	for(x = 0; x < w; x++){
		for(y = 0; y < h; y++){
			fractal_compute_value(fractal,x,y);
			average += fractal_get_value(fractal, x, y);
		}

	}
	average = average/(fractal_get_height(fractal) * fractal_get_width(fractal));
	pthread_mutex_lock(&average_access);
	printf("AVERAGE WAS : %f\n", this_average);
	printf("THIS ONE IS : %f\n", average);
	if(average>this_average){
		average_fractal = fractal_new(fractal_get_name(fractal),
					 fractal_get_width(fractal), 
					 fractal_get_height(fractal), 
					 fractal_get_a(fractal), 
					 fractal_get_b(fractal));

	}
	pthread_mutex_unlock(&average_access);
	if(args->allImages){
		if(access(filename, F_OK) != -1){ //if file exists
			fprintf(stderr, "Fractal %s already exists\n", fractal_get_name(fractal));
		}
		else{
			write_bitmap_sdl(fractal, filename);
			printf("C:Created %s\n", filename);
		}
		
	}
	
	free(filename);
}

void *compute()
{
	struct fractal **fractal;
	fractal = malloc(sizeof(struct fractal*));
	while(1) {
		sem_wait(&full);
		pthread_mutex_lock(&buffer_access);
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
			fflush(stdout);
			pthread_mutex_unlock(&buffer_access);
			int w,h,x,y;
			w=fractal_get_width(*fractal);
			h=fractal_get_height(*fractal);
			char *filename = malloc(strlen(fractal_get_name(*fractal))+strlen(extension)+strlen(args->currentDirectory)+1);
			strcpy(filename, args->currentDirectory);
			strcat(filename, fractal_get_name(*fractal));
			strcat(filename, extension);
			double average = 0.0;
			fflush(stdout);
			for(x = 0; x < w; x++){
				for(y = 0; y < h; y++){
					fractal_compute_value(*fractal,x,y);
					average += fractal_get_value(*fractal, x, y);
				}

			}
			average = average/(fractal_get_height(*fractal) * fractal_get_width(*fractal));
			pthread_mutex_lock(&average_access);
			printf("%f\n", average);
			if(average>*current_average){
				average_fractal = fractal_new(fractal_get_name(*fractal),
							 fractal_get_width(*fractal), 
							 fractal_get_height(*fractal), 
							 fractal_get_a(*fractal), 
							 fractal_get_b(*fractal));
				*current_average = average;

			}
			pthread_mutex_unlock(&average_access);
			if(args->allImages){
				if(access(filename, F_OK) != -1){ //if file exists
					fprintf(stderr, "File %s already exists\n");
				}
				else{
					write_bitmap_sdl(*fractal, filename);
				}
				printf("C:Created %s\n", filename);
			}
			printf("new average is %f\n", *current_average);
			free(filename);
		}
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
	double zero = 0.0;
	current_average = malloc(sizeof(double));
	current_average = &zero;
	average_fractal = malloc(sizeof(struct fractal*));
	//struct fractal *fbuffer = malloc(maxThreads * sizeof(struct fractal));
	//buffer = &fbuffer;	
	
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
	
	free(buffer);
	return 0;
}

void *compute_average(){
	char *output_file = malloc(strlen(args->outputFile)+strlen(args->currentDirectory)+strlen(extension)+1);
	strcpy(output_file, args->currentDirectory);
	strcat(output_file, args->outputFile);
	strcat(output_file, extension);
	int x,y;
	int w = fractal_get_width(average_fractal);
	int h = fractal_get_height(average_fractal);

	printf("Our max average fractal is %s\n", fractal_get_name(average_fractal));

	for(x = 0; x < w; x++){
		for(y = 0; y < h; y++){
			fractal_compute_value(average_fractal,x,y);
		}

	}
	write_bitmap_sdl(average_fractal, output_file);
	fractal_free(average_fractal);
	free(output_file);
}

int main(int argc, const char * argv[])
{
	printf("Execute now %s\n", argv[0]);

	if(handle_no_arguments(argc) < 0){
		return 0;	
	}
	args = parse_arguments(argc, argv);
	
	if(!initialize()){
		fprintf(stderr, "Could not initialize the application\n");
		exit(0);
	}
	launch_threads();
	double avg = *current_average;
	if(args->needInput){
		char *input;
		size_t size = (2*sizeof(int)+2*sizeof(double)+68*sizeof(char));
		fprintf(stdout, "Please, write a fractal as : NAME WIDTH(px) HEIGHT(px) REAL_PART IMAGINARY_PART\n");
		if(fgets(input, size, stdin) != NULL){
			printf("%d\n", avg);
			struct fractal* input_fractal = fractal_parse(input);
			one_computation(input_fractal, avg);
			fractal_free(input_fractal);
			
		}
	}
	compute_average();
	printf("Execution finished\n");
	return 0;
}
