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