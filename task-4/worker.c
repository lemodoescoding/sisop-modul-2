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
