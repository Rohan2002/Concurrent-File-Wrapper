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

void print_buffer(char *word_buffer, int length)
{
    if (word_buffer == NULL)
    {
        printf("Buffer is not init!\n");
    }
    for (int i = 0; i < length; i++)
    {
        printf("%c", word_buffer[i]);
    }
    printf("\n");
}
int wrap_text(char *file_name, int max_width, char word_buffer[BUFSIZE])
{
    int fd_read;
    int rtn = 0;
    fd_read = open(file_name, O_RDWR | O_CREAT, DEF_MODE);

    int fd_write;
    fd_write = open("wrap_out", O_RDWR | O_CREAT | O_TRUNC, DEF_MODE);

    if (fd_read == -1 || fd_write == -1)
    {
        perror("File Open Error");
    }

    int bytes;
    char sentence[BUFSIZE];
    int pos;

    int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
    char c = '\0';
    char prev_c = '\0';
    int finishing_max_width = max_width;

    // int first_word=1;
    word_buffer = NULL;
    int next_line_characters = 0;
    while ((bytes = read(fd_read, sentence, BUFSIZE)) > 0)
    {
        // read buffer and break file into lines
        for (pos = 0; pos < bytes; pos++)
        {
            c = sentence[pos];
            if (c == '\n')
            {
                next_line_characters += 1;
            }
            if (c != ' ' && c != '\n')
            {
                // cursor landed on a alphanumeric character...
                // no more new lines.
                // printf("New line characters %d\n", next_line_characters);
                if (next_line_characters >= 2)
                {
                    write(fd_write, "\n\n", 2);
                    finishing_max_width = max_width;
                }
                // printf("Final new line count %d\n", next_line_characters);
                next_line_characters = 0;
                // printf("non-next line: %c\n", c);
            }
            if (prev_c != ' ' && (c == ' ' || c == '\n'))
            {
                // printf("Final new line count %d\n", next_line_characters);
                if (alpha_numeric_count != 0)
                {
                    int adjusted_word_length_with_space = alpha_numeric_count + 1;
                    if (finishing_max_width == max_width)
                    {
                        write(fd_write, word_buffer, alpha_numeric_count);
                        if (alpha_numeric_count > max_width)
                            rtn = EXIT_FAILURE;
                        finishing_max_width -= alpha_numeric_count;
                    }
                    else if (adjusted_word_length_with_space < finishing_max_width)
                    {
                        write(fd_write, " ", 1);
                        write(fd_write, word_buffer, alpha_numeric_count);
                        finishing_max_width -= adjusted_word_length_with_space;
                    }
                    else
                    {
                        write(fd_write, "\n", 1);
                        if (alpha_numeric_count > max_width)
                            rtn = EXIT_FAILURE;
                        write(fd_write, word_buffer, alpha_numeric_count);
                        finishing_max_width = max_width - alpha_numeric_count;
                    }
                    alpha_numeric_count = 0;
                }
            }
            if (bytes > 0)
            {
                bool current_char_is_alphanumeric = c != ' ' && c != '\n';
                if (current_char_is_alphanumeric)
                {
                    if (alpha_numeric_count == 0)
                    {
                        word_buffer = (char *)malloc(alpha_numeric_count + 1);
                    }
                    else
                    {
                        word_buffer = (char *)realloc(word_buffer, alpha_numeric_count + 1);
                    }

                    word_buffer[alpha_numeric_count] = c;
                    alpha_numeric_count += 1;
                }
            }
            prev_c = c;
        }
    }

    // buffer still exists.
    if (alpha_numeric_count != 0)
    {
        int adjusted_word_length_with_space = alpha_numeric_count + 1;
        if (finishing_max_width == max_width)
        {
            write(fd_write, word_buffer, alpha_numeric_count);
            if (alpha_numeric_count > max_width)
                rtn = EXIT_FAILURE;
            finishing_max_width -= alpha_numeric_count;
        }
        else if (adjusted_word_length_with_space < finishing_max_width)
        {
            write(fd_write, " ", 1);
            write(fd_write, word_buffer, alpha_numeric_count);
            finishing_max_width -= adjusted_word_length_with_space;
        }
        else
        {
            write(fd_write, "\n", 1);
            if (alpha_numeric_count > max_width)
                rtn = EXIT_FAILURE;
            write(fd_write, word_buffer, alpha_numeric_count);
            finishing_max_width = max_width - alpha_numeric_count;
        }
        alpha_numeric_count = 0;
    }
    if (word_buffer != NULL)
        free(word_buffer);

    close(fd_read);
    close(fd_write);
    return rtn;
}
int main(int argv, char **argc)
{
    // init buffer with buffer array
    char *word_buffer = NULL;

    // memset(word_buffer, 0, BUFSIZE);

    int rtn = wrap_text("tests/test5.txt", 30, word_buffer);
    return rtn;
}