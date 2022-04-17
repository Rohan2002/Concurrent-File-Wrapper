CC         = gcc
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -pthread -Werror $(SANITIZERS)

all: bin/pool_test bin/utils_test

bin/utils_test: bin/ utils_test.c
	$(CC) $(CFLAGS) utils_test.c -o $@

bin/pool_test: bin/ pool_test.c
	$(CC) $(CFLAGS) pool_test.c ../obj/pool.o -o $@

bin/:
	mkdir -p $@

clean:
	rm -rf bin/ 
