/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef UTILS_H
#define UTILS_H
#include <sys/stat.h>
#include "queue.h"
#include "pool.h"
// utils infastructure
void print_buffer(char *word_buffer, int length);
int safe_write(int fd, const void *__buf, long __nbyte);
int check_file_or_directory(struct stat *file_in_dir_pointer);
char *concat_string(char *prev_str, char *new_str, int optional_prev_length, int optional_new_length);
char *append_file_path_to_existing_path(char *existing_path, char *new_file);
int fill_param_by_user_arguememt(int argv,char **arg, int *max_width, int *producer_threads, int *consumer_threads,int *isrecursive,int *widthindex );
int fill_queue_and_pool_by_user_arguememt(int widthindex,int argv,char **arg, Queue *optional_file_queue,Pool *dir_pool );

#endif