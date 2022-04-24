#pragma once
#define DEBUG 1
#define debug_print(fmt,__VA_ARGS__ ...)                                                    \
    do                                                                                      \
    {                                                                                       \
        if (DEBUG)                                                                          \
            fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
#define error_print(fmt,__VA_ARGS__ ...)                                                \
    do                                                                                  \
    {                                                                                   \
        fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while (0)
