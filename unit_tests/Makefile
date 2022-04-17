CC         = gcc
SANITIZERS = -fsanitize=address,undefined
CFLAGS     = -g -std=c99 -Wall -Wvla -pthread $(SANITIZERS)

all: bin/pool_test bin/utils_test

bin/utils_test: bin/ ../obj/utils.o utils_test.c
	$(CC) $(CFLAGS) ../obj/utils.o utils_test.c -o $@

bin/pool_test: bin/ ../obj/pool.o pool_test.c
	$(CC) $(CFLAGS) pool_test.c ../obj/pool.o -o $@

bin/:
	mkdir -p $@

clean:
	rm -rf bin/ 
