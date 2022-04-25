/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#ifndef POOL_H
#define POOL_H
#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

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
    int number_of_active_producers;
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
int increment_active_producers(Pool *);
int decrement_active_producers(Pool *);
int pool_destroy(Pool *);
#endif
