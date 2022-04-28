/*
    @ project
    Word Break 2022.

    Synchronized Unbounded Stack.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include "pool.h"
#include "logger.h"
#include <string.h>
#include <stdio.h>

Pool *pool_init(int pool_size)
{
    Pool *pool_pointer = malloc(sizeof(Pool));
    if (pool_pointer == NULL)
    {
        error_print("%s", "Failed to allocate pool!\n");
        return NULL;
    }
    pool_pointer->data = (pool_data_type **)malloc(sizeof(pool_data_type *) * pool_size);
    pool_pointer->end = 0;
    pool_pointer->pool_size = pool_size;
    pool_pointer->number_of_elements_buffered = 0;
    pool_pointer->close = false;
    pool_pointer->number_of_active_producers = 0;

    int mutex_init_status = pthread_mutex_init(&(pool_pointer->lock), NULL);
    if (mutex_init_status != 0)
    {
        error_print("Failed to INIT pool mutex with error code: %d!\n", mutex_init_status);
        return NULL;
    }
    int ready_to_consume_status = pthread_cond_init(&(pool_pointer->ready_to_consume), NULL);
    if (ready_to_consume_status != 0)
    {
        error_print("Failed to INIT ready_to_consume pthread condition variable: %d!\n", ready_to_consume_status);
        return NULL;
    }

    debug_print("Created pool mutex at address %p\n", &(pool_pointer->lock));
    debug_print("Created pool at address %p\n", pool_pointer);

    return pool_pointer;
}
int pool_enqueue(Pool *pool_pointer, pool_data_type *data)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }
    if (pool_is_full(pool_pointer))
    {
        // realloc sucks lol. https://stackoverflow.com/a/46461293/10016132
        pool_data_type **new_pool = (pool_data_type **)malloc(2 * pool_pointer->pool_size * sizeof(pool_data_type *));

        if (new_pool == NULL)
        {
            error_print("%s", "Failed to re-malloc pool_pointer!\n");
            return -1;
        }

        memcpy(new_pool, pool_pointer->data, pool_pointer->pool_size * sizeof(pool_data_type *)); // pool_pointer->pool_size * sizeof(pool_data_type *) = old-size data.
        free(pool_pointer->data);

        pool_pointer->data = new_pool;
        pool_pointer->pool_size *= 2;

        debug_print("Reallocated stack with new size as %d!\n", pool_pointer->pool_size);
    }

    pool_pointer->data[pool_pointer->end++] = data;
    pool_pointer->number_of_elements_buffered += 1;
    pthread_cond_signal(&(pool_pointer->ready_to_consume));

    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return unlock_status;
    }
    return 0;
}
pool_data_type *pool_dequeue(Pool *pool_pointer)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return NULL;
    }
    while (pool_is_empty(pool_pointer))
    {
        if (pool_pointer->close)
        {
            pthread_mutex_unlock(&pool_pointer->lock);
            return NULL;
        }
        debug_print("%s\n", "Pool is empty... waiting for data to come inside pool!");
        int wait_status = pthread_cond_wait(&(pool_pointer->ready_to_consume), &pool_pointer->lock);
        if (wait_status != 0)
        {
            error_print("Failed to wait with error code: %d!\n", wait_status);
            return NULL;
        }
    }

    pool_data_type *dequed_data = pool_pointer->data[--pool_pointer->end];
    pool_pointer->number_of_elements_buffered--;

    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return NULL;
    }
    return dequed_data;
}
bool pool_is_full(Pool *pool_pointer)
{
    return pool_pointer->number_of_elements_buffered == pool_pointer->pool_size;
}
bool pool_is_empty(Pool *pool_pointer)
{
    return pool_pointer->number_of_elements_buffered == 0;
}
int pool_open(Pool *pool_pointer)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }

    pool_pointer->close = false;

    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return lock_status;
    }
    return 0;
}
int pool_close(Pool *pool_pointer)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }

    pool_pointer->close = true;
    int broad_stat = pthread_cond_broadcast(&pool_pointer->ready_to_consume);
    if (broad_stat != 0)
    {
        error_print("Failed to broadcast with error code: %d!\n", broad_stat);
        return broad_stat;
    }
    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return lock_status;
    }
    return 0;
}
int decrement_active_producers(Pool *pool_pointer)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }

    pool_pointer->number_of_active_producers--;

    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return lock_status;
    }
    return 0;
}
int increment_active_producers(Pool *pool_pointer)
{
    int lock_status = pthread_mutex_lock(&pool_pointer->lock);
    if (lock_status != 0)
    {
        error_print("Failed to lock with error code: %d!\n", lock_status);
        return lock_status;
    }

    pool_pointer->number_of_active_producers++;

    int unlock_status = pthread_mutex_unlock(&pool_pointer->lock);
    if (unlock_status != 0)
    {
        error_print("Failed to unlock with error code: %d!\n", unlock_status);
        return lock_status;
    }
    return 0;
}
int pool_destroy(Pool *pool_pointer)
{
    // In theory pool should be empty at this point. However, we cannot guarantee anything so just check if pool is empty, and free from there.
    if (pool_pointer->number_of_elements_buffered != 0)
    {
        error_print("%s", "From Pool Destroy(), the pool is still not empty!\n");
        while (!pool_is_empty(pool_pointer))
        {
            pool_data_type *p_data = pool_dequeue(pool_pointer);
            if (p_data != NULL)
            {
                if (p_data->directory_path != NULL)
                {
                    free(p_data->directory_path);
                }
                free(p_data);
            }
        }
    }
    int mutex_destroy_status = pthread_mutex_destroy(&pool_pointer->lock);
    if (mutex_destroy_status != 0)
    {
        error_print("pool mutex can't be destroyed with error code: %d\n", mutex_destroy_status);
        return mutex_destroy_status;
    }
    int consume_condition_variable_destroy = pthread_cond_destroy(&pool_pointer->ready_to_consume);
    if (consume_condition_variable_destroy != 0)
    {
        error_print("Condition variable for consume can't be destroyed with error code: %d\n", consume_condition_variable_destroy);
        return mutex_destroy_status;
    }
    free(pool_pointer->data); // At this point all pointers exisiting inside queue_pointer->data is freed, so free queue_pointer->data.
    free(pool_pointer);
    debug_print("%s", "Pool has been destroyed!\n");
    return 0;
}