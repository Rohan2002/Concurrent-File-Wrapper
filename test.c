#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int main(int argv, char **argc)
{
    int producer_threads = 0;
    int consumer_threads = 0;
    char *r_with_number_of_threads = argc[1];
    int strlen_r_with_number_of_threads = sizeof(r_with_number_of_threads);

    char *str_producer_thread = malloc(strlen_r_with_number_of_threads);
    int producer_thread = 0;
    r_with_number_of_threads += 2; // skip the first two characters "-r"
    while (r_with_number_of_threads[producer_thread] != ',')
    {
        str_producer_thread[producer_thread] = r_with_number_of_threads[producer_thread];
        producer_thread++;
    }
    str_producer_thread[producer_thread] = '\0';
    r_with_number_of_threads += producer_thread + 1; // get string after ,

    producer_threads = MAX(1, atoi(str_producer_thread));
    consumer_threads = MAX(1, atoi(r_with_number_of_threads));

    free(str_producer_thread);
    printf("%d, %d\n", producer_threads, consumer_threads);
}