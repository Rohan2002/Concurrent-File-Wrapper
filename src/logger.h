#pragma once
#define DEBUG 0
#define debug_print(fmt, ...)                                                    \
    do                                                                                      \
    {                                                                                       \
        if (DEBUG)                                                                          \
            fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
#define error_print(fmt, ...)                                                \
    do                                                                                  \
    {                                                                                   \
        fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
    