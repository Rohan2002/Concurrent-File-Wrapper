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
#include <stdbool.h>

void print_buffer(buffer_dtype *buffer)
{
    for (int i = 0; i < BUF_SIZE; i++)
    {
        printf("From buffer: %s\n", buffer->buffer_arr[i]);
    }
    printf("Size of buffer: %d\n", buffer->bytes_in_buffer);
}

void wrap_text(char *file_name, int width, buffer_dtype *buffer)
{
    int fd;
    fd = open(file_name, O_RDWR | O_CREAT, DEF_MODE);

    int fd_write;
    fd_write = open("wrap_out", O_RDWR | O_CREAT | O_TRUNC, DEF_MODE);
    if (fd == -1 || fd_write == -1)
    {
        perror("File Open Error");
    }

    ssize_t read_bytes_till_now;
    int write_start_position = 0;

    int alpha_numeric_count = 0;
    char c = '\0';
    char prev_c = '\0';
    // dynamically allocate buffer->buffer_arr maybe? because we don't know word length.

    // read "a" word into buffer and flush buffer after a word has been read.
    while (read_bytes_till_now = read(fd, &c, 1) != 0)
    {   
        // prev_c != ' ' && c == ' ' is for "is ". Then "is" is a word.
        // prev_c != '\n' && c == '\n' is for "is\n". Then "is" is a word.
        // prev_c != ' ' && c != ' ' is for "  ". Consecutive spaces cannot be a word
        // prev_c != '\n' && c != '\n' is for "\n\n". Consecutive nextline cannot be a word
        if ((prev_c != ' ' && c == ' ') || (prev_c != '\n' && c == '\n'))
        {
            // if(prev_c == ' ' && c == ' '){
                
            // }
            // if(prev_c == '\n' && c == '\n'){

            // }
            // From write-up, "no more than one space occurs between words". Well... trim the excess fat :)
            // if buffer has at least one non-white space character then we can assume buffer 
            // is not "completely" made of white spaces
            // Notice: This will only occur if a line "starts with" a sequence of white space characters
            bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;
            for (int i = 0; i < alpha_numeric_count; i++)
            {   
                printf("%c", buffer->buffer_arr[i]);
            }
            // printf("\nprev_c: %c and c: %c\n",prev_c, c);
            if (!whole_buffer_has_only_white_spaces){
                printf("\nWord formed!\n");
            }
            else{
                printf("count: %d\n", alpha_numeric_count);
            }
            alpha_numeric_count = 0;
        }
        else if (read_bytes_till_now == 1)
        {   
            bool current_char_is_alphanumeric = c != ' ' && c != '\n';
            if (current_char_is_alphanumeric)
            {
                buffer->buffer_arr[alpha_numeric_count] = c;
                printf("Read bytes in buf[%d]: %c\n",alpha_numeric_count, buffer->buffer_arr[alpha_numeric_count]);
                alpha_numeric_count += 1;
            }
        }
        else
        {
            // handle read error.
        }
        prev_c = c;
    }
    // while(1){
    //     // refill the buffer only when we have written all the the bytes_in_buffer to file.
    //     if (buffer->bytes_in_buffer <= 0){
    //         // Read one character at a time until we reach a empty space \0.
    //         while(buffer->buffer_arr != '\0'){
    //             read_bytes_till_now = read(fd, buffer->buffer_arr, 1);
    //             if(read_bytes_till_now == 1){

    //             }
    //         }
    //         // buffer->bytes_in_buffer = read_bytes_till_now;
    //         // if (read_bytes_till_now == 0){
    //         //     // EOF
    //         //     break;
    //         // }
    //     }

    //     if (count == 2){
    //         break;
    //     }
    //     buffer->buffer_arr[4] = '!'; // mocking future parse buffer function.

    //     lseek(fd, write_start_position, SEEK_SET); // move cursor back to the start of the string so that old string gets replaced by string in buffer.
    //     write(fd_write, buffer->buffer_arr, buffer->bytes_in_buffer);
    //     write_start_position += buffer->bytes_in_buffer;
    //     lseek(fd, write_start_position, SEEK_SET); // move cursor back to the end of the string.

    //     buffer->bytes_in_buffer -= replaced_bytes_till_now;
    //     count +=1;
    // }
    close(fd);
}

int main(int argv, char **argc)
{
    // create a buffer type.
    buffer_dtype *buffer = (buffer_dtype *)malloc(sizeof(buffer_dtype *));

    // init buffer with buffer array
    char local_buffer_arr[BUF_SIZE];
    memset(local_buffer_arr, 0, BUF_SIZE);
    buffer->buffer_arr = local_buffer_arr;

    // init count to 0
    buffer->bytes_in_buffer = 0;
    wrap_text("tests/test2.txt", 80, buffer);
}