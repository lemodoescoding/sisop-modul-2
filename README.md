[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/9LcL5VTQ)
| NRP | Name |
| :--------: | :------------: |
| 5025241054 | Andie Azril Alfrianto |
| 5025241060 | Christina Tan |
| 5025241061 | Ahmad Satrio Arrohman |

# Praktikum Modul 2 _(Module 2 Lab Work)_

</div>

### Daftar Soal _(Task List)_

- [Task 1 - Trabowo & Peddy Movie Night](/task-1/)

- [Task 2 - Organize and Analyze Anthony's Favorite Films](/task-2/)

- [Task 3 - Cella’s Manhwa](/task-3/)

- [Task 4 - Pipip's Load Balancer](/task-4/)

### Laporan Resmi Praktikum Modul 2 _(Module 2 Lab Work Report)_

### Laporan Task 2 - Organize and Analyze Anthony's Favorite Films

#### Soal A - One Click and Done!

```c
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
```

##### Penjelasan
Pada fungsi `problem_a()`. Pertama, menggunakan fork() untuk membuat child process, lalu execvp() menjalankan perintah `wget` untuk mengunduh file netflixData.zip dari google drive. Lalu, program membuat folder baru bernama film menggunakan `mkdir -p film`. Setelah itu, file ZIP yang telah diunduh akan diekstrak ke dalam folder `film/` dengan perintah `unzip -o netflixData.zip -d film`. Terakhir, file ZIP yang sudah diekstrak akan dihapus menggunakan perintah `rm -f netflixData.zip`. Setiap proses berjalan satu per satu dengan wait() untuk memastikan setiap anak proses selesai sebelum melanjutkan ke proses berikutnya.

![Csv](/assets/task-2/soalA.png "SS hasil kode.")

##### Kendala
Tidak ada.

#### Soal B - Sorting Like a Pro

```c
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
```

![Kelompok](/assets/task-2/soalB-kelompok.png "SS hasil kode.")
![Log](/assets/task-2/soalB-log.png "SS hasil kode.")

##### Penjelasan
Pada soal Sorting Like a Pro, program bertujuan untuk mengelompokkan film berdasarkan huruf pertama dari judul film dan tahun rilisnya. Fungsi `separator()` membaca per karakter dan membedakan mana koma yang sebagai pemisah kolom dan mana yang hanya bagian dari teks di dalam tanda kutip dengan variabel `flag` sebagai penanda sehingga setiap baris dapat dipisahkan menjadi array kolom. Fungsi `file_log()` digunakan untuk mencatat setiap aktivitas pengelompokan film ke dalam file `log.txt` dengan format waktu real-time [HH:MM:SS] dengan keterangan proses tersebut kategori "Abjad" atau "Tahun" dan nama film yang dikelompokkan.

Fungsi `judul()`. Pertama membuat folder `judul/` dengan perintah `mkdir`. Lalu membuka file CSV film/netflixData.csv. Lalu skip header dengan perintah `fgets` dan membaca baris per baris, kemudian memisahkan isi baris dengan memanggil fungsi `separator()`. Mengambil huruf pertama dari judul film, menentukan apakah huruf tersebut alfanumerik atau tidak menggunakan `isalnum()` dan menuliskan informasi `Judul - Tahun - Sutradara` ke dalam file teks yang sesuai. Setiap perulangan juga memanggil `file_log` untuk mencatat aktifitas ke dalam `log.txt`. Sedangkan, di fungsi `tahun()` memproses hal yang sama dengan fungsi `judul()` yang membedakan hanya membuat folder `tahun/` dan mengelompokkan film berdasarkan tahun rilis. Agar kedua pengelompokan ini terjadi secara paralel dengan cara menggunakan 2 child proses di `main()`, yaitu pertama membuat child process untuk memanggil fungsi `judul()` dan parent process melakukan wait(). Lalu membuat child process kedua untuk memanggil `tahun()` dan juga dilanjutkan dengan `wait()` sehingga kedua pengelompokan bisa berjalan bersamaan.

##### Kendala
Saat mengelompokkan kolom sesuai header karena ada satu kolom terdapat beberapa data sehingga tidak bisa dipisahkan dengan menggunakan `strtok`

#### Soal C - The Ultimate Movie Report

```c
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
```

![UI & Report](/assets/task-2/soalC.png "SS hasil kode.")

##### Penjelasan
 `struct Report` berisi `negara` untuk menyimpan nama negara, serta `sebelum` dan `setelah` untuk menghitung berapa banyak film dari negara tersebut yang dirilis sebelum dan sesudah tahun 2000. Lalu fungsi `negara()` membuka file CSV dan dibaca per baris dan setiap baris dipisahkan kolomnya menggunakan fungsi `separator()`, diambil data negara dari kolom ketiga dan tahun rilis dari kolom keempat. Lalu dilakukan perulangan dan membandingkan `statistik[i].negara` dengan `negara` jika tidak ada maka `negara` tersebut akan dicopy. Jika `tahun` < 200 maka `sebelum` akan diinisialisai 1 dan `setelah` 0 dan `jumlah_negara` ditambah 1 dan sebaliknya. Jika sudah pernah dicopy maka akan ditambaha sesuai tahunnya dan di break.	

 Setelah semua baris diproses, fungsi `negara()` akan mengembalikan NULL. Setelah thread selesai melakukan tugasnya dan selesai bergabung (pthread_join). Program membuat file laporan dengan nama report_ddmmyyyy.txt di fungsi `laporan()` di mana tanggalnya diambil dari waktu saat ini menggunakan `strftime()`. Lalu diisi dengan daftar negara yang sudah diproses sebelumnya, diawali dengan nomor urut, nama negara, jumlah film sebelum 2000, dan jumlah film setelah 2000. Lalu menambahkan antarmuka terminal di fungsi main.

##### Kendala
Tidak ada.
