#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#define MAX_LINE 1000

void problem_a (){
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0){
        char *argv[] = {"wget", "-O", "netflixData.zip", "https://drive.google.com/uc?export=download&id=12GWsZbSH858h2HExP3x4DfWZB1jLdV-J", NULL};
        execvp("wget", argv);
    } else wait(&status);

    pid = fork();
    if (pid == 0){
        char *argv[] = {"mkdir", "-p", "film", NULL};
        execvp("mkdir", argv);
    } else wait(&status);

    pid = fork();
    if (pid == 0){
        char *argv[] = {"unzip", "-o", "netflixData.zip", "-d", "film", NULL};
        execvp("unzip", argv);
    } else wait(&status);

    pid = fork();
    if (pid == 0){
        char *argv[] = {"rm", "-f", "netflixData.zip", NULL};
        execvp("rm", argv);
    } else wait(&status);
}

void separator (char *baris, char *kolom[]){
    int flag = 0, indeks_kolom = 0, indeks_temp = 0;
    char temp[MAX_LINE], *ptr = baris;

    while (*ptr != '\0'){
        if (*ptr == '"'){
            flag = !flag;
            temp[indeks_temp++] = *ptr;
        } else if (*ptr == ',' && flag == 0){
            temp[indeks_temp] = '\0';
            kolom[indeks_kolom++] = strdup(temp);
            indeks_temp = 0;
        } else {
            temp[indeks_temp++] = *ptr;
        }
        ptr++;
    }

    temp[indeks_temp] = '\0';
    kolom[indeks_kolom++] = strdup(temp);
}

void file_log (char *kategori, char *judul){
    FILE *log = fopen("log.txt", "a");

    char jam[9];

    time_t total_detik;
    struct tm *detail_waktu;
    time(&total_detik);
    detail_waktu = localtime(&total_detik);

    strftime(jam, sizeof(jam), "%H:%M:%S", detail_waktu);

    fprintf(log, "[%s] Proses mengelompokkan berdasarkan %s: sedang mengelompokkan untuk film %s\n", jam, kategori, judul);
    fclose(log);
}


void judul (char *file_csv){
    int status;
    pid_t pid = fork();
    if (pid == 0){
        char *argv[] = {"mkdir", "-p", "judul", NULL};
        execvp("mkdir", argv);
    } else wait(&status);

    FILE *file = fopen(file_csv, "r");

    char baris[MAX_LINE];
    fgets(baris, MAX_LINE, file);

    while (fgets(baris, MAX_LINE, file)){
        baris[strcspn(baris, "\n")] = '\0';

        char *kolom[4];
        separator(baris, kolom);

        char *judul = kolom[0];
        char *sutradara = kolom[1];
        char *tahun = kolom[3];

        file_log("Abjad", judul);

        char karakter = judul[0];
        char nama_file[20];

        if (isalnum(karakter)) snprintf(nama_file, sizeof(nama_file), "judul/%c.txt", karakter);
        else snprintf(nama_file, sizeof(nama_file), "judul/#.txt");

        FILE *txt = fopen(nama_file, "a");
        fprintf(txt, "%s - %s - %s\n", judul, tahun, sutradara);
        fclose(txt);
    }
    fclose(file);
}

void tahun (char *file_csv){
    int status;
    pid_t pid = fork();
    if (pid == 0){
        char *argv[] = {"mkdir", "-p", "tahun", NULL};
        execvp("mkdir", argv);
    } else wait(&status);

    FILE *file = fopen(file_csv, "r");

    char baris[MAX_LINE];
    fgets(baris, MAX_LINE, file);

    while (fgets(baris, MAX_LINE, file)){
        baris[strcspn(baris, "\n")] = '\0';

        char *kolom[4];
        separator(baris, kolom);

        char *judul = kolom[0];
        char *sutradara = kolom[1];
        char *tahun = kolom[3];

        file_log("Tahun", judul);

        char nama_file[15];
        snprintf(nama_file, sizeof(nama_file), "tahun/%s.txt", tahun);

        FILE *txt = fopen(nama_file, "a");
        fprintf(txt, "%s - %s - %s\n", judul, tahun, sutradara);
        fclose(txt);
    }
    fclose(file);
}

typedef struct Report {
    char negara[100];
    int sebelum;
    int setelah;
} Report;

Report statistik[100];
int jumlah_negara = 0;

void *negara (void *arg){
    char *file_csv = (char*)arg;
    FILE *file = fopen(file_csv, "r");

    char baris[MAX_LINE];
    fgets(baris, MAX_LINE, file);

    while (fgets(baris, MAX_LINE, file)){
        baris[strcspn(baris, "\n")] = '\0';

        char *kolom[4];
        separator(baris, kolom);

        char *negara = kolom[2];
        int tahun = atoi(kolom[3]);

        int i;
        for (i = 0; i < jumlah_negara; i++){
            if (strcmp(statistik[i].negara, negara) == 0){
                if (tahun < 2000) statistik[i].sebelum++;
                else statistik[i].setelah++;
                break;
            }
        }

        if (i == jumlah_negara){
            strcpy(statistik[jumlah_negara].negara, negara);
            if (tahun < 2000){
                statistik[jumlah_negara].sebelum = 1;
                statistik[jumlah_negara].setelah = 0;
            } else {
                statistik[jumlah_negara].sebelum = 0;
                statistik[jumlah_negara].setelah = 1;
            }
            jumlah_negara++;
        }
    }
    fclose(file);
    return NULL;
}

void laporan (char *file_csv){
    pthread_t thread;
    pthread_create(&thread, NULL, negara, file_csv);
    pthread_join(thread, NULL);

    char nama_file[100], tanggal[10];

    time_t total_detik;
    struct tm *detail_waktu;
    time(&total_detik);
    detail_waktu = localtime(&total_detik);

    strftime(tanggal, sizeof(tanggal), "%d%m%Y", detail_waktu);
    sprintf(nama_file, "report_%s.txt", tanggal);
    FILE *report = fopen(nama_file, "w");

    for (int i = 0; i < jumlah_negara; i++){
        fprintf(report, "%d. Negara: %s\n", i+1, statistik[i].negara);
        fprintf(report, "Film sebelum 2000: %d\n", statistik[i].sebelum);
        fprintf(report, "Film setelah 2000: %d\n\n", statistik[i].setelah);
    }
    fclose(report);
}

int main (){
    char *path = "film/netflixData.csv";
    int pilihan;

    for (;;){
        printf("1. Download File\n");
        printf("2. Mengelompokkan Film\n");
        printf("3. Membuat Report\n");
        printf("Pilihan: ");
        scanf("%d", &pilihan);

        if (pilihan == 1) problem_a();
        else if (pilihan == 2){
            int status;
            pid_t pid1 = fork();
            if (pid1 == 0){
            	judul(path);
		exit(0);
	    }
            wait(&status);

            pid_t pid2 = fork();
            if (pid2 == 0){
		tahun(path);
		exit(0);
	    }
            wait(&status);
        } else if (pilihan == 3) laporan(path);
         else break;
    }
}
