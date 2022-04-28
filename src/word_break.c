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
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <libgen.h>
#include "word_break.h"
#include "utils.h"
#include "logger.h"

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
    int isrecursive = producer_args->isrecursive;
    int *error_code = producer_args->error_code;
    *error_code = 0;
    while (!dir_pool->close)
    {
        if (pool_is_empty(dir_pool) && dir_pool->number_of_active_producers == 0)
        {
            break;
        }
        pool_data_type *pool_init_data = pool_dequeue(dir_pool);
        if (pool_init_data != NULL)
        {
            increment_active_producers(dir_pool); // a directory thread is working...
            debug_print("The number of directory threads that are working %d\n", dir_pool->number_of_active_producers);
            int fill_status = fill_pool_and_queue_with_data(pool_init_data->directory_path, dir_pool, file_q, isrecursive);
            if (fill_status == -1)
            {
                *error_code = fill_status;
                error_print("%s\n", "Couldn't fill the data");
            }
            if (pool_init_data->directory_path != NULL)
            {
                free(pool_init_data->directory_path);
            }
            free(pool_init_data);
            decrement_active_producers(dir_pool); // a directory thread is done working...
        }
    }
    debug_print("%s\n", "Closing directory POOL");
    int pool_close_status = pool_close(dir_pool);
    if (pool_close_status != 0)
    {
        error_print("%s\n", "Error detected in producer thread worker function while closing pool.");
        *error_code = pool_close_status;
        pthread_exit(error_code);
    }
    else
    {
        debug_print("%s\n", "Closed directory POOL");
    }
    debug_print("%s\n", "Closing file QUEUE");
    int queue_close_status = queue_close(file_q);
    if (queue_close_status != 0)
    {
        error_print("%s\n", "Error detected in producer thread worker function while closing queue.");
        *error_code = queue_close_status;
        pthread_exit(error_code);
    }
    else
    {
        debug_print("%s\n", "Closed file QUEUE");
    }

    debug_print("%s", "Exiting produce_files_to_wrap\n");

    if (*error_code != 0)
    {
        error_print("%s\n", "Error detected in producer thread worker function");
        pthread_exit(error_code);
    }
    return NULL;
}

void *consume_files_to_wrap(void *arg)
{
    debug_print("%s", "Entering consume_files_to_wrap\n");

    consumer_type *consumer_args = arg;

    // unpack thread args for consumer
    Queue *file_q = consumer_args->file_queue;
    int max_width = consumer_args->max_width;
    int *error_code = consumer_args->error_code;
    *error_code = 0;
    while (!queue_is_empty(file_q) || !file_q->close)
    {
        queue_data_type *q_data_pointer = queue_dequeue(file_q);

        if (q_data_pointer != NULL)
        {
            debug_print("Dequeing file, tid: %ld input file path: %s output file path: %s\n", pthread_self(), q_data_pointer->input_file, q_data_pointer->output_file);
            int wrap_text_status = wrap_text(q_data_pointer->input_file, max_width, q_data_pointer->output_file);
            if (wrap_text_status != 0)
            {
                *error_code = wrap_text_status;
            }
            if (q_data_pointer->input_file != NULL)
            {
                free(q_data_pointer->input_file);
            }
            if (q_data_pointer->output_file != NULL)
            {
                free(q_data_pointer->output_file);
            }
            free(q_data_pointer);
        }
        else
        {
            debug_print("Dequeing file, tid: %ld input file path: NULL output file path: NULL\n", pthread_self());
        }
    }
    debug_print("%s", "Exiting consume_files_to_wrap\n");
    if (*error_code != 0)
    {
        error_print("%s\n", "Error detected in consumer thread worker function");
        pthread_exit(error_code);
    }
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
        error_print("Error when opening file %s for reading\n", optional_input_file);
        return EXIT_FAILURE;
    }
    if (fd_write == -1)
    {
        error_print("Error when opening file %s for writing\n", optional_output_file);
        return EXIT_FAILURE;
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
                error_print("%s\n", "Error reading bytes from a file");
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
    rtn = close(fd_read);
    rtn = close(fd_write);
    return rtn;
}
int fill_pool_and_queue_with_data(char *parent_dir_path, Pool *dir_pool, Queue *file_queue, int isrecursive)
{
    DIR *dfd;
    struct dirent *directory_pointer;

    debug_print("Dequeing parent directory, tid: %ld parent directory path: %s\n", pthread_self(), parent_dir_path);

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
                free(new_file_name);

                debug_print("Input-file found with path %s\n", file_path_in_directory);
                debug_print("Output-file found with path %s\n", output_file_name);

                queue_data_type *qd = malloc(sizeof(queue_data_type));
                if (qd == NULL)
                {
                    error_print("%s\n", "Malloc failure!");
                    return -1;
                }
                qd->input_file = file_path_in_directory;
                qd->output_file = output_file_name;

                int q_en_stat = queue_enqueue(file_queue, qd);
                if (q_en_stat != 0)
                {
                    error_print("%s\n", "Could not enqueue data in file queue");
                    free(qd->input_file);
                    free(qd->output_file);
                    free(qd);
                    return -1;
                }
            }
            else
            {
                free(file_path_in_directory);
            }
        }
        else
        {
            free(file_path_in_directory);
            if (directory_pointer->d_name[0] != '.' && isrecursive)
            {
                // it is a sub-directory
                char *sub_directory_path = append_file_path_to_existing_path(parent_dir_path, directory_pointer->d_name);
                pool_data_type *pd = malloc(sizeof(pool_data_type));
                if (pd == NULL)
                {
                    error_print("%s\n", "Malloc failure!");
                    return -1;
                }
                pd->directory_path = sub_directory_path;
                int p_en_stat = pool_enqueue(dir_pool, pd);
                if (p_en_stat != 0)
                {
                    error_print("%s\n", "Could not enqueue data in directory pool!");
                    free(pd->directory_path);
                    free(pd);
                    return -1;
                }
            }
        }
    }
    int close_dir_status = closedir(dfd);
    if (close_dir_status != 0)
    {
        return -1;
    }
    return 0;
}


int handle_multiple_input_files(int widthindex, int max_width, int argv, char **arg, Pool *dir_pool)
{
    int rtn = 0;
    bool isfile = false;
    int count_files = 0;
    for (int i = widthindex + 1; i < argv; i++)
    {
        char *dir_of_interest = concat_string(arg[i], "\0", -1, -1);
        struct stat file_in_dir;
        int status_of_file_metadata = stat(dir_of_interest, &file_in_dir); // directory_pointer->d_name is the filename.
        if (status_of_file_metadata != 0)
        {
            error_print("Can't get stat of file %s\n", dir_of_interest);
            free(dir_of_interest);
            return status_of_file_metadata;
        }
        if (check_file_or_directory(&file_in_dir) == 1)
        {
            isfile = true;
            // regular file
            char *output_file_name = concat_string("wrap.", basename(dir_of_interest), -1, -1);
            char *output_file_path = append_file_path_to_existing_path(dirname(dir_of_interest), output_file_name);
            if (output_file_name != NULL && output_file_path != NULL)
            {
                rtn = wrap_text(arg[i], max_width, output_file_path);
            }
            else
            {
                rtn = -1;
            }

            if (rtn != 0)
            {
                free(output_file_name);
                free(output_file_path);
                free(dir_of_interest);
                return rtn;
            }
            free(output_file_name);
            free(output_file_path);
            free(dir_of_interest);
        }
        else if (check_file_or_directory(&file_in_dir) == 2)
        {
            pool_data_type *pool_init_data = malloc(sizeof(pool_data_type));
            if (pool_init_data == NULL)
            {
                free(dir_of_interest);
                error_print("%s\n", "Malloc Failure!");
                return -1;
            }
            pool_init_data->directory_path = dir_of_interest;
            rtn = pool_enqueue(dir_pool, pool_init_data);
            if (rtn != 0)
            {
                free(pool_init_data->directory_path);
                free(pool_init_data);
                return rtn;
            }
        }
        count_files++;
    }
    // that means the only input file is a regular file, so print to stdout as per extra credit assignment
    if(isfile && count_files == 1){
        int rtn_val = wrap_text(arg[widthindex + 1], max_width, NULL);
        return rtn_val;
    }

    return 0;
}


int threaded_wrap_program(int producer_threads, int consumer_threads, int max_width, int isrecursive, int widthindex, int argv, char **argc)
{
    // data structures setup
    Queue *file_queue = queue_init(QUEUESIZE);
    if (file_queue == NULL)
    {
        error_print("%s\n", "Failed to init file queue.");
        return EXIT_FAILURE;
    }
    Pool *dir_pool = pool_init(POOLSIZE);
    if (dir_pool == NULL)
    {
        queue_destroy(file_queue);
        error_print("%s\n", "Failed to init directory pool.");
        return EXIT_FAILURE;
    }
    // we are using more than one folder or file name (extra credit section)
    // so we declare a new function to use more than one file or folder that is specified in the user arguments
    int multiple_hander = handle_multiple_input_files(widthindex, max_width, argv, argc, dir_pool);
    if (multiple_hander != 0)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        error_print("%s\n", "Error in handeling multiple file/directory arguements");
        return EXIT_FAILURE;
    }
    // threads setup
    pthread_t *producer_tids = malloc(producer_threads * sizeof(pthread_t));
    if (producer_tids == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }
    pthread_t *consumer_tids = malloc(consumer_threads * sizeof(pthread_t));
    if (consumer_tids == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        free(producer_tids);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }
    // thread arguements.
    producer_type *producer_args = malloc(sizeof(producer_type));
    if (producer_args == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        free(producer_tids);
        free(consumer_tids);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }
    producer_args->file_queue = file_queue;
    producer_args->dir_pool = dir_pool;
    producer_args->isrecursive = isrecursive;
    producer_args->error_code = malloc((sizeof(int)));
    if (producer_args->error_code == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        free(producer_tids);
        free(consumer_tids);
        free(producer_args);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }

    consumer_type *consumer_args = malloc(sizeof(consumer_type));
    if (consumer_args == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        free(producer_tids);
        free(consumer_tids);
        free(producer_args);
        free(producer_args->error_code);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }
    consumer_args->file_queue = file_queue;
    consumer_args->max_width = max_width;
    consumer_args->error_code = malloc((sizeof(int)));
    if (consumer_args->error_code == NULL)
    {
        queue_destroy(file_queue);
        pool_destroy(dir_pool);
        free(producer_tids);
        free(consumer_tids);
        free(producer_args);
        free(producer_args->error_code);
        free(consumer_args);
        error_print("%s\n", "Malloc Failure");
        return EXIT_FAILURE;
    }
    int i = 0;
    int j = 0;
    bool err_create_producer = false; // if at least one producer thread create fails return exit failure.
    bool err_create_consumer = false; // if at least one consumer thread create fails return exit failure.
    for (i = 0; i < producer_threads; i++)
    {
        debug_print("%s\n", "Creating producer thread");
        int created_thread_status = pthread_create(&producer_tids[i], NULL, produce_files_to_wrap, producer_args);
        if (created_thread_status != 0)
        {
            error_print("Could not create producer thread %d\n", i);
            err_create_producer = true;
        }
        debug_print("%s\n", "Created producer thread");
    }
    for (j = 0; j < consumer_threads; j++)
    {
        debug_print("%s\n", "Creating consumer thread");
        int created_thread_status = pthread_create(&consumer_tids[j], NULL, consume_files_to_wrap, consumer_args);
        if (created_thread_status != 0)
        {
            error_print("Could not create producer thread %d\n", i);
            err_create_consumer = true;
        }
        debug_print("%s\n", "Created consumer thread");
    }

    int k = 0;
    int l = 0;
    bool err_joining_producer = false; // if at least one producer thread join fails return exit failure.
    bool err_joining_consumer = false; // if at least one consumer thread join fails return exit failure.

    for (k = 0; k < producer_threads; k++)
    {
        debug_print("%s\n", "Joining producer thread\n");
        int *intermediate_producer_err_ptr;
        int join_status = pthread_join(producer_tids[k], (void **)&(intermediate_producer_err_ptr));
        if (join_status != 0)
        {
            err_joining_producer = true;
            error_print("Error code %d while joining producer thread %d\n", join_status, k);
        }
        if (intermediate_producer_err_ptr != NULL && *intermediate_producer_err_ptr != 0)
        {
            err_joining_producer = true;
            error_print("Error code %d after joining producer thread %d\n", *intermediate_producer_err_ptr, k);
        }
        debug_print("%s\n", "Joined producer thread\n");
    }
    for (l = 0; l < consumer_threads; l++)
    {
        debug_print("%s\n", "Joining consumer thread\n");

        int *intermediate_consumer_err_ptr;
        int join_status = pthread_join(consumer_tids[l], (void **)&(intermediate_consumer_err_ptr));
        if (join_status != 0)
        {
            err_joining_consumer = true;
            error_print("Error code %d while joining producer thread %d\n", join_status, k);
        }
        if (intermediate_consumer_err_ptr != NULL && *intermediate_consumer_err_ptr != 0)
        {
            err_joining_consumer = true;
            error_print("Error code %d after joining consumer thread %d\n", *intermediate_consumer_err_ptr, l);
        }
        debug_print("%s\n", "Joined consumer thread\n");
    }
    free(producer_args->error_code);
    free(consumer_args->error_code);
    free(producer_args);
    free(consumer_args);
    free(producer_tids);
    free(consumer_tids);

    int q_destroy_status = queue_destroy(file_queue);
    if (q_destroy_status != 0)
    {
        error_print("Could not destroy queue. Exited with error code %d\n", q_destroy_status);
        return EXIT_FAILURE;
    }
    int p_destroy_status = pool_destroy(dir_pool);
    if (p_destroy_status != 0)
    {
        error_print("Could not destroy pool. Exited with error code %d\n", p_destroy_status);
        return EXIT_FAILURE;
    }

    if (err_joining_producer || err_joining_consumer || err_create_consumer || err_create_producer)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int main(int argv, char **argc)
{
    // thread params
    int producer_threads;
    int consumer_threads;

    // user interface arguements
    int max_width;
    int isrecursive; // checks if -r is present. 1 if yes else 0
    int widthindex;  // calculates where the max_width is located based on the previous user arguements

    int args_filler_status = fill_param_by_user_arguememt(argv, argc, &max_width, &producer_threads, &consumer_threads, &isrecursive, &widthindex);
    if (args_filler_status != 0)
    {
        error_print("%s\n", "Error with parsing arguements.");
        return EXIT_FAILURE;
    }
    int rtn_val;
    rtn_val = threaded_wrap_program(producer_threads, consumer_threads, max_width, isrecursive, widthindex, argv, argc);
    return rtn_val;
}