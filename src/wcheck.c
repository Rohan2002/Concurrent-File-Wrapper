/*
    @ project
    Word Break 2022.

    @ authors
    Professor David Menendez
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>


#define BUFSIZE 256

#ifndef DUMP
#define DUMP 0
#endif

enum state { ANY, NOSP, WORD };

int main(int argc, char **argv)
{
	char buf[BUFSIZE];
	int input;
	int bytes, width, line = 0, col = 0, pos = 0, i, rc = 0;
	enum state next = WORD;
	
	if (argc < 2 || argc > 3) {
		fprintf(stderr, "Usage: %s <width> [file]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	if (argc == 3) {
		input = open(argv[2], O_RDONLY);
		if (input == -1) {
			perror(argv[2]);
			return EXIT_FAILURE;
		}
	} else {
		input = 0;
	}
	
	width = atoi(argv[1]);
	if (width < 1) {
		fprintf(stderr, "Invalid width: %d\n", width);
		return EXIT_FAILURE;
	}
	bool read_at_least_one_alpha_numeric = false;
	while ((bytes = read(input, buf, BUFSIZE)) > 0) {
		for (i = 0; i < bytes; i++, pos++, col++) {
 			if (DUMP) printf("[%10d/%3d:%3d] %02x\n", pos, line, col, buf[i]);
			if (isspace(buf[i])) {
				read_at_least_one_alpha_numeric = true;
				if (buf[i] == ' ') {
					if (next != ANY) {
						fprintf(stderr, "Unexpected space at %d [%d:%d]\n", pos, line, col);
						rc = 1;
					}
					next = WORD;
				} else if (buf[i] == '\n') {
					if (col > width) {
						fprintf(stderr, "Line %d too long: %d\n", line, col);
					}
					if (next == WORD) {
						fprintf(stderr, "Unexpected newline at %d [%d:%d]\n", pos, line, col);
						rc = 1;
					} else if (next == NOSP) {
						next = WORD;
					} else {
						next = NOSP;
					}
					col = -1;
					++line;
				}
			} else {
				next = ANY;
			}
		}
	}
	if (bytes == -1) {
		perror("read");
		return EXIT_FAILURE;
	}
	if (col > width) {
		fprintf(stderr, "Line %d too long: %d\n", line, col);
	}
	if (next != NOSP && read_at_least_one_alpha_numeric) {
		fprintf(stderr, "Input did not end with newline\n");
		rc = 1;
	}
	
	if (close(input) == -1) {
		perror("close");
		rc = 1;
	}
	
	return rc;
}