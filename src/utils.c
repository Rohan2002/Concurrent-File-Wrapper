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

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

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

int check_rsyntax(char *r)
{
    return r[0] == '-' && r[1] == 'r';
}

int fill_param_by_user_arguememt(int argv, char **arg, int *max_width, int *producer_threads, int *consumer_threads, int *isrecursive, int *widthindex)
{
    if (argv == 1)
    {
        error_print("%s\n", "Recursive arguement or Max Width arguement not provided!");
        return EXIT_FAILURE;
    }
    *widthindex = 2;
    if (check_rsyntax(arg[1]))
    {
        // Notice: This will handle all cases actually.
        // Case1: "-r", producer/directory-threads = 1 and consumer/wrapping-threads = 1
        // Case2: "-rN,", producer = 1 and consumer = N
        // Case3: "-rM,N", producer = M and consumer = N
        *isrecursive = 1;
        char *r_with_number_of_threads = arg[1];
        int strlen_r_with_number_of_threads = sizeof(r_with_number_of_threads);

        char *str_producer_thread = malloc(strlen_r_with_number_of_threads);
        if (str_producer_thread == NULL)
        {
            error_print("%s\n", "Malloc Failure");
            return -1;
        }
        int producer_thread = 0;
        r_with_number_of_threads += 2; // skip the first two characters "-r"

        // Case1
        if (strcmp(r_with_number_of_threads, "\0") == 0)
        {
            *producer_threads = 1;
            *consumer_threads = 1;
            *max_width = atoi(arg[*widthindex]);
            free(str_producer_thread);
            if (*max_width == 0)
            {
                error_print("%s\n", "Max width was either not provided or it cannot be 0!");
                return -1;
            }
            debug_print("Producer threads %d\n", *producer_threads);
            debug_print("Consumer threads %d\n", *consumer_threads);
            debug_print("Max Width %d\n", *max_width);

            return 0;
        }
        // Case2 or Case3. We assume there is always a comma -rN, or -rM,N
        while (r_with_number_of_threads[producer_thread] != ',')
        {
            str_producer_thread[producer_thread] = r_with_number_of_threads[producer_thread];
            producer_thread++;
        }
        str_producer_thread[producer_thread] = '\0';
        r_with_number_of_threads += producer_thread + 1; // get string after ,

        int num_producer_threads = atoi(str_producer_thread);
        int num_consumer_threads = atoi(r_with_number_of_threads);

        // extraneous case.
        if (num_producer_threads <= 0)
        {
            *producer_threads = 1;
        }
        if (num_consumer_threads <= 0)
        {
            *consumer_threads = 1;
        }

        if (num_producer_threads > 0 && num_consumer_threads == 0)
        {
            // Case 2. Note order of producer, consumer assignment switches if its -rN,
            *producer_threads = 1;
            *consumer_threads = num_producer_threads;
        }
        else if (num_producer_threads >= 0 && num_consumer_threads >= 0)
        {
            // Case 3.
            *producer_threads = MAX(1, num_producer_threads);
            *consumer_threads = MAX(1, num_consumer_threads);
        }
        free(str_producer_thread);
    }
    else
    {
        // Case when -r is not provided.
        *widthindex = 1;
        *isrecursive = 0;
        *consumer_threads = 1;
        *producer_threads = 1;
    }
    *max_width = atoi(arg[*widthindex]);
    if (*max_width == 0)
    {
        error_print("%s\n", "Max width was either not provided or it cannot be 0!");
        return -1;
    }
    debug_print("Arg: Producer threads %d\n", *producer_threads);
    debug_print("Arg: Consumer threads %d\n", *consumer_threads);
    debug_print("Arg: Max Width %d\n", *max_width);
    return 0;
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