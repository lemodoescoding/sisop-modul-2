#include <ctype.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <dirent.h>
#include <fcntl.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_types.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define ReadEnd 0
#define WriteEnd 1

#define BUF_SIZE 8192
#define MH_COUNT 4

const char *mh_urls[4] = {"https://api.jikan.moe/v4/manga/168827",
                          "https://api.jikan.moe/v4/manga/147205",
                          "https://api.jikan.moe/v4/manga/169731",
                          "https://api.jikan.moe/v4/manga/175521"};

const char *mh_heroines[4] = {"Lia", "Tia", "Adelia",
                              "Ophelia"}; // based on the API an some research,
                                          // and sorted by mh_urls

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

typedef struct {
  char *url_image, *dwn_path;
} ImageThread;

void report_and_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

void createFolderMk(void *arg) {
  char cwd[256], path[512];
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report_and_error("getcwd() error...");

  snprintf(path, sizeof(path), "%s/%s", cwd, (char *)arg);

  /* printf("Creating folder '%s'...\n", (char *)arg); */
  char *argv[] = {"mkdir", "-p", path, NULL};
  execv("/bin/mkdir", argv);
}

void createFolderSysCall(const char *folderName) {
  char cwd[256] = {0}, path[512] = {0};
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report_and_error("getcwd() error...");

  snprintf(path, sizeof(path), "%s/%s", cwd, folderName);

  long res = syscall(SYS_mkdir, path, 0755);
}

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

size_t writeToJpg(void *contents, size_t size, size_t nmemb, FILE *userp) {
  size_t written = fwrite(contents, size, nmemb, userp);
  return written;
}

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
    char path[512] = {0};

    snprintf(path, sizeof(path), "Heroines/%s", mh_heroines[i]);
    createFolderSysCall(path);

    int numdwn = mh[i].mon_of_rel;
    if (fork() == 0) {

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
      }

      for (int j = 0; j < numdwn; j++) {
        pthread_join(hr_downloads[j], NULL);
      }

      exit(0);
    }
    wait(NULL);
  }
}

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
  qsort(folders, cnt, sizeof(char *), compareDirName);

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

void performZipTxt(ManhwaStats *mh) {
  pid_t zip_pid[MH_COUNT];
  for (int i = 0; i < MH_COUNT; i++) {
    if (fork() == 0) {
      performZipOp(&mh[i]);
    }

    wait(NULL);
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

  performZipTxt(mh);

  /* performDownloadImages(mh); */

  /* performZipImages(mh); */
  /**/
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
