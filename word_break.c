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

void print_buffer(char local_buffer_arr[BUFSIZE], int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%c", local_buffer_arr[i]);
    }
    printf("\n");
}

int wrap_text(char *file_name, int max_width, char local_buffer_arr[BUFSIZE])
{
    int fd_read;
    int rtn=0;
    fd_read = open(file_name, O_RDWR | O_CREAT, DEF_MODE);

    int fd_write;
    fd_write = open("wrap_out", O_RDWR | O_CREAT | O_TRUNC, DEF_MODE);
    
    if (fd_read == -1 || fd_write == -1)
    {
        perror("File Open Error");
    }
    int  bytes;
    char buf[BUFSIZE];
    int pos;



    int alpha_numeric_count = 0; // Just created a fancy way of saying the word length.
    char c = '\0';
    char prev_c = '\0';
    int finishing_max_width = max_width;
    //int first_word=1;
    local_buffer_arr=NULL;
    int prev_newlines=2;

    while ((bytes = read(fd_read, buf, BUFSIZE)) > 0) {
	// read buffer and break file into lines


        for (pos = 0; pos < bytes; pos++) {
                c=buf[pos];
            if ((prev_c != ' ' && c == ' ') || (prev_c != '\n' && c == '\n'))
            {

                // From write-up, "no more than one space occurs between words". Well... trim the excess fat :)
                // if buffer has at least one non-white space character then we can assume buffer 
                // is not "completely" made of white spaces
                // Notice: This will only occur if a line "starts with" a sequence of white space characters
                bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;
                
                if (!whole_buffer_has_only_white_spaces){
                    // greedy word-wrap algorithm
                    // first word in the line, print it without spaces
                    if (finishing_max_width==max_width){
                            if (alpha_numeric_count>max_width) rtn=EXIT_FAILURE;                        
                            write(fd_write, local_buffer_arr, alpha_numeric_count);
                            finishing_max_width = max_width - alpha_numeric_count;
                    } else {
                        //  Before printing a word, check whether it will fit on the current line.
                        
                        int adjusted_word_length_with_space = alpha_numeric_count + 1;
                        if(adjusted_word_length_with_space <= finishing_max_width){
                            write(fd_write, " ", 1);
                            write(fd_write, local_buffer_arr, alpha_numeric_count);
                            finishing_max_width -= alpha_numeric_count;
                            //first_word =0;
                        } 
                        else{
                            write(fd_write, "\n", 1);
                            //     first_word =0;
                            if (alpha_numeric_count>max_width) rtn=EXIT_FAILURE;
                            write(fd_write, local_buffer_arr, alpha_numeric_count);
                            finishing_max_width = max_width - alpha_numeric_count;
                        }
                        prev_newlines=0;

                    }
                    
                } else {
                    if (prev_newlines==1) {
                        write(fd_write, "\n", 1);
                    }
                    prev_newlines++;

                }
                if (local_buffer_arr!=NULL)
                    free(local_buffer_arr);
                local_buffer_arr=NULL;
                alpha_numeric_count = 0;
            }

            else if (bytes == -1){
                // handle read error.
                perror("Read Error:");
                break;
            }
            else 
            {   
                bool current_char_is_alphanumeric = c != ' ' && c != '\n';
                if (current_char_is_alphanumeric)
                {
                    if (alpha_numeric_count==0){
                        local_buffer_arr= (char*)malloc(alpha_numeric_count+1);
                        //local_buffer_arr[0]=' ';
                    } else {
                        local_buffer_arr = (char*)realloc(local_buffer_arr, alpha_numeric_count + 1);

                    }
                    
                    local_buffer_arr[alpha_numeric_count] = c;
                    alpha_numeric_count += 1;
                    
                }
            }            
            prev_c = c;
        }
    }
    // remaining buffer.
    //  If the while loop has finished but there remains a last word
    // that has not been processed yet, we process it here
    bool whole_buffer_has_only_white_spaces = alpha_numeric_count == 0;
                
    if (!whole_buffer_has_only_white_spaces){
        // greedy word-wrap algorithm
        // first word in the line, print it without spaces
        if (finishing_max_width==max_width){
                if (alpha_numeric_count>max_width) rtn=EXIT_FAILURE;                        
                write(fd_write, local_buffer_arr, alpha_numeric_count);
                finishing_max_width = max_width - alpha_numeric_count;
        } else {
            //  Before printing a word, check whether it will fit on the current line.
            
            int adjusted_word_length_with_space = alpha_numeric_count + 1;
            if(adjusted_word_length_with_space <= finishing_max_width){
                write(fd_write, " ", 1);
                write(fd_write, local_buffer_arr, alpha_numeric_count);
                finishing_max_width -= alpha_numeric_count;
                //first_word =0;
            } 
            else{
                write(fd_write, "\n", 1);
                //     first_word =0;
                if (alpha_numeric_count>max_width) rtn=EXIT_FAILURE;
                write(fd_write, local_buffer_arr, alpha_numeric_count);
                finishing_max_width = max_width - alpha_numeric_count;
            }

        }
        
    }
    if (local_buffer_arr!=NULL)
            free(local_buffer_arr);

    close(fd_read);
    close(fd_write);
    return rtn;
}

int main(int argv, char **argc)
{
    // init buffer with buffer array
    char *local_buffer_arr=NULL;

    //memset(local_buffer_arr, 0, BUFSIZE);
                        
    int rtn = wrap_text("tests/test3.txt", 20, local_buffer_arr);
    return rtn;
}