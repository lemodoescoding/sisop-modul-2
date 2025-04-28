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
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int total_file = 0;
pthread_mutex_t mutexLog = PTHREAD_MUTEX_INITIALIZER;
int animasi = 0, drama = 0, horror = 0;

void activityLog(const char *judul, const char *genre, const char *penulis) {
  time_t now = time(NULL);
  struct tm *waktu = localtime(&now);

  pthread_mutex_lock(&mutexLog);
  FILE *log = fopen("recap.txt", "a");
  fprintf(log, "[%02d-%02d-%4d %02d:%02d:%02d] %s: %s telah dipindahkan ke %s\n",
          waktu->tm_mday, waktu->tm_mon + 1, waktu->tm_year + 1900,
          waktu->tm_hour, waktu->tm_min, waktu->tm_sec, penulis, judul, genre);
  fclose(log);
  pthread_mutex_unlock(&mutexLog);
}

void *sortirfilm(void *argv) {
  char *genre = (char *)argv;
  DIR *folder = opendir("film");
  if (!folder) {
    perror("Tidak bisa membuka folder film");
    return NULL;
  }

  struct dirent *entri;
  char folderbaru[100];
  char genreS[100];
  strcpy(genreS, genre);
  genreS[0] = toupper(genreS[0]);
  sprintf(folderbaru, "film/Film%s", genreS);

  if (fork() == 0) {
    char *argv[] = {"mkdir", "-p", folderbaru, NULL};
    execv("/bin/mkdir", argv);
  }

  while ((entri = readdir(folder)) != NULL) {
    int len = strlen(entri->d_name);
    if (len > 4 && strcmp(entri->d_name + len - 4, ".jpg") == 0) {
      char genre_file[20];
      sscanf(entri->d_name, "%*[^_]_%*[^_]_%[^.].jpg", genre_file);

      if (strcmp(genre_file, genre) == 0) {
        int nomor = 0;
        sscanf(entri->d_name, "%d_", &nomor);
        const char *penulis = (nomor < total_file / 2) ? "Trabowo" : "Peddy";

        char before[512], after[512];
        snprintf(before, sizeof(before), "film/%s", entri->d_name);
        snprintf(after, sizeof(after), "%s/%s", folderbaru, entri->d_name);

        rename(before, after);

        activityLog(entri->d_name, folderbaru + 5, penulis);

        if (strcmp(genre, "horror") == 0)
          horror++;
        else if (strcmp(genre, "animasi") == 0)
          animasi++;
        else if (strcmp(genre, "drama") == 0)
          drama++;
      }
    }
  }

  closedir(folder);
  return NULL;
}

int main() {
  DIR *folder = opendir("film");
  if (!folder) {
    perror("Tidak bisa membuka folder film");
    return 1;
  }

  struct dirent *entri;
  while ((entri = readdir(folder)) != NULL) {
    int len = strlen(entri->d_name);
    if (len > 4 && strcmp(entri->d_name + len - 4, ".jpg") == 0) {
      total_file++;
    }
  }
  closedir(folder);

  pthread_t threads[3];
  char *genres[] = {"horror", "animasi", "drama"};

  for (int i = 0; i < 3; i++) {
    pthread_create(&threads[i], NULL, sortirfilm, genres[i]);
  }

  for (int i = 0; i < 3; i++) {
    pthread_join(threads[i], NULL);
  }

  FILE *f = fopen("total.txt", "w");
  fprintf(f, "Jumlah film horror: %d\n", horror);
  fprintf(f, "Jumlah film animasi: %d\n", animasi);
  fprintf(f, "Jumlah film drama: %d\n", drama);

  char *top_genre = "horror";
  int countTop = horror;
  if (drama > countTop) {
    top_genre = "drama";
    countTop = drama;
  }
  if (animasi > countTop) {
    top_genre = "animasi";
    countTop = animasi;
  }

  fprintf(f, "Genre dengan jumlah film terbanyak: %s\n", top_genre);
  fclose(f);
  return 0;
}
```

##### Penjelasan
Deklarasikan ```total_file``` untuk menyimpan jumlah file, lalu inisialisasi mutex dengan nama mutexLog. ```void activityLog``` adalah fungsi untuk menuliskan log ```recap.txt```. Menggunakan mutex agar hanya satu thread yang bisa mengakses bagian itu dalam satu waktu. Jika tidak menggunakan mutex, maka bisa terjadi data bentrok.
Lalu, ```struct tm* waktu = localtime(&now)``` untuk mengambil informasi terkait waktu sekarang. Kemudian, file ```recap.txt``` dibuka dan dituliskan log dengan format tertentu ke dalamnya. Lalu, file ditutup dan mutex pun diunlock kembali.

Pada kode, ```void* sortirfilm``` adalah fungsi untuk menyortir film untuk dimasukkan ke dalam folder sesuai genrenya. 
Buka folder film, lalu buat folderbaru dengan format ```film/FilmGenre``` dengan Genrenya adalah horror, animasi, dan drama. Lalu, buat fork dan jalankan execv yang berisi perintah untuk membuat folderbaru tersebut jika belum ada. 
Kemudian, semua file dalam folder lama akan dibaca dan disortir. sscanf akan membaca dan menyimpan string genre dalam file film. Kemudian, akan dibandingkan dengan genre yang sudah ada, jika sama maka sscanf akan mengambil nomor film, dan menentukan penulis log berdasarkan nomor film. 
Kemudian, snprintf pertama akan mengambil lokasi file awal sebelum dipindahkan dan snprintf kedua akan mengambil lokasi setelah file dipindahkan. Kemudian, direname untuk memindahkan lokasi lama ke lokasi baru. 

Lalu, panggil fungsi activityLog untuk membuat log baru ke ```recap.txt```. Lalu, Genre++. Kemudian, close folder.

Pada fungsi main, awalnya buka direktori film dan simpan ke ```*folder```. Lalu, baca semua entri dalam folder, dan hitung ```total_file```. Kemudian, close folder. 
Lalu, bikin thread sebanyak 3 untuk memproses berdasarkan genre masing-masing (horror, animasi, drama). Lalu, gunakan ```pthread_join``` untuk menunggu semua thread selesai bekerja.
Setelah itu, buka file ```total.txt``` dan print masing-masing jumlah film berdasarkan genrenya. Kemudian, tentukan genre dengan jumlah film terbanyak dan print.

##### Kendala
Tidak ada.

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

##### Penjelasan
deklarasi array zip dan array folder. Kemudian, buat 3 fork untuk mengzip masing-masing genre. Jika pid < 0 artinya fork gagal, dan jika pid == 0, child proses akan menjalankan execv yang berisi argumen untuk mengzip folder. Jika execv tidak berhasil dijalankan, 
maka akan mengeluarkan pesan error dan exit(1). Kemudian, ```wait(NULL)``` untuk menunggu proses child selesai dijalankan. 

Setelah itu, buat 3 fork lagi untuk menjalankan perintah remove folder. Kemudian, buat ```wait(NULL)``` sebanyak 3 kali untuk menunggu execv berhasil dijalankan.

#### Kendala
Awalnya, saya tidak tahu bahwa setelah mengzip kembali folder, harus dilakukan juga penghapusan folder lama, karena tidak tertera secara eksplisit pada soal.


