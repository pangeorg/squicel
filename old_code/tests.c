#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "squicel.h"
#include "sqsys.h"

void insert_when_string_too_long_fails();
void insert_file_when_input_valid_succedes();

void run_tests()
{
  // insert_when_string_too_long_fails();
  insert_file_when_input_valid_succedes();
}


void insert_file_when_input_valid_succedes()
{
  InputBuffer *input_buffer = InputBuffer_new();
  SimpleTable *table = SimpleTable_open("test.db");
  char* insert_format_str = "insert %d user%d user%d@mail.com\0";
  input_buffer->buffer = (char*)malloc(200 * sizeof(char*));

  for (int i = 0; i < 5; ++i)
  {
    sprintf(input_buffer->buffer, insert_format_str, i, i, i);
    process_input(input_buffer, table);
  }
  void *node = get_page(table->pager, table->root_page_num);
  uint32_t num_rows = *leaf_node_num_cells(node);
  assert(num_rows == 5);

  insert_format_str = ".exit";
  input_buffer->buffer = insert_format_str;
  process_input(input_buffer, table);

  printf("insert_file_when_input_valid_succedes -> passed\n");
}

void insert_when_string_too_long_fails()
{
  InputBuffer *input_buffer = InputBuffer_new();
  SimpleTable *table = SimpleTable_open("test.db");

  char* str = "insert 1 user1 ";
  char* very_long = (char*)malloc(1000 * sizeof(char*));
  memset(very_long, 'a', 999);
  char* insert_str = concat(str, very_long);
  input_buffer->buffer = insert_str;

  process_input(input_buffer, table);

  InputBuffer_free(input_buffer);
  SimpleTable_free(table);
  printf("insert_when_string_too_long_fails -> passed\n");
}