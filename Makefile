CC         = gcc
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -pthread -Werror $(SANITIZERS)

all: bin/wcheck bin/word_break

bin/wcheck: obj/ bin/ obj/wcheck.o
	$(CC) $(CFLAGS)  obj/wcheck.o -o $@

bin/word_break: obj/ bin/ obj/pool.o obj/utils.o obj/queue.o obj/word_break.o
	$(CC) $(CFLAGS) obj/pool.o obj/utils.o obj/queue.o obj/word_break.o -o $@

obj/word_break.o: obj/ src/pool.c src/pool.h src/utils.c src/utils.h src/queue.c src/queue.h src/word_break.c src/word_break.h
	$(CC) $(CFLAGS) src/word_break.c -c -o $@

obj/wcheck.o: obj/ src/wcheck.c
	$(CC) $(CFLAGS) src/wcheck.c -c -o $@

obj/queue.o: obj/ src/queue.c src/queue.h
	$(CC) $(CFLAGS) src/queue.c -c -o $@

obj/utils.o: obj/ src/utils.c src/utils.h
	$(CC) $(CFLAGS) src/utils.c -c -o $@

obj/pool.o: obj/ src/pool.c src/pool.h
	$(CC) $(CFLAGS) src/pool.c -c -o $@

bin/:
	mkdir -p $@

obj/:
	mkdir -p $@

fresh:
	find $(file) -type f -name "wrap*" -exec rm -rf {} \;

clean:
	rm -rf bin/ obj/
