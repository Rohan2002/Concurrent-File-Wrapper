CFLAGS     = -std=c99 -Wall

build: 
	gcc $(CFLAGS) word_break.c -o word_break.o

fbytes:
	wc -c $(file)

clean:
	rm -rf *.o