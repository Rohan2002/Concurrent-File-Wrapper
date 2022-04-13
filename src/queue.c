/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "queue.h"

Queue *init(size_t q_size)
{
    Queue *queue_pointer = malloc(sizeof(Queue));
    if (queue_pointer == NULL)
    {
        fprintf(stderr, "Failed to allocate queue!\n");
        return NULL;
    }
    queue_pointer->data = malloc(sizeof(size_t *) * q_size);
    queue_pointer->start = 0;
    queue_pointer->end = 0;
    queue_pointer->queue_size = q_size;
    queue_pointer->number_of_elements_buffered = 0;
    queue_pointer->all_producers_finished = false;

    int mutex_init_status = pthread_mutex_init(&(queue_pointer->lock), NULL);
    if (mutex_init_status != 0)
    {
        fprintf(stderr, "Failed to INIT queue mutex with error code: %d!\n", mutex_init_status);
        return NULL;
    }
    int ready_to_consume_status = pthread_cond_init(&(queue_pointer->ready_to_consume), NULL);
    if (ready_to_consume_status != 0)
    {
        fprintf(stderr, "Failed to INIT ready_to_consume pthread condition variable: %d!\n", ready_to_consume_status);
        return NULL;
    }
    int ready_to_produce_status = pthread_cond_init(&(queue_pointer->ready_to_produce), NULL);
    if (ready_to_produce_status != 0)
    {
        fprintf(stderr, "Failed to INIT ready_to_produce pthread condition variable: %d!\n", ready_to_produce_status);
        return NULL;
    }
    if (DEBUG)
    {
        printf("Created queue mutex at address %p\n", &(queue_pointer->lock));
        printf("Created queue at address %p\n", queue_pointer);
    }
    return queue_pointer;
}
int enqueue(Queue *queue_pointer, queue_data_type *data)
{
    if (DEBUG)
    {
        printf("Entering enqueue\n");
    }
    // pthread_mutex_lock is better than pthread_mutex_trylock here because we want the current thread to block (i.e. be locked) until data has been enqueued.
    int lock_init_status = pthread_mutex_lock(&queue_pointer->lock);
    if (lock_init_status != 0)
    {
        fprintf(stderr, "Failed to INIT lock with error code: %d!\n", lock_init_status);
        return lock_init_status;
    }
    // If queue is full, first wait for consumer to deque the data.
    while (is_full(queue_pointer))
    {
        // there will be no more data incoming as queue has enqueued all available data at this point. So whatever data is remaining, just tell consumer to dequeue.
        if (DEBUG)
        {
            printf("From enqueue: The queue is full.\n");
        }
        pthread_cond_wait(&(queue_pointer->ready_to_produce), &queue_pointer->lock);
    }
    queue_pointer->data[queue_pointer->end] = data;
    if (DEBUG)
    {
        printf("From enqueue: Enqued element at index %lu\n", queue_pointer->end);
    }
    queue_pointer->end = queue_pointer->end + 1;
    queue_pointer->end = queue_pointer->end % queue_pointer->queue_size; // mod because its a circular buffer.
    queue_pointer->number_of_elements_buffered++;

    pthread_cond_signal(&(queue_pointer->ready_to_consume)); // there is data in the queue, so signal consumer thread to start dequeing the data.
    pthread_mutex_unlock(&queue_pointer->lock);

    return 0;
}
queue_data_type *dequeue(Queue *queue_pointer)
{
    if (DEBUG)
    {
        printf("Entering dequeue\n");
    }
    // pthread_mutex_lock is better than pthread_mutex_trylock here because we want the current thread to block (i.e. be locked) until data has been enqueued.
    int lock_init_status = pthread_mutex_lock(&queue_pointer->lock);
    if (lock_init_status != 0)
    {
        fprintf(stderr, "Failed to INIT lock with error code: %d!\n", lock_init_status);
        return NULL;
    }
    // if there is no data in queue and there is still available data from the stream then then wait until I get that data.
    while (is_empty(queue_pointer))
    {
        // wait to get the data.
        if (DEBUG)
        {
            printf("From dequeue: The queue is empty\n");
        }
        pthread_cond_wait(&(queue_pointer->ready_to_consume), &queue_pointer->lock);
    }
    queue_data_type *data = queue_pointer->data[queue_pointer->start];
    queue_pointer->start = queue_pointer->start + 1;
    queue_pointer->start = queue_pointer->start % queue_pointer->queue_size; // mod because its a circular buffer.
    queue_pointer->number_of_elements_buffered--;

    pthread_cond_signal(&(queue_pointer->ready_to_produce)); // there is some space in the queue, so signal producer thread to start enqueing the data back aga
    pthread_mutex_unlock(&queue_pointer->lock);

    return data;
}
queue_data_type *peek(Queue *queue_pointer)
{
    return queue_pointer->data[queue_pointer->start];
}
bool data_is_empty(queue_data_type *empty_data)
{
    return empty_data->input_file == NULL && empty_data->output_file == NULL && empty_data->w == -1;
}
bool is_empty(Queue *queue_pointer)
{
    return queue_pointer->number_of_elements_buffered == 0;
}
bool is_full(Queue *queue_pointer)
{
    return queue_pointer->number_of_elements_buffered == queue_pointer->queue_size;
}
int destroy(Queue *queue_pointer)
{
    int mutex_destroy_status = pthread_mutex_destroy(&queue_pointer->lock);
    if (mutex_destroy_status != 0)
    {
        fprintf(stderr, "Queue mutex can't be destroyed with error code: %d\n", mutex_destroy_status);
        return mutex_destroy_status;
    }
    int consume_condition_variable_destroy = pthread_cond_destroy(&queue_pointer->ready_to_consume);
    if (consume_condition_variable_destroy != 0)
    {
        fprintf(stderr, "Condition variable for consume can't be destroyed with error code: %d\n", consume_condition_variable_destroy);
        return mutex_destroy_status;
    }
    int produce_condition_variable_destroy = pthread_cond_destroy(&queue_pointer->ready_to_produce);
    if (produce_condition_variable_destroy != 0)
    {
        fprintf(stderr, "Condition variable for produce can't be destroyed with error code: %d\n", produce_condition_variable_destroy);
        return mutex_destroy_status;
    }
    // In theory queue should be empty at this point. However, we cannot guarantee anything so just check if queue is empty, and free from there.
    if (queue_pointer->number_of_elements_buffered != 0)
    {
        fprintf(stderr, "From Queue Destroy(), the queue is still not empty!\n");
        while (!is_empty(queue_pointer))
        {
            queue_data_type *q_data = dequeue(queue_pointer);
            free(q_data);
        }
    }
    free(queue_pointer->data); // At this point all pointers exisiting inside queue_pointer->data is freed, so free queue_pointer->data.
    free(queue_pointer);
    return 0;
}
void print_queue_node(queue_data_type* data){
    printf("queue_node->input_file: %s\n", data->input_file);
    printf("queue_node->output_file: %s\n", data->output_file);
    printf("queue_node->max_width: %d\n", data->w);

}
void print_queue_metadata(Queue *queue_pointer, size_t index){
    printf("========================================\n");
    printf("Queue->start:%lu\n", queue_pointer->start);
    printf("Queue->end:%lu\n", queue_pointer->end);
    printf("Queue->number_of_elements_buffered:%lu\n", queue_pointer->number_of_elements_buffered);
    
    queue_data_type** q_data = queue_pointer->data;
    print_queue_node(q_data[index]);
    printf("========================================\n");
}