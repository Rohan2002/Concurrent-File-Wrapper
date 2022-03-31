TARGET 	   = word_break
SANITIZERS = -fsanitize=address -fsanitize=undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -Werror $(SANITIZERS) $(OPT)

build: 
	gcc $(CFLAGS) word_break.c -o $(TARGET)

fbytes:
	wc -c $(file)

clean:
	rm -r $(TARGET) *.o *.a *.dylib *.dSYM