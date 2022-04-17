/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#define POOL_DEBUG 1
#define debug_log(MESSAGE)  (POOL_DEBUG ? fprintf(stdout, "Failed to allocate queue!\n"); : (Y))

struct Pool_Data
{
    char *directory_path;
};
typedef struct Pool_Data pool_data_type;

struct Pool_Model
{
    pool_data_type** data;
    int end;
    int pool_size;
    pthread_mutex_t lock;
    pthread_cond_t ready_to_consume;
    int number_of_elements_buffered;
    bool close;
};
typedef struct Pool_Model Pool;
Pool *pool_init(int);
int pool_enqueue(Pool *, pool_data_type *);
pool_data_type* pool_dequeue(Pool *);
bool pool_is_full(Pool *);
bool pool_is_empty(Pool *);
int pool_open(Pool *);
int pool_close(Pool *);