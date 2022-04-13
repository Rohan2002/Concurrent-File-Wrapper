/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include "queue.h"
#ifndef WORD_BREAK_H_INCLUDED

struct consumer_worker_data
{
    Queue *file_queue;
    int keep_on_working;
    int return_status;
};
typedef struct consumer_worker_data consumer_worker_type;

// wrapping infastructure
int wrap_text(char *optional_input_file, int max_width, char *optional_output_file);

// threading infastructure
int wrap_text_for_directory(char *dir_name, int max_width, Queue *file_queue);
void *consumer_wrapper_worker(void *arg);

#define DEF_MODE S_IRUSR | S_IWUSR
#define BUFSIZE 5
#define STDIN 0
#define STDOUT 1
#define STDERR 2

#endif