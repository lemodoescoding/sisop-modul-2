#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
  DIR *folder = opendir("film");

  struct dirent *entri;
  char *listfilm[50];
  int count = 0;

  while ((entri = readdir(folder)) != NULL) {
    int len = strlen(entri->d_name);
    if (len > 4 && strcmp(entri->d_name + len - 4, ".jpg") == 0) {
      listfilm[count] = strdup(entri->d_name);
      count++;
    }
  }
  closedir(folder);

  srand(time(NULL));

  int index = rand() % count;

  printf("Film for Trabowo & Peddy: %s\n", listfilm[index]);

  for (int i = 0; i < count; i++)
    free(listfilm[i]);

  return 0;
}
