# -*- Makefile -*-

all: model_loading

model_loading: model_loading.o model.o mesh.o dynamic_array.o glad.o
	gcc -o model_loading model_loading.o model.o mesh.o dynamic_array.o glad.o -lglfw -lm -ldl -lassimp

model_loading.o: model_loading.c model.h
	gcc -c model_loading.c

model.o: model.c model.h mesh.h
	gcc -c model.c

mesh.o: mesh.c mesh.h dynamic_array.h
	gcc -c mesh.c

dynamic_array.o: dynamic_array.c dynamic_array.h
	gcc -c dynamic_array.c
	
glad.o: glad.c
	gcc -c glad.c

clear:
	rm *.o model_loading
