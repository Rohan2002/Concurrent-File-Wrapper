#include <stdlib.h>
#include <stdio.h>
#include "../src/logger.h"
#include "../src/pool.h"

struct worker_args_model
{
    Pool *pool_pointer;
    int size;
};
typedef struct worker_args_model worker_type;

void *producer(void *vargs)
{
    // produce n elements of data
    debug_print("%s\n", "Entering producer");
    worker_type *w_a = vargs;

    Pool *pool_pointer = w_a->pool_pointer;
    int data_size = w_a->size;
    while (!pool_pointer->close && data_size != 0)
    {
        pool_data_type *ppd = malloc(sizeof(pool_data_type));
        ppd->directory_path = "test-string";
        pool_enqueue(pool_pointer, ppd);
        debug_print("Thread Enqueued with tid %ld and data_size is %d\n", pthread_self(), data_size);
        data_size--;
    }
    int pool_close_status = pool_close(pool_pointer);
    if (pool_close_status == 0)
    {
        debug_print("Exited producer and pool_close status: %s \n", pool_pointer->close ? "yes" : "no");
    }
    else
    {
        error_print("%s %d\n", "Could not exit producer with error code: ", pool_close_status);
    }
    debug_print("%s\n", "Exited producer");
    return NULL;
}
void *consumer(void *vargs)
{
    // consume whatever is in the pool until its empty.
    debug_print("%s\n", "Entering consumer");
    worker_type *w_a = vargs;

    Pool *pool_pointer = w_a->pool_pointer;

    while (!pool_is_empty(pool_pointer) || !pool_pointer->close)
    {
        pool_data_type *dq = pool_dequeue(pool_pointer);
        if (dq != NULL)
        {
            debug_print("Thread Dequeued with tid %ld\n", pthread_self());
            free(dq);
        }
    }
    debug_print("%s\n", "Exited consumer");
    return NULL;
}
void vanilla_test(int pool_init_size)
{
    /*
        Without any threading.
    */
    Pool *pool = pool_init(pool_init_size);

    for (int a = 0; a < pool_init_size; a++)
    {
        int length = snprintf(NULL, 0, "%d", a);
        char *str = malloc(length + 1);
        snprintf(str, length + 1, "%d", a);

        pool_data_type *ppd = malloc(sizeof(pool_data_type));

        ppd->directory_path = str;

        pool_enqueue(pool, ppd);
        debug_print("Enqueued...%s\n", ppd->directory_path);
    }
    while (!pool_is_empty(pool))
    {
        pool_data_type *ppd = pool_dequeue(pool);
        if (ppd != NULL)
        {
            debug_print("Dequed...%s\n", ppd->directory_path);
            free(ppd);
            
        }
    }
    printf("Is the pool of initial size %d empty after vanilla test? %s\n",pool_init_size, pool_is_empty(pool) ? "Yes" : "No");
    pool_destroy(pool);
}
void threading_test(int pool_init_size, int n_producers, int n_consumers, int data_size)
{
    Pool *pool = pool_init(pool_init_size);
    pthread_t *producer_tids = malloc(n_producers * sizeof(pthread_t));
    pthread_t *consumer_tids = malloc(n_consumers * sizeof(pthread_t));

    worker_type worker_args = {pool, data_size};

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
    printf("Is the pool of initial size %d empty after vanilla test? %s\n",pool_init_size, pool_is_empty(pool) ? "Yes" : "No");
    free(producer_tids);
    free(consumer_tids);
    pool_destroy(pool);
}
int main()
{
    int data_size = 1000;
    int pool_initial_sizes[4] = {1, 10, 100, 1000};
    for(int i = 0; i < 4; i++){
        vanilla_test(pool_initial_sizes[i]);
    
    int number_of_producers_t1 = 1;
    int number_of_consumers_t1 = 1;
    threading_test(pool_initial_sizes[i], number_of_producers_t1, number_of_consumers_t1, data_size);

    int number_of_producers_t2 = 3;
    int number_of_consumers_t2 = 5;
    threading_test(pool_initial_sizes[i], number_of_producers_t2, number_of_consumers_t2, data_size);

    int number_of_producers_t3 = 5;
    int number_of_consumers_t3 = 3;
    threading_test(pool_initial_sizes[i], number_of_producers_t3, number_of_consumers_t3, data_size);
    
    }
}