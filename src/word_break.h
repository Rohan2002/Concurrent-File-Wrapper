/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include "queue.h"
#include "pool.h"
#ifndef WORD_BREAK_H
#define  WORD_BREAK_H

struct file_producer
{
    Queue* file_queue;
    Pool* dir_pool;
    int isrecursive;
    int* error_code;
};
typedef struct file_producer producer_type;
// Notice: file consumer never has access to the directory pool as the only means of communication between the producer and consumer is the file queue.
// Which consists of the regular files that will be wrapped in the consumer.
struct file_consumer
{
    Queue* file_queue;
    int max_width;
    int* error_code;
};
typedef struct file_consumer consumer_type;

// wrapping infastructure

// used by consumer
int wrap_text(char *optional_input_file, int max_width, char *optional_output_file);

// used by producer
int fill_pool_and_queue_with_data(char *parent_dir_path, Pool *dir_pool, Queue *file_q,int isrecursive);

// threading infastructure
void* produce_files_to_wrap(void *arg);
void* consume_files_to_wrap(void *arg);

// extra credit
int handle_multiple_input_files(int widthindex, int max_width, int argv, char **arg, Pool *dir_pool);

#define DEF_MODE S_IRUSR | S_IWUSR
#define BUFSIZE 5
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define QUEUESIZE 100
#define POOLSIZE 1

#endif
