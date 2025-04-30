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
