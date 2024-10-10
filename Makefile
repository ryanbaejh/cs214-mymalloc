# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -g

# Default target
all: memgrind

# Build memgrind executable
memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o memgrind memgrind.o mymalloc.o

# Compile memgrind.c into memgrind.o
memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c

# Compile mymalloc.c into mymalloc.o
mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c

# Clean up generated files
clean:
	rm -f *.o memgrind
