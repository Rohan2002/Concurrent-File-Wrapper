CC         = gcc
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -pthread -Werror $(SANITIZERS)

all: bin/pool_test bin/utils_test

bin/utils_test: bin/ ../obj/utils.o utils_test.c
	$(CC) $(CFLAGS) ../obj/utils.o utils_test.c -o $@

bin/pool_test: bin/ ../src/logger.h ../obj/pool.o pool_test.c
	$(CC) $(CFLAGS) ../obj/pool.o pool_test.c -o $@

bin/:
	mkdir -p $@

clean:
	rm -rf bin/ 
