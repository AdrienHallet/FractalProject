#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libfractal/fractal.h"
#include "errorHandling.h"

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
	readFile();
	
}

struct fractal *fractal_parse(char *line){
	printf("%s\n", line);
	const char *delim = " ";
	char **endptr;
	long value;
	//printf("%s\n",strsep(&line, delim));
	char *name = strsep(&line, delim);
	printf("strsep success\n");
	int width = (int) strtol(strsep(&line, delim), endptr, 10);
	int height = (int) strtol(strsep(&line, delim), endptr, 10);
	double a = (double) strtol(strsep(&line, delim), endptr, 10);
	double b= (double) strtol(strsep(&line, delim), endptr, 10);
	printf("Reached\n");
	struct fractal *newfractal = fractal_new(name, width, height, a, b);

	
}

int readFile(){
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("inputFile", "r");
	if (fp == NULL)
	exit(EXIT_FAILURE);
	while ((read = getline(&line, &len, fp)) != -1) {
		if(read>1 && line[0]!='#'){ //ignore empty lines and comments
		struct fractal *myfrac = fractal_parse(line);
			int w,h,x,y;
			for(x = 0; x < 800; x++){
				for(y = 0; y < 800; y++){
					fractal_compute_value(myfrac,x,y);
				}

			}
			write_bitmap_sdl(myfrac, "Image.bmp");
		}
	}
	free(line);
	free(fp);
	exit(EXIT_SUCCESS);
}
