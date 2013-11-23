CC=clang
LDFLAGS=-lSDL2 -lSDL2_mixer -lSDL2_image -llua


all: luna.so

luna.so: luna.c
	$(CC) -Wall -shared -o luna.so -fPIC luna.c $(LDFLAGS)

