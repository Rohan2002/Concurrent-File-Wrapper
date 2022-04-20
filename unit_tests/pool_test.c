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
int producer_data = 0;
pthread_mutex_t producer_data_lock;

void *producer(void *vargs)
{
    worker_type *w_a = vargs;

    Pool *pool_pointer = w_a->pool_pointer;
    int producer_data_size = w_a->size;
    while(!pool_pointer->close)
    {
        pthread_mutex_lock(&producer_data_lock);
        debug_print("Firing thread: %p, Producer_data: %d and pool_close status: %s\n", pthread_self() ,producer_data, pool_pointer->close ? "yes" : "no");
        if(producer_data > producer_data_size){
            break;
        }
        int length = snprintf(NULL, 0, "%d", producer_data);
        char *str = malloc(length + 1);
        snprintf(str, length + 1, "%d", producer_data);

        pool_data_type *ppd = malloc(sizeof(pool_data_type));

        ppd->directory_path = str;

        debug_print("About to enqueue...%s\n", ppd->directory_path);
        pool_enqueue(pool_pointer, ppd);
        debug_print("Producer Enqued: %s with total count as %d \n", ppd->directory_path,  pool_pointer->number_of_elements_buffered);
        producer_data+=1;
        pthread_mutex_unlock(&producer_data_lock);
    }
    int pool_close_status = pool_close(pool_pointer);
    if(pool_close_status == 0){
        debug_print("Exited producer and pool_close status: %s \n", pool_pointer->close ? "yes" : "no");
    }
    else{
        error_print("%s %d\n", "Could not exit producer with error code: ", pool_close_status);
    }
    return NULL;
}
void *consumer(void *vargs)
{
    debug_print("%s\n", "Entering consumer");
    worker_type *w_a = vargs;

    Pool *pool_pointer = w_a->pool_pointer;

    while (!pool_is_empty(pool_pointer) || !pool_pointer->close)
    {
        debug_print("%s %s\n", "About to deque...", pool_pointer->close ? "pool is closed" : "pool is open");
        pool_data_type *dq = pool_dequeue(pool_pointer);
        if (dq != NULL)
        {
            debug_print("Consumer Dequed: %s with total count as %d\n", dq->directory_path, pool_pointer->number_of_elements_buffered);
            // TODO free.
            free(dq->directory_path);
            free(dq);
        }
        else
        {
            debug_print("Consumer Dequed: %s\n", "NULL");
        }
    }
    debug_print("%s\n", "Exited consumer");
    return NULL;
}
void vanilla_test()
{
    /*
        Without any threading.
    */
    Pool *pool = pool_init(4000, 1);

    for (int a = 0; a < 4000; a++)
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
        }
    }
}
void threading_test(Pool *pool, int n_producers, int n_consumers, int data_size)
{
    int number_of_producer = n_producers;
    int number_of_consumer = n_consumers;

    pthread_mutex_init(&producer_data_lock, NULL);

    pthread_t *producer_tids = malloc(number_of_producer * sizeof(pthread_t));
    pthread_t *consumer_tids = malloc(number_of_consumer * sizeof(pthread_t));

    worker_type worker_args = {pool, data_size};

    for (int i = 0; i < number_of_producer; i++)
    {
        debug_print("%s and created thread %p\n", "Creating producer thread\n", pthread_self() );
        pthread_create(&producer_tids[i], NULL, producer, &worker_args);
        debug_print("%s\n", "Created producer thread\n");
    }
    for (int j = 0; j < number_of_consumer; j++)
    {
        debug_print("%s\n", "Creating consumer thread\n");
        pthread_create(&consumer_tids[j], NULL, consumer, &worker_args);
        debug_print("%s\n", "Created consumer thread\n");
    }
    for (int k = 0; k < number_of_producer; k++)
    {
        debug_print("%s\n", "Joining producer thread\n");
        pthread_join(producer_tids[k], NULL);
        debug_print("%s\n", "Joined producer thread\n");
    }
    for (int l = 0; l < number_of_consumer; l++)
    {
        debug_print("%s\n", "Joining consumer thread\n");
        pthread_join(consumer_tids[l], NULL);
        debug_print("%s\n", "Joined consumer thread\n");
    }
    printf("Pool end: %d\n", pool->end);
    free(producer_tids);
    free(consumer_tids);
}
int main()
{
    // vanilla_test();
    int data_size = 10;
    int number_of_producers = 1;
    int number_of_consumers = 5;
    Pool* pool = pool_init(1, number_of_producers);
    threading_test(pool, number_of_producers, number_of_consumers, data_size);
    pool_destroy(pool);
}