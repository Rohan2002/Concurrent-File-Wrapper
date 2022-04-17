#include <stdio.h>
#include "../src/logger.h"
#include <string.h>
#include <stdlib.h>
char* append_file_path_to_existing_path(char* existing_path, char* new_file){
    /*
        Appends the name of a directory or regular file to an existing given path.
        Examples:

        Existing path: foo/a/b
        New File/Directory: c

        Return foo/a/b/c
    */
   int plen = strlen(existing_path);
   printf("plen length: %d\n", plen);
   int nlen = strlen(new_file);
    printf("nlen length: %d\n", nlen);
   char* appended_path = malloc(plen + nlen + 2);
   if(appended_path == NULL){
       error_print("%s\n", "Cannot allocate appended_path");
   }
   memcpy(appended_path, existing_path, plen); // Exisitng path.
   char* extender = "/";
   // printf("Character ending: %c\n", appended_path[plen ]);
   if(strcmp(&existing_path[plen - 1], extender) != 0){
        // printf("Last character of existing_path: %c\n", existing_path[plen - 1]);
        appended_path[plen] = *extender;
        memcpy(appended_path + plen + 1, new_file, nlen + 1);
   }
   else{
       memcpy(appended_path + plen, new_file, nlen + 1);
   }
   return appended_path;
}
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