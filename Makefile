CC         = gcc
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -Werror $(SANITIZERS)

all: bin/wcheck bin/word_break

bin/wcheck: obj/wcheck.o
	$(CC) $(CFLAGS)  obj/wcheck.o -o $@

bin/word_break: obj/work_break.o
	$(CC) $(CFLAGS)  obj/work_break.o -o $@

obj/work_break.o: src/word_break.c src/word_break.h
	$(CC) $(CFLAGS) src/word_break.c -c -o $@

obj/wcheck.o: src/wcheck.c
	$(CC) $(CFLAGS) src/wcheck.c -c -o $@

clean:
	rm -rf bin/* obj/*
