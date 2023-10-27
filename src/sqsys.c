#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void panic(char *msg) {
  printf("%s", msg);
  exit(EXIT_FAILURE);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
