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
int fill_param_by_user_arguememt(int argv,char **arg, int *max_width, int *producer_threads, int *consumer_threads,int *isrecursive,int *widthindex )
{
    *widthindex=2;
    if(arg[1][0]!='-' ) {
        *widthindex=1;
        *isrecursive=0;
        *consumer_threads = 1;
        *producer_threads = 1;

    } else if (strcmp(arg[1], "-r") == 0)
    {
        *isrecursive=1;
        *consumer_threads = 1;
        *producer_threads = 1;
    }
    else
    {
        char *rec_threads = arg[1];
        int strlen_rec_threads = strlen(rec_threads);
        if (strlen_rec_threads != 5)
        {
            error_print("%s\n", "Provide recursive arguement properly! It should be like -rN,M where N is the number of directory/producer threads, and M is the number of wrapping/consumer threads");
            return -1;
        }
        else
        {
            *isrecursive=1;
            *producer_threads = atoi(&rec_threads[2]); // directory threads
            *consumer_threads = atoi(&rec_threads[4]); // wrapping threads
        }
    }
    *max_width = atoi(arg[*widthindex]);


    return 0;
}
int fill_queue_and_pool_by_user_arguememt(int widthindex,int argv,char **arg, Queue *optional_file_queue,Pool *dir_pool )
{
    for (int i=widthindex+1; i<argv; i++) {
         char *dir_of_interest = concat_string(arg[i], "\0", -1, -1);
         struct stat file_in_dir;
            int status_of_file_metadata = stat(dir_of_interest, &file_in_dir); // directory_pointer->d_name is the filename.
            if (status_of_file_metadata != 0)
            {
                error_print("Can't get stat of file %s\n", dir_of_interest);
                return status_of_file_metadata;
            }
            if (check_file_or_directory(&file_in_dir) == 1){ // regular file enque to file queue
                // Only wrap files that don't start with wrap. or .
                if (dir_of_interest[0] != '.' && memcmp(dir_of_interest, "wrap.", 5) != 0)
                {
                    // output file name computation.
                    int index=0;
                    for (int j=0;j<strlen(dir_of_interest);j++){
                        if (dir_of_interest[j]=='/' || dir_of_interest[j]=='\\') {
                            index=j+1;
                        }
                    }
                    char *filename = &dir_of_interest[index];
                    
                    char *dirname=(char *)malloc((strlen(dir_of_interest)+1)*sizeof(char));
                    strcpy(dirname,dir_of_interest);
                    char *new_file_name = concat_string("wrap.", filename, 5, strlen(filename));
                    char *output_file_name;
                    if (index>0){
                        dirname[index]='\0';
                        //printf("new_file_name : %s\n",new_file_name);
                        //printf("dirname : %s\n",dirname);
                        output_file_name = append_file_path_to_existing_path(dirname, new_file_name);
                        //printf("output_file_name : %s\n",output_file_name);

                    } else {
                        output_file_name=(char *)malloc((strlen(new_file_name)+1)*sizeof(char));
                        strcpy(output_file_name,new_file_name);
                        
                    }
                    free(new_file_name);
                    free(dirname);
                    
                    



                    //arg[i] file name with folder ex. tests/alex
                    //output_file_name shoulde be tests/wrap.alex

                    //



                    debug_print("Input-file found with path %s\n", dir_of_interest);
                    debug_print("Output-file found with path %s\n", output_file_name);

                    if (optional_file_queue != NULL)
                    {
                        queue_data_type *qd = malloc(sizeof(queue_data_type));
                        if (qd == NULL)
                        {
                            error_print("%s\n", "Malloc failure!");
                            return -1;
                        }
                        qd->input_file = dir_of_interest;
                        qd->output_file = output_file_name;
                        //strcpy(qd->output_file ,"wrap.test.txt");

                        queue_enqueue(optional_file_queue, qd);
                    }
                }

            }
            else if (check_file_or_directory(&file_in_dir) == 2){
                //printf("%s\n",dir_of_interest);
                pool_data_type *pool_init_data = malloc(sizeof(pool_data_type));
                pool_init_data->directory_path = dir_of_interest;
                pool_enqueue(dir_pool, pool_init_data);
            }
      
    }


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