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
                printf("New line characters %d\n", next_line_characters);
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
                        print_buffer(word_buffer, alpha_numeric_count);
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
    if (word_buffer != NULL)
        free(word_buffer);

    close(fd_read);
    close(fd_write);
    return rtn;
}
// int wrap_text(char *file_name, int max_width, char word_buffer[BUFSIZE])
// {
//     int fd_read;
//     int rtn = 0;
//     fd_read = open(file_name, O_RDWR | O_CREAT, DEF_MODE);

//     int fd_write;
//     fd_write = open("wrap_out", O_RDWR | O_CREAT | O_TRUNC, DEF_MODE);

//     if (fd_read == -1 || fd_write == -1)
//     {
//         perror("File Open Error");
//     }
//     int bytes;
//     char buf[BUFSIZE];
//     int pos;

//     int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
//     char c = '\0';
//     char prev_c = '\0';
//     int finishing_max_width = max_width;
//     // int first_word=1;
//     word_buffer = NULL;
//     int prev_newlines = 0;
//     int cumalative_written_words_length_in_a_line = 0;
//     while ((bytes = read(fd_read, buf, BUFSIZE)) > 0)
//     {
//         // read buffer and break file into lines
//         for (pos = 0; pos < bytes; pos++)
//         {
//             c = buf[pos];
// if (prev_c == '\n' && c == '\n')
// {
//     if (finishing_max_width <= 1)
//     {
//         // Assuming we write the sentence covering the whole width (i.e. the remaining width can be 1 or less.)
//         write(fd_write, "\n", 1);
//     }
//     else
//     {
//         // Assuming we write the sentence covering only part of the width
//         write(fd_write, "\n\n", 2);
//     }
// }
//             // detect a word.
//             if (prev_c != ' ' && (c == ' ' || c == '\n'))
//             {
//                 // From write-up, "no more than one space occurs between words".
//                 // if buffer has at least one non-white space character then we can assume buffer
//                 // is not "completely" made of white spaces
//                 // Notice: This will only occur if a line "starts with" a sequence of white space characters

//                 bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;
//                 if (!whole_buffer_has_only_white_spaces)
//                 {
//                     // Start of the greedy word-wrap algorithm

//                     // Selin, the whole point of using a buffer is to optimize the read, write system calls.
//                     // So let's minimize the two write calls into only one write call.

//                     // always add space to a word.
//                     word_buffer[alpha_numeric_count] = ' ';
//                     alpha_numeric_count += 1; // add space character length to word length.

//                     if (alpha_numeric_count <= finishing_max_width)
//                     {
//                         write(fd_write, word_buffer, alpha_numeric_count);
//                         finishing_max_width = finishing_max_width - alpha_numeric_count;

//                         if (alpha_numeric_count > max_width)
//                             rtn = EXIT_FAILURE;
//                     }
//                     else
//                     {
//                         // move the cursor before the "space" at the end of the line, so that we can replace it with \n;
//                         lseek(fd_write, -1, SEEK_CUR);

//                         write(fd_write, "\n", 1); // ASK: can we minimize this write call?
//                         // alpha_numeric_count -= 1;

//                         write(fd_write, word_buffer, alpha_numeric_count);

//                         finishing_max_width = max_width - alpha_numeric_count;
//                     }
//                 }
//                 if (word_buffer != NULL)
//                     free(word_buffer);

//                 word_buffer = NULL;
//                 alpha_numeric_count = 0;
//             }

//             else if (bytes == -1)
//             {
//                 // handle read error.
//                 perror("Read Error:");
//                 break;
//             }
//             else
//             {
// bool current_char_is_alphanumeric = c != ' ' && c != '\n';
// if (current_char_is_alphanumeric)
// {
//     if (alpha_numeric_count == 0)
//     {
//         word_buffer = (char *)malloc(alpha_numeric_count + 1);
//     }
//     else
//     {
//         word_buffer = (char *)realloc(word_buffer, alpha_numeric_count + 1);
//     }

//     word_buffer[alpha_numeric_count] = c;
//     alpha_numeric_count += 1;
// }
//             }
//             prev_c = c;
//         }
//     }
//     // remaining buffer.
//     //  If the while loop has finished but there remains a last word
//     // that has not been processed yet, we process it here
//     bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;
//     if (!whole_buffer_has_only_white_spaces)
//     {
//         // greedy word-wrap algorithm
//         word_buffer[alpha_numeric_count] = ' ';
//         alpha_numeric_count += 1; // add space character length to word length.

//         if (alpha_numeric_count <= finishing_max_width)
//         {
//             write(fd_write, word_buffer, alpha_numeric_count);
//             finishing_max_width = finishing_max_width - alpha_numeric_count;

//             if (alpha_numeric_count > max_width)
//                 rtn = EXIT_FAILURE;
//         }
//         else
//         {
//             // move the cursor before the "space" at the end of the line, so that we can replace it with \n;
//             lseek(fd_write, -1, SEEK_CUR);

//             write(fd_write, "\n", 1); // ASK: can we minimize this write call?
//             write(fd_write, word_buffer, alpha_numeric_count);

//             finishing_max_width = max_width - alpha_numeric_count;
//         }
//     }

//     if (word_buffer != NULL)
//         free(word_buffer);

//     close(fd_read);
//     close(fd_write);
//     return rtn;
// }

int main(int argv, char **argc)
{
    // init buffer with buffer array
    char *word_buffer = NULL;

    // memset(word_buffer, 0, BUFSIZE);

    int rtn = wrap_text("tests/test5.txt", 30, word_buffer);
    return rtn;
}