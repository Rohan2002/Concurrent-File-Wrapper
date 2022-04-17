/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include "queue.h"
#include "pool.h"
#ifndef WORD_BREAK_H

struct file_producer
{
    Queue* file_queue;
    Pool* dir_pool;
    char* initial_directory;
};
typedef struct file_producer producer_type;
// Note: consumer threads should not access directory pool.
struct file_consumer
{
    Queue* file_queue;
};
typedef struct file_consumer consumer_type;

// wrapping infastructure
int wrap_text(char *optional_input_file, int max_width, char *optional_output_file);
int wrap_text_for_directory(char *dir_name, int max_width, Queue *file_queue, int run_mode);

// threading infastructure
void* produce_files_to_wrap(void *arg);
void* consume_files_to_wrap(void *arg);

#define DEF_MODE S_IRUSR | S_IWUSR
#define BUFSIZE 5
#define STDIN 0
#define STDOUT 1
#define STDERR 2

#endif