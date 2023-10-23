#include <stdio.h>
#include <stdlib.h>

void panic(char *msg) {
  printf("%s", msg);
  exit(EXIT_FAILURE);
}
