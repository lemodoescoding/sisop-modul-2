#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(){
    const char* zip[]={"FilmAnimasi.zip", "FilmHorror.zip", "FilmDrama.zip"};
    const char* folder[]={"film/Filmanimasi", "film/Filmhorror", "film/Filmdrama"};

    for(int i=0; i<3; i++){
        pid_t pid=fork();

        if(pid<0){
            perror("Tidak bisa membuat proses");
            return 1;
        } else if (pid==0){
            char* argv[5]={"zip", "-r", (char*)zip[i], (char*)folder[i], NULL};
            execv("/usr/bin/zip", argv);
            perror("Gagal membuat zip");
            exit(1);
        }
    }

    for(int i=0; i<3; i++){
        wait(NULL);
    }
}