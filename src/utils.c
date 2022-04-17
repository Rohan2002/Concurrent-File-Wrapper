/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/

#include <stdio.h>
#include <stdlib.h>
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

int get_run_mode(char *arg,int *N, int *M){
    int run_mode=0;
    if (strcmp(arg,"-r")==0) {
        run_mode = 1;
    }
    else if (arg[0]=='-' && arg[1]=='r') {
        int i;
        run_mode = 2;
        for (i=0;i<strlen(arg);i++) {
            if (arg[i]==',') {
                run_mode = 3;
            }
        }
        if (run_mode==2) {
            arg+=2;
            *N = atoi(arg);

        } else if (run_mode==3) {
            char tempM[20];
            int digitsM=0;
            arg+=2;
            while (arg[digitsM]!=',') {
                tempM[digitsM] = arg[digitsM];
                digitsM++;
            }
            tempM[digitsM]='\0';
            arg+=digitsM+1;
            *N = atoi(arg);
            *M = atoi(tempM);
        }

    }
    return run_mode;
}
char* append_file_path_to_existing_path(char* existing_path, char* new_file){
    /*
        Appends the name of a directory or regular file to an existing given path.
        Examples:

        Existing path: foo/a/b
        New File/Directory: c

        Return foo/a/b/c
    */
   int plen = strlen(existing_path);
   int nlen = strlen(new_file);

   char* appended_path = malloc(plen + nlen + 2);
   
   memcpy(appended_path, existing_path, plen); // Exisitng path.
   char* extender = "/";
   if(strcmp(&(appended_path[plen]), extender) != 0){
       appended_path[plen] = *extender;
   }
   memcpy(appended_path + plen + 1, new_file, nlen + 1);
   return appended_path;
}