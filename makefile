execute:
	clear && clang -Wall -Wextra -o build/albatross source/main.c -lraylib -lm -lpthread -ldl && ./build/albatross
build:
	clear && clang -Wall -Wextra -o build/albatross source/main.c -lraylib -lm -lpthread -ldl
run:
	./build/albatross
clear:
	rm ./build/albatross

clear_all:
	rm ./build/*
