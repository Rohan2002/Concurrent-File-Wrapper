/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef QUEUE_H_INCLUDED

#include <pthread.h>
#include <stdbool.h>
#define DEBUG 1

struct Queue_Data
{
    char *input_file;
    char *output_file;
    int w;
};
typedef struct Queue_Data queue_data_type;

struct Queue_Model
{
    queue_data_type** data;
    size_t start;
    size_t end;
    size_t queue_size;
    pthread_mutex_t lock;
    pthread_cond_t ready_to_consume;
    pthread_cond_t ready_to_produce;
    size_t number_of_elements_buffered;
    bool all_producers_finished;
};
typedef struct Queue_Model Queue;
Queue *init(size_t);
int enqueue(Queue *, queue_data_type*);
queue_data_type* dequeue(Queue *);
queue_data_type*  peek(Queue *);
bool is_empty(Queue *);
bool is_full(Queue *);
int destroy(Queue *);
void print_queue_metadata(Queue *queue_pointer, size_t index);

#endif