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

        It's not actually recursive, but a directory pool is used to imitate recursion.
    */
    debug_print("%s", "Entering consumer thread\n");

    producer_type *d_args = arg;
    
    Pool *dir_pool = d_args->dir_pool;
    Queue *file_q = d_args->file_queue;
    char* initial_directory_name = d_args->initial_directory;

    // append the first directory to the pool so that loop has something to work with.
    pool_data_type* pool_init_data = malloc(sizeof(pool_data_type));
    pool_init_data->directory_path = initial_directory_name;
    pool_enqueue(dir_pool, pool_init_data);
    
    char extension[6] = "wrap.";
    while (!pool_is_empty(dir_pool))
    {
        pool_data_type* pool_init_data = pool_dequeue(dir_pool);

        if (pool_init_data != NULL)
        {
            char* dir_path = pool_init_data->directory_path;
            debug_print("Dequeing, tid: %p directory path: %s\n", pthread_self(), dir_path);

            DIR *dfd;
            struct dirent *directory_pointer;

            if ((dfd = opendir(dir_path)) == NULL)
            {
                error_print("Can't open directory %s\n", dir_path);
                return NULL;
            }

            while ((directory_pointer = readdir(dfd)) != NULL)
            {
                struct stat file_in_dir;

                int status_of_file_metadata = stat(directory_pointer->d_name, &file_in_dir); // directory_pointer->d_name is the filename.
                if (status_of_file_metadata == -1)
                {
                    error_print("Can't get stat of file %s\n", directory_pointer->d_name);
                    return NULL;
                }
                // if its a regular file then add it to file queue
                if (check_file_or_directory(&file_in_dir) == 1)
                {

                    char *extension_str = (char *)malloc((sizeof(extension) + sizeof(directory_pointer->d_name)) * sizeof(char));
                    if (extension_str == NULL)
                    {
                        error_print("%s","Malloc failure\n");
                        return NULL;
                    }
                    strcpy(extension_str, extension); // copy wrap. into string

                    char *file_name_with_extension = strcat(extension_str, directory_pointer->d_name); // copy rest of the filename in the string.

                    // Only wrap files that don't start with wrap. or .
                    if (memcmp(directory_pointer->d_name, ".", strlen(".")) != 0 && memcmp(directory_pointer->d_name, extension, strlen(extension)) != 0)
                    {
                        queue_data_type *qd = malloc(sizeof(queue_data_type));
                        qd->input_file = (char *)malloc(1 + strlen(directory_pointer->d_name) * sizeof(char));
                        qd->output_file = (char *)malloc(1 + strlen(file_name_with_extension) * sizeof(char));
                        strcpy(qd->input_file, directory_pointer->d_name);
                        strcpy(qd->output_file, file_name_with_extension);
                        qd->w = q_data_pointer->w;
                        queue_enqueue(file_q, qd);
                        print_queue_metadata(file_q, file_q->end - 1);
                    }
                    free(extension_str);
                }
                else
                { // it is a directory
                    queue_data_type *qd = malloc(sizeof(queue_data_type));
                    qd->input_file = (char *)malloc(1 + strlen(directory_pointer->d_name) * sizeof(char));
                    qd->output_file = NULL;
                    strcpy(qd->input_file, directory_pointer->d_name);
                    qd->w = q_data_pointer->w;
                    queue_enqueue(dir_q, qd);
                }
            }
            closedir(dfd);
        }
    }
    if (DEBUG)
    {
        printf("Exiting consumer thread\n");
    }

    return NULL;
}

void *consumer_wrapper_worker(void *arg)
{
    if (DEBUG)
    {
        printf("Entering consumer thread\n");
    }

    consumer_worker_type *consumer_args = arg;
    Queue *file_q = consumer_args->file_queue;

    while (!queue_is_empty(file_q))
    {
        queue_data_type *q_data_pointer = queue_dequeue(file_q);

        if (q_data_pointer != NULL)
        {
            if (DEBUG)
            {
                printf("Dequeing, tid: %p input_file: %s, num_elem: %lu\n", pthread_self(), q_data_pointer->input_file, file_q->number_of_elements_buffered);
            }
            wrap_text(q_data_pointer->input_file, q_data_pointer->w, q_data_pointer->output_file);
        }
    }
    if (DEBUG)
    {
        printf("Exiting consumer thread\n");
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
                            error_print("Malloc failure\n");
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        word_buffer = (char *)realloc(word_buffer, alpha_numeric_count + 1);
                        if (word_buffer == NULL)
                        {
                            error_print("Realloc failure\n");
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
int wrap_text_for_directory(char *dir_name, int max_width, Queue *file_queue, int run_mode)
{
    DIR *dfd;
    struct dirent *directory_pointer;
    char extension[6] = "wrap.";
    int rtn = 0;
    if ((dfd = opendir(dir_name)) == NULL)
    {
        error_print("Can't open %s\n", dir_name);
        return EXIT_FAILURE;
    }

    int directory_of_interest_change_status = chdir(dir_name);

    if (directory_of_interest_change_status == -1)
    {
        error_print("Can't change directory to %s\n", dir_name);
        return EXIT_FAILURE;
    }

    while ((directory_pointer = readdir(dfd)) != NULL)
    {
        struct stat file_in_dir;

        int status_of_file_metadata = stat(directory_pointer->d_name, &file_in_dir); // directory_pointer->d_name is the filename.
        if (status_of_file_metadata == -1)
        {
            error_print("Can't get stat of file %s\n", directory_pointer->d_name);
            return EXIT_FAILURE;
        }
        // if its a file then do the stuff.
        if (check_file_or_directory(&file_in_dir) == 1)
        {

            char *extension_str = (char *)malloc((sizeof(extension) + sizeof(directory_pointer->d_name)) * sizeof(char));
            if (extension_str == NULL)
            {
                error_print("Malloc failure\n");
                return EXIT_FAILURE;
            }
            strcpy(extension_str, extension); // copy wrap. into string

            char *file_name_with_extension = strcat(extension_str, directory_pointer->d_name); // copy rest of the filename in the string.

            // Only wrap files that don't start with wrap. or .
            if (memcmp(directory_pointer->d_name, ".", strlen(".")) != 0 && memcmp(directory_pointer->d_name, extension, strlen(extension)) != 0)
            {
                if (run_mode == 0)
                {
                    rtn = wrap_text(directory_pointer->d_name, max_width, file_name_with_extension);
                }
                else if (run_mode == 2)
                {
                    queue_data_type *qd = malloc(sizeof(queue_data_type));
                    qd->input_file = (char *)malloc(1 + strlen(directory_pointer->d_name) * sizeof(char));
                    qd->output_file = (char *)malloc(1 + strlen(file_name_with_extension) * sizeof(char));
                    strcpy(qd->input_file, directory_pointer->d_name);
                    strcpy(qd->output_file, file_name_with_extension);
                    qd->w = max_width;
                    queue_enqueue(file_queue, qd);
                    print_queue_metadata(file_queue, file_queue->end - 1);
                }
            }
            free(extension_str);
        }
    }
    closedir(dfd);

    return rtn;
}

int main(int argv, char **argc)
{
    int rtn = 0;
    int N = -1;
    int M = -1;
    int run_mode = get_run_mode(argc[1], &N, &M);
    // printf("runmode : %d, M: %d, N:%d\n",rm,M,N);
    if (argv < 2)
    {
        error_print("%s","At least provide the max_width argument\n");
        return EXIT_FAILURE;
    }
    int max_width = atoi(argc[argv - 2]);
    // int rtn = -1;

    // If the file name is not present, ww will read from standard input and print to standard output.
    if (argv == 2)
    {
        wrap_text(NULL, max_width, NULL);
    }
    else
    {
        // If the file name is a regular file, ww will read from the file and print to standard output.
        char *file_name = argc[argv - 1];

        struct stat dir_status;
        stat(file_name, &dir_status);

        // If the file name is a directory, ww will open each regular file in the directory and write to a new
        if (check_file_or_directory(&dir_status) == 2)
        {

            if (run_mode == 0)
            {
                rtn = wrap_text_for_directory(file_name, max_width, NULL, run_mode);
            }
            else if (run_mode == 2)
            {
                Queue *file_queue = queue_init(QUEUESIZE);

                int number_of_wrapper_threads = N; // TODO: put this in user interface.

                pthread_t *wrapper_threads = malloc(number_of_wrapper_threads * sizeof(pthread_t));
                consumer_worker_type *consumer_args = malloc(number_of_wrapper_threads * sizeof(consumer_worker_type));

                rtn = wrap_text_for_directory(file_name, max_width, file_queue, run_mode); // TODO: currently running in main thread. Need to make a producer model for this.

                int i;
                for (i = 0; i < number_of_wrapper_threads; i++)
                {
                    consumer_args[i].file_queue = file_queue;

                    pthread_create(&wrapper_threads[i], NULL, consumer_wrapper_worker, &consumer_args[i]);
                }

                // wait for all threads to finish
                // int total = 0;
                for (i = 0; i < number_of_wrapper_threads; i++)
                {
                    if (DEBUG)
                    {
                        printf("Joining consumer wrapper thread %d\n", i);
                    }
                    pthread_join(wrapper_threads[i], NULL);
                    if (DEBUG)
                    {
                        printf("Joined consumer wrapper thread %d\n", i);
                    }
                }
            }
            else if (run_mode == 3)
            {
                Queue *file_queue = queue_init(QUEUESIZE);
                Queue *dir_queue = queue_init(QUEUESIZE);
                queue_data_type *qd = malloc(sizeof(queue_data_type));
                qd->input_file = (char *)malloc(1 + strlen(file_name) * sizeof(char));
                qd->output_file = NULL;
                strcpy(qd->input_file, file_name);
                qd->w = max_width;
                queue_enqueue(dir_queue, qd);

                int number_of_wrapper_threads = N;
                int number_of_directory_threads = M;

                pthread_t *wrapper_threads = malloc(number_of_wrapper_threads * sizeof(pthread_t));
                consumer_worker_type *arg_file = malloc(number_of_wrapper_threads * sizeof(consumer_worker_type));
                pthread_t *directory_threads = malloc(number_of_directory_threads * sizeof(pthread_t));
                consumer_worker_type *arg_dir = malloc(number_of_directory_threads * sizeof(consumer_worker_type));

                int i;
                for (i = 0; i < number_of_wrapper_threads; i++)
                {
                    arg_dir[i].dir_queue = dir_queue;
                    arg_dir[i].file_queue = file_queue;
                    pthread_create(&directory_threads[i], NULL, consumer_directory_worker, &arg_dir[i]);
                }

                for (i = 0; i < number_of_wrapper_threads; i++)
                {
                    arg_file[i].file_queue = file_queue;
                    pthread_create(&wrapper_threads[i], NULL, consumer_wrapper_worker, &arg_file[i]);
                }

                // wait for all threads to finish
                // int total = 0;
                for (i = 0; i < number_of_wrapper_threads; i++)
                {
                    if (DEBUG)
                    {
                        printf("Joining consumer wrapper thread %d\n", i);
                    }
                    pthread_join(wrapper_threads[i], NULL);
                    if (DEBUG)
                    {
                        printf("Joined consumer wrapper thread %d\n", i);
                    }
                }

                for (i = 0; i < number_of_directory_threads; i++)
                {
                    if (DEBUG)
                    {
                        printf("Joining consumer wrapper thread %d\n", i);
                    }
                    pthread_join(directory_threads[i], NULL);
                    if (DEBUG)
                    {
                        printf("Joined consumer wrapper thread %d\n", i);
                    }
                }

                // rtn = wrap_text_for_directory(file_name, max_width);
            }
            else if (check_file_or_directory(&dir_status) == 1)
            {
                // If the file name is a regular file, ww will read from the file and print to standard output.
                rtn = wrap_text(file_name, max_width, NULL);
            }
            else
            {
                error_print("Invalid file path. Please input only a regular file or directory. Input file: %s\n", file_name);
                return EXIT_FAILURE;
            }
        }
    }
    return rtn;
}