#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Gagal membuat fork");
        return 1;
    } else if (pid == 0) {
        char* downloadargv[6] = {"curl", "-L", "-o", "film.zip", "https://drive.google.com/uc?export=download&id=1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B", NULL};
        execv("/usr/bin/curl", downloadargv); 
        perror("Gagal mendownload zip");
        exit(1); 
    }
    
    wait(NULL);

    pid = fork();
    if (pid < 0) {
        perror("Gagal membuat fork");
        exit(1);
    } else if (pid == 0) {
        char* unzipargv[3] = {"unzip", "film.zip", NULL};
        execv("/usr/bin/unzip", unzipargv);  
        perror("Gagal meng-unzip");
        exit(1); 
    }

    wait(NULL);
    return 0;
}