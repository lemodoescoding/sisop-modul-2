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
  fprintf(log, "[%2d-%2d-%4d %2d:%2d:%2d] %s: %s telah dipindahkan ke %s\n",
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
