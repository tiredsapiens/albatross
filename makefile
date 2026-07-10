
CFLAGS=-Wall -Wextra -ggdb -I/opt/homebrew/opt/raylib/include
LIBS=-L./build/ -lraylib -lm -lpthread -ldl -L/opt/homebrew/opt/raylib/lib
TARGET = build/albatross
ifeq ($(UNAME_S),Darwin)
    # macOS
    CFLAGS += -I/opt/homebrew/opt/raylib/include
    LDFLAGS += -L/opt/homebrew/opt/raylib/lib
endif
execute: build_shared build run
hr ?=1

$(TARGET):source/main.c source/plug.h source/plug.c
ifeq ($(hr),0)
	clear
	clang $(CFLAGS) -o build/albatross source/main.c source/plug.c $(LIBS)
else
	clear
	clang $(CFLAGS) -DHOTRELOAD -o build/albatross source/main.c $(LIBS)
endif
run:
	./build/albatross $(song)
clear:
	rm ./build/albatross

clear_all:
	rm ./build/*
build_shared:
	clang $(CFLAGS) -fPIC -shared  -o build/libplug.so  source/plug.c $(LIBS)
.PHONY: build
build : build_shared $(TARGET) 
ifeq ($(hr),0)
	@echo "---No hot reloading, statically linking plug.c---"
else
	@echo "---Hot reloading enabled---"
endif
