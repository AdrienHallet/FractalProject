CC=gcc
AR=ar
CFLAGS=-g -Wall -W -L ../libfractal -lfractal.a
LDFLAGS=
SRC=main.c
OBJ=$(SRC:.c=.o)
LIBFILE=libfractal.a

all: test

test: main.o errorHandling.o
	gcc -Wall -o main main.o errorHandling.o -L/home/osboxes/Documents/Fractales/libfractal -lfractal -I/usr/include/SDL -lSDLmain -lSDL

main.o: main.c
	gcc -Wall -c main.c -I../libfractal

errorHandling.o : errorHandling.c
	gcc -Wall -c errorHandling.c

lib:
	cd libfractal && $(MAKE)

clean:
	rm -rf *.o
	rm -rf *.a

