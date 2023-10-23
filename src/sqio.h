#ifndef SQIO_H_
#define SQIO_H_

#include <stdio.h>

typedef struct {
  char *buffer;
  size_t buffer_length;
  size_t input_length;
} InputBuffer;

InputBuffer *InputBuffer_new();
void InputBuffer_free(InputBuffer *buffer);
void read_input(InputBuffer *input_buffer);

// helper
size_t getline(char **lineptr, size_t *n, FILE *stream);

#endif // !SQIO_H
