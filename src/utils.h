/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef UTILS_H
#include <sys/stat.h>
// utils infastructure
void print_buffer(char *word_buffer, int length);
int safe_write(int fd, const void *__buf, long __nbyte);
int check_file_or_directory(struct stat *file_in_dir_pointer);
int get_run_mode(char *arg,int *N, int *M); 
char* append_file_path_to_existing_path(char* existing_path, char* new_file);

#endif