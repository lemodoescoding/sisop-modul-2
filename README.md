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

Tulis laporan resmi di sini!

_Write your lab work report here!_

#### Laporan task 1 - Trabowo & Peddy Movie Night

#### Soal A - Ekstraksi File ZIP

```c
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
```
![Hasil curl & unzip](/assets2/taskpertama/task1-soalA.png "SS 1A")
![Hasil curl & unzip](/assets2/taskpertama/task1-soalA2.png "SS 1A")

##### Penjelasan
Dalam soal ini, menggunakan 2 fork. Fork pertama untuk mendownload ```film.zip``` yang terdapat pada link gdrive. Pada fork, jika pid < 0 maka fork gagal membuat proses baru. Jika pid == 0 maka fork berhasil membuat proses child. proses child akan menjalankan
perintah download menggunakan ```downloadargv[6]``` yang berisi perintah ```curl -L -o film.zip https://drive.google.com/uc?export=download&id=1nP5kjCi9ReDk5ILgnM7UCnrQwFH67Z9B```. ```curl``` adalah command untuk mendownload data dari internet. ```-L``` artinya menyuruh ```curl``` mengikuti redirect kalau link diarahkan. ```-o film.zip``` untuk
menyimpan hasil download ke file lokal bernama ```film.zip```. Lalu execv digunakan untuk mengeksekusi argumen yang diberikan. Jika execv berhasil dijalankan, maka program apapun yang tertulis setelahnya tidak akan dijalankan. Jika tidak berhasil, maka ```perror``` akan dijalankan dan mengeluarkan pesan error serta program akan exit dengan status code 1. 

Pada kode, ```wait(NULL)``` digunakan untuk menunggu proses child hingga selesai. ```NULL``` artinya tidak memedulikan exit status dari proses child, hanya tunggu sampai selesai mendownload. Kemudian, fork kedua digunakan untuk mengunzip file ```film.zip```.

###### Kendala
Awalnya, saya tidak tahu kalau soalnya juga bermaksud untuk membuat program untuk mendownload film.zip dari link yang diberikan.

#### Soal B - Pemilihan Film Secara Acak

```c
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

int main(){
    DIR *folder=opendir("film");

    struct dirent *entri;
    char *listfilm[50];
    int count=0;

    while((entri=readdir(folder))!=NULL){
        int len=strlen(entry->d_name);
        if(len>4 && strcmp(entry->d_name+len-4, ".jpg")==0){
            listfilm[count]=strdup(entry->d_name);
            count++;
        }
    }
    closedir(folder);

    srand(time(NULL));

    int index=rand()%count;

    printf("Film for Trabowo & Peddy: %s\n", listfilm[index]);

    for(int i=0; i<count; i++) free(listfilm[i]);

    return 0;
}
```

![Hasil pemilihan film](/assets2/taskpertama/task1-soalB.png "SS 1B")


##### Penjelasan
Pada kode, ```DIR *folder=opendir("film")``` digunakan untuk membuka folder film dan ```*folder``` adalah pointer ke direktori filenya. 
Pada ```struct dirent *entri```, ```*entri``` adalah pointer ke struct dirent yang merupakan struct yang disediakan oleh Linux dan didefinisikan dalam library ```dirent.h```.
Lalu, ```listfilm[50]``` untuk menyimpan judul-judul film nantinya (terdapat 50 film). ```count``` untuk menghitung jumlah file yang ditemukan.

Menggunakan loop ```while((entri=readdir(folder))!=NULL)``` untuk membaca setiap entri dalam folder hingga akhir.
Lalu, jika filenya berakhiran dengan ekstensi ```.jpg``` maka akan masuk ke dalam array listfilm dan count pun bertambah. ```closedir(folder)``` untuk menutup folder.
Lalu, ```srand(time(NULL))``` akan digunakan untuk mengenerate angka random yang berbeda-beda setiap kali program dijalankan. ``` int index=rand()%count``` akan menghasilkan angka random dari 0 hingga count-1.
Lalu, printf akan mencetak judul film yang berbeda setiap kali program dijalankan. ```free(listfilm[i])``` untuk membebaskan memori.

###### Kendala
Tidak ada.

#### Soal C - Memilah Film Berdasarkan Genre

```c
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_logend = PTHREAD_MUTEX_INITIALIZER;

int animasi = 0, drama = 0, horor = 0;

typedef struct {
  char **files;
  char **full_paths;
  char *penulis;
  int count, i_start, i_end;
  int *genre_counter;
} ThreadArgs;

void createFolderMk(const char *dirname) {
  char path[256];
  snprintf(path, sizeof(path), "film/%s", dirname);
  mkdir(path, 0755); 
}


void writeActivityLog(const char *judul, const char *genre,
                      const char *penulis) {
  time_t now = time(NULL);
  struct tm *tme = localtime(&now);

  pthread_mutex_lock(&mutex_log);
  FILE *log = fopen("recap.txt", "a");
  if (log) {
    fprintf(log,
            "[%02d-%02d-%4d %02d:%02d:%02d] %s: %s telah dipindahkan ke %s\n",
            tme->tm_mday, tme->tm_mon + 1, tme->tm_year + 1900, tme->tm_hour,
            tme->tm_min, tme->tm_sec, penulis, judul, genre);
    fclose(log);
  }
  usleep(200);
  pthread_mutex_unlock(&mutex_log);
}

int compareFileName(const void *a, const void *b) {
  char *strA = strdup(*(char **)a);
  char *strB = strdup(*(char **)b);

  char *token_a = strtok(strA, "_");
  char *token_b = strtok(strB, "_");

  int num_a = atoi(token_a);
  int num_b = atoi(token_b);

  return num_a - num_b;
}

void listFiles(char *dirname, char ***files_out, char ***full_path_out,
               int *count) {
  char cwd[512];
  getcwd(cwd, sizeof(cwd));

  DIR *dir = opendir(dirname);
  struct dirent *ent;

  char **files = NULL;
  char **full_paths = NULL;
  int cnt = 0;

  while ((ent = readdir(dir)) != NULL) {
    if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
      continue;

    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s/%s", cwd, dirname, ent->d_name);

    struct stat st;
    if (stat(fullpath, &st) == 0 && !S_ISDIR(st.st_mode)) {
      files = realloc(files, sizeof(char *) * (cnt + 1));
      full_paths = realloc(full_paths, sizeof(char *) * (cnt + 1));
      files[cnt] = strdup(ent->d_name);
      full_paths[cnt] = strdup(ent->d_name);
      cnt++;
    }
  }

  closedir(dir);

  qsort(files, cnt, sizeof(char *), compareFileName);
  qsort(full_paths, cnt, sizeof(char *), compareFileName);

  for (int i = 0; i < cnt; i++) {
    char *filename = strdup(full_paths[i]);
    char full[1024];

    snprintf(full, sizeof(full), "%s/%s/%s", cwd, dirname, filename);

    full_paths[i] = strdup(full);

    free(filename);
  }

  *count = cnt;
  *files_out = files;
  *full_path_out = full_paths;
}

void *moveFilesThread(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;

  char cwd[512];
  getcwd(cwd, sizeof(cwd));

  int count_tot = args->i_end - args->i_start + 1;
  int penulis_sign = (strcmp(args->penulis, "Trabowo") == 0) ? 1 : 0;

  int jml = 0;
  int i = 0;

  if (penulis_sign == 1) {
    i = 0;
  } else {
    i = args->i_end;
  }

  while (jml < count_tot) {

    int len = strlen(args->files[i]);

    char after[1024] = {0};

    if (strcmp(args->files[i] + len - 4, ".jpg") == 0 &&
        strcmp(args->files[i] + len - 9, "drama.jpg") == 0) {
      snprintf(after, sizeof(after), "%s/film/FilmDrama", cwd);
      writeActivityLog(args->files[i], "FilmDrama", args->penulis);
      drama++;
    } else if (strcmp(args->files[i] + len - 4, ".jpg") == 0 &&
               strcmp(args->files[i] + len - 11, "animasi.jpg") == 0) {
      snprintf(after, sizeof(after), "%s/film/FilmAnimasi", cwd);
      writeActivityLog(args->files[i], "FilmAnimasi", args->penulis);
      animasi++;
    } else if (strcmp(args->files[i] + len - 4, ".jpg") == 0 &&
               strcmp(args->files[i] + len - 10, "horror.jpg") == 0) {
      snprintf(after, sizeof(after), "%s/film/FilmHorror", cwd);
      writeActivityLog(args->files[i], "FilmHorror", args->penulis);
      horor++;
    }

    pthread_mutex_lock(&mutex_log);
    pid_t cp_pid;
    if ((cp_pid = fork()) == 0) {
      char *argv[] = {"mv", args->full_paths[i], after, NULL};
      execv("/bin/mv", argv);
    }

    waitpid(cp_pid, NULL, 0);

    pthread_mutex_unlock(&mutex_log);

    usleep(200);

    if (penulis_sign == 1) {
      i++;
    } else {
      i--;
    }

    jml++;
  }

  return NULL;
}

void writeTotalLog() {
  FILE *pf = fopen("total.txt", "w");
  if (pf) {
    fprintf(pf, "Jumlah film horror: %d\n", horor);
    fprintf(pf, "Jumlah film animasi: %d\n", animasi);
    fprintf(pf, "Jumlah film drama: %d\n", drama);
    fprintf(pf, "Genre dengan jumlah film terbanyak: ");

    if (horor >= animasi && horor >= drama)
      fprintf(pf, "horror\n");
    else if (animasi >= horor && animasi >= drama)
      fprintf(pf, "animasi\n");
    else
      fprintf(pf, "drama\n");

    fclose(pf);
  }
}

void sortFileGenre() {
  createFolderMk("FilmHorror");
  createFolderMk("FilmAnimasi");
  createFolderMk("FilmDrama");

  char **files, **full_paths;
  int count;

  int i_begin = 0;
  int i_mid = 0;

  listFiles("film", &files, &full_paths, &count);

  if (count % 2 == 0) {
    i_mid = count / 2 - 1;
  } else {
    i_mid = count / 2;
  }
  int i_end = count - 1;

  printf("%d %d %d\n", count, i_mid, i_begin);

  pthread_t sort_th[2];
  ThreadArgs sort_argst, sort_argnd;

  sort_argst.files = (char **)malloc(sizeof(char *) * (count));
  memcpy(sort_argst.files, files, sizeof(files[0]) * count);
  sort_argst.full_paths = (char **)malloc(sizeof(char *) * (count));
  memcpy(sort_argst.full_paths, full_paths, sizeof(full_paths[0]) * count);
  sort_argst.i_start = i_begin;
  sort_argst.i_end = i_mid;
  sort_argst.count = count / 2;
  sort_argst.penulis = strdup("Trabowo");

  sort_argnd.files = (char **)malloc(sizeof(char *) * (count));
  memcpy(sort_argnd.files, files, sizeof(files[0]) * count);
  sort_argnd.full_paths = (char **)malloc(sizeof(char *) * (count));
  memcpy(sort_argnd.full_paths, full_paths, sizeof(full_paths[0]) * count);
  sort_argnd.i_start = i_mid + 1;
  sort_argnd.i_end = i_end;
  sort_argnd.count = count / 2;
  sort_argnd.penulis = strdup("Peddy");

  pthread_t sort_tid[2];

  pthread_mutex_lock(&mutex_logend);
  pthread_create(&sort_tid[0], NULL, moveFilesThread, (void *)&sort_argst);
  pthread_create(&sort_tid[1], NULL, moveFilesThread, (void *)&sort_argnd);

  pthread_join(sort_tid[0], NULL);
  pthread_join(sort_tid[1], NULL);
  pthread_mutex_unlock(&mutex_logend);

  for (int i = 0; i < count; i++) {
    free(files[i]);
    free(full_paths[i]);
  }

  pthread_mutex_lock(&mutex_logend);
  writeTotalLog();
  pthread_mutex_unlock(&mutex_logend);
}


int main() {
  sortFileGenre();
  writeTotalLog();
  return 0;
}
```

![Hasil pindah folder](/assets2/taskpertama/task1-soalC.png "SS 1C")
![Hasil recap.txt](/assets2/taskpertama/task1-soalC2.png "SS 1C")
![Hasil total.txt](/assets2/taskpertama/task1-soalC3.png "SS 1C")

##### Penjelasan
Pertama, membuat mutex untuk digunakan nanti, buat variabel global, dan juga struct ```ThreadArgs```. void ```createFolderMk``` untuk membuat direktori baru, yaitu FilmAnimasi, FilmHorror, dan FilmDrama. Kemudian, void ```writeActivityLog``` untuk membuat log ```recap.txt```, menggunakan mutex sehingga hanya 1 thread dalam satu waktu yang bisa menulis ke ```recap.txt```, jika tidak menggunakan mutex bisa terjadi race condition. Lalu, menggunakan ```usleep(200)``` untuk memberi jeda thread bekerja agar tidak terjadi race condition.

Lalu, fungsi int ```compareFileName``` digunakan untuk membandingkan file berdasarkan nomor di awal file. Lalu, pada fungsi void ```listFiles``` akan membaca semua entri dalam folder film, menyimpan nama file dan path, dan menggunakan qsort untuk mengurutan file berdasarkan nomor awalnya. Lalu, pada fungsi void ```*moveFilesThread``` akan dijalankan oleh masing-masing thread, yaitu trabowo dan peddy. Lalu, akan dicek genre berdasarkan nama filenya, kemudian menentukan tujuan foldernya dan menjalankan fungsi ```writeActivityLog```. Tidak lupa untuk membebaskan memori dengan free.

Kemudian, fungsi void ```writeTotalLog``` untuk menulis ke total.txt jumlah film masing-masing genre dan juga top filmnya. Lalu, pada fungsi void ```sortFileGenre``` akan membuat folder FilmAnimasi, FilmDrama, dan FilmHorror. Kemudian, dipanggil fungsi ```listFiles``` untuk mendapatkan semua file yang sudah terurut. File akan dibagi 2 bagian, bagian trabowo (1-25) dan peddy (26-50), lalu buat 2 thread untuk memindahkan file secara pararel. Setelah selesai, memori files dan paths akan dibebaskan. Lalu, pada fungsi main tinggal memanggil fungsi ```sortFileGenre``` dan ```writeTotalLog```

##### Kendala
Kode saya direvisi karena hasil log dalam recap.txt tidak berurutan. Pada awalnya, membuat 3 thread untuk masing-masing genre, tapi sekarang saya ganti menjadi 2 thread untuk trabowo dan peddy.

#### Soal D - ZIP Folder Baru

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  const char *zip[] = {"FilmAnimasi.zip", "FilmHorror.zip", "FilmDrama.zip"};
  const char *folder[] = {"film/FilmAnimasi", "film/FilmHorror",
                          "film/FilmDrama"};

  for (int i = 0; i < 3; i++) {
    pid_t pid = fork();

    if (pid < 0) {
      perror("Tidak bisa membuat proses");
      return 1;
    } else if (pid == 0) {
      char *argv[5] = {"zip", "-r", (char *)zip[i], (char *)folder[i], NULL};
      execv("/usr/bin/zip", argv);
      perror("Gagal membuat zip");
      exit(1);
    }

    wait(NULL);
  }

  for (int i = 0; i < 3; i++) {
    if (fork() == 0) {
      char *argv[] = {"rm", "-r", (char *)folder[i], NULL};
      execv("/bin/rm", argv);
    }
  }

  for (int i = 0; i < 3; i++) {
    wait(NULL);
  }
}
```

![Hasil terminal](/assets2/taskpertama/task1-soalD.png "SS 1D")
![Hasil zip folder](/assets2/taskpertama/task1-soalD2.png "SS 1D")

##### Penjelasan
deklarasi array zip dan array folder. Kemudian, buat 3 fork untuk mengzip masing-masing genre. Jika pid < 0 artinya fork gagal, dan jika pid == 0, child proses akan menjalankan execv yang berisi argumen untuk mengzip folder. Jika execv tidak berhasil dijalankan, 
maka akan mengeluarkan pesan error dan exit(1). Kemudian, ```wait(NULL)``` untuk menunggu proses child selesai dijalankan. 

Setelah itu, buat 3 fork lagi untuk menjalankan perintah remove folder. Kemudian, buat ```wait(NULL)``` sebanyak 3 kali untuk menunggu execv berhasil dijalankan.

#### Kendala
Awalnya, saya tidak tahu bahwa setelah mengzip kembali folder, harus dilakukan juga penghapusan folder lama, karena tidak tertera secara eksplisit pada soal.

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

#### Laporan Task 3 - Cella's Manhwa

Compile dengan

```bash
gcc solver.c -o solver -pthread -ljson-c -lcurl
```

```c
void createFolderSysCall(const char *folderName) {
  char cwd[256] = {0}, path[512] = {0};
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report_and_error("getcwd() error...");

  snprintf(path, sizeof(path), "%s/%s", cwd, folderName);

  long res = syscall(SYS_mkdir, path, 0755);
}
```

Dalam pengerjaan Task 3, kami menggunakan sebuah fungsi yang dapat dipanggil untuk membuat suatu folder dengan nama
yang disediakan oleh parameter dari fungsi tersebut. Fungsi tersebut adalah createFolderSysCall yang mempunyai satu argumen
berupa `const char *foldername` yang berisi string yang akan menjadi nama baru folder yang akan dibuat oleh fungsi ini.
Pembuatan folder menggunakan bantuan fungsi lain yakni `syscall` dengan 3 argumen yakni `SYS_mkdir`, `path`, `0755`.
`SYS_mkdir` adalah nama system call yang digunakan dalam membuat folder, kemudian `path` adalah path absolut untuk memberitahu
fungsi syscall dimana letak folder baru yang akan dibuat. Kemudian `0755` adalah folder permission. Untuk mendapatkan absolute path
digunakan sebuah fungsi `getcwd` yang nantinya akan mengembalikan current working directory dimana file solver.c dijalankan.

```c
void report_and_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
```

Selain fungsi createFolderSysCall, juga ada fungsi report_and_error yang digunakan untuk mempersingkat penulisan perror yang digunakan
statement yang ada dalam fungsi report_and_error. Fungsi report_and_error akan dipanggil dalam kasus ketika terjadi kesalahan dalam pemanggilan
fungsi lain seperti `fopen`,`curl_easy_init` dsb. Fungsi report_and_error memiliki satu parameter yang berisi custom message error
dan di dalam fungsi report_and_error terdapat pemanggilan fungsi `perror` dengan argumen berisi custom message error tadi. Fungsi `perror`
akan memberikan suatu buffer ke `stderr` dengan tambahan keterangan error otomatis saat perror ini dipanggil.
Kemudian jika fungsi report_and_error maka program akan langsung berhenti karena adanya pemanggilan fungsi `exit` yang memaksa
program untuk diberhentikan eksekusinya.

#### Soal A - Summoning the Manhwa's Stats

```c
const char *mh_urls[4] = {"https://api.jikan.moe/v4/manga/168827",
                          "https://api.jikan.moe/v4/manga/147205",
                          "https://api.jikan.moe/v4/manga/169731",
                          "https://api.jikan.moe/v4/manga/175521"};

typedef struct Memory {
  char *_mem;
  size_t _size;
} MemoryStruct;

typedef struct {
  int genres_len, themes_len, authors_len, mon_of_rel;
  double scores;
  char *title, *title_english, *status, *publish_date, *url_image;
  char **genres, **themes, **authors;
} ManhwaStats;

char *extractDateT(const char *date) {
  char *t_pos = strchr(date, 'T');
  if (!t_pos)
    return NULL;

  int date_len = t_pos - date;
  char *date_ori = malloc(date_len + 1);
  strncpy(date_ori, date, date_len);

  date_ori[date_len] = '\0';
  return date_ori;
}

static size_t crl_write_callback(void *contents, size_t size, size_t n_memb,
                                 void *userp) {
  size_t total = size * n_memb;
  MemoryStruct *mem = (MemoryStruct *)userp;

  char *ptr = realloc(mem->_mem, mem->_size + total + 1);
  if (!ptr)
    report_and_error("realloc error...");

  mem->_mem = ptr;
  memcpy(&(mem->_mem[mem->_size]), contents, total);
  mem->_size = mem->_size += total;
  mem->_mem[mem->_size] = 0;

  return total;
}

void performCURLManhwaData(int write_fd, const char *url) {
  CURL *curl;
  CURLcode c_res;
  MemoryStruct chunk = {0}, *p_chunk = &chunk;
  p_chunk->_mem = malloc(1);

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if (!curl)
    report_and_error("curl_easy_init error...");

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, crl_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)p_chunk);

  c_res = curl_easy_perform(curl);

  if (c_res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(c_res));
    exit(EXIT_FAILURE);
  }

  if (write(write_fd, p_chunk->_mem, p_chunk->_size) == -1)
    report_and_error("write to pipe failed...");

  free(p_chunk->_mem);
  curl_global_cleanup();
}

void performJSONParsing(int read_fd, ManhwaStats *mh) {
  char buf[BUF_SIZE];
  ssize_t fd = read(read_fd, buf, sizeof(buf) - 1);
  if (fd < 0)
    report_and_error("read from pipe failed...");

  buf[fd] = '\0';

  struct json_object *parsed_json_obj;

  parsed_json_obj = json_tokener_parse(buf);

  struct json_object *datas_obj;
  if (json_object_object_get_ex(parsed_json_obj, "data", &datas_obj)) {

    struct json_object *title_obj, *title_english_obj, *score_obj, *status_obj,
        *genres_obj, *themes_obj, *authors_obj;
    struct json_object *published_obj, *published_from_obj;
    struct json_object *published_prop_obj, *prop_from_obj, *from_month_obj;
    struct json_object *images_obj, *jpg_images_obj, *url_jpg_obj;

    if (!json_object_object_get_ex(datas_obj, "title", &title_obj) ||
        !json_object_object_get_ex(datas_obj, "title_english",
                                   &title_english_obj) ||
        !json_object_object_get_ex(datas_obj, "score", &score_obj) ||
        !json_object_object_get_ex(datas_obj, "status", &status_obj) ||
        !json_object_object_get_ex(datas_obj, "genres", &genres_obj) ||
        !json_object_object_get_ex(datas_obj, "themes", &themes_obj) ||
        !json_object_object_get_ex(datas_obj, "authors", &authors_obj)) {
      fprintf(stderr, "Object needed for extract data not found...");
      _exit(-1);
    }

    if (!json_object_object_get_ex(datas_obj, "images", &images_obj) ||
        !json_object_object_get_ex(images_obj, "jpg", &jpg_images_obj) ||
        !json_object_object_get_ex(jpg_images_obj, "image_url", &url_jpg_obj)) {
      fprintf(stderr, "URL for images not found...");
      _exit(-1);
    }

    if (!json_object_object_get_ex(datas_obj, "published", &published_obj) ||
        !json_object_object_get_ex(published_obj, "from",
                                   &published_from_obj) ||
        !json_object_object_get_ex(published_obj, "prop",
                                   &published_prop_obj) ||
        !json_object_object_get_ex(published_prop_obj, "from",
                                   &prop_from_obj) ||
        !json_object_object_get_ex(prop_from_obj, "month", &from_month_obj)) {
      fprintf(stderr, "Published data object is not found...");
      _exit(-1);
    }

    char date_raw[30];
    strcpy(date_raw, (char *)json_object_get_string(published_from_obj));
    char *date_ori = extractDateT(date_raw);

    // ManhwaDetails copying starts here

    memset(mh, 0, sizeof(ManhwaStats));
    const char *title = json_object_get_string(title_obj);
    const char *title_eng = json_object_get_string(title_english_obj);
    const char *status = json_object_get_string(status_obj);
    const char *url_image = json_object_get_string(url_jpg_obj);

    mh->title = strdup(title);
    mh->title_english = strdup(title_eng);
    mh->status = strdup(status);
    mh->publish_date = strdup(date_ori);
    mh->url_image = strdup(url_image);

    mh->scores = json_object_get_double(score_obj);
    mh->mon_of_rel = json_object_get_int(from_month_obj);

    int genres_len = json_object_array_length(genres_obj);
    mh->genres = (char **)malloc(sizeof(char *) * genres_len);
    mh->genres_len = genres_len;

    for (int i = 0; i < genres_len; i++) {
      struct json_object *genre = json_object_array_get_idx(genres_obj, i);

      struct json_object *genre_name;

      if (json_object_object_get_ex(genre, "name", &genre_name)) {
        const char *name = json_object_get_string(genre_name);
        mh->genres[i] = strdup(name);
      }
    }

    int themes_len = json_object_array_length(themes_obj);
    mh->themes = (char **)malloc(sizeof(char *) * themes_len);
    mh->themes_len = themes_len;

    for (int i = 0; i < themes_len; i++) {
      struct json_object *theme = json_object_array_get_idx(themes_obj, i);

      struct json_object *theme_name;

      if (json_object_object_get_ex(theme, "name", &theme_name)) {
        const char *name = json_object_get_string(theme_name);
        mh->themes[i] = strdup(name);
      }
    }

    int authors_len = json_object_array_length(authors_obj);
    mh->authors = (char **)malloc(sizeof(char *) * authors_len);
    mh->authors_len = authors_len;
    for (int i = 0; i < authors_len; i++) {
      struct json_object *author = json_object_array_get_idx(authors_obj, i);

      struct json_object *author_name;

      if (json_object_object_get_ex(author, "name", &author_name)) {
        const char *name = json_object_get_string(author_name);
        mh->authors[i] = strdup(name);
      }
    }

    free(date_ori);
  }

  json_object_put(parsed_json_obj);
  close(read_fd);
}

void convertTitleToFileName(char *ori, char *res) {
  int ori_len = strlen((char *)ori);
  char *temp = (char *)malloc(sizeof(char) * (ori_len + 1));
  temp[ori_len] = '\0';
  strncpy(temp, ori, ori_len);

  char *token = strtok((char *)temp, " ");

  int f = 1;
  while (token != NULL) {
    if (f != 1)
      strcat(res, "_");

    f = 0;
    int i = 0;
    while (token[i] != '\0') {
      if (isalnum(token[i])) {
        strncat(res, (char *)&token[i], 1);
      }

      i++;
    }

    token = strtok(NULL, " ");
  }

  strncat(res, "\0", 1);
}

void writeToFile(ManhwaStats *mh, const char *folder) {
  FILE *fp;
  char cwd[256] = {0}, path[512] = {0};
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report_and_error("getcwd() error...");

  printf("Creating files...\n");

  for (int i = 0; i < MH_COUNT; i++) {
    int filn_len = strlen(mh[i].title_english);
    char *filn = malloc(sizeof(char) * (filn_len + 1));
    filn[0] = '\0';

    convertTitleToFileName(mh[i].title_english, filn);

    snprintf(path, sizeof(path), "%s/%s/%s.txt", cwd, (char *)folder, filn);

    /* printf("%d\n", mh[i].mon_of_rel); */

    fp = fopen(path, "w");

    fprintf(fp, "Title: %s\n", mh[i].title);
    fprintf(fp, "Status: %s\n", mh[i].status);
    fprintf(fp, "Release: %s\n", mh[i].publish_date);
    fprintf(fp, "Genre: ");

    for (int j = 0; j < mh[i].genres_len; j++) {
      fprintf(fp, "%s", mh[i].genres[j]);
      if (j < mh[i].genres_len - 1)
        fprintf(fp, ", ");
      else
        fprintf(fp, "\n");
    }

    fprintf(fp, "Theme: ");
    for (int j = 0; j < mh[i].themes_len; j++) {
      fprintf(fp, "%s", mh[i].themes[j]);
      if (j < mh[i].themes_len - 1)
        fprintf(fp, ", ");
      else
        fprintf(fp, "\n");
    }

    fprintf(fp, "Author: ");
    for (int j = 0; j < mh[i].authors_len; j++) {
      fprintf(fp, "%s", mh[i].authors[j]);
      if (j < mh[i].authors_len - 1)
        fprintf(fp, ", ");
      else
        fprintf(fp, "\n");
    }

    fclose(fp);
  }
}

void perfromFetchDataManhwa(ManhwaStats *mh) {
  int pipeFD[4][2];
  createFolderSysCall("Manhwa");

  /* printf("Performing curl fetch...\n"); */

  pid_t fetch_pids[MH_COUNT];
  for (int i = 0; i < MH_COUNT; i++) {
    if (pipe(pipeFD[i]) == -1)
      report_and_error("pipe() error...");

    fetch_pids[i] = fork();
    if (fetch_pids[i] == 0) {
      close(pipeFD[i][ReadEnd]);
      performCURLManhwaData(pipeFD[i][WriteEnd], mh_urls[i]);
      _exit(0);
    }

    waitpid(fetch_pids[i], NULL, 0);
  }

  for (int i = 0; i < MH_COUNT; i++) {
    close(pipeFD[i][WriteEnd]);
    performJSONParsing(pipeFD[i][ReadEnd], &mh[i]);

    close(pipeFD[i][ReadEnd]);
  }

  for (int i = 0; i < MH_COUNT; i++) {
    close(pipeFD[i][WriteEnd]);
    close(pipeFD[i][ReadEnd]);
  }
  writeToFile(mh, "Manhwa");

  wait(NULL);
}

int main() {
  ManhwaStats mh[MH_COUNT] = {0};

  perfromFetchDataManhwa(mh);

  for (int i = 0; i < MH_COUNT; i++) {
    free(mh[i].title);
    free(mh[i].genres);
    free(mh[i].status);
    free(mh[i].themes);
    free(mh[i].authors);
    free(mh[i].publish_date);
    free(mh[i].title_english);
  }

  return 0;
}
```

![Hasil Soal A](/assets/task-3/soal-A.jpg)

##### Penjelasan

Proses fetch data manhwa menggunakan Jikan API `https://api.jikan.moe/v4/manga/{id}` dengan parameter yang harus diberikan adalah id dari manga/manhwa yang ingin dicari.
Jikan API akan mengembalikan sebuah responnse HTTP berupa data JSON. Proses fetch data menggunakan bantuan fungsi `performCURLManhwaData` yang menggunakan library curl
untuk melakukan proses download data JSON, kemudian terdapat fungsi `crl_write_callback` yang digunakan sebagai salah satu parameter dalam inisialisasi curl options yakni di bagian
`CURLOPT_WRITEFUNCTION`. Untuk mempermudah dalam melakukan fetch data dari API, kami membuat sebuah struktur bernama `MemoryStruct` yang berisi data yang didapat dari proses curl serta
ukuran data tersebut. Struktur ini nantinya digunakan untuk mencatat data sementara yang didapat melalui proses download curl. Fungsi `performCURLManhwaData` dalam program dijalankan menggunakan
bantuan fork() untuk mempercepat proses pendownload-an agar bisa berjalan bersamaan dengan id manga yang berbeda-beda. Karena dilakukan dalam child process yang berbeda maka dibutuhkan suatu cara agar antara
child process dan parent process dapat saling berkomunikasi. Disini kami menggunakan mekanisme IPC Pipe untuk komunikasi antara parent dengan child process. Terdapat 4 pipe yang digunakan karena data yang diambil
melalui API juga berjumlah 4 link yang berbeda. Implementasi IPC Pipe menggunakan fungsi `pipe` dan `close` untuk menutup Read/Write dari Pipe.

Data manhwa yang diperoleh dari fungsi `performCURLManhwaData` kemudian diteruskan lewat pipe dan diproses lebih lanjut oleh fungsi `performJSONParsing` yang juga menggunakan pipe
sebagai perantara komunikasi antar proses. Di dalam fungsi ini data raw yang diperoleh dari proses cURL diubah dan di-parsing menjadi data JSON dengan bantuan library json-c. Kemudian data di-parsing
dan ditraverse untuk mendapatkan data-data yang diinginkan menggunakan fungsi `json_object_object_get_ex` untuk setiap data yang ingin diekstraksi dari data JSON yang ada, seperti title, title_english,
data url image cover, status, genre, theme, author dan lain lain. Kemudian data yang berhasil diparsing dimasukkan kedalam struktur data custom `ManhwaStats` yang nantinya digunakan untuk menampung data-data
yang dibutuhkan.

Setelah semua data Manhwa berhasil didapat, langkah selanjutnya adalah menuliskan data-data yang ada di struktur `ManhwaStats` kedalam file `.txt`. Ketentuan yang diberikan di soal dalam pemberian
nama file .txt adalah disesuaikan dengan judul Bahasa Inggris dengan tanpa karakter khusus serta karakter spasi diganti dengan `_`. Fungsi yang digunakan disini adalah fungsi `writeToFile` yang didalam
fungsi tersebut akan memanggil fungsi lain agar nama dari file .txt sesuai dengan ketentuan soal. Fungsi yang digunakan selanjutnya adalah `convertTitleToFileName`. Untuk memfilter string judul Manhwa, digunakan
fungsi `strtok` untuk mendapatkan token-token string yang dipisahkan oleh spasi. Kemudian untuk memfilter karakter khusus, digunakan fungsi `isalnum` untuk hanya menuliskan ke string akhir berupa alfanumerik
saja tanpa ada karakter khusus. Pembuatan file .txt menggunakan bantuan file pointer dan `fopen` dengan mode `w` (write) untuk menuliskan karakter kedalam file .txt yang dibuat. Nama file .txt didapat dari
string hasil konversi dari fungsi sebelumnya yang memfilter karakter khusus dan spasi. Untuk menuliskan baris pada file yang sudah dibuat, digunakan fungsi `fprintf` dan data-data yang sudah tercatat di `ManhwaStats`
dituliskan kedalam file .txt

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal A

### Soal B - Seal the Scrolls

```c
void getOnlyCapital(char *ori, char *res) {
  int ori_len = strlen((char *)ori);
  char *temp = (char *)malloc(sizeof(char) * (ori_len));
  strncpy(temp, ori, ori_len);
  temp[ori_len] = '\0';

  int j = 0;
  for (int i = 0; temp[i] != '\0'; i++) {
    if (isupper(ori[i])) {
      res[j++] = temp[i];
    }
  }

  res[j] = '\0';
}

void performZipOp(ManhwaStats *mh) {
  createFolderSysCall("Archive");

  char path[512] = {0}, txt_path[512] = {0};

  int filzip_len = strlen(mh->title);

  char *filtxt = (char *)malloc(sizeof(char) * (filzip_len + 1));
  filtxt[0] = '\0';
  convertTitleToFileName(mh->title, filtxt);
  snprintf(txt_path, sizeof(txt_path), "./Manhwa/%s.txt", filtxt);

  char *filzipn = (char *)malloc(sizeof(char) * (filzip_len + 1));
  filzipn[0] = '\0';
  getOnlyCapital(mh->title, filzipn);
  snprintf(path, sizeof(path), "./Archive/%s.zip", filzipn);

  int path_len = strlen(path);
  int txt_path_len = strlen(txt_path);

  printf("Begin zipping ...\n");

  if (fork() == 0) {
    printf("%s [%d] - %s [%d] %d\n", path, path_len, txt_path, txt_path_len,
           filzip_len);
    char *argv[] = {"zip", "-j", path, txt_path, NULL};
    execv("/bin/zip", argv);
  }

  wait(NULL);

  _exit(0);
}

void performZipTxt(ManhwaStats *mh) {
  pid_t zip_pid[MH_COUNT];
  for (int i = 0; i < MH_COUNT; i++) {
    if (fork() == 0) {
      performZipOp(&mh[i]);
    }

    wait(NULL);
  }
}

int main() {
  ManhwaStats mh[MH_COUNT] = {0};

  perfromFetchDataManhwa(mh);

  performZipTxt(mh);

  for (int i = 0; i < MH_COUNT; i++) {
    free(mh[i].title);
    free(mh[i].genres);
    free(mh[i].status);
    free(mh[i].themes);
    free(mh[i].authors);
    free(mh[i].publish_date);
    free(mh[i].title_english);
  }

  return 0;
}
```

![Hasil Soal B](/assets/task-3/soal-B.jpg)

##### Penjelasan

Pada Soal B, semua file .txt yang ada didalam folder `Manhwa/` diminta untuk di-zip-kan dan dimasukkan ke dalam folder baru `Archive/`.
Ketentuan yang diberikan dalam pemberian nama file .zip adalah huruf kapital dari masing-masing nama file .txt yang ada di folder `Manhwa/`.
Untuk mendapatkan karakter huruf besar saja, maka kami membuat fungsi utility `getOnlyCaptial` dengan dua parameter fungsi, original string serta
string akhir setelah filter dengan memanfaatkan fungsi `isupper()` untuk menentukan apakah karakter di index ke-i string adalah huruf besar.

Setelah nama untuk file .zip sudah diketahui, maka langkah selanjutnya adalah tinggal melakukan operasi zip-ping pada file-file .txt yang ada di folder
dengan memanfaatkan `fork` serta `execv`. fork digunakan untuk menspawn child process disamping parent process dan execv digunakan untuk menjalankan
command `zip` yang nantinya akan meng-zip file .txt sesuai dengan path yang disediakan saat memanggil program zip.

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal B

#### Soal C - Making the Waifu Gallery

```c
typedef struct {
  char *url_image, *dwn_path;
} ImageThread;

size_t writeToJpg(void *contents, size_t size, size_t nmemb, FILE *userp) {
  size_t written = fwrite(contents, size, nmemb, userp);
  return written;
}

pthread_mutex_t download_mutex = PTHREAD_MUTEX_INITIALIZER;

void *downloadImage(void *arg) {
  ImageThread *imgt;
  imgt = (ImageThread *)arg;

  CURL *curl;
  CURLcode res;
  FILE *fp;

  curl = curl_easy_init();
  if (!curl)
    report_and_error("curl_easy_init error...");

  fp = fopen(imgt->dwn_path, "wb");
  if (!fp)
    report_and_error("fopen failed...");
  curl_easy_setopt(curl, CURLOPT_URL, imgt->url_image);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToJpg);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

  res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  fclose(fp);

  free(imgt->dwn_path);
  free(imgt->url_image);
  free(imgt);

  return NULL;
}

void performDownloadImages(ManhwaStats *mh) {
  createFolderSysCall("Heroines");

  for (int i = 0; i < MH_COUNT; i++) {
    pthread_mutex_lock(&download_mutex);
    char path[512] = {0};

    snprintf(path, sizeof(path), "Heroines/%s", mh_heroines[i]);
    createFolderSysCall(path);

    int numdwn = mh[i].mon_of_rel;

    pthread_t hr_downloads[numdwn];
    for (int j = 0; j < numdwn; j++) {

      char dwn_path[512], cwd[256];
      if (getcwd(cwd, sizeof(cwd)) == NULL)
        report_and_error("getcwd() error...");

      snprintf(dwn_path, sizeof(dwn_path), "%s/Heroines/%s/%s_%d.jpg", cwd,
               mh_heroines[i], mh_heroines[i], j + 1);

      ImageThread *imgt = malloc(sizeof(ImageThread));

      imgt->url_image = strdup((&mh[i])->url_image);
      imgt->dwn_path = strdup(dwn_path);

      printf("Downloading images %d-%d: %s...\n", i + 1, j + 1,
             mh[i].title_english);
      pthread_create(&hr_downloads[j], NULL, downloadImage, (void *)imgt);

      pthread_mutex_unlock(&download_mutex);
    }

    for (int j = 0; j < numdwn; j++) {
      pthread_join(hr_downloads[j], NULL);
    }
  }
}

int main() {
  ManhwaStats mh[MH_COUNT] = {0};

  perfromFetchDataManhwa(mh);

  performZipTxt(mh);

  performDownloadImages(mh);

  for (int i = 0; i < MH_COUNT; i++) {
    free(mh[i].title);
    free(mh[i].genres);
    free(mh[i].status);
    free(mh[i].themes);
    free(mh[i].authors);
    free(mh[i].publish_date);
    free(mh[i].title_english);
  }

  return 0;
}
```

![Hasil Soal C](/assets/task-3/soal-C1.jpg)
![Hasil Soal C](/assets/task-3/soal-C2.jpg)

##### Penjelasan

Pada Soal C, terdapat tugas untuk mendownload sebuah image dari internet yang dimana gambar yang akan didownload berisi karakter FMC atau heroine
dari masing-masing manhwa. Kemudian gambar tersebut didownload sebanyak di bulan berapa manhwa tersebut dirilis, sebagai contoh jika manhwa A dirilis
pada bulan Februari (bulan 2) maka gambar akan didownload sebanyak 2 kali. Untuk pendownload-an gambar serta link yang digunakan, kami menggunakan link URL gambar cover yang tersedia saat mengambil data JSON dari Jikan API. URL tersebut
kemudian juga masuk kedalam struktur data `ManhwaStats` untuk mempermudah pengakses-an data. Kemudian untuk seberapa banyak gambar harus didownload, kami juga
menggunakan data yang ada dalam response JSON dari Jikan API di bagian `published` dan juga menambahkan data bulan ke dalam struktur `ManhwaStats` juga.

Untuk pendownload-an gambar secara individual, teknik yang digunakan mirip seperti yang dilakukan di soal-A yakni menggunakan libcurl namun dengan perbedaan terletak
pada `CURLOPT_WRITEFUNCTION`. Fungsi untuk write function disini menggunakan fungsi bantuan `writeToJpg` yang menuliskan binary data ke file gambar yang dibuat
menggunakan `fopen` dan `fwrite` dengan mode file `wb` (write binary). Untuk nama gambar yang akan didownload, di fungsi `downloadImage` terdapat satu buah parameter yang menjadi nama dari file gambar
yang sudah didownload. Fungsi `downloadImage` ini nantinya akan di-run dengan metode multi-threading sehingga argumen dari fungsi ini harus menggunakan sebuah struktur.
Struktur yang digunakan dinamai `ImageThread` yang berisi `url_image` serta `download_path`. url_image digunakan untuk memberi tahu libcurl untuk mem-fetch data
dari url yang diberikan, kemudian dwn_path digunakan untuk meng-set destination path file yang sudah selesai didownload.

Karena gambar yang didownload akan diulang beberapa kali sesuai dengan bulan rilis manhwa tersebut, maka disini kami menerapkan multi-threading untuk melakukan download
gambar secara langsung dan sesuai dengan banyak gambar yang harus didownload. Multi-threading diterapkan didalam fungsi `performDownloadImages` yang sebelumnya akan dibuat terlebih
dahulu folder `Heroines`. Kemudian struktur `ImageThread` akan dibuat dan di isi dengan data-data yang relevan untuk pendownload-an gambar. Pembuatan thread dimulai
dengan mendefinisikan variabel `hr_downloads[numdown]` dengan numdown adalah berapa banyak gambar yang harus didownload serta tipe data yang digunakan adalah `pthread_t`.
Kemudian untuk membuat thread, digunakan fungsi `pthread_create()` dengan fungsi thread yang digunakan adalah `downloadImage`. Setelah thread-thread tersebut selesai melakukan tugas
download gambar, satu-persatu thread akan dijoin ke process menggunakan `pthread_join()`.

Karena di soal diinginkan untuk pendownload-an gambar dilakukan secara urut sesuai dengan list manhwa yang diberikan, maka digunakan juga teknik mutex dengan memanfaatkan `pthread_mutex_lock()` dan
`pthread_mutex_unlock` untuk mencegah thread untuk melakukan race condition dimana dapat menimbulkan pendownload-an gambar pada manhwa B lebih dulu daripada manhwa A.

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal C

#### Soal D - Zip. Save. Goodbye.

```c
void performZipImages(ManhwaStats *mh) {
  createFolderSysCall("Archive");
  createFolderSysCall("Archive/Images");

  printf("Begin to zip images...\n");
  for (int i = 0; i < MH_COUNT; i++) {
    if (fork() == 0) {
      char cwd[256] = {0}, path[512] = {0}, images_path[512] = {0};
      if (getcwd(cwd, sizeof(cwd)) == NULL)
        report_and_error("getcwd() error...");

      int filzip_len = strlen(mh[i].title_english);

      char *filtxt = malloc(sizeof(char) * (filzip_len + 1));
      filtxt[0] = '\0';
      convertTitleToFileName(mh[i].title_english, filtxt);

      char *filzipn = malloc(sizeof(char) * (filzip_len + 1));
      filzipn[0] = '\0';
      getOnlyCapital(mh->title, filzipn);

      snprintf(images_path, sizeof(images_path), "%s/Heroines/%s/", cwd,
               mh_heroines[i]);
      snprintf(path, sizeof(path), "%s/Archive/Images/%s_%s.zip", cwd, filzipn,
               mh_heroines[i]);

      if (fork() == 0) {
        char *argv[] = {"zip", "-r", path, images_path, NULL};
        execv("/bin/zip", argv);
      }

      wait(NULL);

      exit(0);
    }

    wait(NULL);
  }

  wait(NULL);
}

int compareDirName(const void *a, const void *b) {
  return strcmp(*(char **)a, *(char **)b);
}

void listFolders(char *dirname, char ***folders_out, int *count) {
  char cwd[512] = {0};
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report_and_error("getcwd() error...");
  DIR *dir = opendir(dirname);
  if (!dir)
    report_and_error("opendir() fail to open dirname...");

  char **folders = (char **)malloc(sizeof(char *));
  struct dirent *entry;

  int cnt = 1;
  while ((entry = readdir(dir)) != NULL) {
    folders = (char **)realloc(folders, sizeof(char *) * (cnt));

    if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      continue;

    char fullpath[512];
    snprintf(fullpath, sizeof(fullpath), "%s/%s/%s", cwd, dirname,
             entry->d_name);

    struct stat st;
    if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode)) {
      folders[cnt - 1] = strdup(fullpath);
      ++cnt;
    }
  }

  closedir(dir);
  qsort(folders, cnt - 1, sizeof(char *), compareDirName);

  *count = cnt - 1;
  *folders_out = folders;
}

void performDeleteImagesSort() {
  char *dirname = "Heroines";
  char **folders;
  int count;

  listFolders(dirname, &folders, &count);
  /* printf("Begin Deleting... - %d\n", count); */

  for (int i = 0; i < count; i++) {
    if (fork() == 0) {
      /* printf("Deleteting folder on path: %s...\n", folders[i]); */
      char *argv[] = {"rm", "-rf", folders[i], NULL};
      execv("/bin/rm", argv);
    }

    wait(NULL);
  }
}

int main() {
  ManhwaStats mh[MH_COUNT] = {0};

  perfromFetchDataManhwa(mh);

  performZipTxt(mh);

  performDownloadImages(mh);

  performZipImages(mh);

  performDeleteImagesSort();

  for (int i = 0; i < MH_COUNT; i++) {
    free(mh[i].title);
    free(mh[i].genres);
    free(mh[i].status);
    free(mh[i].themes);
    free(mh[i].authors);
    free(mh[i].publish_date);
    free(mh[i].title_english);
  }

  return 0;
}
```

![Hasil Soal D](/assets/task-3/soal-D.jpg)

##### Penjelasan

Pada Soal D, diminta untuk pengarsipan gambar-gambar yang sudah didownload di folder `Heroines/` yang sudah dikelompokkan berdasarkan nama masing-masing Heroine Manhwa.
Hasil pengarsipan gambar-gambar tersebut akan diletakkan pada folder `Archive/Images` dengan format nama file .zip adalah `[HURUFKAPITALNAMAMANHWA]_[namaheroine].zip`.
Untuk mendapatkan huruf kapital nama manhwa, digunakan fungsi yang sudah ada sebelumnya yakni `getOnlyCapital` dan `convertTitleToFileName`. Karena terdapat 4 folder yang akan
diarsipkan, maka teknik yang digunakan disini adalah multi-processing dengan masing-masing child process yang dispawn akan menjalankan `zip` dengan menggunakan `execv`. Proses-proses diatas dibungkus
kedalam fungsi `performZipImages` untuk mempermudah pemanggilan.

Kemudian setelah dilakukan peng-arsip-an folder-folder file gambar, diminta juga untuk melakukan penghapusan file-file gambar yang sudah didownload
di folder `Heroines/` satu persatu namun urut abjad. Approach yang saya gunakan disini adalah melakukan listing direktori-direktori yang ada didalam folder `Heroines/` dan mengurutkan
nama folder secara alfanumerik dan secara ascending menggunakan bantuan fungsi `opendir`, `readdir`, dan `qsort`. Untuk menentukan yang di-list oleh fungsi-fungsi tersebut, diperlukan
kondisi dan pengecakan apakah yang dilist adalah sebuah direktori atau bukan dengan memanfaatkan fungsi `S_ISDIR` serta struktur `dirent` yang menampung informasi dari path yang diperoleh
dan melakukan pemanggilan fungsi `stat()` untuk memverifikasi apakah path tersebut valid atau tidak. Kemudian, nama-nama direktori yang sudah terfilter dimasukkan ke dalam sebuah variabel
bertipe `char` dan berbentuk 2 dimensi `char **` yang dialokasikan secara dinamis menggunakan `malloc` dan `realloc`. Nama-nama direktori yang sudah disimpan kemudian diurutkan berdasarkan
alfanumerik dan secara ascending dengan bantuan fungsi `qsort` dan fungsi tambahan `compareDirname` untuk menentukan prioritas urutan nama direktori dengan memanfaatkan fungsi lain `strcmp`. Proses-proses diatas
dibungkus kedalam satu fungsi bernama `listFolders` yang tujuan-nya tidak lain hanya untuk meng-list folder apa saja yang ada didalam direktori `Heroines/`.

Setelah didapat nama-nama folder yang sudah terurut secara alfanumerik dan secara ascending, maka nama-nama direktori yang sudah dilist oleh fungsi sebelumnya ditampung lagi kedalam
variabel lain didalam fungsi `performDeleteImageSort()` untuk dijalankan penghapusan direktori urut berdasarkan abjad seperti yang diminta pada Soal D. Untuk penghapusan digunakan child process
yang didalamnya terdapat sebuah `execv` yang menjalankan perintah `rm` serta flag `-rf` (rekursif dan force) untuk menghapus direktori yang dimaksud beserta seluruh isi/konten dari direktori tersebut.
Kemudian setiap spawning child process harus menunggu terlebih dahulu child process sebelumnya untuk menyelesaikan tugasnya.

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal D