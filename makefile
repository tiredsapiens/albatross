
CFLAGS=-Wall -Wextra -ggdb
LIBS=-L./build/ -lraylib -lm -lpthread -ldl 
TARGET = build/albatross

execute: build_shared build run

$(TARGET):source/main.c source/plug.h source/plug.c
	clear && clang $(CFLAGS) -o build/albatross source/main.c  $(LIBS)
run:
	./build/albatross
clear:
	rm ./build/albatross

clear_all:
	rm ./build/*
build_shared:
	clang $(CFLAGS) -fPIC -shared  -o build/libplug.so  source/plug.c $(LIBS)
.PHONY: build
build : $(TARGET) build_shared
