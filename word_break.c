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

void print_buffer(char local_buffer_arr[BUF_SIZE], int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%c", local_buffer_arr[i]);
    }
    printf("\n");
}

void wrap_text(char *file_name, int max_width, char local_buffer_arr[BUF_SIZE])
{
    int fd_read;
    fd_read = open(file_name, O_RDWR | O_CREAT, DEF_MODE);

    int fd_write;
    fd_write = open("wrap_out2", O_RDWR | O_CREAT | O_TRUNC, DEF_MODE);

    if (fd_read == -1 || fd_write == -1)
    {
        perror("File Open Error");
    }

    ssize_t read_bytes_till_now;
    int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
    char c = '\0';
    char prev_c = '\0';
    int finishing_max_width = max_width;

    // TODO: Selin, dynamically allocate local_buffer_arr because we don't know word length.
    // (Maybe use malloc (for init allocation) and realloc for increasing the buffer size dynamically if the init size is not sufficient.

    // read "a" word into buffer and flush buffer after a word has been read.
    while ((read_bytes_till_now = read(fd_read, &c, 1)) != 0)
    {
        // prev_c != ' ' && c == ' ' is for "is ". Then "is" is a word.
        // prev_c != '\n' && c == '\n' is for "is\n". Then "is" is a word.
        // prev_c != ' ' && c != ' ' is for "  ". Consecutive spaces cannot be a word
        // prev_c != '\n' && c != '\n' is for "\n\n". Consecutive nextline cannot be a word
        if (prev_c == '\n' && c == '\n')
        {
            // DONE: Fix issue regarding adding extra line break. For example:
            /**
             * @brief Earlier problem with having breaks (Now this issue has been fixed.)
             *
             * UNWRAPPED ORIGINAL test4.txt
             *  Rohan likes wine. Selin likes beer.

                But Selin can't buy neither!

             * WRAPED WITH 30 (covering only part of the width (i.e. beer.))
             *  Rohan likes wine. Selin likes
                beer.
                But Selin can't buy
                neither!

             * WRAPED WITH 20 (covering full width (i.e. Selin likes beer.))
             *  Rohan likes wine.
                Selin likes beer.

                But Selin can't buy
                neither!

            * WITH FIX
            * WRAPED WITH 30 (covering only part of the width (i.e. beer.))
            *   Rohan likes wine. Selin likes
                beer.

                But Selin can't buy
                neither!
             */
            if (finishing_max_width <= 2)
            {
                // Assuming we write the sentence covering the whole width (i.e. the remaining width can be 2 or less.)
                write(fd_write, "\n", 1);
            }
            else
            {
                // Assuming we write the sentence covering only part of the width
                write(fd_write, "\n\n", 2);
            }
        }
        else if ((prev_c != ' ' && c == ' ') || (prev_c != '\n' && c == '\n'))
        {

            // From write-up, "no more than one space occurs between words". Well... trim the excess fat :)
            // if buffer has at least one non-white space character then we can assume buffer
            // is not "completely" made of white spaces
            // Notice: This will only occur if a line "starts with" a sequence of white space characters
            bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;

            if (!whole_buffer_has_only_white_spaces)
            {
                // greedy word-wrap algorithm

                //  Before printing a word, check whether it will fit on the current line.
                int adjusted_word_length_with_space = alpha_numeric_count + 1;
                if (adjusted_word_length_with_space <= finishing_max_width)
                {
                    local_buffer_arr[alpha_numeric_count] = ' ';
                    write(fd_write, local_buffer_arr, adjusted_word_length_with_space);
                    finishing_max_width -= adjusted_word_length_with_space;
                }
                else
                {
                    local_buffer_arr[alpha_numeric_count] = ' ';
                    write(fd_write, "\n", 1);
                    write(fd_write, local_buffer_arr, adjusted_word_length_with_space);
                    finishing_max_width = max_width - adjusted_word_length_with_space;
                }
            }
            alpha_numeric_count = 0;
        }
        else if (read_bytes_till_now == 1)
        {
            bool current_char_is_alphanumeric = c != ' ' && c != '\n';
            if (current_char_is_alphanumeric)
            {
                local_buffer_arr[alpha_numeric_count] = c;
                printf("Read bytes in buf[%d]: %c\n", alpha_numeric_count, local_buffer_arr[alpha_numeric_count]);
                alpha_numeric_count += 1;
            }
        }
        else if (read_bytes_till_now == -1)
        {
            // handle read error.
            perror("Read Error:");
            break;
        }
        prev_c = c;
    }
    // remaining buffer. greedy addition to the text.
    if (alpha_numeric_count != 0)
    {
        if (alpha_numeric_count <= finishing_max_width)
        {
            write(fd_write, local_buffer_arr, alpha_numeric_count);
        }
        else
        {
            write(fd_write, "\n", 1);
            write(fd_write, local_buffer_arr, alpha_numeric_count);
        }
        alpha_numeric_count = 0;
    }
    close(fd_read);
    close(fd_write);
}

int main(int argv, char **argc)
{
    // init buffer with buffer array
    char local_buffer_arr[BUF_SIZE];
    memset(local_buffer_arr, 0, BUF_SIZE);

    wrap_text("tests/test4.txt", 30, local_buffer_arr);
}