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
