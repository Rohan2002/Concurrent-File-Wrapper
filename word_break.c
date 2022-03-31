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
#include <dirent.h>
#include <string.h>
#include <ctype.h>

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
int wrap_text(char *optional_input_file, int max_width, char *optional_output_file)
{
    int fd_read;
    int rtn = 0;
    fd_read = optional_input_file ? open(optional_input_file, O_RDWR | O_CREAT, DEF_MODE) : 0;

    int fd_write;
    fd_write = optional_output_file ? open(optional_output_file, O_RDWR | O_CREAT | O_TRUNC, DEF_MODE) : 1;

    if (fd_read == -1)
    {
        perror("Error when opening file for reading");
    }
    if (fd_write == -1)
    {
        perror("Error when opening file for writing");
    }

    int bytes;
    char sentence_buffer[BUFSIZE];
    int pos;

    int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
    char c = '\0';
    char prev_c = '\0';
    int finishing_max_width = max_width;

    char *word_buffer = NULL;
    int next_line_characters = 0;
    while ((bytes = read(fd_read, sentence_buffer, BUFSIZE)) > 0)
    {
        // read buffer and break file into lines
        for (pos = 0; pos < bytes; pos++)
        {
            c = sentence_buffer[pos];
            if (c == '\n')
            {
                next_line_characters += 1;
            }
            if (!isspace(c))
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
            if (prev_c != ' ' && (isspace(c)))
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
                    free(word_buffer);
                    word_buffer = NULL;
                    alpha_numeric_count = 0;
                }
            }
            if (bytes > 0)
            {
                bool current_char_is_alphanumeric = !isspace(c);
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
            if (bytes == -1)
            {
                perror("Error reading bytes from a file");
                return EXIT_FAILURE;
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
    {
        free(word_buffer);
    }
    close(fd_read);
    close(fd_write);
    return rtn;
}
int check_file_or_directory(struct stat *file_in_dir_pointer)
{
    /*
        If name is a regular file then return 1
        If name is a directory name then return 2
        else return 0
    */

    struct stat file_in_dir = *file_in_dir_pointer;
    if (S_ISREG(file_in_dir.st_mode))
    {
        return 1;
    }
    if (S_ISDIR(file_in_dir.st_mode))
    {
        return 2;
    }
    return 0;
}
int wrap_text_for_directory(char *dir_name, int max_width)
{
    DIR *dfd;
    struct dirent *directory_pointer;
    char extension[6] = "wrap.";
    if ((dfd = opendir(dir_name)) == NULL)
    {
        fprintf(stderr, "Can't open %s\n", dir_name);
        return EXIT_FAILURE;
    }

    int directory_of_interest_change_status = chdir(dir_name);

    if (directory_of_interest_change_status == -1)
    {
        fprintf(stderr, "Can't change directory to %s\n", dir_name);
        return EXIT_FAILURE;
    }

    while ((directory_pointer = readdir(dfd)) != NULL)
    {
        struct stat file_in_dir;

        int status_of_file_metadata = stat(directory_pointer->d_name, &file_in_dir); // directory_pointer->d_name is the filename.
        if (status_of_file_metadata == -1)
        {
            fprintf(stderr, "Can't get stat of file %s\n", directory_pointer->d_name);
            return EXIT_FAILURE;
        }
        // if its a file then do the stuff.
        if (check_file_or_directory(&file_in_dir) == 1)
        {

            char *extension_str = (char *)malloc((sizeof(extension) + sizeof(directory_pointer->d_name)) * sizeof(char));
            strcpy(extension_str, extension); // copy wrap. into string

            char *file_name_with_extension = strcat(extension_str, directory_pointer->d_name); // copy rest of the filename in the string.

            // check if filename contains wrap.
            // int status_of_cmp = memcmp(directory_pointer->d_name, extension, strlen(extension));
            // printf("d_name %s, extension %s, Strlen of extension %lu, status of cmp %d\n", directory_pointer->d_name, extension, strlen(extension), status_of_cmp);
            if (memcmp(directory_pointer->d_name, ".", strlen(".")) != 0 && memcmp(directory_pointer->d_name, extension, strlen(extension)) != 0)
            {
                wrap_text(directory_pointer->d_name, max_width, file_name_with_extension);
            }
            else
            {
                // printf("Skipping text-wrap for file %s\n", directory_pointer->d_name);
            }
            free(extension_str);
        }
    }
    closedir(dfd);
    return EXIT_SUCCESS;
}
int main(int argv, char **argc)
{
    if (argv < 2)
    {
        fprintf(stderr, "At least provide the max_width argument\n");
        return EXIT_FAILURE;
    }
    int max_width = atoi(argc[1]);
    // If the file name is not present, ww will read from standard input and print to standard output.
    if (argv == 2)
    {
        wrap_text(NULL, max_width, NULL);
    }
    else
    {
        // If the file name is a regular file, ww will read from the file and print to standard output.
        char *file_name = argc[2];

        struct stat dir_status;
        stat(file_name, &dir_status);

        // If the file name is a directory, ww will open each regular file in the directory and write to a new
        if (check_file_or_directory(&dir_status) == 2)
        {
            wrap_text_for_directory(file_name, max_width);
        }
        else if (check_file_or_directory(&dir_status) == 1)
        {
            // If the file name is a regular file, ww will read from the file and print to standard output.
            wrap_text(file_name, max_width, NULL);
        }
        else
        {
            fprintf(stderr, "Invalid file path. Please input only a regular file or directory. Input file: %s\n", file_name);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}