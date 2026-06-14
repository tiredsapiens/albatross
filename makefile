execute:
	clear && clang -Wall -Wextra -o albatross main.c -lraylib -lm -lpthread -ldl && ./albatross
build:
	clear && clang -Wall -Wextra -o albatross main.c -lraylib -lm -lpthread -ldl 
run:
	./albatross
clear:
	rm ./albatross

