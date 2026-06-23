
CFLAGS=-Wall -Wextra -ggdb
LIBS=-L./build/ -lraylib -lm -lpthread -ldl 
TARGET = build/albatross

execute: build_shared build run
song ?= ./audio/celestial_symphony.mp3
hr ?=1

$(TARGET):source/main.c source/plug.h source/plug.c
	ifeq($(hr),0)
		clear && clang $(CFLAGS) -o build/albatross source/main.c source/plug.c $(LIBS)
	else
		clear && clang $(CFLAGS)-DHOTRELOAD -o build/albatross  source/plug.c $(LIBS)
run:
	./build/albatross $(song)
clear:
	rm ./build/albatross

clear_all:
	rm ./build/*
build_shared:
	clang $(CFLAGS) -fPIC -shared  -o build/libplug.so  source/plug.c $(LIBS)
.PHONY: build
build : $(TARGET) build_shared
