#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libfractal/fractal.h"

int main(int argc, const char * argv[])
{
/*
	struct fractal *myfrac = fractal_new("Test 1", 800, 800, -0.52, 0.57);
	int w,h,x,y;
	for(x = 0; x < 800; x++){
		for(y = 0; y < 800; y++){
			fractal_compute_value(myfrac,x,y);
		}
		
	}
	write_bitmap_sdl(myfrac, "Image.bmp");

    	return 0;
*/

	int argCount = 1; //Defaullt value when no args given
	if(argCount==argc){
		printf("Error : misusing the application\n");
	}
}
