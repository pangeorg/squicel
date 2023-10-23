#ifdef TESTS

#include "squicel.h"

static void run_insert_test();

void run_tests(){
  run_insert_test();
}

static void run_insert_test(){
  InputBuffer *input_buffer = InputBuffer_new();
  Table *table = Table_new();

  input_buffer->buffer = "insert 1 user1 user1@mail.com";
  process_input(input_buffer, table);

  InputBuffer_free(input_buffer);
  Table_free(table);
}


#endif
