/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef UTILS_H

// utils infastructure
void print_buffer(char *word_buffer, int length);
ssize_t safe_write(int fd, const void *__buf, size_t __nbyte);
int check_file_or_directory(struct stat *file_in_dir_pointer);
int get_run_mode(char *arg,int *N, int *M); 

#endif