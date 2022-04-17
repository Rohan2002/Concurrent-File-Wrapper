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
void test_concat_strings()
{
    char *added_1 = concat_string("five", "rohan", -1, -1);
    // length 1 < length 2
    if (strcmp(added_1, "fiverohan") == 0)
    {
        debug_print("%s", "Passed Concat String Test 1\n");
    }
    else
    {
        debug_print("Failed Concat String Test 1: %s\n", added_1);
    }
    free(added_1);
    // length 1 > length2
    char *added_2 = concat_string("rohan", "five", -1, -1);
    if (strcmp(added_2, "rohanfive") == 0)
    {
        debug_print("%s", "Passed Concat String Test 2\n");
    }
    else
    {
        debug_print("Failed Concat String Test 2: %s\n", added_2);
    }
    free(added_2);
    // same length.
    char *added_3 = concat_string("rohan", "selin", -1, -1);
    if (strcmp(added_3, "rohanselin") == 0)
    {
        debug_print("%s", "Passed Concat String Test 3\n");
    }
    else
    {
        debug_print("Failed Concat String Test 3: %s\n", added_3);
    }
    free(added_3);
    char *added_4 = concat_string("", "", -1, -1);
    if (strcmp(added_4, "") == 0)
    {
        debug_print("%s", "Passed Concat String Test 4\n");
    }
    else
    {
        debug_print("Failed Concat String Test 4: %s\n", added_4);
    }
    free(added_4);
    char *added_5 = concat_string("s", "", -1, -1);
    if (strcmp(added_5, "s") == 0)
    {
        debug_print("%s", "Passed Concat String Test 5\n");
    }
    else
    {
        debug_print("Failed Concat String Test 5: %s\n", added_5);
    }
    free(added_5);
    char *added_6 = concat_string("", "s", -1, -1);
    if (strcmp(added_6, "s") == 0)
    {
        debug_print("%s", "Passed Concat String Test 6\n");
    }
    else
    {
        debug_print("Failed Concat String Test 6: %s\n", added_6);
    }
    free(added_6);
    char *test_one = "lorem";
    char *test_two = "ipsum";

    int one_length = strlen(test_one);
    int two_length = strlen(test_two);
    char *added_7 = concat_string(test_one, test_two, one_length, two_length);
    if (strcmp(added_7, "loremipsum") == 0)
    {
        debug_print("%s", "Passed Concat String Test 7\n");
    }
    else
    {
        debug_print("Failed Concat String Test 7: %s\n", added_7);
    }
    free(added_7);
}
int main()
{
    test_concat_strings();
    test_append_file_path_to_existing_path();
    return EXIT_SUCCESS;
}
// cd .. && make obj/utils.o && cd unit_tests/ && make && ./bin/utils_test