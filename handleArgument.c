#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "handleArgument.h"

int handle_no_arguments(int argc){
	if(argc < 3){ //We need at last one input and one output
		printf("Error : misusing the application\n");
		printf("   The correct usage would be :\n");
		printf("   ./main [-d] [--maxthreads n] inputFile [inputFile ..] [-] outputFile\n");
		return -1;
	}
	return 0; //I'm fine
}

struct arguments *parse_arguments(int argc, const char* argv[]){
	struct arguments* args = malloc(sizeof(struct arguments));
	if(args==NULL) return NULL;
	
	char cwd[1024];	

	//Output file
	args->outputFile = (char*)malloc(sizeof(argv[argc-1]));
	if(args->outputFile ==NULL) return NULL;
	strcpy(args->outputFile, argv[argc-1]);

	//Path to current directory
	
   	if (getcwd(cwd, sizeof(cwd)) != NULL){
		args->currentDirectory = malloc(sizeof(cwd));
		strcpy(args->currentDirectory, cwd);
		char* separator = "/";
		strcat(args->currentDirectory, separator);
	}	
   	else{
       		fprintf(stderr, "Failed current directory retrieval\n");
		free(args);
		exit(0);
	}

	int i;
	int input = -1;
	for(i = 1; i < argc; i++){
		if(argv[i][0] != '-')
			input++;
	}
	char *in[input];
	for(i = 0; i<input; i++){
		in[i] = malloc(sizeof(cwd));
	}
	args->inputFiles = in;
	args->inputCount = input;
	input = 0;

	args->allImages = 0;
	args->maxThreads = 10;
	args->needInput = 0;
	int interpreted = 0;
	for(i = 1; i < argc-1; i++){
		if(argv[i][0] != '-'){
			strcpy(args->inputFiles[input], argv[i]);
			input++;
			interpreted++;
		}else if(argv[i][1] == 'd'){
			args->allImages = 1;
		}
		else if(argv[i][1] == '-'){
			const char* thread;
			thread = argv[i];
			args->maxThreads = (int) atoi(thread+2);
		}
		else{
			interpreted++;
		}
			
	}
	args->inputCount = input;

	if(interpreted!=args->inputCount)
		args->needInput = 1;
	else
		args->needInput = 0;

	return args;
}

int free_arguments(struct arguments* args){
	free(args->outputFile);	
	free(args->currentDirectory);
	int i;
	for(i=0; i < args->inputCount; i++){
		free(args->inputFiles[i]);
	}
	return 1;
}
