CC=gcc
AR=ar
CFLAGS=-g -Wall -W -L ../libfractal -lfractal.a
LDFLAGS=
SRC=main.c
OBJ=$(SRC:.c=.o)
LIBFILE=libfractal.a

all: test

test: main.o handleArgument.o
	gcc -Wall -o main main.o handleArgument.o -L/home/osboxes/Documents/Fractales/libfractal -lfractal -I/usr/include/SDL -lSDLmain -lSDL -pthread

main.o: main.c
	gcc -Wall -c main.c -I../libfractal

handleArgument.o : handleArgument.c
	gcc -Wall -c handleArgument.c

lib:
	cd libfractal && $(MAKE)
libclean:
	cd libfractal && rm -rf *.o && rm -rf *.a

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.bmp

