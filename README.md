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

#### Laporan Task 4 - Pipip's Load Balancer

Compile dengan

```bash
gcc client.c -o client
gcc loadbalancer.c -o loadbalancer
gcc worker.c -o worker -pthread
```

```c
void report_and_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}
```

Terdapat fungsi report_and_error yang digunakan untuk mempersingkat penulisan perror yang digunakan
statement yang ada dalam fungsi report_and_error. Fungsi report_and_error akan dipanggil dalam kasus ketika terjadi kesalahan dalam pemanggilan
fungsi lain seperti `fopen`,`shmget`, `semget`, dsb. Fungsi report_and_error memiliki satu parameter yang berisi custom message error
dan di dalam fungsi report_and_error terdapat pemanggilan fungsi `perror` dengan argumen berisi custom message error tadi. Fungsi `perror`
akan memberikan suatu buffer ke `stderr` dengan tambahan keterangan error otomatis saat perror ini dipanggil.
Kemudian jika fungsi report_and_error maka program akan langsung berhenti karena adanya pemanggilan fungsi `exit` yang memaksa
program untuk diberhentikan eksekusinya.

#### Soal A -

```c
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678

#define SYSTEM_LOG "./sistem.log"

#define SEM_EMPTY 0
#define SEM_FULL 1

#define STR_INIT_SIZE 256

typedef struct {
  char message[STR_INIT_SIZE];
  unsigned long count;
  int ready;
} Message;

void report(const char *message, int terminate) {
  perror(message);
  if (terminate)
    exit(-1);
}

void mtx_lock(int sem_id, int sem_num) {
  struct sembuf sb = {sem_num, -1, 0};
  semop(sem_id, &sb, 1);
}

void mtx_unlock(int sem_id, int sem_num) {
  struct sembuf sb = {sem_num, 1, 0};
  semop(sem_id, &sb, 1);
}

int main() {
  int shmid, semid;
  Message *shm_ptr;
  sem_t *log_sem;

  char cwd[256] = {0}, path[512] = {0};
  if (getcwd(cwd, sizeof(cwd)) == NULL)
    report("getcwd()...", 1);

  shmid = shmget(SHM_KEY, sizeof(Message), IPC_CREAT | 0666);
  if (shmid < 0)
    report("shmget()...", 1);

  if ((shm_ptr = (Message *)shmat(shmid, NULL, 0)) == (void *)-1)
    report("shmat()...", 1);

  semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
  if (semid == -1)
    report("semget...", 1);

  semctl(semid, SEM_EMPTY, SETVAL, 1);
  semctl(semid, SEM_FULL, SETVAL, 0);

  char input[512];
  char message[256];
  unsigned long count = 0;
  fgets(input, sizeof(input), stdin);
  sscanf(input, "%[^\n;];%lu", message, &count);

  for (unsigned long i = 0; i <= count + 1; ++i) {
    /* printf("yes"); */
    mtx_lock(semid, SEM_EMPTY);

    if (i == count + 1) {
      snprintf(shm_ptr->message, STR_INIT_SIZE, "end_of_message");
      shm_ptr->count = 0;
      shm_ptr->ready = 1;
    } else if (i > 0 && i != count + 1) {
      snprintf(shm_ptr->message, STR_INIT_SIZE, "%s", message);
      shm_ptr->ready = 1;
      shm_ptr->count = i;

      FILE *fp;
      fp = fopen(SYSTEM_LOG, "a");
      if (fp) {
        fprintf(fp, "Message from client: %s\n", message);
        fprintf(fp, "Message count: %lu\n", i);
        fclose(fp);
      } else {
        report("fopen", 0);
      }
      usleep(100);
    } else {
      snprintf(shm_ptr->message, STR_INIT_SIZE, "start_of_message");
      shm_ptr->ready = 1;
      shm_ptr->count = count;
    }

    mtx_unlock(semid, SEM_FULL);

    usleep(100);
  }

  shmdt(shm_ptr);

  return 0;
}
```

##### Penjelasan

Pada Soal A, `client.c` menerima input-an berupa string dari user saat dijalankan dengan format `MESSAGE;MESSAGE_COUNT` dimana MESSAGE adalah pesan yang hendak dikirimkan dan MESSAGE_COUNT adalah berapa kali
pesan tersebut berulang. Komunikasi client.c untuk mengirimkan pesan menggunakan mekanisme IPC dengan tipe shared memory. Untuk memudahkan pengiriman pesan, maka didefinisikan suatu struktur baru
bernama `Message` yang berisi `message` yang akan dikirim, `count` untuk menandakan message ke-n, dan `ready` untuk sinkronasisasi pada receiver.

IPC shared memory memanfaatkan library `sys/ipc.h`, `sys/shm.h` yang merupakan Shared Memory tipe System V. Penginisalisasian shared memory menggunakan bantuan fungsi `shmget()` dengan argumen
yang diberikan kepada fungsi tersebut adalah `SHM_KEY` yang berisi angka unik untuk identifier shared memory, `sizeof(Message)` agar memori yang dialokasikan menyesuaikan ruang yang digunakan oleh sebuah
instansi struktur `Message` serta `IPC_CREAT | 0666` yang berarti akan membuat shared memory apabila belum dibuat dengan flag `0666` untuk access permission-nya. Kemudian karena shared memory ini akan digunakan
dan dikonsumsi tidak hanya di `client.c` maka dibutuhkan mutual exclusion untuk mencegah process lain untuk meng-overwrite data yang ada di shared memory ketika process lain sedang menggunakan shared memory tersebut. Untuk mengatasi hal ini,
kami menggunakan semaphore yang diimpor dari `sys/sem.h` dan 2 fungsi utility tambahan `mtx_unlock` dan `mtx_lock`. Masing-masing fungsi bekerja sesuai dengan namanya yaitu meng-lock dan meng-unlock shared memory yang digunakan
dengan memanfaatkan semaphore. Inisialisasi semaphore memanfaatkan fungsi `semget` dengan 3 parameter yakni `SEM_KEY`, angka `2` dan `IPC_CREAT | 0666`. Kemudian terdapat dua pemanggilan fungsi `semctl`, masing-masing untuk meng-set
value ketika semaphore sedang digunakan `(SEM_FULL)` atau sedang tidak digunakan `(SEM_EMPTY)`.

Pesan dan Jumlah pesan yang akan dikirim didapatkan melalui `scanf` dari hasil inputan user saat menjalankan program, kemudian terdapat sebuah loop yang mengloop sebanyak `n + 2` kali dengan n adalah berapa kali pesan harus dikirim
Ketika mengirimkan pesan, client.c juga harus melakukan log ke dalam suatu file log `sistem.log` dengan format yang sudah ditentukan di soal dan juga harus melakukan write ke shared memory. Untuk menghindari data pada shared memory tertimpa
oleh process lain yang juga sedang mengakses shared memory, diperlukan melakukan sebuah mutual exclusion dengan memanfaatkan semaphore yang sudah dipersiapkan sebelumnya. Semaphore pertama di-lock terlebih dahulu kemudian data dituliskan ke shared
memory baru kemudian shared memory di-unlock semaphore nya. Semaphore ini nantinya berguna untuk memberitahu process lain yang menggunakan shared memory yang sama agar tidak terjadi data yang corrupt. Pada pengiriman pesan, sebelum pesan dikirim,
disini saya custom pengiriman pesan agar mengirimkan sebuah string sebagai penanda awal pesan dan juga setelah semua pesan sudah selesai terkirim, akan ada string yang dikirimkan sebagai penanda akhir dari pesan.

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal A

### Soal B -

```c
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678

#define SYSTEM_LOG "./sistem.log"
#define SEM_FILELOG "/task4file_semaphore_key"

#define SEM_EMPTY 0
#define SEM_FULL 1

#define STR_INIT_SIZE 256

#define MQ_ID 550
#define MQ_PATH "/tmp/msgqueue550.key"

int STOP_PROGRAM = 0; // false by default

typedef struct {
  char message[STR_INIT_SIZE];
  unsigned long count;
  int ready;
} Message;

typedef struct {
  long type;
  char payload[STR_INIT_SIZE];
} QueuedMessage;

void report(const char *message, int terminate) {
  perror(message);
  if (terminate)
    exit(-1);
}

void mtx_lock(int sem_id, int sem_num) {
  struct sembuf sb = {sem_num, -1, 0};
  semop(sem_id, &sb, 1);
}

void mtx_unlock(int sem_id, int sem_num) {
  struct sembuf sb = {sem_num, 1, 0};
  semop(sem_id, &sb, 1);
}

int createFileForMQ(const char *name) {
  int fd = open(name, O_CREAT | O_RDWR, 0666);
  if (fd < 0)
    return -1;

  close(fd);
  return 0;
}

void cleanup_sighandler(int signum) {
  kill(getpgrp(), signum);
  exit(0);
}

void stp_sig_handler() {
  struct sigaction sa;
  sa.sa_handler = cleanup_sighandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "You need to specify how many worker on the 2nd arg");
    exit(0);
  }

  int loadBalancer = 0;
  if (argc == 2)
    loadBalancer = atoi(argv[1]);

  if (createFileForMQ(MQ_PATH) == -1)
    report("open() failed to create mq file...", 1);

  int shmid, semid;
  int sem_logid;
  Message *shm_ptr;

  shmid = shmget(SHM_KEY, sizeof(Message), IPC_CREAT | 0666);
  if (shmid < 0)
    report("shmget()...", 1);

  if ((shm_ptr = (Message *)shmat(shmid, NULL, 0)) == (void *)-1)
    report("shmat()...", 1);

  semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);
  if (semid == -1)
    report("semget failed...", 1);

  key_t key = ftok(MQ_PATH, MQ_ID);
  if (key < 0)
    report("ftok() couldn't get key...", 1);

  int mq_id = msgget(key, IPC_CREAT | 0666);
  if (mq_id < 0)
    report("msgget() failed...", 1);

  Message msg = {0}, *p_msg = &msg;
  QueuedMessage **qms;

  static int workerX = 1;
  long qmIdentifier = 0;
  long countReceive = 0;
  while (1) {

    mtx_lock(semid, SEM_FULL);

    if (strcmp(shm_ptr->message, "start_of_message") == 0 &&
        shm_ptr->ready == 1) {
      /* printf("Message about to send: %lu\n", shm_ptr->count); */
      countReceive = shm_ptr->count;

      qms = (QueuedMessage **)malloc(sizeof(QueuedMessage) * countReceive);

      for (int i = 0; i < countReceive; i++) {
        qms[i] = malloc(sizeof(QueuedMessage));
      }
    }

    else if (strcmp(shm_ptr->message, "end_of_message") == 0 &&
             shm_ptr->ready == 1) {
      /* printf("%lu messages received\n", countReceive); */

      if (fork() == 0) {
        for (int i = 0; i < countReceive; i++) {

          msgsnd(mq_id, qms[i], sizeof(qms[i]->payload), IPC_NOWAIT);
          /* printf("%s - %lu\n", qms[i]->payload, qms[i]->type); */

          free(qms[i]);

          usleep(500);
        }

        usleep(500);

        QueuedMessage qm = {0};
        for (int i = 1; i <= loadBalancer; i++) {

          qm.type = i;
          snprintf(qm.payload, STR_INIT_SIZE, "end_of_mq_worker");
          msgsnd(mq_id, &qm, sizeof(qm.payload), IPC_NOWAIT);

          usleep(100);
        }

        exit(0);
      }

      workerX = 1;
      countReceive = 0;
      qmIdentifier = 0;
      shm_ptr->ready = 0;
      free(qms);
    }

    else if (shm_ptr->ready == 1) {

      strcpy(p_msg->message, shm_ptr->message);
      p_msg->count = shm_ptr->count;
      p_msg->ready = shm_ptr->ready;

      QueuedMessage *qm = qms[qmIdentifier++];
      memset(qm, 0, sizeof(&qm));

      snprintf(qm->payload, STR_INIT_SIZE, "%s", shm_ptr->message);
      qm->payload[strlen(qm->payload)] = '\0';

      if (workerX % loadBalancer == 0)
        qm->type = loadBalancer;
      else
        qm->type = (workerX % loadBalancer);

      FILE *pf;
      pf = fopen(SYSTEM_LOG, "a");

      fprintf(pf, "Received at lb: %s (#message %d)\n", p_msg->message,
              workerX);

      fclose(pf);
      workerX++;

      shm_ptr->ready = 0;
    }

    mtx_unlock(semid, SEM_EMPTY);

    if (STOP_PROGRAM == 1) {
      /* kill(worker_pid, SIGTERM); */

      workerX = 0;
      shmdt(shm_ptr);

      printf("\nshm_ptr cleared...");
      break;
    }
  }

  return 0;
}
```

##### Penjelasan

Pada Soal B, ```loadbalancer

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal B

#### Soal C -

```c
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define STR_INIT_SIZE 256

#define MQ_ID 550
#define MQ_PATH "/tmp/msgqueue550.key"
#define SYSTEM_LOG "./sistem.log"
int STOP_PROGRAM = 0;

typedef struct {
  long type;
  char payload[STR_INIT_SIZE];
} QueuedMessage;

typedef struct {
  int worker_id;
} ThreadWorker;

void sigint_handler(int sig) { STOP_PROGRAM = 1; }

void report(const char *message, int terminate) {
  perror(message);
  if (terminate)
    _exit(-1);
}

int createFileForMQ(const char *name) {
  int fd = open(name, O_CREAT | O_RDWR, 0666);
  if (fd < 0)
    return -1;

  close(fd);
  return 0;
}

void *processMessageThread(void *args) {
  ThreadWorker *targs = (ThreadWorker *)args;

  int worker_id = targs->worker_id;
  if (createFileForMQ(MQ_PATH) == -1)
    report("open() failed to create mq file...", 1);

  key_t key = ftok(MQ_PATH, MQ_ID);
  if (key < 0)
    report("key not have set up...", 1);

  int mq_id = msgget(key, 0666 | IPC_CREAT);
  if (mq_id < 0)
    report("no access to queue...", 1);

  int countMessages = 0;

  QueuedMessage msg = {0};
  while (1) {

    if (msgrcv(mq_id, &msg, sizeof(msg.payload), worker_id,
               MSG_NOERROR | IPC_NOWAIT) > 0) {
      msg.payload[STR_INIT_SIZE - 1] = '\0';

      FILE *pf;
      pf = fopen(SYSTEM_LOG, "a");

      if (strcmp(msg.payload, "end_of_mq_worker") == 0) {
        /* printf("END OF QUEUE\n"); */
        fprintf(pf, "Worker %d: %d messages\n", worker_id, countMessages);

        countMessages = 0;
      } else {

        ++countMessages;
        fprintf(pf, "Worker%d: message received\n", worker_id);
      }

      fclose(pf);

      usleep(200);
    } else {
      if (errno != ENOMSG) {
        report("msgrcv() failed...", 1);
      }
    }

    usleep(100);

    if (STOP_PROGRAM) {
      break;
    }
  }

  msgctl(mq_id, IPC_RMID, NULL);
  /* printf("Thread exiting....\n"); */

  pthread_exit(NULL);
}

void terminateProgram(int sig) { STOP_PROGRAM = 1; }

int main(int argc, char *argv[]) {
  int num_workers = atoi(argv[1]);
  if (num_workers < 0 || argc < 2) {
    fprintf(stderr, "num of workers need to be greater than 1");
    exit(-1);
  }

  signal(SIGINT, terminateProgram);
  signal(SIGTERM, terminateProgram);

  pthread_t workers[num_workers];
  for (int i = 0; i < num_workers; i++) {
    ThreadWorker *args = malloc(sizeof(ThreadWorker));
    args->worker_id = i + 1;

    if (pthread_create(&workers[i], NULL, processMessageThread, (void *)args) !=
        0)
      report("pthread_create failed...", 1);
  }

  for (int i = 0; i < num_workers; i++) {
    pthread_join(workers[i], NULL);

    /* printf("Thread closing...\n"); */
  }

  return 0;
}
```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal C

#### Soal D -

```c
void *processMessageThread(void *args) {
  ...

  while (1) {

    if (msgrcv(mq_id, &msg, sizeof(msg.payload), worker_id,
               MSG_NOERROR | IPC_NOWAIT) > 0) {
      msg.payload[STR_INIT_SIZE - 1] = '\0';

      FILE *pf;
      pf = fopen(SYSTEM_LOG, "a");

      if (strcmp(msg.payload, "end_of_mq_worker") == 0) {
        /* printf("END OF QUEUE\n"); */
        fprintf(pf, "Worker %d: %d messages\n", worker_id, countMessages);

        countMessages = 0;
      } else {

        ++countMessages;
        fprintf(pf, "Worker%d: message received\n", worker_id);
      }

      fclose(pf);

      usleep(200);
    }

    ...

    if (STOP_PROGRAM) {
      break;
    }
  }

  msgctl(mq_id, IPC_RMID, NULL);
  /* printf("Thread exiting....\n"); */

  pthread_exit(NULL);
}

void terminateProgram(int sig) { STOP_PROGRAM = 1; }

int main(int argc, char *argv[]) {
  ...

  signal(SIGINT, terminateProgram);
  signal(SIGTERM, terminateProgram);

  ...
}
```

##### Penjelasan

##### Kendala

Untuk saat ini belum ada kendala untuk mengerjakan soal D
