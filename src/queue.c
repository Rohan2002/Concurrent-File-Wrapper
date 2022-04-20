/*
    @ project
    Word Break 2022.

    Synchronized Bounded Queue.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "queue.h"
#include "logger.h"

Queue *queue_init(size_t q_size)
{
    Queue *queue_pointer = malloc(sizeof(Queue));
    if (queue_pointer == NULL)
    {
        error_print("%s", "Failed to allocate queue!\n");
        return NULL;
    }
    queue_pointer->data = malloc(sizeof(queue_data_type *) * q_size);
    queue_pointer->start = 0;
    queue_pointer->end = 0;
    queue_pointer->queue_size = q_size;
    queue_pointer->number_of_elements_buffered = 0;
    queue_pointer->close = false;

    int mutex_init_status = pthread_mutex_init(&(queue_pointer->lock), NULL);
    if (mutex_init_status != 0)
    {
        error_print("Failed to INIT queue mutex with error code: %d!\n", mutex_init_status);
        return NULL;
    }
    int ready_to_consume_status = pthread_cond_init(&(queue_pointer->ready_to_consume), NULL);
    if (ready_to_consume_status != 0)
    {
        error_print("Failed to INIT ready_to_consume pthread condition variable: %d!\n", ready_to_consume_status);
        return NULL;
    }
    int ready_to_produce_status = pthread_cond_init(&(queue_pointer->ready_to_produce), NULL);
    if (ready_to_produce_status != 0)
    {
        error_print("Failed to INIT ready_to_produce pthread condition variable: %d!\n", ready_to_produce_status);
        return NULL;
    }
    
    debug_print("Created queue mutex at address %p\n", &(queue_pointer->lock));
    debug_print("Created queue with size %lu at address %p\n",q_size, queue_pointer);
    
    return queue_pointer;
}
int queue_enqueue(Queue *queue_pointer, queue_data_type *data)
{
    // pthread_mutex_lock is better than pthread_mutex_trylock here because we want the current thread to block (i.e. be locked) until data has been queue_enqueued.
    int lock_init_status = pthread_mutex_lock(&queue_pointer->lock);
    if (lock_init_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_init_status);
        return lock_init_status;
    }
    debug_print("%s", "Entering queue_enqueue\n");
    // If queue is full, first wait for consumer to deque the data.
    while (queue_is_full(queue_pointer))
    {
        // there will be no more data incoming as queue has queue_enqueued all available data at this point. So whatever data is remaining, just tell consumer to queue_dequeue.
        debug_print("%s", "From queue_enqueue: The queue is full.\n");
        pthread_cond_wait(&(queue_pointer->ready_to_produce), &queue_pointer->lock);
    }
    queue_pointer->data[queue_pointer->end] = data;
    debug_print("From queue_enqueue: Enqued element at index %lu\n", queue_pointer->end);
    queue_pointer->end = queue_pointer->end + 1;
    queue_pointer->end = queue_pointer->end % queue_pointer->queue_size; // mod because its a circular buffer.
    queue_pointer->number_of_elements_buffered++;

    pthread_cond_signal(&(queue_pointer->ready_to_consume)); // there is data in the queue, so signal consumer thread to start dequeing the data.
    int unlock_init_status = pthread_mutex_unlock(&queue_pointer->lock);
    if(unlock_init_status != 0){
        error_print("Failed to unlock with error code: %d!\n", unlock_init_status);
        return unlock_init_status;
    }
    return 0;
}
queue_data_type *queue_dequeue(Queue *queue_pointer)
{
    // pthread_mutex_lock is better than pthread_mutex_trylock here because we want the current thread to block (i.e. be locked) until data has been queue_enqueued.
    int lock_init_status = pthread_mutex_lock(&queue_pointer->lock);
    if (lock_init_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_init_status);
        return NULL;
    }
    debug_print("%s", "Entering queue_dequeue\n");
    // if there is no data in queue and there is still available data from the stream then then wait until I get that data.
    while (queue_is_empty(queue_pointer))
    {
        if(queue_pointer->close){
            int unlock_init_status = pthread_mutex_unlock(&queue_pointer->lock);
            if(unlock_init_status != 0){
                error_print("Failed to unlock with error code: %d!\n", unlock_init_status);
                return NULL;
            } 
            return NULL;
        }
        // wait to get the data.
        debug_print("%s", "From queue_dequeue: The queue is empty\n");
        pthread_cond_wait(&(queue_pointer->ready_to_consume), &queue_pointer->lock);
    }
    queue_data_type *data = queue_pointer->data[queue_pointer->start];
    queue_pointer->start = queue_pointer->start + 1;
    queue_pointer->start = queue_pointer->start % queue_pointer->queue_size; // mod because its a circular buffer.
    queue_pointer->number_of_elements_buffered--;

    pthread_cond_signal(&(queue_pointer->ready_to_produce)); // there is some space in the queue, so signal producer thread to start enqueing the data back aga
    int unlock_init_status = pthread_mutex_unlock(&queue_pointer->lock);
    if(unlock_init_status != 0){
        error_print("Failed to unlock with error code: %d!\n", unlock_init_status);
        return NULL;
    }

    return data;
}
queue_data_type *queue_peek(Queue *queue_pointer)
{
    return queue_pointer->data[queue_pointer->start];
}
bool data_queue_is_empty(queue_data_type *empty_data)
{
    return empty_data->input_file == NULL && empty_data->output_file == NULL;
}
bool queue_is_empty(Queue *queue_pointer)
{
    return queue_pointer->number_of_elements_buffered == 0;
}
bool queue_is_full(Queue *queue_pointer)
{
    return queue_pointer->number_of_elements_buffered == queue_pointer->queue_size;
}
int queue_destroy(Queue *queue_pointer)
{
    int mutex_destroy_status = pthread_mutex_destroy(&queue_pointer->lock);
    if (mutex_destroy_status != 0)
    {
        error_print("Queue mutex can't be destroyed with error code: %d\n", mutex_destroy_status);
        return mutex_destroy_status;
    }
    int consume_condition_variable_destroy = pthread_cond_destroy(&queue_pointer->ready_to_consume);
    if (consume_condition_variable_destroy != 0)
    {
        error_print("Condition variable for consume can't be destroyed with error code: %d\n", consume_condition_variable_destroy);
        return mutex_destroy_status;
    }
    int produce_condition_variable_destroy = pthread_cond_destroy(&queue_pointer->ready_to_produce);
    if (produce_condition_variable_destroy != 0)
    {
        error_print("Condition variable for produce can't be destroyed with error code: %d\n", produce_condition_variable_destroy);
        return mutex_destroy_status;
    }
    // In theory queue should be empty at this point. However, we cannot guarantee anything so just check if queue is empty, and free from there.
    if (queue_pointer->number_of_elements_buffered != 0)
    {
        error_print("%s", "From Queue Destroy(), the queue is still not empty!\n");
        while (!queue_is_empty(queue_pointer))
        {
            queue_data_type *q_data = queue_dequeue(queue_pointer);
            free(q_data->input_file);
            free(q_data->output_file);
            free(q_data);
        }
    }
    free(queue_pointer->data); // At this point all pointers exisiting inside queue_pointer->data is freed, so free queue_pointer->data.
    free(queue_pointer);
    return 0;
}
int queue_close(Queue *queue_pointer){
    int lock_status = pthread_mutex_lock(&queue_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }

    queue_pointer->close = true;
    pthread_cond_broadcast(&queue_pointer->ready_to_consume);

    int unlock_status = pthread_mutex_unlock(&queue_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return lock_status;
    }
    return 0;
}
void print_queue_node(queue_data_type* data){
    debug_print("queue_node->input_file: %s\n", data->input_file);
    debug_print("queue_node->output_file: %s\n", data->output_file);

}
void print_queue_metadata(Queue *queue_pointer, size_t index){
    printf("========================================\n");
    debug_print("Queue->start:%lu\n", queue_pointer->start);
    debug_print("Queue->end:%lu\n", queue_pointer->end);
    debug_print("Queue->number_of_elements_buffered:%lu\n", queue_pointer->number_of_elements_buffered);
    
    queue_data_type** q_data = queue_pointer->data;
    print_queue_node(q_data[index]);
    printf("========================================\n");
}