#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define SHM_SIZE 4096
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

  if ((shmid = shmget(SHM_KEY, sizeof(Message), IPC_CREAT | 0666)) < 0)
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
    printf("yes");
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
