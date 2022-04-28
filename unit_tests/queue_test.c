#include <stdlib.h>
#include <stdio.h>
#include "../src/logger.h"
#include "../src/queue.h"
#include "../src/utils.h"

struct worker_args_model
{
    Queue *queue_pointer;
    int size;
};
typedef struct worker_args_model worker_type;

void *producer(void *vargs)
{
    // produce n elements of data
    debug_print("%s\n", "Entering producer");
    worker_type *w_a = vargs;

    Queue *queue_pointer = w_a->queue_pointer;
    int data_size = w_a->size;
    while (!queue_pointer->close && data_size != 0)
    {
        queue_data_type *qqd = malloc(sizeof(queue_data_type));
        qqd->input_file = concat_string("test-input-string", "\0", -1, -1);   // heap-space allocation
        qqd->output_file = concat_string("test-output-string", "\0", -1, -1); // heap-space allocation
        queue_enqueue(queue_pointer, qqd);
        debug_print("Thread Enqueued with tid %ld and data_size is %d\n", pthread_self(), data_size);
        data_size--;
    }
    int queue_close_status = queue_close(queue_pointer);
    if (queue_close_status == 0)
    {
        debug_print("Exited producer and queue_close status: %s \n", queue_pointer->close ? "yes" : "no");
    }
    else
    {
        error_print("%s %d\n", "Could not exit producer with error code: ", queue_close_status);
    }
    debug_print("%s\n", "Exited producer");
    return NULL;
}
void *consumer(void *vargs)
{
    // consume whatever is in the queue until its empty.
    debug_print("%s\n", "Entering consumer");
    worker_type *w_a = vargs;

    Queue *queue_pointer = w_a->queue_pointer;

    while (!queue_is_empty(queue_pointer) || !queue_pointer->close)
    {
        queue_data_type *dq = queue_dequeue(queue_pointer);
        if (dq != NULL)
        {
            debug_print("Thread Dequeued with tid %ld\n", pthread_self());
            free(dq->input_file);
            free(dq->output_file);
            free(dq);
        }
    }
    debug_print("%s\n", "Exited consumer");
    return NULL;
}
void vanilla_test(int q_init_size)
{
    /*
        Without any threading.
    */
    Queue *q = queue_init(q_init_size);

    for (int a = 0; a < q_init_size; a++)
    {
        queue_data_type *qqd = malloc(sizeof(queue_data_type));
        qqd->input_file = concat_string("test-input-string", "\0", -1, -1);   // heap-space allocation
        qqd->output_file = concat_string("test-output-string", "\0", -1, -1); // heap-space allocation

        queue_enqueue(q, qqd);
        debug_print("Enqueued...input: %s and output: %s\n", qqd->input_file, qqd->output_file);
    }
    while (!queue_is_empty(q))
    {
        queue_data_type *qqd = queue_dequeue(q);
        if (qqd != NULL)
        {
            debug_print("Dequed...input: %s and output: %s\n", qqd->input_file, qqd->output_file);
            free(qqd->input_file);
            free(qqd->output_file);
            free(qqd);
        }
    }
    printf("Is the queue of initial size %d empty after vanilla test? %s\n", q_init_size, queue_is_empty(q) ? "Yes" : "No");
    queue_destroy(q);
}
void threading_test(int q_init_size, int n_producers, int n_consumers, int data_size)
{
    Queue *q = queue_init(q_init_size);
    pthread_t *producer_tids = malloc(n_producers * sizeof(pthread_t));
    pthread_t *consumer_tids = malloc(n_consumers * sizeof(pthread_t));

    worker_type worker_args = {q, data_size};

    for (int i = 0; i < n_producers; i++)
    {
        debug_print("%s\n", "Creating producer thread\n");
        pthread_create(&producer_tids[i], NULL, producer, &worker_args);
        debug_print("%s\n", "Created producer thread\n");
    }
    for (int j = 0; j < n_consumers; j++)
    {
        debug_print("%s\n", "Creating consumer thread\n");
        pthread_create(&consumer_tids[j], NULL, consumer, &worker_args);
        debug_print("%s\n", "Created consumer thread\n");
    }
    for (int k = 0; k < n_producers; k++)
    {
        debug_print("%s\n", "Joining producer thread\n");
        pthread_join(producer_tids[k], NULL);
        debug_print("%s\n", "Joined producer thread\n");
    }
    for (int l = 0; l < n_consumers; l++)
    {
        debug_print("%s\n", "Joining consumer thread\n");
        pthread_join(consumer_tids[l], NULL);
        debug_print("%s\n", "Joined consumer thread\n");
    }
    printf("Is the quue of initial size %d empty after threading test? %s\n", q_init_size, queue_is_empty(q) ? "Yes" : "No");
    free(producer_tids);
    free(consumer_tids);
    queue_destroy(q);
}
int main()
{
    int queue_initial_sizes[5] = {1, 10, 100, 1000, 10000};
    for (int i = 0; i < 5; i++)
    {
        vanilla_test(queue_initial_sizes[i]);

        int number_of_producers_t1 = 1;
        int number_of_consumers_t1 = 1;
        threading_test(queue_initial_sizes[i], number_of_producers_t1, number_of_consumers_t1, queue_initial_sizes[i]);

        int number_of_producers_t2 = 3;
        int number_of_consumers_t2 = 5;
        threading_test(queue_initial_sizes[i], number_of_producers_t2, number_of_consumers_t2, queue_initial_sizes[i]);

        int number_of_producers_t3 = 5;
        int number_of_consumers_t3 = 3;
        threading_test(queue_initial_sizes[i], number_of_producers_t3, number_of_consumers_t3, queue_initial_sizes[i]);

        int number_of_producers_t4 = 4;
        int number_of_consumers_t4 = 4;
        threading_test(queue_initial_sizes[i], number_of_producers_t4, number_of_consumers_t4, queue_initial_sizes[i]);
    }
}