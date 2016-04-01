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

	if(handle_no_arguments(argc) < 0){
		return 0;	
	}
}
