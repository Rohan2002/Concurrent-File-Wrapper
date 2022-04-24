/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef QUEUE_H
#define QUEUE_H
#include <pthread.h>
#include <stdbool.h>


struct Queue_Data
{
    char *input_file;
    char *output_file;
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
    bool close;
};
typedef struct Queue_Model Queue;
Queue *queue_init(size_t);
int queue_enqueue(Queue *, queue_data_type*);
queue_data_type* queue_dequeue(Queue *);
queue_data_type*  queue_peek(Queue *);
bool queue_is_empty(Queue *);
bool queue_is_full(Queue *);
int queue_destroy(Queue *);
void print_queue_metadata(Queue *queue_pointer, size_t index);
int queue_close(Queue *queue_pointer);

#endif