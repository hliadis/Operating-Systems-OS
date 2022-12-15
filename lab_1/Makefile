CC = gcc
CFLAGS = -Wall 
OBJECTS = find_roots_lib

all: $(OBJECTS)
	@echo Compilation terminated successfully!


roots.o: roots.c roots.h
	$(CC) $(CFLAGS) -c roots.c


libroots.a: roots.o
	@echo Creating static library libroots.a
	ar -rcs libroots.a *.o


find_roots_lib: find_roots_lib.c libroots.a
	$(CC) $(CFLAGS) find_roots_lib.c -o find_roots_lib  -L. libroots.a


clean:
	rm *.o *.a $(OBJECTS)
