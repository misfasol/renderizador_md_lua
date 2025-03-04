build:
	gcc -o main main.c -L./lua_src -llua -L./raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -Wall -Werror
	
run: build
	./main

elem:
	gcc -o elem elem.c -Wall -Werror -L./raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
	./elem