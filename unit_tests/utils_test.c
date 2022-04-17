#include <stdio.h>
#include "../src/logger.h"
#include "../src/utils.h"
#include <string.h>
#include <stdlib.h>
void test_append_file_path_to_existing_path()
{

    char *e_file_name_1 = "/a/b/c";
    char *new_file_1 = "d";

    char *new_path_1 = append_file_path_to_existing_path(e_file_name_1, new_file_1);
    if (strcmp(new_path_1, "/a/b/c/d") == 0)
    {
        debug_print("%s\n", "Passed Test 1");
    }
        else{
        debug_print("Failed Test 1 with output %s\n", new_path_1);
    }
    free(new_path_1);

    char *e_file_name_2 = "/a/b/c/";
    char *new_file_2 = "d";

    char *new_path_2 = append_file_path_to_existing_path(e_file_name_2, new_file_2);
    if (strcmp(new_path_2, "/a/b/c/d") == 0)
    {
        debug_print("%s\n", "Passed Test 2");
    }
    else{
        debug_print("Failed Test 2 with output %s\n", new_path_2);
    }
    free(new_path_2);
}
int main()
{
    test_append_file_path_to_existing_path();
    return EXIT_SUCCESS;
}