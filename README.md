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

- Kendala
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

Pada Soal B, semua file .txt yang ada didalam folder `Manhwa/` diminta untuk di-zip-kan dan dimasukkan ke dalam folder baru `Archive/`.
Ketentuan yang diberikan dalam pemberian nama file .zip adalah huruf kapital dari masing-masing nama file .txt yang ada di folder `Manhwa/`.
Untuk mendapatkan karakter huruf besar saja, maka kami membuat fungsi utility `getOnlyCaptial` dengan dua parameter fungsi, original string serta
string akhir setelah filter dengan memanfaatkan fungsi `isupper()` untuk menentukan apakah karakter di index ke-i string adalah huruf besar.

Setelah nama untuk file .zip sudah diketahui, maka langkah selanjutnya adalah tinggal melakukan operasi zip-ping pada file-file .txt yang ada di folder
dengan memanfaatkan `fork` serta `execv`. fork digunakan untuk menspawn child process disamping parent process dan execv digunakan untuk menjalankan
command `zip` yang nantinya akan meng-zip file .txt sesuai dengan path yang disediakan saat memanggil program zip.

- Kendala
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
Kemudian untuk membuat thread, digunakan fungsi `pthread_create()` dengan fungsi thread yang digunakan adalah `downloadImage`.

- Kendala
  Untuk saat ini belum ada kendala untuk mengerjakan soal C
