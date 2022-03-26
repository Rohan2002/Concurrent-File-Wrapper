CC         = clang
SANITIZERS = -fsanitize=address $(if $(findstring clang,$(CC)),-fsanitize=undefined)
OPT        =
CFLAGS     = -g -std=c99 -Wall -Wvla -Werror $(SANITIZERS) $(OPT)

build: 
	gcc $(CFLAGS) word_break.c -o word_break.o

fbytes:
	wc -c $(file)