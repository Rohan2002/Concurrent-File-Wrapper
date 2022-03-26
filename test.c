#include <dirent.h>
#include<stdio.h>
  
// chdir function is declared
// inside this header
#include<unistd.h> 
int main(int argc, char** argv){
    char* dir_name = "foo";
    chdir(dir_name);

    DIR *d;
    struct dirent *dir;

    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
        printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    return(0);

}