#ifndef _HANDLEARGUMENT_H
#define _HANDLEARGUMENT_H
struct arguments
{
	int* needInput;
	int* allImages;
	int* maxThreads;
	int* inputCount;
	char** inputFiles;
	char* outputFile;
	char* currentDirectory;
};

int handle_no_arguments(int argc);

struct arguments *parse_arguments(int argc, const char* argv[]);

int free_arguments(struct arguments *args);
#endif
