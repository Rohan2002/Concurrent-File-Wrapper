/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "word_break.h"
#include <string.h>

void print_buffer(buffer_dtype* buffer){
    for(int i =0; i < BUF_SIZE; i++){
        printf("From buffer: %c\n", buffer->buffer_arr[i]);
    }
    printf("Size of buffer: %d\n", buffer->bytes_in_buffer);
}

void wrap_text(char* file_name, int width, buffer_dtype* buffer){
    int fd;
    fd = open(file_name, O_RDWR | O_CREAT, DEF_MODE);
    if (fd == -1)
    {
        perror("File Open Error");
        printf("fd: %d\n", fd);
    }
    
    ssize_t read_bytes_till_now;
    int replaced_bytes_till_now = 0;

    while(1){
        if (buffer->bytes_in_buffer <= 0){
            // refill the buffer only when we have written all the the bytes_in_buffer to file.
            read_bytes_till_now = read(fd, buffer->buffer_arr, BUF_SIZE); // TODO: check for errors.
            buffer->bytes_in_buffer = read_bytes_till_now;
        }
        
        buffer->buffer_arr[4] = '!'; // mocking future parse buffer function.
        
        lseek(fd, replaced_bytes_till_now, SEEK_SET);
        write(fd, buffer->buffer_arr, buffer->bytes_in_buffer);
        
        replaced_bytes_till_now += buffer->bytes_in_buffer;
        break;
    }
    close(fd);
}

int main(int argv, char **argc)
{
    // create a buffer type.
    buffer_dtype* buffer = (buffer_dtype*) malloc(sizeof(buffer_dtype*));

    // init buffer with buffer array
    char local_buffer_arr[BUF_SIZE];
    memset(local_buffer_arr, 0, BUF_SIZE);
    buffer->buffer_arr = local_buffer_arr;

    // init count to 0
    buffer->bytes_in_buffer = 0;

    wrap_text("tests/test1.txt", 80, buffer);
}