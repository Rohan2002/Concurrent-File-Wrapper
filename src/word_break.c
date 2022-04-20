/*
    @ project
    Word Break 2022.

    @ authors
    Rohan Deshpande and Selin Altiparmak
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "word_break.h"
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "utils.h"
#include "logger.h"

#define QUEUESIZE 100

void *produce_files_to_wrap(void *arg)
{
    /*
        Recursive directory traversal function.

        It's not actually recursive,
        but a directory pool is used to imitate recursion.
    */
    debug_print("%s", "Entering produce_files_to_wrap\n");

    producer_type *producer_args = arg;

    // unpack thread args for producer
    Pool *dir_pool = producer_args->dir_pool;
    Queue *file_q = producer_args->file_queue;
    // int number_of_producers = producer_args->alive_producers;

    while (!pool_is_empty(dir_pool) || dir_pool->number_of_active_producers > 0)
    {
        pool_data_type *pool_init_data = pool_dequeue(dir_pool);
        if (pool_is_empty(dir_pool))
        {
            decrement_producers(dir_pool);
            debug_print("Active directory threads: %d\n", dir_pool->number_of_active_producers);
        }
        else{
            debug_print("Dequed directory: %s\n", pool_init_data->directory_path);
        }
        if (pool_init_data != NULL)
        {
            int fill_status = fill_pool_and_queue_with_data(pool_init_data->directory_path, dir_pool, file_q);
            if (fill_status == -1)
            {
                error_print("%s\n", "Couldn't fill the data");
            }
            // if(pool_init_data->directory_path!=NULL){
            //     free(pool_init_data->directory_path);
            // }
            // free(pool_init_data);
        }
    }
    pool_close(dir_pool);
    queue_close(file_q);

    debug_print("%s", "Exiting produce_files_to_wrap\n");

    return NULL;
}

void *consume_files_to_wrap(void *arg)
{
    debug_print("%s", "Entering consume_files_to_wrap\n");

    consumer_type *consumer_args = arg;

    // unpack thread args for consumer
    Queue *file_q = consumer_args->file_queue;
    int max_width = consumer_args->max_width;
    // Pool *dir_pool = consumer_args->dir_pool;

    while (!queue_is_empty(file_q) || !file_q->close)
    {
        queue_data_type *q_data_pointer = queue_dequeue(file_q);

        if (q_data_pointer != NULL)
        {
            debug_print("Dequeing file, tid: %p input file path: %s output file path: %s\n", pthread_self(), q_data_pointer->input_file, q_data_pointer->output_file);
            wrap_text(q_data_pointer->input_file, max_width, q_data_pointer->output_file);
        }
        free(q_data_pointer->input_file);
        free(q_data_pointer->output_file);
        free(q_data_pointer);
    }
    debug_print("%s", "Exiting consume_files_to_wrap\n");
    return NULL;
}

int wrap_text(char *optional_input_file, int max_width, char *optional_output_file)
{
    bool read_at_least_one_alphanumeric = false;
    int fd_read;
    int rtn = 0;
    fd_read = optional_input_file ? open(optional_input_file, O_RDONLY | O_CREAT, DEF_MODE) : STDIN;

    int fd_write;
    fd_write = optional_output_file ? open(optional_output_file, O_WRONLY | O_CREAT | O_TRUNC, DEF_MODE) : STDOUT;

    if (fd_read == -1)
    {
        perror("Error when opening file for reading");
    }
    if (fd_write == -1)
    {
        perror("Error when opening file for writing");
    }

    int bytes;
    char sentence_buffer[BUFSIZE];
    int pos;

    int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
    char c = '\0';
    char prev_c = '\0';
    int finishing_max_width = max_width;

    char *word_buffer = NULL;
    int next_line_characters = 0;
    while ((bytes = read(fd_read, sentence_buffer, BUFSIZE)) > 0)
    {
        // read buffer and break file into lines
        for (pos = 0; pos < bytes; pos++)
        {
            c = sentence_buffer[pos];
            if (c == '\n')
            {
                next_line_characters += 1;
            }
            if (!isspace(c))
            {
                read_at_least_one_alphanumeric = true;
                // cursor landed on a alphanumeric character...
                // no more new lines.
                if (next_line_characters >= 2)
                {
                    safe_write(fd_write, "\n\n", 2);
                    finishing_max_width = max_width;
                }
                next_line_characters = 0;
            }
            if (prev_c != ' ' && (isspace(c)))
            {
                if (alpha_numeric_count != 0)
                {
                    int adjusted_word_length_with_space = alpha_numeric_count + 1;
                    if (finishing_max_width == max_width)
                    {
                        safe_write(fd_write, word_buffer, alpha_numeric_count);
                        if (alpha_numeric_count > max_width)
                            rtn = EXIT_FAILURE;
                        finishing_max_width -= alpha_numeric_count;
                    }
                    else if (adjusted_word_length_with_space <= finishing_max_width)
                    {
                        safe_write(fd_write, " ", 1);
                        safe_write(fd_write, word_buffer, alpha_numeric_count);
                        finishing_max_width -= adjusted_word_length_with_space;
                    }
                    else
                    {
                        safe_write(fd_write, "\n", 1);
                        if (alpha_numeric_count > max_width)
                            rtn = EXIT_FAILURE;
                        safe_write(fd_write, word_buffer, alpha_numeric_count);
                        finishing_max_width = max_width - alpha_numeric_count;
                    }
                    free(word_buffer);
                    word_buffer = NULL;
                    alpha_numeric_count = 0;
                }
            }
            if (bytes > 0)
            {
                bool current_char_is_alphanumeric = !isspace(c);
                if (current_char_is_alphanumeric)
                {
                    if (alpha_numeric_count == 0)
                    {
                        word_buffer = (char *)malloc(alpha_numeric_count + 1);
                        if (word_buffer == NULL)
                        {
                            error_print("%s", "Malloc failure\n");
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        word_buffer = (char *)realloc(word_buffer, alpha_numeric_count + 1);
                        if (word_buffer == NULL)
                        {
                            error_print("%s", "Realloc failure\n");
                            return EXIT_FAILURE;
                        }
                    }

                    word_buffer[alpha_numeric_count] = c;
                    alpha_numeric_count += 1;
                }
            }
            if (bytes == -1)
            {
                perror("Error reading bytes from a file");
                return EXIT_FAILURE;
            }
            prev_c = c;
        }
    }
    // buffer still exists.
    if (alpha_numeric_count != 0)
    {
        int adjusted_word_length_with_space = alpha_numeric_count + 1;
        if (finishing_max_width == max_width)
        {
            safe_write(fd_write, word_buffer, alpha_numeric_count);
            if (alpha_numeric_count > max_width)
                rtn = EXIT_FAILURE;
            finishing_max_width -= alpha_numeric_count;
        }
        else if (adjusted_word_length_with_space <= finishing_max_width)
        {
            safe_write(fd_write, " ", 1);
            safe_write(fd_write, word_buffer, alpha_numeric_count);
            finishing_max_width -= adjusted_word_length_with_space;
        }
        else
        {
            safe_write(fd_write, "\n", 1);
            if (alpha_numeric_count > max_width)
                rtn = EXIT_FAILURE;
            safe_write(fd_write, word_buffer, alpha_numeric_count);
            finishing_max_width = max_width - alpha_numeric_count;
        }
        alpha_numeric_count = 0;
    }
    // Since each line is terminated by a newline, the final character in a non-empty file will always be a newline.
    if (read_at_least_one_alphanumeric)
    {
        safe_write(fd_write, "\n", 1);
    }
    if (word_buffer != NULL)
    {
        free(word_buffer);
    }
    close(fd_read);
    close(fd_write);
    return rtn;
}
int fill_pool_and_queue_with_data(char *parent_dir_path, Pool *optional_dir_pool, Queue *optional_file_queue)
{
    DIR *dfd;
    struct dirent *directory_pointer;

    debug_print("Dequeing parent directory, tid: %p parent directory path: %s\n", pthread_self(), parent_dir_path);

    if ((dfd = opendir(parent_dir_path)) == NULL)
    {
        error_print("Can't open parent directory %s\n", parent_dir_path);
        return -1;
    }

    while ((directory_pointer = readdir(dfd)) != NULL)
    {
        char *file_path_in_directory = append_file_path_to_existing_path(parent_dir_path, directory_pointer->d_name);
        struct stat file_in_dir;

        int status_of_file_metadata = stat(file_path_in_directory, &file_in_dir); // directory_pointer->d_name is the filename.
        if (status_of_file_metadata != 0)
        {
            error_print("Can't get stat of file %s\n", directory_pointer->d_name);
            return status_of_file_metadata;
        }
        // if its a regular file then add it to file queue
        if (check_file_or_directory(&file_in_dir) == 1)
        {
            // Only wrap files that don't start with wrap. or .
            if (directory_pointer->d_name[0] != '.' && memcmp(directory_pointer->d_name, "wrap.", 5) != 0)
            {
                // output file name computation.
                char *new_file_name = concat_string("wrap.", directory_pointer->d_name, 5, strlen(directory_pointer->d_name));
                char *output_file_name = append_file_path_to_existing_path(parent_dir_path, new_file_name);
                free(new_file_name); // Maybe?

                debug_print("Input-file found with path %s\n", file_path_in_directory);
                debug_print("Output-file found with path %s\n", output_file_name);

                if (optional_file_queue != NULL)
                {
                    queue_data_type *qd = malloc(sizeof(queue_data_type));
                    if (qd == NULL)
                    {
                        error_print("%s\n", "Malloc failure!");
                        return -1;
                    }
                    qd->input_file = file_path_in_directory;
                    qd->output_file = output_file_name;

                    queue_enqueue(optional_file_queue, qd);
                    // print_queue_metadata(file_q, file_q->end - 1);
                }
            }
        }
        else
        {
            if (directory_pointer->d_name[0] != '.')
            {
                // it is a sub-directory
                char *sub_directory_path = append_file_path_to_existing_path(parent_dir_path, directory_pointer->d_name); // TODO: free.
                debug_print("Sub-directory found with path %s %s\n", sub_directory_path, directory_pointer->d_name);

                if (optional_dir_pool != NULL)
                {
                    pool_data_type *pd = malloc(sizeof(pool_data_type));
                    if (pd == NULL)
                    {
                        error_print("%s\n", "Malloc failure!");
                        return -1;
                    }
                    pd->directory_path = sub_directory_path;
                    pool_enqueue(optional_dir_pool, pd);
                }
            }
        }

    }
    closedir(dfd);
    
    return 0;
}
int main(int argv, char **argc)
{
    // wrapping params
    int max_width = 1;

    // thread params
    int producer_threads = 5;
    int consumer_threads = 20;

    // directory of interest
    char *dir_of_interest = "tests/";

    // data structures setup
    Queue *file_queue = queue_init(QUEUESIZE);
    if (file_queue == NULL)
    {
        error_print("%s\n", "Failed to init file queue.");
        return EXIT_FAILURE;
    }
    Pool *dir_pool = pool_init(QUEUESIZE, producer_threads);
    if (dir_pool == NULL)
    {
        error_print("%s\n", "Failed to init directory pool.");
        return EXIT_FAILURE;
    }

    pool_data_type *pool_init_data = malloc(sizeof(pool_data_type));
    pool_init_data->directory_path = dir_of_interest;
    pool_enqueue(dir_pool, pool_init_data);

    // threads setup
    pthread_t *producer_tids = malloc(producer_threads * sizeof(pthread_t));
    pthread_t *consumer_tids = malloc(consumer_threads * sizeof(pthread_t));

    // thread arguements.
    producer_type *producer_args = malloc(sizeof(producer_type));
    producer_args->file_queue = file_queue;
    producer_args->dir_pool = dir_pool;
    producer_args->alive_producers = producer_threads;

    consumer_type *consumer_args = malloc(sizeof(consumer_type));
    consumer_args->file_queue = file_queue;
    consumer_args->max_width = max_width;

    int i = 0;
    int j = 0;
    for (i = 0; i < producer_threads; i++)
    {
        debug_print("%s\n", "Creating producer thread");
        pthread_create(&producer_tids[i], NULL, produce_files_to_wrap, producer_args);
        debug_print("%s\n", "Created producer thread");
    }
    for (j = 0; j < consumer_threads; j++)
    {
        debug_print("%s\n", "Creating consumer thread");
        pthread_create(&consumer_tids[j], NULL, consume_files_to_wrap, consumer_args);
        debug_print("%s\n", "Created consumer thread");
    }

    int k = 0;
    int l = 0;
    for (k = 0; k < producer_threads; k++)
    {
        debug_print("%s\n", "Joining producer thread\n");
        pthread_join(producer_tids[k], NULL);
        debug_print("%s\n", "Joined producer thread\n");
    }
    for (l = 0; l < consumer_threads; l++)
    {
        debug_print("%s\n", "Joining consumer thread\n");
        pthread_join(consumer_tids[l], NULL);
        debug_print("%s\n", "Joined consumer thread\n");
    }

    free(producer_args);
    free(consumer_args);
    free(producer_tids);
    free(consumer_tids);
    queue_destroy(file_queue);
    pool_destroy(dir_pool);
}