CC 		   = gcc
TARGET 	   = word_break
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -Werror $(SANITIZERS)

bin/word_break_bin: obj/work_break.o
	$(CC) obj/work_break.o -o $@

obj/work_break.o: src/word_break.c src/word_break.h
	$(CC) $(CFLAGS) src/word_break.c -c -o $@

clean:
	rm -rf bin/* obj/*