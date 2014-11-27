# Procedural texture generator project
# Miloslav Ciz, 2012

CC=gcc
CC2=c++
CFLAGS=-std=c99 -g -pedantic -Wall -Wextra
CFLAGS2=-Wall -pedantic -g -std=c++0x
SOURCEDIR=src

all: main.o colorbuffer.o lodepng.o perlin.o raytracing.o
	$(CC2) $(CFLAGS2) -lm -o skygen.exe main.o colorbuffer.o lodepng.o perlin.o raytracing.o

main.o: $(SOURCEDIR)/main.cpp
	$(CC) $(CFLAGS) -c -o main.o $(SOURCEDIR)/main.cpp

lodepng.o: $(SOURCEDIR)/lodepng.c $(SOURCEDIR)/lodepng.h
	$(CC) $(CFLAGS) -c -o lodepng.o $(SOURCEDIR)/lodepng.c

colorbuffer.o: $(SOURCEDIR)/colorbuffer.c $(SOURCEDIR)/colorbuffer.h $(SOURCEDIR)/lodepng.h
	$(CC) $(CFLAGS) -c -o colorbuffer.o $(SOURCEDIR)/colorbuffer.c

perlin.o: $(SOURCEDIR)/perlin.c $(SOURCEDIR)/perlin.h
	$(CC) $(CFLAGS) -c -o perlin.o $(SOURCEDIR)/perlin.c

raytracing.o: $(SOURCEDIR)/raytracing.cpp
	$(CC) $(CFLAGS) -c -o raytracing.o $(SOURCEDIR)/raytracing.cpp

