#define DEF_MODE S_IRUSR | S_IWUSR
#define BUF_SIZE 20

typedef struct buffer_data {
    char* buffer_arr;
    int bytes_in_buffer;
} buffer_dtype;