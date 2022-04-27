#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argv, char **argc)
{
    char *rec_threads = argc[1];
    int strlen_rec_threads = sizeof(rec_threads);

    char *tempM = malloc(strlen_rec_threads);
    int digitsM = 0;
    rec_threads += 2;
    while (rec_threads[digitsM] != ',')
    {
        tempM[digitsM] = rec_threads[digitsM];
        digitsM++;
    }
    tempM[digitsM] = '\0';
    rec_threads += digitsM + 1;
    int producer_threads = atoi(tempM);
    int consumer_threads = atoi(rec_threads);

    printf("%d, %d\n", producer_threads, consumer_threads);
    free(tempM);
}