TARGET 	   = word_break
CC         = clang
SANITIZERS = -fsanitize=address $(if $(findstring clang,$(CC)),-fsanitize=undefined)
OPT        =
CFLAGS     = -g -std=c99 -Wall -Wvla -Werror $(SANITIZERS) $(OPT)

build: 
	gcc $(CFLAGS) word_break.c -o $(TARGET)

fbytes:
	wc -c $(file)

clean:
	rm -r $(TARGET) *.o *.a *.dylib *.dSYM