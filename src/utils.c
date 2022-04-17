/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "logger.h"

void print_buffer(char *word_buffer, int length)
{
    if (word_buffer == NULL)
    {
        debug_print("%s\n", "Buffer is not init!");
    }
    for (int i = 0; i < length; i++)
    {
        debug_print("%c", word_buffer[i]);
    }
    debug_print("%s", "\n");
}
int safe_write(int fd, const void *__buf, long __nbyte)
{
    int write_status = write(fd, __buf, __nbyte);
    if (write_status == -1)
    {
        error_print("%s\n", "Write error.");
        return EXIT_FAILURE;
    }
    else
    {
        return write_status;
    }
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

int get_run_mode(char *arg, int *N, int *M)
{
    int run_mode = 0;
    if (strcmp(arg, "-r") == 0)
    {
        run_mode = 1;
    }
    else if (arg[0] == '-' && arg[1] == 'r')
    {
        int i;
        run_mode = 2;
        for (i = 0; i < strlen(arg); i++)
        {
            if (arg[i] == ',')
            {
                run_mode = 3;
            }
        }
        if (run_mode == 2)
        {
            arg += 2;
            *N = atoi(arg);
        }
        else if (run_mode == 3)
        {
            char tempM[20];
            int digitsM = 0;
            arg += 2;
            while (arg[digitsM] != ',')
            {
                tempM[digitsM] = arg[digitsM];
                digitsM++;
            }
            tempM[digitsM] = '\0';
            arg += digitsM + 1;
            *N = atoi(arg);
            *M = atoi(tempM);
        }
    }
    return run_mode;
}
char *concat_string(char *prev_str, char *new_str, int optional_prev_length, int optional_new_length)
{
    /*
        Efficient String Concat.

        Note if optional_prev_length = -1 or optional_new_length -1, the string length for the appropriate string will be computed.
            else it is the client's responsibility to give a valid string length for the appropriate string.

        Example usage:  
        
        char* concatenated = concat_string("lorem", "ipsum", -1, -1);
                        concatenated = "loremipsum".
        
        char* concatenated_two = concat_string("lorem", "ipsum", 5, 5);
                        concatenated = "loremipsum".

        Warning: It's the client's responsibility to free the output from concat_string.
    */
    int str_len_prev = optional_prev_length == -1 ? strlen(prev_str) : optional_prev_length;
    int str_len_new = optional_new_length == -1 ? strlen(new_str) : optional_new_length;

    char *extension_str = (char *)malloc((str_len_prev + str_len_new + 2) * sizeof(char));
    if (extension_str == NULL)
    {
        error_print("%s", "Malloc failure for concat_string\n");
        return NULL;
    }
    memcpy(extension_str, prev_str, str_len_prev + 1);
    memcpy(extension_str + str_len_prev, new_str, str_len_new + 1);
    return extension_str;
}
char *append_file_path_to_existing_path(char *existing_path, char *new_file)
{
    /*
        Appends the name of a directory or regular file to an existing given path.
        Examples:

        Existing path: foo/a/b
        New File/Directory: c

        Return foo/a/b/c
    */
    int plen = strlen(existing_path);
    int nlen = strlen(new_file);

    // check if existing_paths end character has "/"
    // printf("Last char of existing %c\n", existing_path[plen - 1]);
    char *extender = "/";
    bool extended = false;
    if (strcmp(&existing_path[plen - 1], extender) != 0)
    {
        existing_path = concat_string(existing_path, extender, plen, 1);
        extended = true;
    }

    char *appended_path = concat_string(existing_path, new_file, extended ? plen + 1 : plen, nlen);
    if (extended)
    {
        free(existing_path);
    }
    return appended_path;
}